/**
 * @author Inaki Fernandez
 *
 */



#include "odNeatRandom/include/odNeatRandomSharedData.h"

std::string odNeatRandomSharedData::gGenomeLogFolder = "";
std::string odNeatRandomSharedData::gEvolutionLogFile = "";
std::ofstream odNeatRandomSharedData::gEvoLog;
int odNeatRandomSharedData::gSelectionMethod = -1;
int odNeatRandomSharedData::gFitness = -1;


double odNeatRandomSharedData::gSigmaMin = 0.0;
double odNeatRandomSharedData::gProbaMutation = 0.0;
double odNeatRandomSharedData::gUpdateSigmaStep = 0.0;
double odNeatRandomSharedData::gSigmaRef = 0.0; // reference value of sigma
double odNeatRandomSharedData::gSigmaMax = 0.0; // maximal value of sigma
int odNeatRandomSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double odNeatRandomSharedData::gProbReEval = 0.01;
double odNeatRandomSharedData::gBehThresh = 1500.0;

bool odNeatRandomSharedData::gSynchronization = true;

bool odNeatRandomSharedData::gEnergyRequestOutput = false;

double odNeatRandomSharedData::gMonitorPositions;

bool odNeatRandomSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int odNeatRandomSharedData::gNbHiddenLayers = 0;
int odNeatRandomSharedData::gNbNeuronsPerHiddenLayer = 5;
int odNeatRandomSharedData::gNeuronWeightRange = 800;

bool odNeatRandomSharedData::gSnapshots = false; // take snapshots
int odNeatRandomSharedData::gSnapshotsFrequency = 50; // every N generations

int odNeatRandomSharedData::gControllerType = -1; // cf. header for description

double odNeatRandomSharedData::gDefaultInitialEnergy = 100;
double odNeatRandomSharedData::gEnergyThreshold = 0;
double odNeatRandomSharedData::gMaxEnergy = 2*odNeatRandomSharedData::gDefaultInitialEnergy;

int odNeatRandomSharedData::gMaturationPeriod = 50;
unsigned int odNeatRandomSharedData::gMaxPopSize = 20;
double odNeatRandomSharedData::gCompatThreshold = 10.0;

int odNeatRandomSharedData::gFitnessFreq = 10;
int odNeatRandomSharedData::gTabuTimeout = 15;
double odNeatRandomSharedData::gTabuThreshold = 1.0;

double odNeatRandomSharedData::gEnergyItemValue = 20.0;
double odNeatRandomSharedData::gEnergyConsumption = 0.5;

int odNeatRandomSharedData::gMaxRandomInnov = 1000;
