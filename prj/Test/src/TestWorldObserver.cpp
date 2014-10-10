/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "Test/include/TestWorldObserver.h"
#include "Test/include/TestController.h"
#include "World/World.h"


TestWorldObserver::TestWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;

	// ==== loading project-specific properties

	gProperties.checkAndGetPropertyValue("gSigmaRef",&TestSharedData::gSigmaRef,true);
	gProperties.checkAndGetPropertyValue("gSigmaMin",&TestSharedData::gSigmaMin,true);
	gProperties.checkAndGetPropertyValue("gSigmaMax",&TestSharedData::gSigmaMax,true);

	gProperties.checkAndGetPropertyValue("gProbaMutation",&TestSharedData::gProbaMutation,true);
	gProperties.checkAndGetPropertyValue("gUpdateSigmaStep",&TestSharedData::gUpdateSigmaStep,true);
	gProperties.checkAndGetPropertyValue("gEvaluationTime",&TestSharedData::gEvaluationTime,true);
	gProperties.checkAndGetPropertyValue("gSynchronization",&TestSharedData::gSynchronization,true);

    gProperties.checkAndGetPropertyValue("gEnergyRequestOutput",&TestSharedData::gEnergyRequestOutput,false);
    
	gProperties.checkAndGetPropertyValue("gMonitorPositions",&TestSharedData::gMonitorPositions,true);

    gProperties.checkAndGetPropertyValue("gNbHiddenLayers",&TestSharedData::gNbHiddenLayers,true);
	gProperties.checkAndGetPropertyValue("gNbNeuronsPerHiddenLayer",&TestSharedData::gNbNeuronsPerHiddenLayer,true);
	gProperties.checkAndGetPropertyValue("gNeuronWeightRange",&TestSharedData::gNeuronWeightRange,true);
    
	gProperties.checkAndGetPropertyValue("gSnapshots",&TestSharedData::gSnapshots,false);
	gProperties.checkAndGetPropertyValue("gSnapshotsFrequency",&TestSharedData::gSnapshotsFrequency,false);

    gProperties.checkAndGetPropertyValue("gControllerType",&TestSharedData::gControllerType,false);

    
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

TestWorldObserver::~TestWorldObserver()
{
	// nothing to do.
}

void TestWorldObserver::reset()
{
	// nothing to do.
}

void TestWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >= TestSharedData::gEvaluationTime ) // switch to next generation.
	{
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

	updateEnvironment();
    
}


void TestWorldObserver::updateEnvironment()
{
	// ...
}

void TestWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= TestSharedData::gEvaluationTime ) // switch to next generation.
	{
		// * monitoring: count number of active agents.
        
		int activeCount = 0;
		for ( int i = 0 ; i != gNumberOfRobots ; i++ )
		{
			if ( (dynamic_cast<TestController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
				activeCount++;
		}
        
		if ( gVerbose )
		{
			std::cout << "[gen:" << (gWorld->getIterations()/TestSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
		}
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
		gLogManager->flush();
	}
    
    // * Every N generations, take a video (one generation)
    
    if ( TestSharedData::gSnapshots )
    {
        if ( ( gWorld->getIterations() ) % ( TestSharedData::gEvaluationTime * TestSharedData::gSnapshotsFrequency ) == 0 )
        {
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" << (gWorld->getIterations() / TestSharedData::gEvaluationTime ) << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % ( TestSharedData::gEvaluationTime * TestSharedData::gSnapshotsFrequency ) == TestSharedData::gEvaluationTime - 1 )
            {
                std::cout << "[STOP]  Video recording: generation #" << (gWorld->getIterations() / TestSharedData::gEvaluationTime ) << ".\n";
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

