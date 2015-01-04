/*
 * odNeatConfigurationLoader.h
 */

#ifndef ODNEATCONFIGURATIONLOADER_H
#define ODNEATCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class odNeatConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		odNeatConfigurationLoader();
		~odNeatConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
