#if defined PRJ_REDOUANE || !defined MODULAR

#include "Config/RedouaneConfigurationLoader.h"

#include "Redouane/include/RedouaneWorldObserver.h"
#include "Redouane/include/RedouaneAgentObserver.h"
#include "Redouane/include/RedouaneController.h"

#include "WorldModels/RobotWorldModel.h"

RedouaneConfigurationLoader::RedouaneConfigurationLoader()
{
}

RedouaneConfigurationLoader::~RedouaneConfigurationLoader()
{
	//nothing to do
}

WorldObserver* RedouaneConfigurationLoader::make_WorldObserver(World* wm)
{
	return new RedouaneWorldObserver(wm);
}

RobotWorldModel* RedouaneConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* RedouaneConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new RedouaneAgentObserver(wm);
}

Controller* RedouaneConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new RedouaneController(wm);
}

#endif
