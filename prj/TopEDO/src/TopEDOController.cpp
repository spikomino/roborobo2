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

    //load_neat_params ("prj/TopEDO/src/forag.ne", false);

    _sigma = TopEDOSharedData::gSigmaRef;


    initRobot ();

    _iteration = 0;
    _birthdate = 0;
    _fitness = 0.0;

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
    _fitness = 0.0;
    _birthdate = gWorld->getIterations ();

    emptyBasket();
    emptyGenomeList();

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

    // Inputs, outputs, 0 hidden neurons, fully connected.
    //Start with Simple Perceptron
    //All agents are given the same initial topology.
    //Thus, those genes are not identified by <idrobot,genecounter>
    //but just <-1, genecounter>, -1 meaning that the genes are shared
    //by all agents
    _genome = new Genome (_wm->getId(),_nbInputs, _nbOutputs,-1);
    _genome->mom_id = -1;
    _genome->dad_id = -1;
    _genome->genome_id = _wm->getId ();

    _genome->mutate_link_weights (1.0);
    createNN ();

    if (gVerbose)
        std::cout << std::flush;

    setNewGenomeStatus (true);

    emptyGenomeList();
    emptyBasket();
    //NEAT-like innovation number and node id FOR THIS ROBOT
    _innovNumber = nn->linkcount () +1;
    _nodeId = 1 + _nbInputs + _nbOutputs;
    if(gVerbose){
        std::cout << "[initRobot] "
                  << "id="  << _wm->getId() << " "
                  << "in="  << _nbInputs    << " "
                  << "out=" << _nbOutputs
                  << std::endl;
    }

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
    
    if (lifeTimeOver())
    {
        save_genome();
        printAll();

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

    updateFitness (inputs, outputs);

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

void
TopEDOController::updateFitness (std::vector < double >in,
                                 std::vector < double >out)
{
    float deltaFit = 0.0;
    double vT,vR,minSensor;
    float newFitness;

    switch(TopEDOSharedData::gFitness)
    {
    case 0:
        updateFitnessForaging();
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
        newFitness= _fitness;
        deltaFit += fabs(vT) * (1 - fabs(vR)) * minSensor;
        if(deltaFit < 0.0)
        {
            std::cerr << "[ERROR] Negative fitness in navigation" << std::endl;
            exit(-1);
        }

        newFitness += deltaFit;
        _fitness= newFitness;
        break;

    default:
        std::cerr << "[ERROR] Unknown fitness function selected. Check gFitness parameter in properties file." << std::endl;
        exit(-1);
    }
}
/*
// update fitness for navigation
void TopEDOController::updateFitnessNavigation(){
    _fitness += (fabs(_lv) + fabs(_rv)) *
    (1.0 -sqrt(fabs(_lv - _rv))) *
    (1.0 - _md) ;
}*/
// update fitness for foraging
void TopEDOController::updateFitnessForaging(){
    _fitness = (double) _items / (double) get_lifetime();
}

void
TopEDOController::broadcastGenome ()
{
    std::vector<TopEDOController *> neighbors;

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

            /* add to the list  */
            neighbors.push_back(targetRobotController);
        }
    }

    /* if found neighbors, broadcast my genome */
    if(neighbors.size() > 0) {
        message msg (_genome, _fitness, _sigma, _birthdate,_nodeId,_innovNumber);

        /* remove duplicates */
        std::sort(neighbors.begin(), neighbors.end());
        auto last = std::unique(neighbors.begin(), neighbors.end());
        neighbors.erase(last, neighbors.end());

        /* send */
        for (const auto& c : neighbors)
            c->storeMessage (_wm->getId(), msg);

        /* some screen output */
        if (gVerbose){
           /* std::cout << "@"  << _iteration << " R" << _wm->getId() << " -> " ;
            for (const auto& c : neighbors)
                std::cout << c->_wm->getId() << " ";
            std::cout << std::endl;*/
        }
        /* delete neighbors list */
        neighbors.clear();
    }
}
void TopEDOController::storeMessage(int id, message msg){
    _gList[id] = msg;

    //Update genetic clocks for nodes and links
    //This minimizes the number of arbitrary sorting orders in genome alignment
    //due to concurrent mutations in different agents
    _nodeId = max(_nodeId,std::get<4>(msg));
    _innovNumber = max(_innovNumber,std::get<5>(msg));

}

