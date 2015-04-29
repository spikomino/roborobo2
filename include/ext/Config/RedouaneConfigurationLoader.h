/*
 * RedouaneConfigurationLoader.h
 */

#ifndef REDOUANECONFIGURATIONLOADER_H
#define REDOUANECONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class RedouaneConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		RedouaneConfigurationLoader();
		~RedouaneConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
