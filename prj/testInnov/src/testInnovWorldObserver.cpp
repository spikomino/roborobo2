/**
 * @author Inaki Fernandez
 *
 */

#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "testInnov/include/testInnovWorldObserver.h"
#include "testInnov/include/testInnovController.h"
#include "World/World.h"
#include <time.h>


testInnovWorldObserver::testInnovWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",&testInnovSharedData::gEvaluationTime,true);
    gProperties.checkAndGetPropertyValue("gGenomeLogFolder",&testInnovSharedData::gGenomeLogFolder,true);
    gProperties.checkAndGetPropertyValue("gEvolutionLogFile",&testInnovSharedData::gEvolutionLogFile,true);

    gProperties.checkAndGetPropertyValue("gControllerType",&testInnovSharedData::gControllerType,true);

    gProperties.checkAndGetPropertyValue("mutate_only_prob",&mutateOnlyProb,true);
    gProperties.checkAndGetPropertyValue("mutate_link_weights_prob",&mutateLinkWeightsProb,true);
    gProperties.checkAndGetPropertyValue("mutate_toggle_enable_prob",&mutateToggleEnableProb,true);
    gProperties.checkAndGetPropertyValue("mutate_gene_reenable_prob",&mutateGeneReenableProb,true);
    gProperties.checkAndGetPropertyValue("mutate_add_node_prob",&mutateAddNodeProb,true);
    gProperties.checkAndGetPropertyValue("mutate_add_link_prob",&mutateAddLinkProb,true);
    gProperties.checkAndGetPropertyValue("mate_only_prob",&mateOnlyProb,true);
    gProperties.checkAndGetPropertyValue("recur_only_prob",&recurOnlyProb,true);
    gProperties.checkAndGetPropertyValue("newstructure_tries",&newStructureTries,true);

    gProperties.checkAndGetPropertyValue("gSelectionMethod",&testInnovSharedData::gSelectionMethod,true);
    gProperties.checkAndGetPropertyValue("gFitness",&testInnovSharedData::gFitness,true);
    gProperties.checkAndGetPropertyValue("gSigmaRef",&testInnovSharedData::gSigmaRef,true);

    //testInnov parameters
    gProperties.checkAndGetPropertyValue("gDefaultInitialEnergy",&testInnovSharedData::gDefaultInitialEnergy,true);
    gProperties.checkAndGetPropertyValue("gEnergyThreshold",&testInnovSharedData::gEnergyThreshold,true);
    gProperties.checkAndGetPropertyValue("gMaturationPeriod",&testInnovSharedData::gMaturationPeriod,true);
    int aux;
    gProperties.checkAndGetPropertyValue("gMaxPopSize",&aux,true);
    if(aux > 0 )
        testInnovSharedData::gMaxPopSize = aux;
    else
    {
        std::cerr << "[ERROR] Wrong gMaxPopSize" << std::endl;
        exit(-1);
    }
    gProperties.checkAndGetPropertyValue("gCompatThreshold",&testInnovSharedData::gCompatThreshold,true);
    gProperties.checkAndGetPropertyValue("gTabuTimeout",&testInnovSharedData::gTabuTimeout,true);
    gProperties.checkAndGetPropertyValue("gTabuThreshold",&testInnovSharedData::gTabuThreshold,true);
    gProperties.checkAndGetPropertyValue("gEnergyItemValue",&testInnovSharedData::gEnergyItemValue,true);
    gProperties.checkAndGetPropertyValue("gFitnessFreq",&testInnovSharedData::gFitnessFreq,true);


    testInnovSharedData::gEvoLog.open(testInnovSharedData::gEvolutionLogFile);//, std::ofstream::out | std::ofstream::app);

    if(!testInnovSharedData::gEvoLog)
    {
        std::cerr << "[ERROR] Could not open log file " << testInnovSharedData::gEvolutionLogFile << std::endl;
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

testInnovWorldObserver::~testInnovWorldObserver()
{
    // nothing to do.
}

void testInnovWorldObserver::reset()
{
    // nothing to do.
}

void testInnovWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >=
            testInnovSharedData::gEvaluationTime ) // switch to next generation.
    {
        // update iterations and generations counters
        _lifeIterationCount = 0;
        _generationCount++;
    }

    updateEnvironment();
    
}


