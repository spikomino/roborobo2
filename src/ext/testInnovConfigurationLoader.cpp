#if defined PRJ_TESTINNOV || !defined MODULAR

#include "Config/testInnovConfigurationLoader.h"

#include "testInnov/include/testInnovWorldObserver.h"
#include "testInnov/include/testInnovAgentObserver.h"
#include "testInnov/include/testInnovController.h"

#include "WorldModels/RobotWorldModel.h"

testInnovConfigurationLoader::testInnovConfigurationLoader()
{
}

testInnovConfigurationLoader::~testInnovConfigurationLoader()
{
	//nothing to do
}

WorldObserver* testInnovConfigurationLoader::make_WorldObserver(World* wm)
{
	return new testInnovWorldObserver(wm);
}

RobotWorldModel* testInnovConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* testInnovConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new testInnovAgentObserver(wm);
}

Controller* testInnovConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new testInnovController(wm);
}

#endif
