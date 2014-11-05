/*
 * SimpleConfigurationLoader.h
 */

#ifndef SIMPLECONFIGURATIONLOADER_H
#define SIMPLECONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class SimpleConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		SimpleConfigurationLoader();
		~SimpleConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
