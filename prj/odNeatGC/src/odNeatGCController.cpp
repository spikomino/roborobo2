/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 *
 */

#include "odNeatGC/include/odNeatGCController.h"
#include "odNeatGC/include/odNeatGCWorldObserver.h"

#include "World/World.h"
#include "Utilities/Misc.h"
#include <math.h>
#include <stdio.h>
#include <string>

#include <odneatgc/genome.h>
#include <odneatgc/network.h>

using namespace ODNEATGC;

odNeatGCController::odNeatGCController (RobotWorldModel * wm)
{

    _wm = wm;

    nn = NULL;

    _sigma = odNeatGCSharedData::gSigmaRef;

    initRobot ();

    _iteration = 0;
    _birthdate = 0;
    _fitness = 0.0;
    _fitnessUpdateCounter = 0;
    _genome->nbFitnessUpdates = 0;

}

odNeatGCController::~odNeatGCController ()
{
    if(nn != NULL)
        delete nn;
    nn = NULL;
}

void
odNeatGCController::reset()
{
    _fitness = 0.0;
    _fitnessUpdateCounter = 0;
    _genome->nbFitnessUpdates = 0;

    _birthdate = gWorld->getIterations ();
    _energy = odNeatGCSharedData::gDefaultInitialEnergy;
    _fitness = _energy;
    emptyBasket();

    //Fitness initialized to 0, so species will be "hindered"
    //Pay attention to initial species (see constructor above)
    _genome ->species = -1;
    add_unconditional_to_population(message(_genome,_energy,_sigma,_birthdate,_nodeId,_innovNumber));
    adjust_species_fitness();
    recomputeAllSpecies();
}

void
odNeatGCController::initRobot ()
{
    _nbInputs = 1;		// Bias constant input (1.0)

    if ((gExtendedSensoryInputs) && (odNeatGCSharedData::gFitness == 0))
    {
        _nbInputs += (1) * _wm->_cameraSensorsNb;	// Switch
    }

    _nbInputs += _wm->_cameraSensorsNb;	// proximity sensors

    _nbOutputs = 2;

    // Inputs, outputs, 0 hidden neurons, fully connected.
    //Start with Simple Perceptron
    //All agents are given the same initial topology.
    //Thus, those genes are not identified by a common historical marker
    _genome = new Genome (_wm->getId(),_nbInputs, _nbOutputs);

    _genome->mom_id = -1;
    _genome->dad_id = -1;
    _genome->genome_id = _wm->getId ();

    _genome->mutate_link_weights (1.0);
    createNN ();
    _innovNumber = computeRequiredNumberOfWeights();
    _nodeId = (1 + _nbInputs + _nbOutputs);

    if (gVerbose)
        std::cout << std::flush;

    setNewGenomeStatus (true);

    emptyGenomeList();
    emptyBasket();


    if(gVerbose){
        std::cout << "[initRobot] "
                  << "id="  << _wm->getId() << " "
                  << "in="  << _nbInputs    << " "
                  << "out=" << _nbOutputs
                  << std::endl;
    }



    _energy = odNeatGCSharedData::gDefaultInitialEnergy;

    //TOUNCOMMENT : this has been commented to use irace to tune the parameters
    //save_genome();

}


void
odNeatGCController::createNN ()
{
    if (nn != NULL)
        delete nn;

    nn = _genome->genesis ();

}

unsigned int
odNeatGCController::computeRequiredNumberOfWeights ()
{
    unsigned int res = nn->linkcount ();
    return res;
}

