/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 *
 */

#ifndef TOPEDOCONTROLLER_H
#define TOPEDOCONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/Graphics.h"
#include "Controllers/Controller.h"
#include "WorldModels/RobotWorldModel.h"
#include "TopEDO/include/TopEDOAgentObserver.h"
#include <neuralnetworks/NeuralNetwork.h>
#include <pure_neat/gene.h>
#include <pure_neat/genome.h>


#include <iomanip>

using namespace PURENEAT;
using namespace std;

typedef tuple<Genome*, double, double, int,int,int> message; //Genome, fitness, sigma, birthdate, node id, link gene counter

class TopEDOController:public Controller
{
private:
    int _iteration;

    int _birthdate;		// evaluation when this controller was initialized.

    Network *nn;

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
    void broadcastGenome ();

    void storeMessage(int id, message msg);

    void stepEvolution ();
    int get_lifetime();
    bool lifeTimeOver();
    void logGenome();
    void loadNewGenome ();
    void emptyGenomeList();

    int selectBest ();
    int selectRankBased();
    int selectBinaryTournament ();
    int selectRandom ();

    static bool compareFitness(std::pair<int,float> i,std::pair<int,float> j);


    /* misc */

    void printRobot      ();
    void print_genome     (Genome* g);
    void save_genome     ();
    void printAll        ();

    void printIO(std::pair<std::vector<double>,std::vector<double>> io);
    void printVector(std::vector<double> v);
    void printFitnessList();
    
    bool getNewGenomeStatus ()
    {
        return _isNewGenome;
    }
    void setNewGenomeStatus (bool __status)
    {
        _isNewGenome = __status;
    }

    // evolutionary engine

    Genome *_genome;

    std::vector<double> _previousOut;

    std::map<int, message>  _gList;

    float _fitness;
    int _items;
    float _sigma;
    /* Floreano's navigation fitness related attributes */
    double _transV, _rotV, _md; /* tran rot velocities and min dist */

    //NOTE: NEAT-like innovation number and number of nodes FOR THIS ROBOT
    int _innovNumber;
    int _nodeId;

    // ANN
    unsigned int _nbInputs;
    unsigned int _nbOutputs;



public:

    TopEDOController (RobotWorldModel * wm);
    ~TopEDOController ();

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
