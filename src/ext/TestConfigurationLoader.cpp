#if defined PRJ_TEST || !defined MODULAR

#include "Config/TestConfigurationLoader.h"

#include "Test/include/TestWorldObserver.h"
#include "Test/include/TestAgentObserver.h"
#include "Test/include/TestController.h"

#include "WorldModels/RobotWorldModel.h"

TestConfigurationLoader::TestConfigurationLoader()
{
}

TestConfigurationLoader::~TestConfigurationLoader()
{
	//nothing to do
}

WorldObserver* TestConfigurationLoader::make_WorldObserver(World* wm)
{
	return new TestWorldObserver(wm);
}

RobotWorldModel* TestConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* TestConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new TestAgentObserver(wm);
}

Controller* TestConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new TestController(wm);
}

#endif