void testInnovWorldObserver::updateEnvironment()
{
    // ...
}

void testInnovWorldObserver::updateMonitoring()
{

    //DEBUG - INNOV NUMBERS
    if ( gWorld->getIterations() == floor(gMaxIt/5) )
    {
        Genome* g1 = (dynamic_cast<testInnovController*>(gWorld->getRobot(1)->getController()))->_genome;
        Genome* g2 = (dynamic_cast<testInnovController*>(gWorld->getRobot(2)->getController()))->_genome;
        Gene* gene1 = g1->genes[g1->genes.size()-1];
        //Gene* gene1 = g1->genes[12];
        Gene* gene2 = g2->genes[g2->genes.size()-1];
        //Gene* gene2 = g2->genes[12];
        char time1[30], time2[30];

        struct tm  *t1 = gmtime(&(gene1->innovation_num.timestamp.tv_sec));

        strftime(time1, 30, "%a-%m-%d-%Y-%H:%M:%S", t1);
        char tempbuf[128];

        sprintf(tempbuf, "gene 1 %s %.9ld ",
                &(time1[0]), gene1->innovation_num.timestamp.tv_nsec);
        std::cout << tempbuf;
        std::cout << std::endl;

        struct tm  *t2 = gmtime(&(gene2->innovation_num.timestamp.tv_sec));

        strftime(time2, 30, "%a-%m-%d-%Y-%H:%M:%S", t2);

        sprintf(tempbuf, "gene 2 %s %.9ld ",
                &(time2[0]), gene2->innovation_num.timestamp.tv_nsec);
        std::cout << tempbuf;
        std::cout << std::endl;
        std::cout << (gene1->innovation_num< gene2->innovation_num) << std::endl;
        std::cout << (gene1->innovation_num> gene2->innovation_num) << std::endl;
        std::cout << (gene1->innovation_num== gene2->innovation_num) << std::endl;
    }

    // * Log at end of each generation
    
    if( _lifeIterationCount >= testInnovSharedData::gEvaluationTime ) // switch to next generation.
    {
        // * monitoring: count number of active agents.
        
        int activeCount = 0;
        for ( int i = 0 ; i != gNumberOfRobots ; i++ )
        {
            if ( (dynamic_cast<testInnovController*>(gWorld->getRobot(i)->getController()))->getWorldModel()->isAlive() == true )
                activeCount++;
        }
        
        if ( gVerbose )
        {
            //testInnov not generational
            //std::cout << "[gen:" << (gWorld->getIterations()/testInnovSharedData::gEvaluationTime) << ";pop:" << activeCount << "]\n";
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
                testInnovController* c = (dynamic_cast<testInnovController*>(gWorld->getRobot(i)->getController()));

                testInnovSharedData::gEvoLog << gWorld->getIterations()
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
            //(dynamic_cast<testInnovController*>(gWorld->getRobot(i)->getController()))->printAll();
            //(dynamic_cast<testInnovController*>(gWorld->getRobot(i)->getController()))->save_genome();
            //(dynamic_cast<testInnovController*>(gWorld->getRobot(i)->getController()))->logGenome();
            testInnovController* c = (dynamic_cast<testInnovController*>(gWorld->getRobot(i)->getController()));

            testInnovSharedData::gEvoLog << gWorld->getIterations() +1
                                         << " " << c->getWorldModel()->getId()
                                         << " " << c->_genome->genome_id
                                         <<   " " << c->_energy
                                           << " " << c->_fitness
                                           << std::endl;
        }
        testInnovSharedData::gEvoLog.close();
    }
}

