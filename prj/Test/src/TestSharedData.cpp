/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "Test/include/TestSharedData.h"

double TestSharedData::gSigmaMin = 0.0;
double TestSharedData::gProbaMutation = 0.0;
double TestSharedData::gUpdateSigmaStep = 0.0;
double TestSharedData::gSigmaRef = 0.0; // reference value of sigma
double TestSharedData::gSigmaMax = 0.0; // maximal value of sigma
int TestSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double TestSharedData::gProbReEval = 0.01;
double TestSharedData::gBehThresh = 1500.0;

bool TestSharedData::gSynchronization = true;

bool TestSharedData::gEnergyRequestOutput = false;

double TestSharedData::gMonitorPositions;

bool TestSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int TestSharedData::gNbHiddenLayers = 0;
int TestSharedData::gNbNeuronsPerHiddenLayer = 5;
int TestSharedData::gNeuronWeightRange = 800;

bool TestSharedData::gSnapshots = false; // take snapshots
int TestSharedData::gSnapshotsFrequency = 50; // every N generations

int TestSharedData::gControllerType = -1; // cf. header for description
