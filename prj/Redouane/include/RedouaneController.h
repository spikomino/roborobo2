/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#ifndef REDOUANECONTROLLER_H
#define REDOUANECONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/Graphics.h"
#include "Controllers/Controller.h"
#include "WorldModels/RobotWorldModel.h"
#include "Redouane/include/RedouaneAgentObserver.h"
#include <neuralnetworks/NeuralNetwork.h>

#include <iomanip>

using namespace Neural;


class RedouaneController : public Controller
{
	private:
		int _iteration;
        int _birthdate; // evaluation when this controller was initialized.
    

	std::vector<double> _parameters;
	std::string _nnType;
	std::vector<int> _nbHiddenNeuronsPerLayer;
	std::vector<int> _nbBiaisNeuronsPerLayer;
	NeuralNetwork* nn;
	std::vector<std::pair< std::vector<double>, std::vector<double> > > _behavior;
	std::map<int, std::vector<std::pair< std::vector<double>, std::vector<double> > > > _behBase;
	std::map<int, int> _behBaseCounter;
	double dist(std::vector<std::pair< std::vector<double>, std::vector<double> > > b1, std::vector<std::pair< std::vector<double>, std::vector<double> > > b2);

	void feedBehaviorBase(	std::vector<std::pair< std::vector<double>, std::vector<double> > > b);
	
	void createNN();
		
        //bool _isAlive; // agent stand still if not.
        bool _isNewGenome;
	int _lastSwitch;

	int selectBest(std::map<int,float> lFitness);
        void selectRandomGenome();
        void mutate(float sigma);

        void stepBehaviour();
        void stepEvolution();
    
        void broadcastGenome();
        void loadNewGenome();
	float dist(float x1, float y1, float x2, float y2);
	float updateFitness(std::vector<double> in,std::vector<double> out);
	void storeBehavior(std::vector<double> in,std::vector<double> out);

        unsigned int computeRequiredNumberOfWeights();
    
//        void setAliveStatus( bool isAlive ) { _isAlive = isAlive; }
        bool getNewGenomeStatus() { return _isNewGenome; }
        void setNewGenomeStatus( bool __status ) { _isNewGenome = __status; }
    
        // evolutionary engine
        std::vector<double> _genome; // todo: accessing
        std::map<int, std::vector<double> > _genomesList;
        std::map<int, float > _sigmaList;
	std::map<int, float > _fitnessList;
        std::map<int,int> _birthdateList; // store the birthdate of the received controllers (useful for monitoring).
        std::vector<double> _currentGenome;
	float _currentFitness;
        float _currentSigma;
    
        // ANN
        double _minValue;
        double _maxValue;
        unsigned int _nbInputs;
        unsigned int _nbOutputs;
        unsigned int _nbHiddenLayers;
	//        std::vector<unsigned int>* _nbNeuronsPerHiddenLayer;
        std::vector<unsigned int> _nbNeuronsPerHiddenLayer;
    
        void storeGenome(std::vector<double> genome, int senderId, int senderBirthdate, float sigma);
        void resetRobot();
    
	public:

        RedouaneController(RobotWorldModel *wm);
		~RedouaneController();

		void reset();
		void step();
    
        int getBirthdate() { return _birthdate; }
    
        //bool isAlive() { return _isAlive; }


    

};


#endif

