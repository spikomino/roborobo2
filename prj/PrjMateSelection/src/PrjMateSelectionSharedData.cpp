/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "PrjMateSelection/include/PrjMateSelectionSharedData.h"

double PrjMateSelectionSharedData::gSigmaMin = 0.0;
double PrjMateSelectionSharedData::gProbaMutation = 0.0;
double PrjMateSelectionSharedData::gUpdateSigmaStep = 0.0;
double PrjMateSelectionSharedData::gSigmaRef = 0.0; // reference value of sigma
double PrjMateSelectionSharedData::gSigmaMax = 0.0; // maximal value of sigma
int PrjMateSelectionSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot

bool PrjMateSelectionSharedData::gSynchronization = true;

bool PrjMateSelectionSharedData::gEnergyRequestOutput = 1;

double PrjMateSelectionSharedData::gMonitorPositions;

bool PrjMateSelectionSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int PrjMateSelectionSharedData::gNbHiddenLayers = 1;
int PrjMateSelectionSharedData::gNbNeuronsPerHiddenLayer = 5;
int PrjMateSelectionSharedData::gNeuronWeightRange = 800;

bool PrjMateSelectionSharedData::gSnapshots = true; // take snapshots
int PrjMateSelectionSharedData::gSnapshotsFrequency = 50; // every N generations

int PrjMateSelectionSharedData::gControllerType = -1; // cf. header for description

bool PrjMateSelectionSharedData::gLimitGenomeTransmission = false; // default: do not limit.
int PrjMateSelectionSharedData::gMaxNbGenomeTransmission = 65535; // default: arbitrarily set to 65535.

int PrjMateSelectionSharedData::gSelectionMethod = 0; // default: random selection

int PrjMateSelectionSharedData::gNotListeningStateDelay = 0;    // -1: infinite ; 0: no delay ; >0: delay
int PrjMateSelectionSharedData::gListeningStateDelay = -1;      // -1: infinite ; 0: no delay ; >0: delay (ignored if gNotListeningStateDelay=-1)
