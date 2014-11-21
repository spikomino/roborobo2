/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "neatest/include/neatestWorldObserver.h"
#include "neatest/include/neatestController.h"
#include "World/World.h"


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

    gProperties.checkAndGetPropertyValue("gNeatParameters",
					 &neatestSharedData::gNeatParameters,
					 true);

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

    // switch to next generation.
    if( _lifeIterationCount >= neatestSharedData::gEvaluationTime ) {
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

    updateEnvironment();
}


void neatestWorldObserver::updateEnvironment(){
	// ...
}

void neatestWorldObserver::updateMonitoring(){
    // * Log at end of each generation
    
    // switch to next generation.
    if( _lifeIterationCount >= neatestSharedData::gEvaluationTime ) {
	/* * monitoring: count number of active agents. */        
	int activeCount = 0;
	for ( int i = 0 ; i != gNumberOfRobots ; i++ ){
	    if ( (dynamic_cast<neatestController*>
		  (gWorld->getRobot(i)->getController()))->getWorldModel()->
		 isAlive() == true )
		activeCount++;
	}

	/* get all agents fitness */
	double fitness =0.0;
	for ( int i = 0 ; i != gNumberOfRobots ; i++ )
	    fitness += (dynamic_cast<neatestController*>
			(gWorld->getRobot(i)->getController()))->getFitness();
	
	    
	if ( gVerbose )
	    std::cout << "[gen:" 
		      << (gWorld->getIterations()/
			  neatestSharedData::gEvaluationTime) 
		      << ";pop:" << activeCount 
		      << ";fit:" << fitness << "]\n";		
        
        // Logging
        std::string s = std::string("") + 
	    "{" + std::to_string(gWorld->getIterations()) + 
	    "}[all] [pop_alive:" + std::to_string(activeCount) + 
	    " [fit:" + std::to_string(fitness) + "]\n";
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

