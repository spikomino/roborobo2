/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "TestMedea/include/TestMedeaWorldObserver.h"
#include "TestMedea/include/TestMedeaController.h"
#include "World/World.h"


TestMedeaWorldObserver::TestMedeaWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;

	// ==== loading project-specific properties

	gProperties.checkAndGetPropertyValue("gSigmaRef",&TestMedeaSharedData::gSigmaRef,true);
	gProperties.checkAndGetPropertyValue("gSigmaMin",&TestMedeaSharedData::gSigmaMin,true);
	gProperties.checkAndGetPropertyValue("gSigmaMax",&TestMedeaSharedData::gSigmaMax,true);

	gProperties.checkAndGetPropertyValue("gProbaMutation",&TestMedeaSharedData::gProbaMutation,true);
	gProperties.checkAndGetPropertyValue("gUpdateSigmaStep",&TestMedeaSharedData::gUpdateSigmaStep,true);
	gProperties.checkAndGetPropertyValue("gEvaluationTime",&TestMedeaSharedData::gEvaluationTime,true);
	gProperties.checkAndGetPropertyValue("gSynchronization",&TestMedeaSharedData::gSynchronization,true);

    gProperties.checkAndGetPropertyValue("gEnergyRequestOutput",&TestMedeaSharedData::gEnergyRequestOutput,false);
    
	gProperties.checkAndGetPropertyValue("gMonitorPositions",&TestMedeaSharedData::gMonitorPositions,true);

    gProperties.checkAndGetPropertyValue("gNbHiddenLayers",&TestMedeaSharedData::gNbHiddenLayers,true);
	gProperties.checkAndGetPropertyValue("gNbNeuronsPerHiddenLayer",&TestMedeaSharedData::gNbNeuronsPerHiddenLayer,true);
	gProperties.checkAndGetPropertyValue("gNeuronWeightRange",&TestMedeaSharedData::gNeuronWeightRange,true);
    
	gProperties.checkAndGetPropertyValue("gSnapshots",&TestMedeaSharedData::gSnapshots,false);
	gProperties.checkAndGetPropertyValue("gSnapshotsFrequency",&TestMedeaSharedData::gSnapshotsFrequency,false);

    gProperties.checkAndGetPropertyValue("gControllerType",&TestMedeaSharedData::gControllerType,false);

    
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

TestMedeaWorldObserver::~TestMedeaWorldObserver()
{
	// nothing to do.
}

void TestMedeaWorldObserver::reset()
{
	// nothing to do.
}

void TestMedeaWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >= TestMedeaSharedData::gEvaluationTime ) // switch to next generation.
	{
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

	updateEnvironment();
    
}


void TestMedeaWorldObserver::updateEnvironment()
{
	// ...
}

void TestMedeaWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= TestMedeaSharedData::gEvaluationTime ) // end of generation.
	{
		// * monitoring: count number of active agents.
        
		int activeCount = 0;
		for ( int i = 0 ; i != gNumberOfRobots ; i++ )
		{
			if ( (dynamic_cast<TestMedeaController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
				activeCount++;
		}
        
		if ( gVerbose )
		{
			std::cout << "[gen:" << (gWorld->getIterations()/TestMedeaSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
		}
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
		gLogManager->flush();
	}
    
    // * Every N generations, take a video (duration: one generation time)
    
    if ( TestMedeaSharedData::gSnapshots )
    {
        if ( ( gWorld->getIterations() ) % ( TestMedeaSharedData::gEvaluationTime * TestMedeaSharedData::gSnapshotsFrequency ) == 0 )
        {
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" << (gWorld->getIterations() / TestMedeaSharedData::gEvaluationTime ) << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % ( TestMedeaSharedData::gEvaluationTime * TestMedeaSharedData::gSnapshotsFrequency ) == TestMedeaSharedData::gEvaluationTime - 1 )
            {
                if ( gVerbose )
                    std::cout << "[STOP]  Video recording: generation #" << (gWorld->getIterations() / TestMedeaSharedData::gEvaluationTime ) << ".\n";
                saveTrajectoryImage();
            }
    }    
}

