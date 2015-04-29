/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "Redouane/include/RedouaneSharedData.h"

double RedouaneSharedData::gSigmaMin = 0.0;
double RedouaneSharedData::gProbaMutation = 0.0;
double RedouaneSharedData::gUpdateSigmaStep = 0.0;
double RedouaneSharedData::gSigmaRef = 0.0; // reference value of sigma
double RedouaneSharedData::gSigmaMax = 0.0; // maximal value of sigma
int RedouaneSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double RedouaneSharedData::gProbReEval = 0.01;
double RedouaneSharedData::gBehThresh = 1500.0;

bool RedouaneSharedData::gSynchronization = true;

bool RedouaneSharedData::gEnergyRequestOutput = false;

double RedouaneSharedData::gMonitorPositions;

bool RedouaneSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int RedouaneSharedData::gNbHiddenLayers = 0;
int RedouaneSharedData::gNbNeuronsPerHiddenLayer = 5;
int RedouaneSharedData::gNeuronWeightRange = 800;

bool RedouaneSharedData::gSnapshots = false; // take snapshots
int RedouaneSharedData::gSnapshotsFrequency = 50; // every N generations

int RedouaneSharedData::gControllerType = -1; // cf. header for description
