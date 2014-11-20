/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 *
 */

#include "TopEDO/include/TopEDOController.h"
#include "TopEDO/include/TopEDOWorldObserver.h"

#include "World/World.h"
#include "Utilities/Misc.h"
#include <math.h>
#include <string>

#include <neuralnetworks/MLP.h>
#include <neuralnetworks/Perceptron.h>
#include <neuralnetworks/Elman.h>

#include <pure_neat/genome.h>
#include <pure_neat/network.h>

using namespace Neural;
using namespace PURENEAT;

TopEDOController::TopEDOController (RobotWorldModel * wm)
{

    _wm = wm;
    //true = perceptron, false=neat genome

    nn = NULL;

    load_neat_params ("prj/TopEDO/src/forag.ne", false);

    _currentSigma = TopEDOSharedData::gSigmaRef;


    initRobot ();

    _iteration = 0;
    _birthdate = 0;
    _currentFitness = 0.0;

}

TopEDOController::~TopEDOController ()
{
    if(nn != NULL)
        delete nn;
    nn = NULL;
}

void
TopEDOController::reset ()
{
    _currentFitness = 0.0;
    _birthdate = gWorld->getIterations ();


    _genomesList.clear ();

    _fitnessList.clear ();
    _sigmaList.clear ();
    _birthdateList.clear ();

}

void
TopEDOController::initRobot ()
{
    _nbInputs = 1;		// Bias constant input (1.0)

    if ((gExtendedSensoryInputs) && (TopEDOSharedData::gFitness == 0))
    {
        _nbInputs += (1) * _wm->_cameraSensorsNb;	// Switch
    }

    _nbInputs += _wm->_cameraSensorsNb;	// proximity sensors

    _nbOutputs = 2;

    // Inputs, outputs, 0 hidden neurons, fully connected. Start with Simple Perceptron
    _genome = new Genome (_nbInputs, _nbOutputs,_wm->getId());

    _genome->mom_id = -1;
    _genome->dad_id = -1;
    _genome->genome_id = _wm->getId ();

    _genome->mutate_link_weights (1.0);
    createNN ();

    if (gVerbose)
        std::cout << std::flush;

    setNewGenomeStatus (true);

    _genomesList.clear ();
    //NEAT-like innovation number and node id FOR THIS ROBOT
    innovNumber = nn->linkcount () +1;
    nodeId = 1 + _nbInputs + _nbOutputs;

    _fitnessList.clear ();
}


void
TopEDOController::createNN ()
{
    if (nn != NULL)
        delete nn;

    nn = _genome->genesis ();

}

unsigned int
TopEDOController::computeRequiredNumberOfWeights ()
{
    unsigned int res = nn->linkcount ();
    return res;
}

void
TopEDOController::step ()
{
    _iteration++;

    stepBehaviour ();
    
    if (dynamic_cast <TopEDOWorldObserver *>
            (gWorld->getWorldObserver ())->getLifeIterationCount () >=
            TopEDOSharedData::gEvaluationTime - 1)
    {
        stepEvolution ();
        reset();
    }
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void
TopEDOController::stepBehaviour ()
{
    std::pair<std::vector<double>,std::vector<double>> io ;
    std::vector<double> outputs;
    std::vector < double >inputs;

    io = act();
    outputs= io.second;
    inputs= io.first;

    _wm->_desiredTranslationalValue = outputs[0];
    //Rotational velocity in [-1,+1]. Neat uses sigmoid [0,+1]
    _wm->_desiredRotationalVelocity = 2 * (outputs[1] - 0.5);

    // normalize to motor interval values
    _wm->_desiredTranslationalValue =
            _wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
            _wm->_desiredRotationalVelocity * gMaxRotationalSpeed;

    _currentFitness += updateFitness (inputs, outputs);

    // broadcasting genome : robot broadcasts its genome
    //to all neighbors (contact-based wrt proximity sensors)
    broadcastGenome();
}

std::pair<std::vector<double>,std::vector<double>> TopEDOController::act()
{
                                                   // ---- Build inputs ----

                                                   std::vector < double >inputs(_nbInputs);
                                                   int inputToUse = 0;


                                                   // distance sensors
                                                   for (int i = 0; i < _wm->_cameraSensorsNb; i++)
{
    inputs[inputToUse] =
            _wm->getDistanceValueFromCameraSensor (i) /
            _wm->getCameraSensorMaximumDistanceValue (i);
    inputToUse++;

    if (gExtendedSensoryInputs && (TopEDOSharedData::gFitness == 0))
    {
        int objectId = _wm->getObjectIdFromCameraSensor (i);

        // input: physical object? which type?
        if (PhysicalObject::isInstanceOf (objectId))
        {
            //Switch is type 3
            if ((gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset]
                 ->getType ()) == 3)
            {
                inputs[inputToUse] = 	_wm->getDistanceValueFromCameraSensor (i) /
                        _wm->getCameraSensorMaximumDistanceValue (i);//Match
            }
            else
                inputs[inputToUse] = 1.0;
            inputToUse++;
        }
        else //Not physical object
        {
            inputs[inputToUse] = 1.0;
            inputToUse++;
        }

    }
}
//Bias
inputs[inputToUse++] = 1.0;

// ---- compute and read out ----
nn->load_sensors (&(inputs[0]));

if (!(nn->activate ()))
{
    std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
    exit (-1);
}

std::vector < double >outputs;
for (std::vector < NNode * >::iterator out_iter = nn->outputs.begin ();
out_iter != nn->outputs.end (); out_iter++)
{
    outputs.push_back ((*out_iter)->activation);
}
return std::make_pair(inputs,outputs);
}

