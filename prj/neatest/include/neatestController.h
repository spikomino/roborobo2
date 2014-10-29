/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 *
 */

#ifndef NEATESTCONTROLLER_H
#define NEATESTCONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/Graphics.h"
#include "Controllers/Controller.h"
#include "WorldModels/RobotWorldModel.h"
#include "neatest/include/neatestAgentObserver.h"
#include <neat/gene.h>
#include <neat/genome.h>
#include <neat/genomeAdapted.h>

#include <iomanip>

using namespace NEAT;
using namespace std;

class neatestController:public Controller {

private:
    int _iteration;
    int _birthdate;	 // iteration number at witch are created 
    
    GenomeAdapted* _genome;
    Network*       _neurocontroller;

        
    float _currentFitness;
    float _currentSigma;

    //NOTE: NEAT-like innovation number and number of nodes FOR THIS ROBOT
    double innovNumber;
    int nodeId;

    // ANN
    unsigned int _nbInputs;
    unsigned int _nbOutputs;
    
  

    // evolutionary engine

    map <int, GenomeAdapted*> _genomesList;
    map <int, float>          _sigmaList;
    map <int, float>          _fitnessList;
    map <int, int>            _birthdateList;	
    // store the birthdate of the received controllers (useful for monitoring).
    

    // Methods 

    void initRobot();
    void createNeuroController();
    
    void stepBehaviour ();
    void broadcastGenome ();
    void emptyGenomeList();

    bool lifeTimeOver();

    std::vector<double>  stepNeuralController();
    pair<vector<double>, vector<double>> act();
    float updateFitness (vector<double> in, vector<double> out);

    void storeGenome (GenomeAdapted* genome, 
		      int senderId, 
		      int senderBirthdate,
		      float sigma, 
		      float fitness);  




    void stepEvolution ();
    void logGenome();
    
    int selectBest   (map<int, float> lFitness);
    int selectRandom (map<int, float> lFitness);

    void mutate (float sigma);

    void printRobot();


public:

    neatestController (RobotWorldModel * wm);
    ~neatestController ();

    void reset ();
    void step ();

    int getBirthdate ()
    {
	return _birthdate;
    }
    

};


#endif
