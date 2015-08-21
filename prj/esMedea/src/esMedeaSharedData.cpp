#include "esMedea/include/esMedeaSharedData.h"




int    esMedeaSharedData::gEvaluationTime      = 10000;
double esMedeaSharedData::gMaturationTime      = 0.75;
double esMedeaSharedData::gSigmaRef            = 1.0;
double esMedeaSharedData::gSelectionPressure   = 0.8;
   
int    esMedeaSharedData::gControllerType      = 0 ;
      
int    esMedeaSharedData::gFitnessFunction     = 2;
  
bool   esMedeaSharedData::gHidePicked          = true;
int    esMedeaSharedData::gBasketCapacity      = 10;

bool   esMedeaSharedData::gPaintFloor          = true;
int    esMedeaSharedData::gPaintFloorIteration = 1000000;

bool   esMedeaSharedData::gExtendedVerbose     = true;
std::string esMedeaSharedData::gControllersDirectory = "";

int    esMedeaSharedData::gNbHiddenLayers          =1; 
int    esMedeaSharedData::gNbNeuronsPerHiddenLayer =5; 
int    esMedeaSharedData::gNeuronWeightRange       =800;
bool   esMedeaSharedData::gOnlyUseBiaisForFirstHiddenLayer = true;
bool   esMedeaSharedData::gActiveBiais = true;
