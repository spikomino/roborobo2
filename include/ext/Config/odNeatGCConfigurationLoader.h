/*
 * odNeatGCConfigurationLoader.h
 */

#ifndef ODNEATGCCONFIGURATIONLOADER_H
#define ODNEATGCCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class odNeatGCConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		odNeatGCConfigurationLoader();
		~odNeatGCConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
