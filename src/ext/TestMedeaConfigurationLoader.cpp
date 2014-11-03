#if defined PRJ_TESTMEDEA || !defined MODULAR

#include "Config/TestMedeaConfigurationLoader.h"

#include "TestMedea/include/TestMedeaWorldObserver.h"
#include "TestMedea/include/TestMedeaAgentObserver.h"
#include "TestMedea/include/TestMedeaController.h"

#include "WorldModels/RobotWorldModel.h"

TestMedeaConfigurationLoader::TestMedeaConfigurationLoader()
{
}

TestMedeaConfigurationLoader::~TestMedeaConfigurationLoader()
{
	//nothing to do
}

WorldObserver* TestMedeaConfigurationLoader::make_WorldObserver(World* wm)
{
	return new TestMedeaWorldObserver(wm);
}

RobotWorldModel* TestMedeaConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* TestMedeaConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new TestMedeaAgentObserver(wm);
}

Controller* TestMedeaConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new TestMedeaController(wm);
}

#endif
