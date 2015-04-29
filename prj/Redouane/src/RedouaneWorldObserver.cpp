/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "Redouane/include/RedouaneWorldObserver.h"
#include "Redouane/include/RedouaneController.h"
#include "World/World.h"


RedouaneWorldObserver::RedouaneWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;

	// ==== loading project-specific properties

	gProperties.checkAndGetPropertyValue("gSigmaRef",&RedouaneSharedData::gSigmaRef,true);
	gProperties.checkAndGetPropertyValue("gSigmaMin",&RedouaneSharedData::gSigmaMin,true);
	gProperties.checkAndGetPropertyValue("gSigmaMax",&RedouaneSharedData::gSigmaMax,true);

	gProperties.checkAndGetPropertyValue("gProbaMutation",&RedouaneSharedData::gProbaMutation,true);
	gProperties.checkAndGetPropertyValue("gUpdateSigmaStep",&RedouaneSharedData::gUpdateSigmaStep,true);
	gProperties.checkAndGetPropertyValue("gEvaluationTime",&RedouaneSharedData::gEvaluationTime,true);
	gProperties.checkAndGetPropertyValue("gSynchronization",&RedouaneSharedData::gSynchronization,true);

    gProperties.checkAndGetPropertyValue("gEnergyRequestOutput",&RedouaneSharedData::gEnergyRequestOutput,false);
    
	gProperties.checkAndGetPropertyValue("gMonitorPositions",&RedouaneSharedData::gMonitorPositions,true);

    gProperties.checkAndGetPropertyValue("gNbHiddenLayers",&RedouaneSharedData::gNbHiddenLayers,true);
	gProperties.checkAndGetPropertyValue("gNbNeuronsPerHiddenLayer",&RedouaneSharedData::gNbNeuronsPerHiddenLayer,true);
	gProperties.checkAndGetPropertyValue("gNeuronWeightRange",&RedouaneSharedData::gNeuronWeightRange,true);
    
	gProperties.checkAndGetPropertyValue("gSnapshots",&RedouaneSharedData::gSnapshots,false);
	gProperties.checkAndGetPropertyValue("gSnapshotsFrequency",&RedouaneSharedData::gSnapshotsFrequency,false);

    gProperties.checkAndGetPropertyValue("gControllerType",&RedouaneSharedData::gControllerType,false);

    
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

RedouaneWorldObserver::~RedouaneWorldObserver()
{
	// nothing to do.
}

void RedouaneWorldObserver::reset()
{
	// nothing to do.
}

void RedouaneWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >= RedouaneSharedData::gEvaluationTime ) // switch to next generation.
	{
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

	updateEnvironment();
    
}


void RedouaneWorldObserver::updateEnvironment()
{
	// ...
}

void RedouaneWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= RedouaneSharedData::gEvaluationTime ) // switch to next generation.
	{
		// * monitoring: count number of active agents.
        
		int activeCount = 0;
		for ( int i = 0 ; i != gNumberOfRobots ; i++ )
		{
			if ( (dynamic_cast<RedouaneController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
				activeCount++;
		}
        
		if ( gVerbose )
		{
			std::cout << "[gen:" << (gWorld->getIterations()/RedouaneSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
		}
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
		gLogManager->flush();
	}
    
    // * Every N generations, take a video (one generation)
    
    if ( RedouaneSharedData::gSnapshots )
    {
        if ( ( gWorld->getIterations() ) % ( RedouaneSharedData::gEvaluationTime * RedouaneSharedData::gSnapshotsFrequency ) == 0 )
        {
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" << (gWorld->getIterations() / RedouaneSharedData::gEvaluationTime ) << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % ( RedouaneSharedData::gEvaluationTime * RedouaneSharedData::gSnapshotsFrequency ) == RedouaneSharedData::gEvaluationTime - 1 )
            {
                std::cout << "[STOP]  Video recording: generation #" << (gWorld->getIterations() / RedouaneSharedData::gEvaluationTime ) << ".\n";
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

