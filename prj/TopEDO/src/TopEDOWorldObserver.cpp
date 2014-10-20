/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "TopEDO/include/TopEDOWorldObserver.h"
#include "TopEDO/include/TopEDOController.h"
#include "World/World.h"


TopEDOWorldObserver::TopEDOWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",&TopEDOSharedData::gEvaluationTime,true);
    gProperties.checkAndGetPropertyValue("gGenomeLogFolder",&TopEDOSharedData::gGenomeLogFolder,true);
    gProperties.checkAndGetPropertyValue("gEvolutionLogFile",&TopEDOSharedData::gEvolutionLogFile,true);

    gProperties.checkAndGetPropertyValue("gControllerType",&TopEDOSharedData::gControllerType,false);

    TopEDOSharedData::gEvoLog.open(TopEDOSharedData::gEvolutionLogFile, std::ofstream::out | std::ofstream::app);
    if(!TopEDOSharedData::gEvoLog)
      {
	std::cerr << "[ERROR] Could not open log file " << TopEDOSharedData::gEvolutionLogFile << std::endl;
	exit(-1);
      }
    // ====

    if ( !gRadioNetwork)
      {
	std::cout << "Error : gRadioNetwork must be true." << std::endl;
	exit(-1);
      }
    
    // * iteration and generation counters
    
    _lifeIterationCount = -1;
    _generationCount = -1;
    
}

TopEDOWorldObserver::~TopEDOWorldObserver()
{
	// nothing to do.
}

void TopEDOWorldObserver::reset()
{
	// nothing to do.
}

void TopEDOWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >= TopEDOSharedData::gEvaluationTime ) // switch to next generation.
	{
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

	updateEnvironment();
    
}


void TopEDOWorldObserver::updateEnvironment()
{
	// ...
}

void TopEDOWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= TopEDOSharedData::gEvaluationTime ) // switch to next generation.
	{
		// * monitoring: count number of active agents.
        
		int activeCount = 0;
		for ( int i = 0 ; i != gNumberOfRobots ; i++ )
		{
			if ( (dynamic_cast<TopEDOController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
				activeCount++;
		}
        
		if ( gVerbose )
		{
			std::cout << "[gen:" << (gWorld->getIterations()/TopEDOSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
		}
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
		gLogManager->flush();
	}
    
    // * Every N generations, take a video (one generation)
    
    if ( TopEDOSharedData::gSnapshots )
    {
        if ( ( gWorld->getIterations() ) % ( TopEDOSharedData::gEvaluationTime * TopEDOSharedData::gSnapshotsFrequency ) == 0 )
        {
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" << (gWorld->getIterations() / TopEDOSharedData::gEvaluationTime ) << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % ( TopEDOSharedData::gEvaluationTime * TopEDOSharedData::gSnapshotsFrequency ) == TopEDOSharedData::gEvaluationTime - 1 )
            {
                std::cout << "[STOP]  Video recording: generation #" << (gWorld->getIterations() / TopEDOSharedData::gEvaluationTime ) << ".\n";
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

