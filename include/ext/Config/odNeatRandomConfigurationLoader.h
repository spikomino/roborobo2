/*
 * odNeatRandomConfigurationLoader.h
 */

#ifndef ODNEATRANDOMCONFIGURATIONLOADER_H
#define ODNEATRANDOMCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class odNeatRandomConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		odNeatRandomConfigurationLoader();
		~odNeatRandomConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
