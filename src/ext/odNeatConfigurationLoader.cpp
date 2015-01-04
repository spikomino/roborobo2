#if defined PRJ_ODNEAT || !defined MODULAR

#include "Config/odNeatConfigurationLoader.h"

#include "odNeat/include/odNeatWorldObserver.h"
#include "odNeat/include/odNeatAgentObserver.h"
#include "odNeat/include/odNeatController.h"

#include "WorldModels/RobotWorldModel.h"

odNeatConfigurationLoader::odNeatConfigurationLoader()
{
}

odNeatConfigurationLoader::~odNeatConfigurationLoader()
{
	//nothing to do
}

WorldObserver* odNeatConfigurationLoader::make_WorldObserver(World* wm)
{
	return new odNeatWorldObserver(wm);
}

RobotWorldModel* odNeatConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* odNeatConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new odNeatAgentObserver(wm);
}

Controller* odNeatConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new odNeatController(wm);
}

#endif
