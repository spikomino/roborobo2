/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 *
 */

#ifndef ODNEATRANDOMCONTROLLER_H
#define ODNEATRANDOMCONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/Graphics.h"
#include "Controllers/Controller.h"
#include "WorldModels/RobotWorldModel.h"
#include "odNeatRandom/include/odNeatRandomAgentObserver.h"
#include <neuralnetworks/NeuralNetwork.h>
#include <odneatrandom/gene.h>
#include <odneatrandom/genome.h>
#include <set>

#include <iomanip>

using namespace ODNEATGRANDOM;
using namespace std;

typedef tuple<Genome*, double, double, int> message; //Genome, fitness, sigma, birthdate

class odNeatRandomController:public Controller
{
private:
    int _iteration;

    int _birthdate;		// evaluation when this controller was initialized.

    Network *nn;


    std::vector< std::pair<Genome*, int> > tabu; //Set of dropped genomes, with the timeout when they are not to be accepted
    int _fitnessUpdateCounter; //Fitness is measured every gFitnessFreq steps    

    bool in_maturation_period();


    double update_energy_level();
    double updateEnergyNavigation();
    double updateEnergyForaging();

    void add_to_tabu_list(Genome* g);
    bool tabu_list_approves(Genome* g);
    int tabu_contains(Genome*  g);


    bool population_accepts(message msg);
    void add_to_population(message msg);
    void add_unconditional_to_population(message msg);
    int findInPopulation(Genome* g);
    int findInSpecies(Genome* g);
    void cleanPopAndSpecies();
    void add_to_species(message msg);
    int computeSpeciesId(Genome* g);
    void recomputeAllSpecies();

    void adjust_population_size();

    void adjust_species_fitness();
    void adjust_active_species_fitness(int species);
    Genome* generate_offspring();
    int selectSpecies();
    Genome* selectParent(int spId);

    void update_population(Genome* offspring);

    void createNN ();
    unsigned int computeRequiredNumberOfWeights ();
    void initRobot ();

    bool _isNewGenome;
    bool _isFixedTopo;

    void stepBehaviour ();
    void act();

    void updateFitness ();
    void updateFitnessForaging();
    void updateFitnessNavigation();

    bool doBroadcast();
    void broadcastGenome ();

    void storeMessage(message msg);

    void stepEvolution ();
    int get_lifetime();
    bool lifeTimeOver();

    void loadNewGenome ();
    void emptyGenomeList();

    int selectBest ();
    int selectRankBased();
    int selectBinaryTournament ();
    int selectRandom ();

    static bool compareFitness(std::pair<int,float> i,std::pair<int,float> j);



    bool getNewGenomeStatus ()
    {
        return _isNewGenome;
    }
    void setNewGenomeStatus (bool __status)
    {
        _isNewGenome = __status;
    }

    // evolutionary engine

    std::vector<double> _previousOut;

    int _items;
    float _sigma;
    /* Floreano's navigation fitness related attributes */
    double _transV, _rotV, _md; /* tran rot velocities and min dist */


    // ANN
    unsigned int _nbInputs;
    unsigned int _nbOutputs;



public:

    odNeatRandomController (RobotWorldModel * wm);
    ~odNeatRandomController ();

    void gatherEnergy();
    double _energy;
    Genome *_genome;
    float _fitness;

    std::map<int, message> population; //<genomeId, <genome, fitness,sigma, birthdate>>

    //Map of all <idspecies,species> with their respective fitness values
    std::map<int,std::pair<std::set<Genome*>,double>> species;

    std::vector<int> _newGenes;


    /* misc */
    void logGenome();
    void printRobot      ();
    void print_genome     (Genome* g);
    void save_genome     ();
    void printAll        ();

    void printIO(std::pair<std::vector<double>,std::vector<double>> io);
    void printVector(std::vector<double> v);
    void printFitnessList();

    void reset ();
    void step ();
    void pickItem();
    void emptyBasket();
    int getBirthdate ()
    {
        return _birthdate;
    }

};


#endif
