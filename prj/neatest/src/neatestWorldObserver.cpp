/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "neatest/include/neatestWorldObserver.h"
#include "neatest/include/neatestController.h"
#include "World/World.h"

#include <list>

neatestWorldObserver::neatestWorldObserver(World* world):WorldObserver(world){
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",
					 &neatestSharedData::gEvaluationTime,
					 true);

    gProperties.checkAndGetPropertyValue("gControllerType",
					 &neatestSharedData::gControllerType,
					 true);

    gProperties.checkAndGetPropertyValue("gSigmaRef",
					 &neatestSharedData::gSigmaRef,
					 false);

    gProperties.checkAndGetPropertyValue("gFitnessFunction",
					 &neatestSharedData::gFitnessFunction,
					 true);
    
    gProperties.checkAndGetPropertyValue("gSelectionPressure",
					 &neatestSharedData::gSelectionPressure,
					 true);

    gProperties.checkAndGetPropertyValue("gNeatParameters",
					 &neatestSharedData::gNeatParameters,
					 true);

    gProperties.checkAndGetPropertyValue("gMaturationTime",
					 &neatestSharedData::gMaturationTime,
					 false);

    gProperties.checkAndGetPropertyValue("gBasketCapacity",
					 &neatestSharedData::gBasketCapacity,
					 false);
    
    gProperties.checkAndGetPropertyValue("gPaintFloor",
					 &neatestSharedData::gPaintFloor,
					 false);
    gProperties.checkAndGetPropertyValue("gPaintFloorIteration",
					 &neatestSharedData::gPaintFloorIteration,
					 false);
    gProperties.checkAndGetPropertyValue("gHidePicked",
					 &neatestSharedData::gHidePicked,
					 false);
    
    NEAT::load_neat_params (neatestSharedData::gNeatParameters.c_str(), true);
    
    // ====
    //if ( !gRadioNetwork){
    //	std::cout << "Error : gRadioNetwork must be true." << std::endl;
    //	exit(-1);
    //  }

    // * iteration and generation counters
    _lifeIterationCount = -1;
    _generationCount = -1;
}

neatestWorldObserver::~neatestWorldObserver(){
	// nothing to do.
}

void neatestWorldObserver::reset(){
	// nothing to do.
}

void neatestWorldObserver::step(){
    _lifeIterationCount++;
    
    updateMonitoring();
    updateEnvironment();

    // switch to next generation.
    if( _lifeIterationCount >= neatestSharedData::gEvaluationTime ) {
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }
    
    
}


void neatestWorldObserver::updateEnvironment(){

    // hide items that were picked 
    if(neatestSharedData::gHidePicked){
	_pickedItems.clear();
	for ( int i=0 ; i != gNumberOfRobots ; i++ ){
	    std::list<int> basket = (dynamic_cast<neatestController*>
				     (gWorld->getRobot(i)->getController()))
		->getBasket();
	    
	    for(const auto& c : basket)
		_pickedItems.push_back(c);
	}
	
	for(const auto& c : gPhysicalObjects){
	    if(c->canRegister())
		c->registerObject();
	    c->display();
	}	
	
	for(const auto& c : _pickedItems){
	    gPhysicalObjects[c]->unregisterObject();
	    gPhysicalObjects[c]->hide();
	}

	std::cout << "Hiding " << _pickedItems.size() << " items: " ;
	for(const auto& c : _pickedItems)
		std::cout << c << " ";
	std::cout<< std::endl;
    }
}

void neatestWorldObserver::updateMonitoring(){
    // * Log at end of each generation
    
    // switch to next generation.
    if( _lifeIterationCount >= neatestSharedData::gEvaluationTime ) {
	
	/* * monitoring: count number of active agents. */
	
	double fitness  = 0.0;
	double popsize  = 0.0;
	double basket_usage =0.0;
	int activeCount = 0;
	int droped      = 0;
	int collected   = 0;
	int forraged    = 0;
	int basket      = 0;
	
	for ( int i = 0 ; i != gNumberOfRobots ; i++ ){
	    neatestController* controller = dynamic_cast<neatestController*>
		(gWorld->getRobot(i)->getController());
	    
	    if ( controller->getWorldModel()->isAlive() == true )
		activeCount++;

	    fitness   += controller->getFitness();
	    popsize   += controller->getPopsize();
	    droped    += controller->getMisseDroped();
	    collected += controller->getCollected();
	    forraged  += controller->getForraged();
	    basket    += controller->getBasketSize();
	    basket_usage += controller->getBasketUsage();
	}
	
	popsize /= gNumberOfRobots;
	    
	if ( gVerbose )
	    std::cout << "[gen:" 
		      << (gWorld->getIterations()/
			  neatestSharedData::gEvaluationTime) 
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
		      << "]\n";		
	
        // Logging
        std::string s = std::string("") + 
	    "{" + std::to_string(gWorld->getIterations()) + 
	    "}[all] [pop_alive:" + std::to_string(activeCount) + 
	    "] [fit:" + std::to_string(fitness) +
	    "] [popsize:" + std::to_string(popsize) +
	    "] [col:" + std::to_string(collected) +
	    "] [for:" + std::to_string(forraged) +
	    "] [use:" + std::to_string(basket_usage) +
	    "] [mis:" + std::to_string(droped) + "]\n";
        gLogManager->write(s);
	gLogManager->flush();
    }
    
    // * Every N generations, take a video (one generation)
    if ( neatestSharedData::gSnapshots ){
        if ( ( gWorld->getIterations() ) % 
	     ( neatestSharedData::gEvaluationTime * 
	       neatestSharedData::gSnapshotsFrequency ) == 0 ){
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" 
			  << (gWorld->getIterations() / 
			      neatestSharedData::gEvaluationTime ) 
			  << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % 
		 ( neatestSharedData::gEvaluationTime * 
		   neatestSharedData::gSnapshotsFrequency ) 
		 == neatestSharedData::gEvaluationTime - 1 ){
                std::cout << "[STOP]  Video recording: generation #" 
			  << (gWorld->getIterations() / 
			      neatestSharedData::gEvaluationTime ) << ".\n";
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

