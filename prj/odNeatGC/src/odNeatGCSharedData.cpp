/**
 * @author Inaki Fernandez
 *
 */



#include "odNeatGC/include/odNeatGCSharedData.h"

std::string odNeatGCSharedData::gGenomeLogFolder = "";
std::string odNeatGCSharedData::gEvolutionLogFile = "";
std::ofstream odNeatGCSharedData::gEvoLog;
int odNeatGCSharedData::gSelectionMethod = -1;
int odNeatGCSharedData::gFitness = -1;


double odNeatGCSharedData::gSigmaMin = 0.0;
double odNeatGCSharedData::gProbaMutation = 0.0;
double odNeatGCSharedData::gUpdateSigmaStep = 0.0;
double odNeatGCSharedData::gSigmaRef = 0.0; // reference value of sigma
double odNeatGCSharedData::gSigmaMax = 0.0; // maximal value of sigma
int odNeatGCSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double odNeatGCSharedData::gProbReEval = 0.01;
double odNeatGCSharedData::gBehThresh = 1500.0;

bool odNeatGCSharedData::gSynchronization = true;

bool odNeatGCSharedData::gEnergyRequestOutput = false;

double odNeatGCSharedData::gMonitorPositions;

bool odNeatGCSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int odNeatGCSharedData::gNbHiddenLayers = 0;
int odNeatGCSharedData::gNbNeuronsPerHiddenLayer = 5;
int odNeatGCSharedData::gNeuronWeightRange = 800;

bool odNeatGCSharedData::gSnapshots = false; // take snapshots
int odNeatGCSharedData::gSnapshotsFrequency = 50; // every N generations

int odNeatGCSharedData::gControllerType = -1; // cf. header for description

double odNeatGCSharedData::gDefaultInitialEnergy = 100;
double odNeatGCSharedData::gEnergyThreshold = 0;
double odNeatGCSharedData::gMaxEnergy = 2*odNeatGCSharedData::gDefaultInitialEnergy;

int odNeatGCSharedData::gMaturationPeriod = 50;
unsigned int odNeatGCSharedData::gMaxPopSize = 20;
double odNeatGCSharedData::gCompatThreshold = 10.0;

int odNeatGCSharedData::gFitnessFreq = 10;
int odNeatGCSharedData::gTabuTimeout = 15;
double odNeatGCSharedData::gTabuThreshold = 1.0;


double odNeatGCSharedData::gEnergyItemValue = 10.0;
double odNeatGCSharedData::gEnergyConsumption = 0.1;

bool odNeatGCSharedData::gUpdateGC = true;
