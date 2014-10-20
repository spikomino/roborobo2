/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "neattest/include/neattestWorldObserver.h"
#include "neattest/include/neattestController.h"
#include "World/World.h"


neattestWorldObserver::neattestWorldObserver(World* world):WorldObserver(world){
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",
					 &neattestSharedData::gEvaluationTime,
					 true);
    gProperties.checkAndGetPropertyValue("gGenomeLogFolder",
					 &neattestSharedData::gGenomeLogFolder,
					 true);

    gProperties.checkAndGetPropertyValue("gControllerType",
					 &neattestSharedData::gControllerType,
					 false);
    
    // ====
    if ( !gRadioNetwork){
	std::cout << "Error : gRadioNetwork must be true." << std::endl;
	exit(-1);
    }

    // * iteration and generation counters
    _lifeIterationCount = -1;
    _generationCount = -1;
}

neattestWorldObserver::~neattestWorldObserver(){
	// nothing to do.
}

void neattestWorldObserver::reset(){
	// nothing to do.
}

void neattestWorldObserver::step(){
    _lifeIterationCount++;
    updateMonitoring();

    // switch to next generation.
    if( _lifeIterationCount >= neattestSharedData::gEvaluationTime ) {
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

    updateEnvironment();
}


void neattestWorldObserver::updateEnvironment(){
	// ...
}

void neattestWorldObserver::updateMonitoring(){
    // * Log at end of each generation
    
    // switch to next generation.
    if( _lifeIterationCount >= neattestSharedData::gEvaluationTime ) {
	// * monitoring: count number of active agents.
        
	int activeCount = 0;
	for ( int i = 0 ; i != gNumberOfRobots ; i++ )
	    if ( (dynamic_cast<neattestController*>
		  (gWorld->getRobot(i)->getController()))->getWorldModel()->
		 isAlive() == true )
		activeCount++;
	        
	if ( gVerbose )
	    std::cout << "[gen:" 
		      << (gWorld->getIterations()/
			  neattestSharedData::gEvaluationTime) 
		      << ";pop:" << activeCount << "]\n";		
        
        // Logging
        std::string s = std::string("") + 
	    "{" + std::to_string(gWorld->getIterations()) + 
	    "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
	gLogManager->flush();
    }
    
    // * Every N generations, take a video (one generation)
    if ( neattestSharedData::gSnapshots ){
        if ( ( gWorld->getIterations() ) % 
	     ( neattestSharedData::gEvaluationTime * 
	       neattestSharedData::gSnapshotsFrequency ) == 0 ){
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" 
			  << (gWorld->getIterations() / 
			      neattestSharedData::gEvaluationTime ) 
			  << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % 
		 ( neattestSharedData::gEvaluationTime * 
		   neattestSharedData::gSnapshotsFrequency ) 
		 == neattestSharedData::gEvaluationTime - 1 ){
                std::cout << "[STOP]  Video recording: generation #" 
			  << (gWorld->getIterations() / 
			      neattestSharedData::gEvaluationTime ) << ".\n";
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

