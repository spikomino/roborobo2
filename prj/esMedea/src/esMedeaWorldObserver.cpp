/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 */


#include "esMedea/include/esMedeaWorldObserver.h"
#include "esMedea/include/esMedeaSharedData.h"

#include "World/World.h"


esMedeaWorldObserver::esMedeaWorldObserver(World *__world) : WorldObserver( __world ){

    _world = __world;
    
    gProperties.checkAndGetPropertyValue("gEvaluationTime",
					 &esMedeaSharedData::gEvaluationTime,
					 true);

     gProperties.checkAndGetPropertyValue("gMaturationTime",
					 &esMedeaSharedData::gMaturationTime,
					 false);

    gProperties.checkAndGetPropertyValue("gSigmaRef",
					 &esMedeaSharedData::gSigmaRef,
					 false);
    
    gProperties.checkAndGetPropertyValue("gSelectionPressure",
					 &esMedeaSharedData::gSelectionPressure,
					 false);

    
    
    gProperties.checkAndGetPropertyValue("gFitnessFunction",
					 &esMedeaSharedData::gFitnessFunction,
					 true);

    gProperties.checkAndGetPropertyValue("gHidePicked",
					 &esMedeaSharedData::gHidePicked,
					 true);
 
    gProperties.checkAndGetPropertyValue("gBasketCapacity",
					 &esMedeaSharedData::gBasketCapacity,
					 false);
    
    gProperties.checkAndGetPropertyValue("gPaintFloor",
					 &esMedeaSharedData::gPaintFloor,
					 false);

    gProperties.checkAndGetPropertyValue("gPaintFloorIteration",
					 &esMedeaSharedData::gPaintFloorIteration,
					 false);

    gProperties.checkAndGetPropertyValue("gExtendedVerbose",
					 &esMedeaSharedData::gExtendedVerbose,
					 false);

    gProperties.checkAndGetPropertyValue("gControllersDirectory",
					 &esMedeaSharedData::gControllersDirectory,
					 false);
    

    

    gProperties.checkAndGetPropertyValue("gControllerType",
					 &esMedeaSharedData::gControllerType,
					 true);

    gProperties.checkAndGetPropertyValue("gNbHiddenLayers",
					 &esMedeaSharedData::gNbHiddenLayers,
					 false);
    
    gProperties.checkAndGetPropertyValue("gNbNeuronsPerHiddenLayer",
					 &esMedeaSharedData::gNbNeuronsPerHiddenLayer,
					 false);
    
    gProperties.checkAndGetPropertyValue("gNeuronWeightRange",
					 &esMedeaSharedData::gNeuronWeightRange,
					 false);

    gProperties.checkAndGetPropertyValue("gOnlyUseBiaisForFirstHiddenLayer",
					 &esMedeaSharedData::gOnlyUseBiaisForFirstHiddenLayer,
					 true);
    
    gProperties.checkAndGetPropertyValue("gActiveBiais",
					 &esMedeaSharedData::gActiveBiais,
					 true);
    
    


    // * iteration and generation counters
    _lifeIterationCount = -1;
    _generationCount = -1;
}

esMedeaWorldObserver::~esMedeaWorldObserver()
{
	// nothing to do.
}

void esMedeaWorldObserver::reset()
{
	// nothing to do.
}

void esMedeaWorldObserver::step() {
    _lifeIterationCount++; 
}