float
TopEDOController::updateFitness (std::vector < double >in,
                                 std::vector < double >out)
{
    float deltaFit = 0.0;
    int targetIndex;
    double vT,vR,minSensor;


    switch(TopEDOSharedData::gFitness)
    {
    case 0:
        targetIndex = _wm->getGroundSensorValue ();

        if (PhysicalObject::isInstanceOf (targetIndex))
        {
            if((gPhysicalObjects[targetIndex - gPhysicalObjectIndexStartOffset]
                ->getType ()) == 3)
                deltaFit += 1.0;
        }
        break;
    case 1:
        //[Floreano2000] locomotion fitness function
        //abs(Translational speed) * (1 - abs(Rotational Speed)) * minimal(distance to obstacle)

        //Neat uses sigmoid[0,1] activation function
        vT = 2 * (out[0] - 0.5);
        vR =  2 * (out[1] - 0.5);

        minSensor = in[_wm->_cameraSensorsNb - 1];

        for (int i = 0; i < _wm->_cameraSensorsNb; i++)
        {
            if(in[i] < minSensor)
                minSensor = in[i];
        }
        deltaFit += fabs(vT) * (1 - fabs(vR)) * minSensor;
        if(deltaFit < 0.0)
        {
            std::cerr << "[ERROR] Negative fitness in navigation" << std::endl;
            exit(-1);
        }
        break;

    default:
        std::cerr << "[ERROR] Unknown fitness function selected. Check gFitness parameter in properties file." << std::endl;
        exit(-1);
    }

    return deltaFit;
}

void
TopEDOController::broadcastGenome ()
{
    // only if agent is active (ie. not just revived) and deltaE>0.
    for (int i = 0; i < _wm->_cameraSensorsNb; i++)
    {
        int targetIndex = _wm->getObjectIdFromCameraSensor (i);

        // sensor ray bumped into a robot : communication is possible
        if (targetIndex >= gRobotIndexStartOffset)
        {
            // convert image registering index into robot id.
            targetIndex = targetIndex - gRobotIndexStartOffset;

            TopEDOController *targetRobotController =
                    dynamic_cast <
                    TopEDOController *
                    >(gWorld->getRobot (targetIndex)->getController ());

            if (!targetRobotController)
            {
                std::
                        cerr << "Error from robot " << _wm->getId () <<
                                " : the observer of robot " << targetIndex <<
                                " is not compatible" << std::endl;
                exit (-1);
            }

            // other agent stores my genome.

            targetRobotController->
                    storeGenome (_genome, _wm->getId (), _birthdate,
                                 _currentSigma, _currentFitness);
        }
    }
}

void
TopEDOController::storeGenome (Genome *genome, int senderId,
                               int senderBirthdate, float sigma,
                               float fitness)
{
    //08/10/14 (storeGenome Adaptedto NEAT, I think)
    _genomesList[senderId] = genome;
    _sigmaList[senderId] = sigma;
    _birthdateList[senderId] = senderBirthdate;
    _fitnessList[senderId] = fitness;

}

// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void
TopEDOController::stepEvolution ()
{

    logGenome();
    
    //L = L + A

    _genomesList[_wm->getId ()] = _genome;

    _fitnessList[_wm->getId()] = _currentFitness;

    _sigmaList[_wm->getId ()] = _currentSigma;
    _birthdateList[_wm->getId ()] = _birthdate;
    
    int selected =-1;
    switch(TopEDOSharedData::gSelectionMethod)
    {
    case 1:
        selected = selectBest (_fitnessList);
        break;
    case 2:
        selected = selectRankBased (_fitnessList);
        break;
    case 3:
        selected = selectBinaryTournament (_fitnessList);
        break;
    case 4:
        selected = selectRandom(_fitnessList);
        break;
    default:
        std::cerr << "[ERROR] Selection method unknown (value: " << TopEDOSharedData::gSelectionMethod << ").\n";
        exit(-1);
    }

    if(_genomesList[selected] == NULL)
    {
        std::cerr << "[ERROR] Selected genome not existing: " << selected << " in robot " << _wm->getId() << std::endl;
        exit(-1);
    }


    _genome = _genomesList[selected];

    _currentFitness = _fitnessList[selected];

    
    int newId =
            _wm->getId () + 10000 * (1 +
                                     (gWorld->getIterations () /
                                      TopEDOSharedData::gEvaluationTime));
    if(TopEDOSharedData::gControllerType == 0)
    {
        _genome = _genome -> mutate (_currentSigma,
                                     _wm->getId (), newId, nodeId, innovNumber);
        createNN ();
    }
    else if(TopEDOSharedData::gControllerType == 1)
        _genome -> mutate_link_weights(_currentSigma);
    else
    {
        std::cerr << "[ERROR] Undefined gControllerType: " << TopEDOSharedData::gControllerType << std::endl;
        exit(-1);
    }
    
}


