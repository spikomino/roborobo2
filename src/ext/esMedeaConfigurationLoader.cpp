#if defined PRJ_ESMEDEA || !defined MODULAR

#include "Config/esMedeaConfigurationLoader.h"

#include "esMedea/include/esMedeaWorldObserver.h"
#include "esMedea/include/esMedeaAgentObserver.h"
#include "esMedea/include/esMedeaController.h"

#include "WorldModels/RobotWorldModel.h"


esMedeaConfigurationLoader::esMedeaConfigurationLoader()
{
	// create the single instance of Agent-World Interface.
}

esMedeaConfigurationLoader::~esMedeaConfigurationLoader()
{
	//nothing to do
}

WorldObserver* esMedeaConfigurationLoader::make_WorldObserver(World* wm)
{
	return new esMedeaWorldObserver(wm);
}

RobotWorldModel* esMedeaConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* esMedeaConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new esMedeaAgentObserver(wm);
}

Controller* esMedeaConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new esMedeaController(wm);
}


#endif
