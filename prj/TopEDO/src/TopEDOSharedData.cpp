/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "TopEDO/include/TopEDOSharedData.h"

double TopEDOSharedData::gSigmaMin = 0.0;
double TopEDOSharedData::gProbaMutation = 0.0;
double TopEDOSharedData::gUpdateSigmaStep = 0.0;
double TopEDOSharedData::gSigmaRef = 0.0; // reference value of sigma
double TopEDOSharedData::gSigmaMax = 0.0; // maximal value of sigma
int TopEDOSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double TopEDOSharedData::gProbReEval = 0.01;
double TopEDOSharedData::gBehThresh = 1500.0;

bool TopEDOSharedData::gSynchronization = true;

bool TopEDOSharedData::gEnergyRequestOutput = false;

double TopEDOSharedData::gMonitorPositions;

bool TopEDOSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int TopEDOSharedData::gNbHiddenLayers = 0;
int TopEDOSharedData::gNbNeuronsPerHiddenLayer = 5;
int TopEDOSharedData::gNeuronWeightRange = 800;

bool TopEDOSharedData::gSnapshots = false; // take snapshots
int TopEDOSharedData::gSnapshotsFrequency = 50; // every N generations

int TopEDOSharedData::gControllerType = -1; // cf. header for description
