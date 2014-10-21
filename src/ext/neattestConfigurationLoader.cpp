#if defined PRJ_NEATTEST || !defined MODULAR

#include "Config/neattestConfigurationLoader.h"

#include "neattest/include/neattestWorldObserver.h"
#include "neattest/include/neattestAgentObserver.h"
#include "neattest/include/neattestController.h"

#include "WorldModels/RobotWorldModel.h"

neattestConfigurationLoader::neattestConfigurationLoader()
{
}

neattestConfigurationLoader::~neattestConfigurationLoader()
{
	//nothing to do
}

WorldObserver* neattestConfigurationLoader::make_WorldObserver(World* wm)
{
	return new neattestWorldObserver(wm);
}

RobotWorldModel* neattestConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* neattestConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new neattestAgentObserver(wm);
}

Controller* neattestConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new neattestController(wm);
}

#endif
