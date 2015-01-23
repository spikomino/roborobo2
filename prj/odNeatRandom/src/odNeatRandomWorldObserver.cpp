/**
 * @author Inaki Fernandez
 *
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "odNeatRandom/include/odNeatRandomWorldObserver.h"
#include "odNeatRandom/include/odNeatRandomController.h"
#include "World/World.h"


odNeatRandomWorldObserver::odNeatRandomWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",&odNeatRandomSharedData::gEvaluationTime,true);
    gProperties.checkAndGetPropertyValue("gGenomeLogFolder",&odNeatRandomSharedData::gGenomeLogFolder,true);
    gProperties.checkAndGetPropertyValue("gEvolutionLogFile",&odNeatRandomSharedData::gEvolutionLogFile,true);

    gProperties.checkAndGetPropertyValue("gControllerType",&odNeatRandomSharedData::gControllerType,true);

    gProperties.checkAndGetPropertyValue("mutate_only_prob",&mutateOnlyProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("mutate_link_weights_prob",&mutateLinkWeightsProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("mutate_toggle_enable_prob",&mutateToggleEnableProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("mutate_gene_reenable_prob",&mutateGeneReenableProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("mutate_add_node_prob",&mutateAddNodeProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("mutate_add_link_prob",&mutateAddLinkProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("mate_only_prob",&mateOnlyProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("recur_only_prob",&recurOnlyProbOdNRandom,true);
    gProperties.checkAndGetPropertyValue("newstructure_tries",&newStructureTriesOdNRandom,true);

    gProperties.checkAndGetPropertyValue("gSelectionMethod",&odNeatRandomSharedData::gSelectionMethod,true);
    gProperties.checkAndGetPropertyValue("gFitness",&odNeatRandomSharedData::gFitness,true);
    gProperties.checkAndGetPropertyValue("gSigmaRef",&odNeatRandomSharedData::gSigmaRef,true);

    //odNeatRandom parameters
    gProperties.checkAndGetPropertyValue("gDefaultInitialEnergy",&odNeatRandomSharedData::gDefaultInitialEnergy,true);
    gProperties.checkAndGetPropertyValue("gEnergyThreshold",&odNeatRandomSharedData::gEnergyThreshold,true);
    gProperties.checkAndGetPropertyValue("gMaturationPeriod",&odNeatRandomSharedData::gMaturationPeriod,true);
    int aux;
    gProperties.checkAndGetPropertyValue("gMaxPopSize",&aux,true);
    if(aux > 0 )
        odNeatRandomSharedData::gMaxPopSize = aux;//abs(aux);
    else
    {
        std::cerr << "[ERROR] Wrong gMaxPopSize" << std::endl;
        exit(-1);
    }
    gProperties.checkAndGetPropertyValue("gCompatThreshold",&odNeatRandomSharedData::gCompatThreshold,true);
    gProperties.checkAndGetPropertyValue("gTabuTimeout",&odNeatRandomSharedData::gTabuTimeout,true);
    gProperties.checkAndGetPropertyValue("gTabuThreshold",&odNeatRandomSharedData::gTabuThreshold,true);
    gProperties.checkAndGetPropertyValue("gEnergyItemValue",&odNeatRandomSharedData::gEnergyItemValue,true);
    gProperties.checkAndGetPropertyValue("gFitnessFreq",&odNeatRandomSharedData::gFitnessFreq,true);


    odNeatRandomSharedData::gEvoLog.open(odNeatRandomSharedData::gEvolutionLogFile);//, std::ofstream::out | std::ofstream::app);

    if(!odNeatRandomSharedData::gEvoLog)
    {
        std::cerr << "[ERROR] Could not open log file " << odNeatRandomSharedData::gEvolutionLogFile << std::endl;
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

}

odNeatRandomWorldObserver::~odNeatRandomWorldObserver()
{
    // nothing to do.
}

void odNeatRandomWorldObserver::reset()
{
    // nothing to do.
}

void odNeatRandomWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >=
            odNeatRandomSharedData::gEvaluationTime ) // switch to next generation.
    {
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

    updateEnvironment();
    
}


void odNeatRandomWorldObserver::updateEnvironment()
{
    // ...
}

void odNeatRandomWorldObserver::updateMonitoring()
{
    // * Log at end of each generation
    
    if( _lifeIterationCount >= odNeatRandomSharedData::gEvaluationTime ) // switch to next generation.
    {
        // * monitoring: count number of active agents.
        
        int activeCount = 0;
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            if ( (dynamic_cast<odNeatRandomController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
                activeCount++;
        }
        
        if ( gVerbose )
        {
            //odNeatRandom not generational
            //std::cout << "[gen:" << (gWorld->getIterations()/odNeatRandomSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
        }
        
        // Logging
        std::string s = std::string("") + "{" + std::to_string(gWorld->getIterations()) + "}[all] [pop_alive:" + std::to_string(activeCount) + "]\n";
        gLogManager->write(s);
        gLogManager->flush();
    }

    /*if(gWorld->getIterations() == 0)
    {
        //For counting concurrent gene counters
        //Initialize WorldObserver::usedGeneCounters (set<int>)
        int lastInnov = dynamic_cast<odNeatRandomController*>(gWorld->getRobot(0)->getController()) -> _innovNumber;
        for(int i = 0; i < lastInnov ; i++)
        {
            usedGeneCounters.insert(i);
        }
    }*/

    if(gWorld->getIterations() >= 1)
    {//Log "iteration idRobot idGenome energy fitness" every N iterations
        int n = 100;
        if((gWorld->getIterations() % n) == 0)
        {
            for ( int i = 0 ; i != gNumberOfRobots ; i++ )
            {
                odNeatRandomController* c = (dynamic_cast<odNeatRandomController*>(gWorld->getRobot(i)->getController()));

                odNeatRandomSharedData::gEvoLog << gWorld->getIterations()
                                            << " " << c->getWorldModel()->getId()
                                            << " " << c->_genome->genome_id
                                            <<   " " << c->_energy
                                              << " " << c->_fitness
                                              << std::endl;
            }

        }

        //Measure concurrent events
        /*odNeatRandomController* c;
        int geneCounter = -1;
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            c =dynamic_cast<odNeatRandomController*>(gWorld->getRobot(i)->getController());
            std::vector<int>::iterator it = c->_newGenes.begin();
            for(; it !=c->_newGenes.end(); it++)
            {
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
        }*/
    }

    if ( gWorld->getIterations() == gMaxIt-1 )
    {
        //It may be possible to force fitness log on all robots
        //before closing (for all robots printAll())
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            // (dynamic_cast<odNeatRandomController*>(gWorld->getRobot(i)->getController()))->printAll();
             (dynamic_cast<odNeatRandomController*>(gWorld->getRobot(i)->getController()))->save_genome();
            // (dynamic_cast<odNeatRandomController*>(gWorld->getRobot(i)->getController()))->logGenome();

            odNeatRandomController* c = (dynamic_cast<odNeatRandomController*>(gWorld->getRobot(i)->getController()));

            odNeatRandomSharedData::gEvoLog << gWorld->getIterations() +1
                                        << " " << c->getWorldModel()->getId()
                                        << " " << c->_genome->genome_id
                                        <<   " " << c->_energy
                                          << " " << c->_fitness
                                          << std::endl;
        }
        odNeatRandomSharedData::gEvoLog.close();
        //Out number of concurrent gene counters
       // std::cout << "Concurrent events= " << _countGeneClockCollisions << std::endl;
    }
}
void odNeatRandomWorldObserver::incrementCollisions()
{
    _countGeneClockCollisions++;
}