void TopEDOController::emptyGenomeList(){
    _gList.clear();
}
void TopEDOController::pickItem(){
    _items++;
}
void TopEDOController::emptyBasket(){
    _items = 0;
}
// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void
TopEDOController::stepEvolution ()
{

    logGenome();
    message msg (_genome, _fitness, _sigma, _birthdate,_nodeId,_innovNumber);
    //L = L + A
    storeMessage(_wm->getId(), msg);

    int selected =-1;
    switch(TopEDOSharedData::gSelectionMethod)
    {
    case 1:
        selected = selectBest ();
        break;
    case 2:
        selected = selectRankBased ();
        break;
    case 3:
        selected = selectBinaryTournament ();
        break;
    case 4:
        selected = selectRandom();
        break;
    default:
        std::cerr << "[ERROR] Selection method unknown (value: " << TopEDOSharedData::gSelectionMethod << ").\n";
        exit(-1);
    }

    if(_gList.find(selected) == _gList.end())
    {
        std::cerr << "[ERROR] Selected genome not existing: " << selected << " in robot " << _wm->getId() << std::endl;
        exit(-1);
    }


    _genome = std::get<0>(_gList[selected]);

    _fitness = std::get<1>(_gList[selected]);

    
    int newId =
            _wm->getId () + 10000 * (1 +
                                     (gWorld->getIterations () /
                                      TopEDOSharedData::gEvaluationTime));
    if(TopEDOSharedData::gControllerType == 0)
    {
        _genome = _genome -> mutate (_sigma,
                                     _wm->getId (), newId, _nodeId, _innovNumber);
        createNN ();
    }
    else if(TopEDOSharedData::gControllerType == 1)
        _genome -> mutate_link_weights(_sigma);
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
                                 _fitness << " " << _gList.size() << " " << _genome->genome_id << " " << _genome->mom_id << std::endl;



    std::string filename;

    filename = TopEDOSharedData::gGenomeLogFolder;
    filename += std::to_string(_genome -> genome_id);

    // _genome -> print_to_filename(const_cast<char*>(filename.c_str()));


}


int
TopEDOController::selectBest ()
{
    std::map < int, message >::iterator it = _gList.begin();

    double bestFit = std::get<1>(it->second);
    int idx = it->first;

    for (; it != _gList.end (); it++)
    {
        if (std::get<1>(it->second) > bestFit)
        {
            bestFit = std::get<1>(it->second);
            idx = it->first;
        }

    }
    return idx;
}
int
TopEDOController::selectRankBased()
{       
    std::map < int, message>::iterator it = _gList.begin();

    std::vector<std::pair<int,float>> rankedFitness;
    //sum of indexes from (1:n) = n(n+1)/2
    int totalIndex = (_gList.size()) *( _gList.size()+1) / 2;
    //Vector for Rank for each individual
    for (it->first; it != _gList.end (); it++)
    {
        rankedFitness.push_back(std::make_pair(it->first,std::get<1>(it->second)));
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
TopEDOController::selectBinaryTournament()
{
    int result = -1;

    std::vector<int> v;

    //Vector for storing the keys (robot ID)
    for(std::map<int,message>::iterator it = _gList.begin(); it != _gList.end(); ++it) {
        v.push_back(it->first);
    }

    if(_gList.size() > 1)
    {
        int ind1 =  rand () % _gList.size ();
        ind1 = v[ind1];
        int ind2 =  rand () % _gList.size ();
        ind2 = v[ind2];

        while(ind1 == ind2)
        {
            ind2 =  rand () % _gList.size ();
            ind2 = v[ind2];
        }

        if(std::get<1>(_gList[ind1]) >= std::get<1>(_gList[ind2]))
            result = ind1;
        else
            result = ind2;
    }
    else
        result = _gList.begin()->first;

    if(result == -1)
    {
        std::cerr << "[ERROR] No individual selected by binary tournament." << std::endl << std::flush;
        exit(-1);
    }

    if(_gList.find(result) == _gList.end())
    {
        std::cerr << "[ERROR] Unexisting individual selected by binary tournament." << std::endl << std::flush;
        exit(-1);
    }

    return result;
}
int
TopEDOController::selectRandom()
{       
    int randomIndex = rand()%_gList.size();
    std::map<int, message >::iterator it = _gList.begin();
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
    std::map<int,message>::iterator it = _gList.begin();
    std::cout << "Fitness List" << std::endl;
    for(;it != _gList.end(); it++)
    {
        std::cout << "R[" << it->first << "] " << std::get<1>(it->second) << std::endl;
    }
    std::cout << "------------------------------" << std::endl;
}
// Save a genome (file name = robot_id-genome_id.gen)
void TopEDOController::save_genome(){
    char fname[128];
    snprintf(fname, 127, "logs/genomes/%04d-%010d.gen",
             _wm->getId(), _genome->genome_id);
    std::ofstream oFile(fname);
    _genome->print_to_file(oFile);
    oFile.close();
}


void TopEDOController::print_genome(Genome* g){
    std::cout << "[Genome: id=" << _wm->getId()
              << " idtrace="    << g->genome_id
              << " mom="        << g->mom_id
              << " dad="        << g->dad_id << " ]";
}

void TopEDOController::printRobot(){
    std::cout << "[Robot: id=" + to_string(_wm->getId())
              << " iteration=" + to_string(_iteration)
              << " birthdate=" + to_string(_birthdate)
              << " fitness="   + to_string(_fitness)
              << " items="     + to_string(_items)
              << " sigma="     + to_string(_sigma)
              << " nodeId="     + to_string(_nodeId)
              << " geneId="     + to_string(_innovNumber)
              << " ]";
}

void TopEDOController::printAll(){
    printRobot();
    print_genome(_genome);
    std::cout << "\n";
}
bool TopEDOController::lifeTimeOver(){
    return get_lifetime()
            >= TopEDOSharedData::gEvaluationTime - 1;
}

int TopEDOController::get_lifetime(){
    return dynamic_cast <TopEDOWorldObserver*>
            (gWorld->getWorldObserver())->getLifeIterationCount();
}
