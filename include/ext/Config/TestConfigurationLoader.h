/*
 * TestConfigurationLoader.h
 */

#ifndef TESTCONFIGURATIONLOADER_H
#define TESTCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class TestConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		TestConfigurationLoader();
		~TestConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
