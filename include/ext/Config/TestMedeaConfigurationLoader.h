/*
 * MedeaConfigurationLoader.h
 */

#ifndef TESTMEDEACONFIGURATIONLOADER_H
#define TESTMEDEACONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class TestMedeaConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		TestMedeaConfigurationLoader();
		~TestMedeaConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
