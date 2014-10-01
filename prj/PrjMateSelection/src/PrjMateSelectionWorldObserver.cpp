/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "PrjMateSelection/include/PrjMateSelectionWorldObserver.h"
#include "PrjMateSelection/include/PrjMateSelectionController.h"
#include "World/World.h"


PrjMateSelectionWorldObserver::PrjMateSelectionWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;

	// ==== loading project-specific properties

	gProperties.checkAndGetPropertyValue("gSigmaRef",&PrjMateSelectionSharedData::gSigmaRef,true);
	gProperties.checkAndGetPropertyValue("gSigmaMin",&PrjMateSelectionSharedData::gSigmaMin,true);
	gProperties.checkAndGetPropertyValue("gSigmaMax",&PrjMateSelectionSharedData::gSigmaMax,true);

	gProperties.checkAndGetPropertyValue("gProbaMutation",&PrjMateSelectionSharedData::gProbaMutation,true);
	gProperties.checkAndGetPropertyValue("gUpdateSigmaStep",&PrjMateSelectionSharedData::gUpdateSigmaStep,true);
	gProperties.checkAndGetPropertyValue("gEvaluationTime",&PrjMateSelectionSharedData::gEvaluationTime,true);
	gProperties.checkAndGetPropertyValue("gSynchronization",&PrjMateSelectionSharedData::gSynchronization,true);

    gProperties.checkAndGetPropertyValue("gEnergyRequestOutput",&PrjMateSelectionSharedData::gEnergyRequestOutput,false);
    
	gProperties.checkAndGetPropertyValue("gMonitorPositions",&PrjMateSelectionSharedData::gMonitorPositions,true);

    gProperties.checkAndGetPropertyValue("gNbHiddenLayers",&PrjMateSelectionSharedData::gNbHiddenLayers,true);
	gProperties.checkAndGetPropertyValue("gNbNeuronsPerHiddenLayer",&PrjMateSelectionSharedData::gNbNeuronsPerHiddenLayer,true);
	gProperties.checkAndGetPropertyValue("gNeuronWeightRange",&PrjMateSelectionSharedData::gNeuronWeightRange,true);
    
	gProperties.checkAndGetPropertyValue("gSnapshots",&PrjMateSelectionSharedData::gSnapshots,false);
	gProperties.checkAndGetPropertyValue("gSnapshotsFrequency",&PrjMateSelectionSharedData::gSnapshotsFrequency,false);

    gProperties.checkAndGetPropertyValue("gControllerType",&PrjMateSelectionSharedData::gControllerType,false);

    
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

PrjMateSelectionWorldObserver::~PrjMateSelectionWorldObserver()
{
	// nothing to do.
}

void PrjMateSelectionWorldObserver::reset()
{
	// nothing to do.
}

void PrjMateSelectionWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >= PrjMateSelectionSharedData::gEvaluationTime ) // switch to next generation.
	{
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

	updateEnvironment();
    
}


void PrjMateSelectionWorldObserver::updateEnvironment()
{
	// ...
}

void PrjMateSelectionWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= PrjMateSelectionSharedData::gEvaluationTime ) // end of generation.
	{
		// * monitoring: count number of active agents.
        
		int activeCount = 0;
		for ( int i = 0 ; i != gNumberOfRobots ; i++ )
		{
			if ( (dynamic_cast<PrjMateSelectionController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
				activeCount++;
		}
        
		if ( gVerbose )
		{
			std::cout << "[gen:" << (gWorld->getIterations()/PrjMateSelectionSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
		}
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
		gLogManager->flush();
	}
    
    // * Every N generations, take a video (duration: one generation time)
    
    if ( PrjMateSelectionSharedData::gSnapshots )
    {
        if ( ( gWorld->getIterations() ) % ( PrjMateSelectionSharedData::gEvaluationTime * PrjMateSelectionSharedData::gSnapshotsFrequency ) == 0 )
        {
            if ( gVerbose )
                std::cout << "[START] Video recording: generation #" << (gWorld->getIterations() / PrjMateSelectionSharedData::gEvaluationTime ) << ".\n";
            gTrajectoryMonitorMode = 0;
            initTrajectoriesMonitor();
        }
        else
            if ( ( gWorld->getIterations() ) % ( PrjMateSelectionSharedData::gEvaluationTime * PrjMateSelectionSharedData::gSnapshotsFrequency ) == PrjMateSelectionSharedData::gEvaluationTime - 1 )
            {
                if ( gVerbose )
                    std::cout << "[STOP]  Video recording: generation #" << (gWorld->getIterations() / PrjMateSelectionSharedData::gEvaluationTime ) << ".\n";
                saveTrajectoryImage();
            }
    }    
}