void
odNeatGCController::step ()
{
    _iteration++;

    //If Inter-robot reproduction event
    if(doBroadcast())
    {
        // broadcasting genome : robot broadcasts its genome
        //to all neighbors (contact-based wrt proximity sensors)
        broadcastGenome();
    }

    stepBehaviour ();
    
    if ((_energy <=odNeatGCSharedData::gEnergyThreshold)
            && !(in_maturation_period()))
    {
        printAll();
        cleanPopAndSpecies();
        stepEvolution ();

        //TOUNCOMMENT : this has been commented to use irace to tune the parameters
       // save_genome();
        reset();        
    }
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void
odNeatGCController::stepBehaviour ()
{
    act();
    _energy = update_energy_level();
    updateFitness ();
    adjust_active_species_fitness(_genome -> species);

}

void odNeatGCController::act()
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

        if (gExtendedSensoryInputs && (odNeatGCSharedData::gFitness == 0))
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

    /* get the most activated obstacle sensor for floreano fitness*/
    _md =10.0;
    for(int i = 0; i < _wm->_cameraSensorsNb; i++)
        if(_md > inputs[i] &&
                gExtendedSensoryInputs)//TOFIX (if the activation does not correspond to an item) && inputs[i+_wm->_cameraSensorsNb] < 1.0)
            _md = inputs[i];

    //Bias
    inputs[inputToUse++] = 1.0;

    // ---- compute and read out ----
    nn->load_sensors (&(inputs[0]));

    if (!(nn->activate ()))
    {
        std::cerr << "[ERROR] Activation of ANN not correct: genome " << _genome->genome_id << std::endl;
        save_genome();
        exit (-1);
    }
    // Read the output
    std::vector<double> outputs;
    std::vector<NNode*>::iterator out_iter;
    for (out_iter  = nn->outputs.begin();
         out_iter != nn->outputs.end();
         out_iter++)
        outputs.push_back((*out_iter)->activation);

    /* store translational and rotational velocities for floreano fitness */
    _transV = 2 * (outputs[0] - 0.5);
    _rotV = 2 * (outputs[1] - 0.5);

    //Set the outputs to the right effectors, and rescale the intervals
    //Translational velocity in [-1,+1]. Robots can move backwards. Neat uses sigmoid [0,+1]
    _wm->_desiredTranslationalValue = 2 * (outputs[0] - 0.5);
    //Rotational velocity in [-1,+1]. Neat uses sigmoid [0,+1]
    _wm->_desiredRotationalVelocity = 2 * (outputs[1] - 0.5);

    // normalize to motor interval values
    _wm->_desiredTranslationalValue =
            _wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
            _wm->_desiredRotationalVelocity * gMaxRotationalSpeed;

}

void
odNeatGCController::updateFitness ()
{
    switch(odNeatGCSharedData::gFitness)
    {
    case 0:
        updateFitnessForaging();
        break;
    case 1:
        updateFitnessNavigation();
        break;

    default:
        std::cerr << "[ERROR] Unknown fitness function selected. Check gFitness parameter in properties file." << std::endl;
        exit(-1);
    }
    std::get<1>(population[_genome -> genome_id]) = _fitness;
}

// update fitness for navigation
void odNeatGCController::updateFitnessNavigation(){
    _fitnessUpdateCounter++;
    if(_fitnessUpdateCounter >= odNeatGCSharedData::gFitnessFreq)
    {
        _genome->nbFitnessUpdates++;
        _fitness = (_fitness) + ((_energy -  _fitness)/_genome->nbFitnessUpdates);
        _fitnessUpdateCounter =  0;

    }
    //[Floreano2000] locomotion fitness function
    //abs(Translational speed) * (1 - abs(Rotational Speed)) * minimal(distance to obstacle)
    //_fitness += (fabs(_transV)) * (1.0 -sqrt(fabs(_rotV))) * _md;
}
// update fitness for foraging
void odNeatGCController::updateFitnessForaging(){
    _fitnessUpdateCounter++;
    if(_fitnessUpdateCounter >= odNeatGCSharedData::gFitnessFreq)
    {
        _genome->nbFitnessUpdates++;
        _fitness = (_fitness) + ((_energy -  _fitness)/_genome->nbFitnessUpdates);
        _fitnessUpdateCounter =  0;
    }
    //_fitness = (double) _items / (double) get_lifetime();
}

