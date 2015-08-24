/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 */


#include "esMedea/include/esMedeaWorldObserver.h"
#include "esMedea/include/esMedeaSharedData.h"
#include "esMedea/include/esMedeaController.h"

#include "World/World.h"

#include <list>

esMedeaWorldObserver::esMedeaWorldObserver(
    World *__world) : WorldObserver( __world ){

    _world = __world;
    
    gProperties.checkAndGetPropertyValue(
	"gEvaluationTime",
	&esMedeaSharedData::gEvaluationTime,
	true);
    
     gProperties.checkAndGetPropertyValue(
	 "gMaturationTime",
	 &esMedeaSharedData::gMaturationTime,
	 false);

    gProperties.checkAndGetPropertyValue(
	"gSigmaRef",
	&esMedeaSharedData::gSigmaRef,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gSelectionPressure",
	&esMedeaSharedData::gSelectionPressure,
	false);

    
    
    gProperties.checkAndGetPropertyValue(
	"gFitnessFunction",
	&esMedeaSharedData::gFitnessFunction,
	true);

    gProperties.checkAndGetPropertyValue(
	"gHidePicked",
	&esMedeaSharedData::gHidePicked,
	true);
    
    gProperties.checkAndGetPropertyValue(
	"gBasketCapacity",
	&esMedeaSharedData::gBasketCapacity,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gPaintFloor",
	&esMedeaSharedData::gPaintFloor,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gPaintFloorIteration",
	&esMedeaSharedData::gPaintFloorIteration,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gExtendedVerbose",
	&esMedeaSharedData::gExtendedVerbose,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gControllersDirectory",
	&esMedeaSharedData::gControllersDirectory,
	false);
    

    

    gProperties.checkAndGetPropertyValue(
	"gControllerType",
	&esMedeaSharedData::gControllerType,
	true);
    
    gProperties.checkAndGetPropertyValue(
	"gNbHiddenLayers",
	&esMedeaSharedData::gNbHiddenLayers,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gNbNeuronsPerHiddenLayer",
	&esMedeaSharedData::gNbNeuronsPerHiddenLayer,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gNeuronWeightRange",
	&esMedeaSharedData::gNeuronWeightRange,
	false);
    
    gProperties.checkAndGetPropertyValue(
	"gOnlyUseBiaisForFirstHiddenLayer",
	&esMedeaSharedData::gOnlyUseBiaisForFirstHiddenLayer,
	true);
    
    gProperties.checkAndGetPropertyValue(
	"gActiveBiais",
	&esMedeaSharedData::gActiveBiais,
	true);
    
    


    // * iteration and generation counters
    _lifeIterationCount = -1;
    _generationCount    = -1;
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
    
    updateMonitoring();
    updateEnvironment();

    // switch to next generation.
    if( _lifeIterationCount >= esMedeaSharedData::gEvaluationTime ) {
	_lifeIterationCount = 0;
        _generationCount++;
    }
}

void esMedeaWorldObserver::updateEnvironment(){

    // hide items that were picked 
    if(esMedeaSharedData::gHidePicked){
	_pickedItems.clear();
	
	/* record all items from agent basket */
	for ( int i=0 ; i != gNumberOfRobots ; i++ ){
	    std::list<int>& basket = (dynamic_cast<esMedeaController*>
				      (gWorld->getRobot(i)->getController()))
		->getBasket();
	    
	    for(const auto& c : basket)
		_pickedItems.push_back(c);
	}
	
	/* make sure all items in world are drawn */
	for(const auto& c : gPhysicalObjects){
	    if(c->canRegister())
		c->registerObject();
	    c->display();
	}	
	
	/* hide all picked */
	for(const auto& c : _pickedItems){
	    gPhysicalObjects[c]->unregisterObject();
	    gPhysicalObjects[c]->hide();
	}

	/**/
	if( esMedeaSharedData::gExtendedVerbose ){
	    std::cout << "Hiding " << _pickedItems.size() << " items: " ;
	    for(const auto& c : _pickedItems)
		std::cout << c << " ";
	    std::cout<< std::endl;
	}
    }
}

