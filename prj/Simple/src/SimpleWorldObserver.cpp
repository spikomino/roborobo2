#include "Observers/AgentObserver.h"
#include "Observers/WorldObserver.h"
#include "Simple/include/SimpleWorldObserver.h"
#include "Simple/include/SimpleController.h"
#include "World/World.h"


SimpleWorldObserver::SimpleWorldObserver( World* world ) : WorldObserver( world )
{
    _world = world;


    gProperties.checkAndGetPropertyValue("gEvaluationTime",&SimpleSharedData::gEvalTime,true);
    gProperties.checkAndGetPropertyValue("gEvolutionLogFile",&SimpleSharedData::gEvolutionLogFile,true);
    gProperties.checkAndGetPropertyValue("gSigma",&SimpleSharedData::gSigma,true);

    SimpleSharedData::gEvoLog.open(SimpleSharedData::gEvolutionLogFile, std::ofstream::out | std::ofstream::app);
    if(!SimpleSharedData::gEvoLog)
      {
	std::cerr << "[ERROR] Could not open log file " << SimpleSharedData::gEvolutionLogFile << std::endl;
	exit(-1);
      }
    
    _lifeIterationCount = -1;
    _generationCount = -1;
    
}

SimpleWorldObserver::~SimpleWorldObserver()
{
}

void SimpleWorldObserver::reset()
{
}

void SimpleWorldObserver::step()
{
    _lifeIterationCount++;
    
    updateMonitoring();

    if( _lifeIterationCount >= 	SimpleSharedData::gEvalTime ) 
      {
	_lifeIterationCount = 0;
	_generationCount++;
      }
    
    updateEnvironment();
    
}


void SimpleWorldObserver::updateEnvironment()
{
}

void SimpleWorldObserver::updateMonitoring()
{
    if ( gWorld->getIterations() == gMaxIt-1 )
      {
	SimpleSharedData::gEvoLog.close();
      } 
}