void
odNeatGCController::broadcastGenome ()
{
    std::vector<odNeatGCController *> neighbors;

    // only if agent is active (ie. not just revived) and deltaE>0.
    for (int i = 0; i < _wm->_cameraSensorsNb; i++)
    {
        int targetIndex = _wm->getObjectIdFromCameraSensor (i);

        // sensor ray bumped into a robot : communication is possible
        if (targetIndex >= gRobotIndexStartOffset)
        {
            // convert image registering index into robot id.
            targetIndex = targetIndex - gRobotIndexStartOffset;

            odNeatGCController *targetRobotController =
                    dynamic_cast <
                    odNeatGCController *
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
        //message msg (_genome, _fitness, _sigma, _birthdate,_nodeId,_innovNumber);

        /* remove duplicates */
        std::sort(neighbors.begin(), neighbors.end());
        auto last = std::unique(neighbors.begin(), neighbors.end());
        neighbors.erase(last, neighbors.end());

        /* send */
        for (const auto& c : neighbors)
        {
            Genome* copy = _genome->duplicate();
            copy->nbFitnessUpdates= 0;
            message msg (copy, _energy, _sigma, _birthdate,_nodeId,_innovNumber);
            message send (msg);

            c->storeMessage (send);
        }

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
void odNeatGCController::storeMessage(message msg){
    //Received species info is no longer valid
    std::get<0>(msg) -> species = -1;
    std::get<0>(msg) -> nbFitnessUpdates++;

    if(tabu_list_approves(std::get<0>(msg)) && population_accepts(msg))
    {    
        cleanPopAndSpecies();
        add_to_population(msg);        
        adjust_population_size();
        adjust_species_fitness();
        cleanPopAndSpecies();
    }


    //Update gene clocks for nodes and links
    //This minimizes the number of arbitrary sorting orders in genome alignment
    //due to concurrent mutations in different agents
    _nodeId = max(_nodeId,std::get<4>(msg));
    _innovNumber = max(_innovNumber,std::get<5>(msg));

}

void odNeatGCController::emptyGenomeList(){
    population.clear();
    species.clear();
}
void odNeatGCController::pickItem(){
    _items++;
    _energy += odNeatGCSharedData::gEnergItemValue;
}
void odNeatGCController::emptyBasket(){
    _items = 0;
}
// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void
odNeatGCController::stepEvolution ()
{

    //logGenome();

    add_to_population(message(_genome, _fitness, _sigma, _birthdate,_nodeId,_innovNumber));
    add_to_tabu_list(_genome);
    Genome* offspring =  generate_offspring();
    update_population(offspring);
    _genome =  offspring;
    createNN();
}


void odNeatGCController::logGenome()
{

    //GENERATION ID-ROBOT FITNESS SIZE(localPop) IDGENOME IDMOM
    odNeatGCSharedData::gEvoLog <<
                                 dynamic_cast <odNeatGCWorldObserver *>
                                 (gWorld->getWorldObserver ())->getGenerationCount() +1
                              << " " << _wm->getId () << " " <<
                                 _fitness << " " << population.size() << " " << _genome->genome_id << std::endl;



    std::string filename;

    filename = odNeatGCSharedData::gGenomeLogFolder;
    filename += std::to_string(_genome -> genome_id);

    //_genome -> print_to_filename(const_cast<char*>(filename.c_str()));
}


int
odNeatGCController::selectBest ()
{
    std::map < int, message >::iterator it = population.begin();

    double bestFit = std::get<1>(it->second);
    int idx = it->first;

    for (; it != population.end (); it++)
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
odNeatGCController::selectRankBased()
{       
    std::map < int, message>::iterator it = population.begin();

    std::vector<std::pair<int,float>> rankedFitness;
    //sum of indexes from (1:n) = n(n+1)/2
    int totalIndex = (population.size()) *( population.size()+1) / 2;
    //Vector for Rank for each individual
    for (it->first; it != population.end (); it++)
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
odNeatGCController::selectBinaryTournament()
{
    int result = -1;

    std::vector<int> v;

    //Vector for storing the keys (robot ID)
    for(std::map<int,message>::iterator it = population.begin(); it != population.end(); ++it) {
        v.push_back(it->first);
    }

    if(population.size() > 1)
    {
        int ind1 =  rand () % population.size ();
        ind1 = v[ind1];
        int ind2 =  rand () % population.size ();
        ind2 = v[ind2];

        while(ind1 == ind2)
        {
            ind2 =  rand () % population.size ();
            ind2 = v[ind2];
        }

        if(std::get<1>(population[ind1]) >= std::get<1>(population[ind2]))
            result = ind1;
        else
            result = ind2;
    }
    else
        result = population.begin()->first;

    if(result == -1)
    {
        std::cerr << "[ERROR] No individual selected by binary tournament." << std::endl << std::flush;
        exit(-1);
    }

    if(population.find(result) == population.end())
    {
        std::cerr << "[ERROR] Unexisting individual selected by binary tournament." << std::endl << std::flush;
        exit(-1);
    }

    return result;
}
int
odNeatGCController::selectRandom()
{       
    int randomIndex = rand()%population.size();
    std::map<int, message >::iterator it = population.begin();
    while (randomIndex !=0 )
    {
        it ++;
        randomIndex --;
    }

    return it->first;
}

bool odNeatGCController::compareFitness(std::pair<int,float> i,std::pair<int,float> j)
{
    return (i.second < j.second);
}

void odNeatGCController::printIO( std::pair< std::vector<double>, std::vector<double> > io)
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

void odNeatGCController::printVector(std::vector<double> v)
{
    for(unsigned int i = 0; i < v.size(); i++)
    {
        std::cout << "[" << i << "] =" << v[i] << " | ";
    }
}
void odNeatGCController::printFitnessList()
{
    std::cout << "------------------------------" << std::endl;
    std::map<int,message>::iterator it = population.begin();
    std::cout << "Fitness List" << std::endl;
    for(;it != population.end(); it++)
    {
        std::cout << "R[" << it->first << "] " << std::get<1>(it->second) << std::endl;
    }
    std::cout << "------------------------------" << std::endl;
}
// Save a genome (file name = robot_id-genome_id.gen)
void odNeatGCController::save_genome(){
    char fname[128];
    snprintf(fname, 127, (odNeatGCSharedData::gGenomeLogFolder+"%04d-%010d.gen").c_str(),
             _wm->getId(), _genome->genome_id);
    std::ofstream oFile(fname);
    _genome->print_to_file(oFile);
    oFile.close();
}


void odNeatGCController::print_genome(Genome* g){
    std::cout << "[Genome: idRobot=" << _wm->getId()
              << " idgenome="    << g->genome_id
              << " mom="        << g->mom_id
              << " dad="        << g->dad_id << " ]";
}

void odNeatGCController::printRobot(){
    std::cout << "[Robot: id=" + to_string(_wm->getId())
              << " iteration=" + to_string(_iteration)
              << " birthdate=" + to_string(_birthdate)
              << " fitness="   + to_string(_fitness)
              << " energy="    + to_string(_energy)
              << " items="     + to_string(_items)
              << " sigma="     + to_string(_sigma)
              << " nbSpecies=" + to_string(species.size())
              << " nbIndiv="     + to_string(population.size())
              << " sizeTabu="     + to_string(tabu.size())
              << " nodeId="     + to_string(_nodeId)
              << " geneId="     + to_string(_innovNumber)
              << " ]";
}

void odNeatGCController::printAll(){
    printRobot();
    print_genome(_genome);
    std::cout << "\n";
}
bool odNeatGCController::lifeTimeOver(){
    return get_lifetime()
            >= odNeatGCSharedData::gEvaluationTime - 1;
}

int odNeatGCController::get_lifetime(){
    return dynamic_cast <odNeatGCWorldObserver*>
            (gWorld->getWorldObserver())->getLifeIterationCount();
}
bool odNeatGCController::doBroadcast(){
    bool result = false;

    double adjustedFitness = std::get<1>(species[_genome -> species]);
    double totalAdjFitness = 0.0;
    std::map<int,std::pair<std::set<Genome*>,double>>::iterator it = species.begin();
    for(; it != species.end(); it++)
    {
        totalAdjFitness += std::get<1>(it->second);
    }

    if(!(totalAdjFitness == 0.0))
        if(randFloatGc() < (adjustedFitness / totalAdjFitness))
            result = true;
    return result;
}
bool odNeatGCController::in_maturation_period(){

    bool result = false;
    if(gWorld->getIterations () <=
            _birthdate + odNeatGCSharedData::gMaturationPeriod)
        result = true;

    return result;

}
bool odNeatGCController::tabu_list_approves(Genome* g)
{
    bool result = true;
    std::vector<std::pair<Genome*,int> >::iterator it = tabu.begin(), tabuEnd =  tabu.end();

    for(;it != tabuEnd;it++)
    {
        if(std::get<0>(*it)->dissimilarity(g) < odNeatGCSharedData::gTabuThreshold)
        {
            std::pair<Genome*, int> & pair  = (*it);
            std::get<1>(pair) = odNeatGCSharedData::gTabuTimeout;

            result = false;
        }
        else
        {
            //Decrease timeout
            //If timeout over, erase genome from tabu list
            std::get<1>(*it) -= 1;
            if(std::get<1>(*it) <= 0)
            {
                it = tabu.erase(it);
                tabuEnd =  tabu.end();
                if( (it) == tabuEnd)
                    break;
            }
        }
    }

    return result;
}

bool odNeatGCController::population_accepts(message msg)
{
    bool result = false;

    if(population.size() < odNeatGCSharedData::gMaxPopSize)
    {
        result = true;
    }
    else
    {
        std::map<int,message>::iterator it = population.begin();
        for(;it != population.end();it++)
        {
            //if there exists a genome with a lower fitness than the received
            if(std::get<1>(it->second) < std::get<1>(msg))
            {
                result = true;
                break;
            }
        }
    }

    return result;
}

void odNeatGCController::add_to_tabu_list(Genome* g)
{
    tabu.push_back(std::make_pair(g, odNeatGCSharedData::gTabuTimeout));
}

void odNeatGCController::add_to_population(message msg)
{

    int receivedId = std::get<0>(msg)->genome_id;
    //If the received genome already exists
    if(population.find(receivedId) != population.end())
    {
        //Update fitness by incrementally averaging with received energy
        //New_fitness = fitness + (receivedEnergy - fitness)/(numberUpdatesInThisRobot)

        std::get<1>(population[receivedId]) =
                std::get<1>(population[receivedId]) +
                        ( std::get<1>(msg) - std::get<1>(population[receivedId]) )
                        /(std::get<0>(population[receivedId])->nbFitnessUpdates);
    }
    else //new genome
    {
        //If there is still room available then add (to population and corresponding species)
        if(population.size() < odNeatGCSharedData::gMaxPopSize)
        {
            if(population.find(receivedId) == population.end())
            {
                population[receivedId] = msg;
                add_to_species(msg);
            }
        }
        else
        {
            //For searching the worse genome (the one to be replaced)
            std::map<int,message>::iterator it = population.begin();
            int worseGenomeId = -1;
            //Initialize to received genome's fitness (the replacement )
            double worseFitness = std::get<1>(msg);

            //Search for the worse genome
            for(;it != population.end();it++)
            {
                //if there exists a genome with a lower fitness
                //which is not the active one
                if( (std::get<1>(it->second) < worseFitness) && (std::get<0>(it->second) != _genome) )
                {
                    worseFitness = std::get<1>(it->second);
                    worseGenomeId = std::get<0>(it->second)->genome_id;
                }
            }

            if(worseFitness < std::get<1>(msg))
            {
                //Erase from species
                int sp = std::get<0>(population[worseGenomeId])->species;

                //Verify if species effectively exists
                if(species.find(sp) != species.end())
                {
                    std::get<0>(species[sp]).erase(std::get<0>(population[worseGenomeId]));

                    //Erase species if empty

                    if(std::get<0>(species.find(sp)->second).size() == 0)
                        species.erase(sp);
                }
                else
                {
                    std::cerr << "[ERROR] Trying to erase individual from unexisting species: " <<  sp << " in robot: " << _wm->_id << std::endl;
                    exit(-1);
                }
                Genome* worseGenome = std::get<0>(population[worseGenomeId]);

                population.erase(worseGenomeId);


                if(tabu_contains(worseGenome) == -1)
                {
                    //delete worseGenome;
                    //If tabu list does not already contain the genome to be
                    //dropped from the population, then add the genome to it
                    tabu.push_back(std::make_pair(worseGenome, odNeatGCSharedData::gTabuTimeout));
                }
                else
                {
                    tabu.erase(tabu.begin() + tabu_contains(worseGenome));
                    //Reset time out counter of the worseGenome on the tabu list
                    tabu.push_back(std::make_pair(worseGenome, odNeatGCSharedData::gTabuTimeout));
                }

                population[receivedId] = msg;
                add_to_species(msg);
            }
            else
            {
                //Not to add the active genome to be dropped, because it's not competitive enough
                //Delete from species. It does not belong to population
                if(worseGenomeId != -1)
                    std::get<0>(species[std::get<0>(population[worseGenomeId])->species]).erase(std::get<0>(population[worseGenomeId]));
            }
        }


    }

}
void odNeatGCController::add_unconditional_to_population(message msg)
{

    int receivedId = std::get<0>(msg)->genome_id;
    //If the received genome already exists
    if(population.find(receivedId) != population.end())
    {
        std::get<1>(population[receivedId]) = (std::get<1>(population[receivedId]) + std::get<1>(msg))/2; //TOCHECK: is this the way to average?
    }
    else //new genome
    {
        //If there is still room available then add (to population and corresponding species)
        if(population.size() < odNeatGCSharedData::gMaxPopSize)
        {
            if(population.find(receivedId) == population.end())
            {
                population[receivedId] = msg;
                add_to_species(msg);
            }
        }
        else
        {
            //For searching the worse genome (the one to be replaced)
            std::map<int,message>::iterator it = population.begin();
            int worseGenomeId = std::get<0>(it->second)->genome_id;
            //Replace the worse genome in population
            //By the one newly created
            double worseFitness = std::get<1>(it->second);

            //Search for the worse genome
            for(;it != population.end();it++)
            {
                //if there exists a genome with a lower fitness
                //which is not the active one
                if( (std::get<1>(it->second) <= worseFitness) && (std::get<0>(msg) == _genome) )
                {
                    worseFitness = std::get<1>(it->second);
                    worseGenomeId = std::get<0>(it->second)->genome_id;
                }
            }


            //Erase from species
            int sp = std::get<0>(population[worseGenomeId])->species;
            //Verify if species effectively exists
            if(species.find(sp) != species.end())
            {
                std::get<0>(species[sp]).erase(std::get<0>(population[worseGenomeId]));

                //Erase species if empty

                if(std::get<0>(species.find(sp)->second).size() == 0)
                    species.erase(sp);
            }
            else
            {
                std::cerr << "[ERROR] Trying to erase individual from unexisting species." << std::endl;
                exit(-1);
            }
            Genome* worseGenome = std::get<0>(population[worseGenomeId]);

            population.erase(worseGenomeId);


            if(tabu_contains(worseGenome) == -1)
            {
                //delete worseGenome;
                //If tabu list does not already contain the genome to be
                //dropped from the population, then add the genome to it
                tabu.push_back(std::make_pair(worseGenome, odNeatGCSharedData::gTabuTimeout));
            }
            else
            {
                tabu.erase(tabu.begin() + tabu_contains(worseGenome));
                //Reset time out counter of the worseGenome on the tabu list
                tabu.push_back(std::make_pair(worseGenome, odNeatGCSharedData::gTabuTimeout));
            }

            population[receivedId] = msg;
            add_to_species(msg);

        }


    }

}

int odNeatGCController::tabu_contains(Genome* g)
{
    int result = -1;
    std::vector<std::pair<Genome*,int> >::iterator it = tabu.begin(), tabuEnd =  tabu.end();
    for(; it < tabuEnd; it++)
    {
        if(g == std::get<0>(*it))
            result= it  - tabu.begin();
    }
    return result;
}
int odNeatGCController::findInPopulation(Genome* g)
{
    int result = -1;
    std::map<int,message>::iterator it = population.begin();

    for(;it != population.end();it++)
    {
        if((std::get<0>(it->second)->genome_id == g->genome_id) )
        {
            if(result == -1)
                result = it->first; //species Id
            else
            {
                std::cerr << "[ERROR] Duplicate genome in population" << std::endl;
                exit(-1);
            }
        }
    }
    return result;
}

int odNeatGCController::findInSpecies(Genome* g)
{
    int result = -1;

    std::map<int,std::pair<std::set<Genome*>,double>>::iterator it = species.begin();
    std::set<Genome*>::iterator itG;
    for(;it != species.end();it++)
    {
        itG = std::get<0>(it->second).begin();
        for(;itG != std::get<0>(it->second).end(); itG++)
        {
            if((*itG)->genome_id == g->genome_id)
            {
                if(result == -1)
                    result = it->first;
                else
                {
                    std::cerr << "[ERROR] Duplicate genome in species "
                              << result << " and " << it->first << std::endl;
                    exit(-1);
                }
            }
        }

    }
    return result;
}

void odNeatGCController::add_to_species(message msg)
{
    if(std::get<0>(msg) -> species == -1)
    {
        int speciesId = computeSpeciesId(std::get<0>(msg));

        if(species.find(speciesId) != species.end()) //If the species already exists
        {
            std::get<0>(msg)->species = speciesId;
            std::get<0>(species[speciesId]).insert(std::get<0>(msg)); //Species fitness is adjusted just later
        }
        else
        {
            std::set<Genome*> newSpecies;
            std::get<0>(msg)->species = speciesId;
            newSpecies.insert(std::get<0>(msg));
            std::get<0>(species[speciesId]) = newSpecies;
            std::get<1>(species[speciesId]) = std::get<1>(msg);//New species' fitness equals the new genome's fitness
        }
    }
    else if((species.find(std::get<0>(msg) -> species) == species.end()) ||
            (std::get<0>(species.find(std::get<0>(msg) -> species)-> second).find(std::get<0>(msg))
             == std::get<0>(species.find(std::get<0>(msg) -> species)-> second).end()))
    {
        //Error, unexisting species!
        std::cerr << "[ERROR] Unexisting species recorded on genome" << std::endl;
        exit(-1);
    }
}

int odNeatGCController::computeSpeciesId(Genome* g)
{
    std::map<int,std::pair<std::set<Genome*>,double>>::iterator it = species.begin();
    std::set<Genome*>::iterator itSp;
    int randIndiv = 0;
    int result = -1; //species' ID

    //A genome's species corresponds to the first species
    //in which a randomly sampled individual has a
    //compatibility measure lower than the threshold
    for(; it != species.end(); it++)
    {
        itSp = std::get<0>(it->second).begin();
        //Get random individual in species
        //randIndiv = randInt(1, std::get<0>(it->second).size()) - 1;
        //or not, and take first ( begin() ) as representative
        std::advance(itSp,randIndiv);

        if(g->dissimilarity(*itSp) < odNeatGCSharedData::gCompatThreshold)
        {
            result = it -> first;
            break;
        }
    }
    //No species found
    //New species ID
    if(result == -1)
    {
        if(species.size() > 0)
            result = species.rbegin()->first+1;
        else
            result = 1;
    }
    return result;
}
void odNeatGCController::recomputeAllSpecies()
{
    species.clear();

    std::map < int, message>::iterator it = population.begin();
    unsigned int i = 0;
    for(;it != population.end() ; it++)
    {
        //Invalidate previous species
        std::get<0>(it->second)->species = -1;
        add_to_species(it->second);
        i++;
    }
    if(i > odNeatGCSharedData::gMaxPopSize)
    {
        std::cerr << "[ERROR] Excess in population : " << i << "/" << odNeatGCSharedData::gMaxPopSize  << std::endl;
        exit(-1);
    }
}

void odNeatGCController::adjust_population_size()
{    
    //Not needed, already done in add_to_population()
}
void odNeatGCController::adjust_species_fitness()
{
    std::map<int,std::pair<std::set<Genome*>,double>>::iterator it = species.begin();
    std::set<Genome*>::iterator itSp;
    double adjFit = 0.0;

    for(; it != species.end(); it++)
    {
        adjFit = 0.0;
        //iterate over the individuals of current species
        itSp = std::get<0>(it->second).begin();
        for(; itSp != std::get<0>(it->second).end(); itSp++)
        {
            //Cumulate each individual's adjusted fitness
            adjFit += std::get<1>(population[(*itSp)->genome_id])/std::get<0>(it->second).size();
        }

        //The species fitness equals the average adjusted fitness of
        //the individuals belonging to it
        std::get<1>(it->second) = adjFit / std::get<0>(it->second).size();

    }


}
void odNeatGCController::adjust_active_species_fitness(int spId)
{
    std::set<Genome*>::iterator itSp;
    double adjFit = 0.0;
    //iterate over the individuals of current species
    itSp = std::get<0>(species[spId]).begin();
    int sizeSpecies = std::get<0>(species[spId]).size();
    for(; itSp != std::get<0>(species[spId]).end(); itSp++)
    {
        //Cumulate each individual's adjusted fitness
        adjFit += std::get<1>(population[(*itSp)->genome_id])/sizeSpecies;
    }
    //The species fitness equals the average adjusted fitness of
    //the individuals belonging to it
    std::get<1>(species[spId]) = adjFit / sizeSpecies;
}

Genome* odNeatGCController::generate_offspring()
{
    Genome* result = NULL;

    int spId = selectSpecies();
    Genome* g1 = selectParent(spId);
    Genome* g2 = selectParent(spId);

    int newId = _wm->getId () + 10000 * (1 + (gWorld->getIterations () /
                                              odNeatGCSharedData::gEvaluationTime));



    //Mate
    if((randFloatGc() < mateOnlyProbGc) && (g1 != g2))
    {
        result = g1 -> mate_multipoint(g2,newId, std::get<1>(population[g1->genome_id]),std::get<1>(population[g2->genome_id]));

    }
    else
    {
        result = g1 -> duplicate();
        result->genome_id = -1;
    }
    if(randFloatGc() < mutateOnlyProbGc)//Mutate
    {
        result = result-> mutate (_sigma,_wm->_id,newId,_nodeId,_innovNumber);

    }
    if((result->mom_id != -1) && (result->genome_id != -1) && (result->dad_id != -1))
    {
        //offspring comes from mate
        //with the right id's
    }
    else
    {
        result->genome_id = newId;
        result->mom_id = g1->genome_id;
        result->dad_id = -1;
    }
    return result;
}
int odNeatGCController::selectSpecies()
{
    int result = -1;
    double totalAdjFitness = 0.0;
    std::map<int,std::pair<std::set<Genome*>,double>>::iterator it = species.begin();

    for(; it != species.end(); it++)
    {
        totalAdjFitness += std::get<1>(it->second);
    }
    double random = randFloatGc() * totalAdjFitness;
    it = species.begin();
    while (random > 0.0)
    {
        random -= std::get<1>(it->second);
        it++;
    }
    if(random <=0.0)
    {
        it--;
        result = it->first;
    }
    else
    {
        std::cerr << "[ERROR] Bad species selection(?)" << std::endl;
        exit(-1);
    }
    if(result == -1)
    {
        std::cerr << "[ERROR] Bad species selection(-1)" << std::endl;
        exit(-1);
    }

    return result;
}

Genome* odNeatGCController::selectParent(int spId)
{
    //Intraspecies binary tournament
    Genome* result = NULL;
    std::set<Genome*> sp = std::get<0>(species[spId]);
    std::set<Genome*>::iterator randomIt1, randomIt2;

    if(sp.size() > 1)
    {
        int ind1 =  rand () % sp.size ();
        int ind2 =  rand () % sp.size ();

        while(ind1 == ind2)
        {
            ind2 =  rand () % sp.size ();
        }

        randomIt1 = sp.begin();
        std::advance(randomIt1,ind1);

        randomIt2 = sp.begin();
        std::advance(randomIt2,ind2);

        if(std::get<1>(population[(*randomIt1)->genome_id]) >= std::get<1>(population[(*randomIt2)->genome_id]))
        {
            result = (*randomIt1);
        }
        else
        {
            result = (*randomIt2);
        }
    }
    else
        result =(*sp.begin());

    return result;
}

void odNeatGCController::update_population(Genome* offspring)
{
    //Nothing to do (?)
}
double odNeatGCController::update_energy_level()
{
    double result = 0.0;

    switch(odNeatGCSharedData::gFitness)
    {
    case 0:
        result = updateEnergyForaging();
        break;
    case 1:
        result = updateEnergyNavigation();
        break;

    default:
        std::cerr << "[ERROR] Unknown fitness function selected. Check gFitness parameter in properties file." << std::endl;
        exit(-1);
    }
    return result;
}
double odNeatGCController::updateEnergyForaging()
{
    double result = _energy;

    double vR,vL;//Speed Tranformed into left and right wheels activation

    vR = _transV - (_rotV / 2);
    vL = _rotV + vR;
    //Consumption
    if(vL * vR < 0.0)
        result += -1;
    else
        result += (_transV/1.0) * sqrt(vL * vR);

    //cap energy (in [0,maxEnergy])
    return std::max(0.0,std::min(result,odNeatGCSharedData::gMaxEnergy));
}
double odNeatGCController::updateEnergyNavigation()
{
    double result = 0.0;
    double vR,vL;//Speed Tranformed into left and right wheels activation

    vR = _transV - (_rotV / 2);
    vL = _rotV + vR;

    result = (fabs(vR) + fabs(vL))/2 * (1 - sqrt(fabs(vR - vL))) * (_md);
    result = 2 * (result - 0.5);

    //cap energy (in [0,maxEnergy])
    return std::max(0.0,std::min(result + _energy,odNeatGCSharedData::gMaxEnergy));
}
void odNeatGCController::cleanPopAndSpecies()
{
    bool ok = true;
    std::map<int,std::pair<std::set<Genome*>,double>>::iterator itTestSp = species.begin();
    std::set<Genome*>::iterator itG;
    std::map<int,message>::iterator itNull = population.begin();
    //Erase Null genomes  (?)
    for(;itNull!= population.end();itNull++)
    {
        if(std::get<0>((itNull->second)) == NULL)
        {
            //Iterator is not invalidated
            population.erase(itNull->first);
        }
    }
    for(;itTestSp != species.end();itTestSp++)
    {
        if(std::get<0>(itTestSp->second).size() == 0 )
            itTestSp = species.erase(itTestSp);
        else
        {
            itG = ((std::get<0>(itTestSp->second)).begin());
            for(; itG != (std::get<0>(itTestSp->second)).end(); itG++)
            {
                if(findInPopulation(*itG) == -1)
                {
                    std::cerr << "[ERROR] Indiv in species not in pop" << std::endl;
                    ok = false;
                }
                if((*itG)->species == -1)
                {
                    std::cerr << "[ERROR] Id species == -1 on cleaning species. Genome == " << (*itG)->genome_id << std::endl;
                    exit(-1);
                }
            }
        }
    }

    std::map<int,message>::iterator itTest = population.begin();

    for(;itTest != population.end();itTest++)
    {
        if(findInSpecies(std::get<0>(itTest->second)) == -1)
        {
            std::cerr << "[ERROR] Cleaning, indiv. in pop not in species" << std::endl;
            ok = false;
        }
        if(std::get<0>(itTest->second)->species == -1)
        {
            std::cerr << "[ERROR] Id species == -1 on cleaning pop. Genome == " << std::get<0>(itTest->second)->genome_id << std::endl;
            exit(-1);
        }

    }
    if(!ok)
    {
        std::cerr << "[ERROR] Something happened on cleaning" << std::endl;
        exit(-1);
    }
}
