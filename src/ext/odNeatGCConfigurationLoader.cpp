#if defined PRJ_ODNEATGC || !defined MODULAR

#include "Config/odNeatGCConfigurationLoader.h"

#include "odNeatGC/include/odNeatGCWorldObserver.h"
#include "odNeatGC/include/odNeatGCAgentObserver.h"
#include "odNeatGC/include/odNeatGCController.h"

#include "WorldModels/RobotWorldModel.h"

odNeatGCConfigurationLoader::odNeatGCConfigurationLoader()
{
}

odNeatGCConfigurationLoader::~odNeatGCConfigurationLoader()
{
	//nothing to do
}

WorldObserver* odNeatGCConfigurationLoader::make_WorldObserver(World* wm)
{
	return new odNeatGCWorldObserver(wm);
}

RobotWorldModel* odNeatGCConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* odNeatGCConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new odNeatGCAgentObserver(wm);
}

Controller* odNeatGCConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new odNeatGCController(wm);
}

#endif
