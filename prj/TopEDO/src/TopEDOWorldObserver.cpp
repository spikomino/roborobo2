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

    gProperties.checkAndGetPropertyValue("gControllerType",&TopEDOSharedData::gControllerType,true);

    gProperties.checkAndGetPropertyValue("mutate_only_prob",&mutate_only_prob,true);
    gProperties.checkAndGetPropertyValue("mutate_link_weights_prob",&mutate_link_weights_prob,true);
    gProperties.checkAndGetPropertyValue("mutate_toggle_enable_prob",&mutate_toggle_enable_prob,true);
    gProperties.checkAndGetPropertyValue("mutate_gene_reenable_prob",&mutate_gene_reenable_prob,true);
    gProperties.checkAndGetPropertyValue("mutate_add_node_prob",&mutate_add_node_prob,true);
    gProperties.checkAndGetPropertyValue("mutate_add_link_prob",&mutate_add_link_prob,true);    
    gProperties.checkAndGetPropertyValue("mate_only_prob",&mate_only_prob,true);
    gProperties.checkAndGetPropertyValue("recur_only_prob",&recur_only_prob,true);
    gProperties.checkAndGetPropertyValue("newstructure_tries",&newstructure_tries,true);

    gProperties.checkAndGetPropertyValue("gSelectionMethod",&TopEDOSharedData::gSelectionMethod,true);
    gProperties.checkAndGetPropertyValue("gFitness",&TopEDOSharedData::gFitness,true);
    gProperties.checkAndGetPropertyValue("gSigmaRef",&TopEDOSharedData::gSigmaRef,true);


    TopEDOSharedData::gEvoLog.open(TopEDOSharedData::gEvolutionLogFile, std::ofstream::out | std::ofstream::app);
    if(!TopEDOSharedData::gEvoLog)
    {
        std::cerr << "[ERROR] Could not open log file " << TopEDOSharedData::gEvolutionLogFile << std::endl;
        exit(-1);
    }


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

    if( _lifeIterationCount >=
            TopEDOSharedData::gEvaluationTime ) // switch to next generation.
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

    if ( gWorld->getIterations() == gMaxIt-1 )
    {
        TopEDOSharedData::gEvoLog.close();
    }
}

