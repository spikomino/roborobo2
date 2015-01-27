/**
 * @author Inaki Fernandez
 *
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "odNeatGC/include/odNeatGCWorldObserver.h"
#include "odNeatGC/include/odNeatGCController.h"
#include "World/World.h"


odNeatGCWorldObserver::odNeatGCWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",&odNeatGCSharedData::gEvaluationTime,true);
    gProperties.checkAndGetPropertyValue("gGenomeLogFolder",&odNeatGCSharedData::gGenomeLogFolder,true);
    gProperties.checkAndGetPropertyValue("gEvolutionLogFile",&odNeatGCSharedData::gEvolutionLogFile,true);

    gProperties.checkAndGetPropertyValue("gControllerType",&odNeatGCSharedData::gControllerType,true);

    gProperties.checkAndGetPropertyValue("mutate_only_prob",&mutateOnlyProbGc,true);
    gProperties.checkAndGetPropertyValue("mutate_link_weights_prob",&mutateLinkWeightsProbGc,true);
    gProperties.checkAndGetPropertyValue("mutate_toggle_enable_prob",&mutateToggleEnableProbGc,true);
    gProperties.checkAndGetPropertyValue("mutate_gene_reenable_prob",&mutateGeneReenableProbGc,true);
    gProperties.checkAndGetPropertyValue("mutate_add_node_prob",&mutateAddNodeProbGc,true);
    gProperties.checkAndGetPropertyValue("mutate_add_link_prob",&mutateAddLinkProbGc,true);
    gProperties.checkAndGetPropertyValue("mate_only_prob",&mateOnlyProbGc,true);
    gProperties.checkAndGetPropertyValue("recur_only_prob",&recurOnlyProbGc,true);
    gProperties.checkAndGetPropertyValue("newstructure_tries",&newStructureTriesGc,true);

    gProperties.checkAndGetPropertyValue("gSelectionMethod",&odNeatGCSharedData::gSelectionMethod,true);
    gProperties.checkAndGetPropertyValue("gFitness",&odNeatGCSharedData::gFitness,true);
    gProperties.checkAndGetPropertyValue("gSigmaRef",&odNeatGCSharedData::gSigmaRef,true);

    //odNeatGC parameters
    gProperties.checkAndGetPropertyValue("gDefaultInitialEnergy",&odNeatGCSharedData::gDefaultInitialEnergy,true);
    gProperties.checkAndGetPropertyValue("gEnergyThreshold",&odNeatGCSharedData::gEnergyThreshold,true);
    gProperties.checkAndGetPropertyValue("gMaturationPeriod",&odNeatGCSharedData::gMaturationPeriod,true);
    int aux;
    gProperties.checkAndGetPropertyValue("gMaxPopSize",&aux,true);
    if(aux > 0 )
        odNeatGCSharedData::gMaxPopSize = aux;//abs(aux);
    else
    {
        std::cerr << "[ERROR] Wrong gMaxPopSize" << std::endl;
        exit(-1);
    }
    gProperties.checkAndGetPropertyValue("gCompatThreshold",&odNeatGCSharedData::gCompatThreshold,true);
    gProperties.checkAndGetPropertyValue("gTabuTimeout",&odNeatGCSharedData::gTabuTimeout,true);
    gProperties.checkAndGetPropertyValue("gTabuThreshold",&odNeatGCSharedData::gTabuThreshold,true);
    gProperties.checkAndGetPropertyValue("gEnergyItemValue",&odNeatGCSharedData::gEnergyItemValue,true);
    gProperties.checkAndGetPropertyValue("gFitnessFreq",&odNeatGCSharedData::gFitnessFreq,true);
    gProperties.checkAndGetPropertyValue("gUpdateGC",&odNeatGCSharedData::gUpdateGC,false);


    odNeatGCSharedData::gEvoLog.open(odNeatGCSharedData::gEvolutionLogFile);//, std::ofstream::out | std::ofstream::app);

    if(!odNeatGCSharedData::gEvoLog)
    {
        std::cerr << "[ERROR] Could not open log file " << odNeatGCSharedData::gEvolutionLogFile << std::endl;
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

    _countGeneClockCollisions = 0;
    _countGenes = 0;

}

odNeatGCWorldObserver::~odNeatGCWorldObserver()
{
    // nothing to do.
}

void odNeatGCWorldObserver::reset()
{
    // nothing to do.
}

void odNeatGCWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >=
            odNeatGCSharedData::gEvaluationTime ) // switch to next generation.
    {
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

    updateEnvironment();
    
}


void odNeatGCWorldObserver::updateEnvironment()
{
    // ...
}

void odNeatGCWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= odNeatGCSharedData::gEvaluationTime ) // switch to next generation.
    {
        // * monitoring: count number of active agents.
        
        int activeCount = 0;
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            if ( (dynamic_cast<odNeatGCController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
                activeCount++;
        }
        
        if ( gVerbose )
        {
            //odNeatGC not generational
            //std::cout << "[gen:" << (gWorld->getIterations()/odNeatGCSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
        }
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
        gLogManager->flush();
    }

    if(gWorld->getIterations() == 0)
    {
        //For counting concurrent gene counters
        //Initialize WorldObserver::usedGeneCounters (set<int>)
        int lastInnov = dynamic_cast<odNeatGCController*>(gWorld->getRobot(0)->getController()) -> _innovNumber;
        for(int i = 0; i < lastInnov ; i++)
        {
            usedGeneCounters.insert(i);
        }
    }

    if(gWorld->getIterations() >= 1)
    {//Log "iteration idRobot idGenome energy fitness popsize nbspecies nbnodes nblinks" every N iterations
        int n = 100;
        if((gWorld->getIterations() % n) == 0)
        {
            for ( int i = 0 ; i != gNumberOfRobots ; i++ )
            {
                odNeatGCController* c = (dynamic_cast<odNeatGCController*>(gWorld->getRobot(i)->getController()));

                odNeatGCSharedData::gEvoLog << gWorld->getIterations()
                                            << " " << c->getWorldModel()->getId()
                                            << " " << c->_genome->genome_id
                                            <<   " " << c->_energy
                                              << " " << c->_fitness
                                              << " " << c->population.size()
                                              << " " << c->species.size()
                                              << " " << c->_genome->nodes.size()
                                              << " " << c->_genome->genes.size() //Including disabled links
                                              << std::endl;
            }

        }

        //Measure concurrent events
        odNeatGCController* c;
        int geneCounter = -1;
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            c =dynamic_cast<odNeatGCController*>(gWorld->getRobot(i)->getController());
            std::vector<int>::iterator it = c->_newGenes.begin();
            for(; it !=c->_newGenes.end(); it++)
            {
                incrementCounterGenes();
                geneCounter = *it;
                if(usedGeneCounters.find(geneCounter) != usedGeneCounters.end())
                {//If it exists already, increment gene counter collision (concurrent event) counter
                    incrementCollisions();
                }
                else
                {
                    //Insert it otherwise
                    usedGeneCounters.insert(geneCounter + i);
                }
            }
            c->_newGenes.clear();
        }
    }

    if ( gWorld->getIterations() == gMaxIt-1 )
    {
        //It may be possible to force fitness log on all robots
        //before closing (for all robots printAll())
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            // (dynamic_cast<odNeatGCController*>(gWorld->getRobot(i)->getController()))->printAll();
            // (dynamic_cast<odNeatGCController*>(gWorld->getRobot(i)->getController()))->save_genome();
            // (dynamic_cast<odNeatGCController*>(gWorld->getRobot(i)->getController()))->logGenome();

            odNeatGCController* c = (dynamic_cast<odNeatGCController*>(gWorld->getRobot(i)->getController()));

            odNeatGCSharedData::gEvoLog << gWorld->getIterations() +1
                                        << " " << c->getWorldModel()->getId()
                                        << " " << c->_genome->genome_id
                                        <<   " " << c->_energy
                                          << " " << c->_fitness
                                          << " " << c->population.size()
                                          << " " << c->species.size()
                                          << " " << c->_genome->nodes.size()
                                          << " " << c->_genome->genes.size() //Including disabled links
                                          << std::endl;
        }
        odNeatGCSharedData::gEvoLog.close();
        if(odNeatGCSharedData::gUpdateGC)
            std::cout << "Updating GC" << std::endl;
        else
            std::cout << "No update GC" << std::endl;
        //Out number of concurrent gene counters
        std::cout << "Concurrent events= " << _countGeneClockCollisions << " (out of " << _countGenes
                  << " ). Ratio: " << (double)(1.0 * _countGeneClockCollisions) /_countGenes <<   std::endl;

    }
}
void odNeatGCWorldObserver::incrementCollisions()
{
    _countGeneClockCollisions++;
}
void odNeatGCWorldObserver::incrementCounterGenes()
{
    _countGenes++;
}
