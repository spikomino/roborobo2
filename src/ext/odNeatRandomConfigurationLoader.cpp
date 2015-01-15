#if defined PRJ_ODNEATRANDOM || !defined MODULAR

#include "Config/odNeatRandomConfigurationLoader.h"

#include "odNeatRandom/include/odNeatRandomWorldObserver.h"
#include "odNeatRandom/include/odNeatRandomAgentObserver.h"
#include "odNeatRandom/include/odNeatRandomController.h"

#include "WorldModels/RobotWorldModel.h"

odNeatRandomConfigurationLoader::odNeatRandomConfigurationLoader()
{
}

odNeatRandomConfigurationLoader::~odNeatRandomConfigurationLoader()
{
	//nothing to do
}

WorldObserver* odNeatRandomConfigurationLoader::make_WorldObserver(World* wm)
{
	return new odNeatRandomWorldObserver(wm);
}

RobotWorldModel* odNeatRandomConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* odNeatRandomConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new odNeatRandomAgentObserver(wm);
}

Controller* odNeatRandomConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new odNeatRandomController(wm);
}

#endif
