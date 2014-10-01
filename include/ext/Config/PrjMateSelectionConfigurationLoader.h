/*
 * MedeaConfigurationLoader.h
 */

#ifndef PRJMATESELECTIONCONFIGURATIONLOADER_H
#define PRJMATESELECTIONCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class PrjMateSelectionConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		PrjMateSelectionConfigurationLoader();
		~PrjMateSelectionConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
