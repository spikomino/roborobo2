#ifndef ESMEDEASHAREDDATA_H
#define ESMEDEASHAREDDATA_H
#include <string>

class esMedeaSharedData {
	
 public: 
  
    static int    gEvaluationTime;
    static double gMaturationTime;
    static double gSigmaRef;
    static double gSelectionPressure;
    
    static int    gFitnessFunction;

    static bool   gHidePicked;
    static int    gBasketCapacity;

    static bool   gPaintFloor;
    static int    gPaintFloorIteration;

    static bool   gExtendedVerbose;
    static std::string gControllersDirectory;

    static int    gControllerType;
    static int    gNbHiddenLayers; 
    static int    gNbNeuronsPerHiddenLayer;
    static int    gNeuronWeightRange;
    static bool   gOnlyUseBiaisForFirstHiddenLayer;
    static bool   gActiveBiais;

    static bool   gSnapshots; // take snapshots
    static int    gSnapshotsFrequency; // every N generations

};


#endif