void TopEDOController::logGenome()
{

    //GENERATION ID-ROBOT FITNESS SIZE(localPop) IDGENOME IDMOM
    TopEDOSharedData::gEvoLog <<
                                 dynamic_cast <TopEDOWorldObserver *>
                                 (gWorld->getWorldObserver ())->getGenerationCount() +1
                              << " " << _wm->getId () << " " <<
                                 _currentFitness << " " << _fitnessList.size() << " " << _genome->genome_id << " " << _genome->mom_id << std::endl;



    std::string filename;

    filename = TopEDOSharedData::gGenomeLogFolder;
    filename += std::to_string(_genome -> genome_id);

    _genome -> print_to_filename(const_cast<char*>(filename.c_str()));


}


int
TopEDOController::selectBest (std::map < int, float >lFitness)
{
    std::map < int, float >::iterator it = lFitness.begin();

    float bestFit = it->second;
    int idx = it->first;

    for (; it != lFitness.end (); it++)
    {
        if (it->second > bestFit)
        {
            bestFit = it->second;
            idx = it->first;
        }

    }
    return idx;
}
int
TopEDOController::selectRankBased(std::map < int, float >lFitness)
{       
    std::map < int, float >::iterator it = lFitness.begin();

    std::vector<std::pair<int,float>> rankedFitness;
    //sum of indexes from (1:n) = n(n+1)/2
    int totalIndex = (lFitness.size()) *( lFitness.size()+1) / 2;
    //Vector for Rank for each individual
    for (it->first; it != lFitness.end (); it++)
    {
        rankedFitness.push_back(std::make_pair(it->first,it->second));
    }

    int random = (rand() % totalIndex) + 1; //Uniform between 1 and n * (n+1) / 2

    //Fitness ascending order
    std::sort(rankedFitness.begin(), rankedFitness.end(), compareFitness);

    for(unsigned int i = 0; i < rankedFitness.size(); i++)
    {
        random -= i + 1;
        if(random <= 0)
            return rankedFitness[i].first;
    }
    return -1;
}
int
TopEDOController::selectBinaryTournament(std::map < int, float >lFitness)
{
    int result = -1;

    std::vector<int> v;

    //Vector for storing the keys (robot ID)
    for(std::map<int,float>::iterator it = lFitness.begin(); it != lFitness.end(); ++it) {
        v.push_back(it->first);
    }

    if(lFitness.size() > 1)
    {
        int ind1 =  rand () % lFitness.size ();
        ind1 = v[ind1];
        int ind2 =  rand () % lFitness.size ();
        ind2 = v[ind2];

        while(ind1 == ind2)
        {
            ind2 =  rand () % lFitness.size ();
            ind2 = v[ind2];
        }

        if(lFitness[ind1] >= lFitness[ind2])
            result = ind1;
        else
            result = ind2;
    }
    else
        result = lFitness.begin()->first;

    if(result == -1)
    {
        std::cerr << "[ERROR] No individual selected by binary tournament." << std::endl << std::flush;
        exit(-1);
    }

    if(lFitness.find(result) == lFitness.end())
    {
        std::cerr << "[ERROR] Unexisting individual selected by binary tournament." << std::endl << std::flush;
        exit(-1);
    }

    return result;
}
int
TopEDOController::selectRandom(std::map < int, float >lFitness)
{       
    int randomIndex = rand()%lFitness.size();
    std::map<int, float >::iterator it = lFitness.begin();
    while (randomIndex !=0 )
    {
        it ++;
        randomIndex --;
    }

    return it->first;
}

bool TopEDOController::compareFitness(std::pair<int,float> i,std::pair<int,float> j) 
{
    return (i.second < j.second);
}

void TopEDOController::printIO( std::pair< std::vector<double>, std::vector<double> > io)
{
    std::vector<double> in = io.first;
    std::vector<double> out = io.second;

    std::cout << "------------------------------" << std::endl;

    std::cout << "Input vector" << std::endl;
    printVector(in);

    std::cout << std::endl;

    std::cout << "Output vector" << std::endl;
    printVector(out);

    std::cout << std::endl;
    std::cout << "------------------------------" << std::endl;
}

void TopEDOController::printVector(std::vector<double> v)
{
    for(unsigned int i = 0; i < v.size(); i++)
    {
        std::cout << "[" << i << "] =" << v[i] << " | ";
    }
}
void TopEDOController::printFitnessList()
{
    std::cout << "------------------------------" << std::endl;
    std::map<int,float>::iterator it = _fitnessList.begin();
    std::cout << "Fitness List" << std::endl;
    for(;it != _fitnessList.end(); it++)
    {
        std::cout << "R[" << it->first << "] " << it->second << std::endl;
    }
    std::cout << "------------------------------" << std::endl;
}
