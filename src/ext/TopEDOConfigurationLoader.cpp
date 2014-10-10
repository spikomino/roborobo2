#if defined PRJ_TOPEDO || !defined MODULAR

#include "Config/TopEDOConfigurationLoader.h"

#include "TopEDO/include/TopEDOWorldObserver.h"
#include "TopEDO/include/TopEDOAgentObserver.h"
#include "TopEDO/include/TopEDOController.h"

#include "WorldModels/RobotWorldModel.h"

TopEDOConfigurationLoader::TopEDOConfigurationLoader()
{
}

TopEDOConfigurationLoader::~TopEDOConfigurationLoader()
{
	//nothing to do
}

WorldObserver* TopEDOConfigurationLoader::make_WorldObserver(World* wm)
{
	return new TopEDOWorldObserver(wm);
}

RobotWorldModel* TopEDOConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* TopEDOConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new TopEDOAgentObserver(wm);
}

Controller* TopEDOConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new TopEDOController(wm);
}

#endif
