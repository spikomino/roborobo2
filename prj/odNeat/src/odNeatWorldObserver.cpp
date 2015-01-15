/**
 * @author Inaki Fernandez
 *
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "odNeat/include/odNeatWorldObserver.h"
#include "odNeat/include/odNeatController.h"
#include "World/World.h"


odNeatWorldObserver::odNeatWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",&odNeatSharedData::gEvaluationTime,true);
    gProperties.checkAndGetPropertyValue("gGenomeLogFolder",&odNeatSharedData::gGenomeLogFolder,true);
    gProperties.checkAndGetPropertyValue("gEvolutionLogFile",&odNeatSharedData::gEvolutionLogFile,true);

    gProperties.checkAndGetPropertyValue("gControllerType",&odNeatSharedData::gControllerType,true);

    gProperties.checkAndGetPropertyValue("mutate_only_prob",&mutateOnlyProb,true);
    gProperties.checkAndGetPropertyValue("mutate_link_weights_prob",&mutateLinkWeightsProb,true);
    gProperties.checkAndGetPropertyValue("mutate_toggle_enable_prob",&mutateToggleEnableProb,true);
    gProperties.checkAndGetPropertyValue("mutate_gene_reenable_prob",&mutateGeneReenableProb,true);
    gProperties.checkAndGetPropertyValue("mutate_add_node_prob",&mutateAddNodeProb,true);
    gProperties.checkAndGetPropertyValue("mutate_add_link_prob",&mutateAddLinkProb,true);
    gProperties.checkAndGetPropertyValue("mate_only_prob",&mateOnlyProb,true);
    gProperties.checkAndGetPropertyValue("recur_only_prob",&recurOnlyProb,true);
    gProperties.checkAndGetPropertyValue("newstructure_tries",&newStructureTries,true);

    gProperties.checkAndGetPropertyValue("gSelectionMethod",&odNeatSharedData::gSelectionMethod,true);
    gProperties.checkAndGetPropertyValue("gFitness",&odNeatSharedData::gFitness,true);
    gProperties.checkAndGetPropertyValue("gSigmaRef",&odNeatSharedData::gSigmaRef,true);

    //odNeat parameters
    gProperties.checkAndGetPropertyValue("gDefaultInitialEnergy",&odNeatSharedData::gDefaultInitialEnergy,true);
    gProperties.checkAndGetPropertyValue("gEnergyThreshold",&odNeatSharedData::gEnergyThreshold,true);
    gProperties.checkAndGetPropertyValue("gMaturationPeriod",&odNeatSharedData::gMaturationPeriod,true);
    int aux;
    gProperties.checkAndGetPropertyValue("gMaxPopSize",&aux,true);
    if(aux > 0 )
        odNeatSharedData::gMaxPopSize = aux;
    else
    {
        std::cerr << "[ERROR] Wrong gMaxPopSize" << std::endl;
        exit(-1);
    }
    gProperties.checkAndGetPropertyValue("gCompatThreshold",&odNeatSharedData::gCompatThreshold,true);
    gProperties.checkAndGetPropertyValue("gTabuTimeout",&odNeatSharedData::gTabuTimeout,true);
    gProperties.checkAndGetPropertyValue("gTabuThreshold",&odNeatSharedData::gTabuThreshold,true);
    gProperties.checkAndGetPropertyValue("gEnergItemValue",&odNeatSharedData::gEnergyItemValue,true);
    gProperties.checkAndGetPropertyValue("gFitnessFreq",&odNeatSharedData::gFitnessFreq,true);


    odNeatSharedData::gEvoLog.open(odNeatSharedData::gEvolutionLogFile);//, std::ofstream::out | std::ofstream::app);

    if(!odNeatSharedData::gEvoLog)
    {
        std::cerr << "[ERROR] Could not open log file " << odNeatSharedData::gEvolutionLogFile << std::endl;
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

odNeatWorldObserver::~odNeatWorldObserver()
{
    // nothing to do.
}

void odNeatWorldObserver::reset()
{
    // nothing to do.
}

void odNeatWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >=
            odNeatSharedData::gEvaluationTime ) // switch to next generation.
    {
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

    updateEnvironment();
    
}


void odNeatWorldObserver::updateEnvironment()
{
    // ...
}

void odNeatWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= odNeatSharedData::gEvaluationTime ) // switch to next generation.
    {
        // * monitoring: count number of active agents.
        
        int activeCount = 0;
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            if ( (dynamic_cast<odNeatController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
                activeCount++;
        }
        
        if ( gVerbose )
        {
            //odNeat not generational
            //std::cout << "[gen:" << (gWorld->getIterations()/odNeatSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
        }
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
        gLogManager->flush();
    }
    if(gWorld->getIterations() >= 1)
    {
        //Log "iteration idRobot idGenome energy fitness" every N iterations
        int n = 100;
        if((gWorld->getIterations() % n) == 0)
        {
            for ( int i = 0 ; i != gNumberOfRobots ; i++ )
            {
                odNeatController* c = (dynamic_cast<odNeatController*>(gWorld->getRobot(i)->getController()));

                odNeatSharedData::gEvoLog << gWorld->getIterations()
                                            << " " << c->getWorldModel()->getId()
                                            << " " << c->_genome->genome_id
                                            <<   " " << c->_energy
                                              << " " << c->_fitness
                                              << std::endl;
            }

        }
    }
    if ( gWorld->getIterations() == gMaxIt-1 )
    {
        //It may be possible to force fitness log on all robots
        //before closing (for all robots printAll())
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            //(dynamic_cast<odNeatController*>(gWorld->getRobot(i)->getController()))->printAll();
            //(dynamic_cast<odNeatController*>(gWorld->getRobot(i)->getController()))->save_genome();
            //(dynamic_cast<odNeatController*>(gWorld->getRobot(i)->getController()))->logGenome();
            odNeatController* c = (dynamic_cast<odNeatController*>(gWorld->getRobot(i)->getController()));

            odNeatSharedData::gEvoLog << gWorld->getIterations() +1
                                        << " " << c->getWorldModel()->getId()
                                        << " " << c->_genome->genome_id
                                        <<   " " << c->_energy
                                          << " " << c->_fitness
                                          << std::endl;
        }
        odNeatSharedData::gEvoLog.close();
    }
}

