/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "neatest/include/neatestSharedData.h"



double neatestSharedData::gSigmaMin = 0.01;
double neatestSharedData::gProbaMutation = 0.0;
double neatestSharedData::gUpdateSigmaStep = 0.0;
double neatestSharedData::gSigmaRef = 0.3; // reference value of sigma
double neatestSharedData::gSigmaMax = 0.0; // maximal value of sigma

int neatestSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot
double neatestSharedData::gProbReEval = 0.01;
double neatestSharedData::gBehThresh = 1500.0;

bool neatestSharedData::gSynchronization = true;

bool neatestSharedData::gEnergyRequestOutput = false;

double neatestSharedData::gMonitorPositions;

bool neatestSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int neatestSharedData::gNbHiddenLayers = 0;
int neatestSharedData::gNbNeuronsPerHiddenLayer = 5;
int neatestSharedData::gNeuronWeightRange = 800;

bool neatestSharedData::gSnapshots = false; // take snapshots
int neatestSharedData::gSnapshotsFrequency = 0; // every N generations

int neatestSharedData::gControllerType = 0; // cf. header for description



std::string neatestSharedData::gNeatParameters="prj/neatest/src/explo.ne";
