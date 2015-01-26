/**
 * @author Inaki Fernandez
 */



#include "testInnov/include/testInnovSharedData.h"

std::string testInnovSharedData::gGenomeLogFolder = "";
std::string testInnovSharedData::gEvolutionLogFile = "";
std::ofstream testInnovSharedData::gEvoLog;
int testInnovSharedData::gSelectionMethod = -1;
int testInnovSharedData::gFitness = -1;


double testInnovSharedData::gSigmaMin = 0.0;
double testInnovSharedData::gProbaMutation = 0.0;
double testInnovSharedData::gUpdateSigmaStep = 0.0;
double testInnovSharedData::gSigmaRef = 0.0; // reference value of sigma
double testInnovSharedData::gSigmaMax = 0.0; // maximal value of sigma
int testInnovSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double testInnovSharedData::gProbReEval = 0.01;
double testInnovSharedData::gBehThresh = 1500.0;

bool testInnovSharedData::gSynchronization = true;

bool testInnovSharedData::gEnergyRequestOutput = false;

double testInnovSharedData::gMonitorPositions;

bool testInnovSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int testInnovSharedData::gNbHiddenLayers = 0;
int testInnovSharedData::gNbNeuronsPerHiddenLayer = 5;
int testInnovSharedData::gNeuronWeightRange = 800;

bool testInnovSharedData::gSnapshots = false; // take snapshots
int testInnovSharedData::gSnapshotsFrequency = 50; // every N generations

int testInnovSharedData::gControllerType = -1; // cf. header for description

double testInnovSharedData::gDefaultInitialEnergy = 100;
double testInnovSharedData::gEnergyThreshold = 0;
double testInnovSharedData::gMaxEnergy = 2*testInnovSharedData::gDefaultInitialEnergy;

int testInnovSharedData::gMaturationPeriod = 50;
unsigned int testInnovSharedData::gMaxPopSize = 20;
double testInnovSharedData::gCompatThreshold = 10.0;

int testInnovSharedData::gFitnessFreq = 10;
int testInnovSharedData::gTabuTimeout = 15;
double testInnovSharedData::gTabuThreshold = 1.0;


double testInnovSharedData::gEnergyItemValue = 10.0;
double testInnovSharedData::gEnergyConsumption = 0.1;
