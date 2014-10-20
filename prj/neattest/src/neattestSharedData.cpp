/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "neattest/include/neattestSharedData.h"

std::string neattestSharedData::gGenomeLogFolder = "";

double neattestSharedData::gSigmaMin = 0.0;
double neattestSharedData::gProbaMutation = 0.0;
double neattestSharedData::gUpdateSigmaStep = 0.0;
double neattestSharedData::gSigmaRef = 0.0; // reference value of sigma
double neattestSharedData::gSigmaMax = 0.0; // maximal value of sigma
int neattestSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double neattestSharedData::gProbReEval = 0.01;
double neattestSharedData::gBehThresh = 1500.0;

bool neattestSharedData::gSynchronization = true;

bool neattestSharedData::gEnergyRequestOutput = false;

double neattestSharedData::gMonitorPositions;

bool neattestSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int neattestSharedData::gNbHiddenLayers = 0;
int neattestSharedData::gNbNeuronsPerHiddenLayer = 5;
int neattestSharedData::gNeuronWeightRange = 800;

bool neattestSharedData::gSnapshots = false; // take snapshots
int neattestSharedData::gSnapshotsFrequency = 50; // every N generations

int neattestSharedData::gControllerType = -1; // cf. header for description
