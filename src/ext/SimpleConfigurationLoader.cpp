#if defined PRJ_SIMPLE || !defined MODULAR

#include "Config/SimpleConfigurationLoader.h"

#include "Simple/include/SimpleWorldObserver.h"
#include "Simple/include/SimpleAgentObserver.h"
#include "Simple/include/SimpleController.h"

#include "WorldModels/RobotWorldModel.h"

SimpleConfigurationLoader::SimpleConfigurationLoader()
{
}

SimpleConfigurationLoader::~SimpleConfigurationLoader()
{
	//nothing to do
}

WorldObserver* SimpleConfigurationLoader::make_WorldObserver(World* wm)
{
	return new SimpleWorldObserver(wm);
}

RobotWorldModel* SimpleConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* SimpleConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new SimpleAgentObserver(wm);
}

Controller* SimpleConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new SimpleController(wm);
}

#endif
