#if defined PRJ_PRJMATESELECTION || !defined MODULAR

#include "Config/PrjMateSelectionConfigurationLoader.h"

#include "PrjMateSelection/include/PrjMateSelectionWorldObserver.h"
#include "PrjMateSelection/include/PrjMateSelectionAgentObserver.h"
#include "PrjMateSelection/include/PrjMateSelectionController.h"

#include "WorldModels/RobotWorldModel.h"

PrjMateSelectionConfigurationLoader::PrjMateSelectionConfigurationLoader()
{
}

PrjMateSelectionConfigurationLoader::~PrjMateSelectionConfigurationLoader()
{
	//nothing to do
}

WorldObserver* PrjMateSelectionConfigurationLoader::make_WorldObserver(World* wm)
{
	return new PrjMateSelectionWorldObserver(wm);
}

RobotWorldModel* PrjMateSelectionConfigurationLoader::make_RobotWorldModel()
{
	return new RobotWorldModel();
}

AgentObserver* PrjMateSelectionConfigurationLoader::make_AgentObserver(RobotWorldModel* wm)
{
	return new PrjMateSelectionAgentObserver(wm);
}

Controller* PrjMateSelectionConfigurationLoader::make_Controller(RobotWorldModel* wm)
{
	return new PrjMateSelectionController(wm);
}

#endif
