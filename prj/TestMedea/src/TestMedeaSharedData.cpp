/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */



#include "TestMedea/include/TestMedeaSharedData.h"

double TestMedeaSharedData::gSigmaMin = 0.0;
double TestMedeaSharedData::gProbaMutation = 0.0;
double TestMedeaSharedData::gUpdateSigmaStep = 0.0;
double TestMedeaSharedData::gSigmaRef = 0.0; // reference value of sigma
double TestMedeaSharedData::gSigmaMax = 0.0; // maximal value of sigma
int TestMedeaSharedData::gEvaluationTime = 0; // how long a controller will be evaluated on a robot

bool TestMedeaSharedData::gSynchronization = true;

bool TestMedeaSharedData::gEnergyRequestOutput = 1;

double TestMedeaSharedData::gMonitorPositions;

bool TestMedeaSharedData::gPropertiesLoaded = false; // global variable local to file -- TODO: move specific properties loader in dedicated WorldObserver

int TestMedeaSharedData::gNbHiddenLayers = 1;
int TestMedeaSharedData::gNbNeuronsPerHiddenLayer = 5;
int TestMedeaSharedData::gNeuronWeightRange = 800;

bool TestMedeaSharedData::gSnapshots = true; // take snapshots
int TestMedeaSharedData::gSnapshotsFrequency = 50; // every N generations

int TestMedeaSharedData::gControllerType = -1; // cf. header for description
