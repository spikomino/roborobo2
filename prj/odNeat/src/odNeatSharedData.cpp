/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "odNeat/include/odNeatSharedData.h"

std::string odNeatSharedData::gGenomeLogFolder = "";
std::string odNeatSharedData::gEvolutionLogFile = "";
std::ofstream odNeatSharedData::gEvoLog;
int odNeatSharedData::gSelectionMethod = -1;
int odNeatSharedData::gFitness = -1;


double odNeatSharedData::gSigmaMin = 0.0;
double odNeatSharedData::gProbaMutation = 0.0;
double odNeatSharedData::gUpdateSigmaStep = 0.0;
double odNeatSharedData::gSigmaRef = 0.0; // reference value of sigma
double odNeatSharedData::gSigmaMax = 0.0; // maximal value of sigma
int odNeatSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double odNeatSharedData::gProbReEval = 0.01;
double odNeatSharedData::gBehThresh = 1500.0;

bool odNeatSharedData::gSynchronization = true;

bool odNeatSharedData::gEnergyRequestOutput = false;

double odNeatSharedData::gMonitorPositions;

bool odNeatSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int odNeatSharedData::gNbHiddenLayers = 0;
int odNeatSharedData::gNbNeuronsPerHiddenLayer = 5;
int odNeatSharedData::gNeuronWeightRange = 800;

bool odNeatSharedData::gSnapshots = false; // take snapshots
int odNeatSharedData::gSnapshotsFrequency = 50; // every N generations

int odNeatSharedData::gControllerType = -1; // cf. header for description

double odNeatSharedData::gDefaultInitialEnergy = 100;
double odNeatSharedData::gEnergyThreshold = 0;
double odNeatSharedData::gMaxEnergy = 2*odNeatSharedData::gDefaultInitialEnergy;

int odNeatSharedData::gMaturationPeriod = 50;
unsigned int odNeatSharedData::gMaxPopSize = 20;
double odNeatSharedData::gCompatThreshold = 10.0;

int odNeatSharedData::gFitnessFreq = 10;
int odNeatSharedData::gTabuTimeout = 15;
double odNeatSharedData::gTabuThreshold = 1.0;


double odNeatSharedData::gEnergItemValue = 1.0;
