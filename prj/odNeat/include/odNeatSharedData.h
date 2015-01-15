/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#ifndef ODNEATSHAREDDATA_H
#define ODNEATSHAREDDATA_H
#include <string>
#include <fstream>

class odNeatSharedData {
	
 public: 
  
  // -----
  static std::string gGenomeLogFolder;
  static std::string gEvolutionLogFile;
  static std::ofstream gEvoLog;
  static int gSelectionMethod;
  static int gFitness;
  static int gListeningTime;
  
  static double gSigmaMin; //! used with gDynamicSigma defined to true
  static double gUpdateSigmaStep; //!step used in the drecrease or increas of the value of sigma
  static double gSigmaRef; //! reference value of sigma
  static double gSigmaMax; //! maximal value of sigma


  static double gProbaMutation; //! probability of transmitting the current genome mutated with sigma ref
  static double gProbReEval; //!Probability of reevaluating champion individual in (1+1)-ES
  static double gBehThresh;


  static int gEvaluationTime; //! theoretical duration of a generation (ie. maximum time a controller will be evaluated on a robot)

  static int gIteration; //! used by every class to know what is the current iteration step of roborobo
  

  static bool gSynchronization; //!If set to false, a robot will restart its controller as soon as it has no more energy. If set to true, the robot without energy will wait and reload its controller at the same time as every other robots.
  
  static bool gEnergyRequestOutput; // does the robot can modulate its energy request (when being given some) ?
  
  static double gMonitorPositions; //! used in WorldObserver. Compute and log all necessary information for monitoring position and orientation wrt. center.
  
  static bool gPropertiesLoaded;
  


  static int gNbHiddenLayers; // default: 1
  static int gNbNeuronsPerHiddenLayer; // default: 5
  static int gNeuronWeightRange; // default: 800.0 (ie. weights are in [-400,+400[
  
  static bool gSnapshots; // take snapshots
  static int gSnapshotsFrequency; // every N generations
  
  static int gControllerType; // controller type (0: MLP, 1: Perceptron, 2: Elman)
    

  static double gDefaultInitialEnergy;
  static double gEnergyThreshold;
  static double gMaxEnergy;

  static int gMaturationPeriod;
  static unsigned int gMaxPopSize;
  static double gCompatThreshold;

  static int gFitnessFreq;
  static int gTabuTimeout;
  static double gTabuThreshold;

  static double gEnergyItemValue;
  static double gEnergyConsumption;
  // -----  
  
};


#endif