void esMedeaWorldObserver::updateMonitoring(){
    // * Log at end of each generation
    
    // switch to next generation.
    if( _lifeIterationCount >= esMedeaSharedData::gEvaluationTime ) {
	
	/* * monitoring: count number of active agents. */
	
	double fitness      = 0.0;
	double popsize      = 0.0;
	double basket_usage = 0.0;
	double locomotion   = 0.0;
	int activeCount     = 0;
	int droped          = 0;
	int collected       = 0;
	int forraged        = 0;
	int basket          = 0;
	int frg_at_landmark = 0; 

	for ( int i = 0 ; i != gNumberOfRobots ; i++ ){
	    esMedeaController* controller = dynamic_cast<esMedeaController*>
		(gWorld->getRobot(i)->getController());
	    
	    if ( controller->getWorldModel()->isAlive() == true )
		activeCount++;

	    fitness         += controller->getFitness();
	    popsize         += controller->getPopsize();
	    droped          += controller->getMisseDroped();
	    collected       += controller->getCollected();
	    forraged        += controller->getForraged();
	    basket          += controller->getBasketSize();
	    basket_usage    += controller->getBasketUsage();
	    frg_at_landmark += controller->getItemsAtLandmark();
	    locomotion      += controller->getLocomotion();
	}
	
	popsize      /= gNumberOfRobots;
	basket_usage /= gNumberOfRobots; 
	locomotion   /= gNumberOfRobots; 


	if ( gVerbose )
	    std::cout << "[gen:" 
		      << (gWorld->getIterations()/
			  esMedeaSharedData::gEvaluationTime) 
		      << ";pop:"     << activeCount 
		      << ";fit:"     << fitness
		      << ";popsize:" << popsize  
		      << ";col:"     << collected 
		      << ";for:"     << forraged  
		      << ";mis:"     << droped
		      << ";bsk:"     << basket
		      << ";lst"      << _pickedItems.size()
		      << ";total:"   << gPhysicalObjects.size()
		      << ";bskuse:"  << basket_usage
		      << ";lnd:"     << frg_at_landmark
		      << ";loc:"     << locomotion
		      << "]\n";		
	
        // Logging
        std::string s = std::string("") + 
	    "{" + std::to_string(gWorld->getIterations()) + 
	    "}[all] [pop_alive:" + std::to_string(activeCount) + 
	    "] [fit:" + std::to_string(fitness) +
	    "] [popsize:" + std::to_string(popsize) +
	    "] [col:" + std::to_string(collected) +
	    "] [for:" + std::to_string(forraged) +
	    "] [mis:" + std::to_string(droped) +
	    "] [bsk:" + std::to_string(basket) +
	    "] [lst:" + std::to_string(_pickedItems.size()) +
	    "] [tot:" + std::to_string(gPhysicalObjects.size()) +
	    "] [use:" + std::to_string(basket_usage) + 
	    "] [lnd:" + std::to_string(frg_at_landmark) + 
	    "] [loc:" + std::to_string(locomotion) + "]\n";
        gLogManager->write(s);
	gLogManager->flush();
    }
    
    // * Every N generations, take a video (one generation)
    if ( esMedeaSharedData::gSnapshots ){
        if ( ( gWorld->getIterations() ) % 
	     ( esMedeaSharedData::gEvaluationTime * 
	       esMedeaSharedData::gSnapshotsFrequency ) == 0 ){
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" 
			  << (gWorld->getIterations() / 
			      esMedeaSharedData::gEvaluationTime ) 
			  << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % 
		 ( esMedeaSharedData::gEvaluationTime * 
		   esMedeaSharedData::gSnapshotsFrequency ) 
		 == esMedeaSharedData::gEvaluationTime - 1 ){
                std::cout << "[STOP]  Video recording: generation #" 
			  << (gWorld->getIterations() / 
			      esMedeaSharedData::gEvaluationTime ) << ".\n";
                saveTrajectoryImage();
            }
    }

    // * Snapshots: take screenshots of first and ~ultimate iteration
    /* //todelete
    if ( gWorld->getIterations() == 1 )
    {
        saveScreenshot("firstIteration");
        saveRobotTrackerIndex("firstIteration");
    }
    else
    {
        if ( gWorld->getIterations() == gMaxIt-2 )
        {
            gDisplayMode = 0;
            //gDisplaySensors = 2; // prepare for next it.
        }
        else
        {
            if ( gWorld->getIterations() == gMaxIt-1 )
            {
                saveScreenshot("lastIteration");
                saveRobotTrackerIndex("lastIteration");
            }
        }
    }
    */
    
}

