#if defined PRJ_NEATEST || !defined MODULAR

#include "Config/neatestConfigurationLoader.h"

#include "neatest/include/neatestWorldObserver.h"
#include "neatest/include/neatestAgentObserver.h"
#include "neatest/include/neatestController.h"

#include "WorldModels/RobotWorldModel.h"

neatestConfigurationLoader::neatestConfigurationLoader()
{
}

neatestConfigurationLoader::~neatestConfigurationLoader()
{
	//nothing to do
}

WorldObserver* neatestConfigurationLoader::make_WorldObserver(World* wm)
{
	return new neatestWorldObserver(wm);
}

RobotWorldModel* neatestConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* neatestConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new neatestAgentObserver(wm);
}

Controller* neatestConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new neatestController(wm);
}

#endif
