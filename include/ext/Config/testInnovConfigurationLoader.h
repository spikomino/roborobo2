/*
 * testInnovConfigurationLoader.h
 */

#ifndef TESTINNOVCONFIGURATIONLOADER_H
#define TESTINNOVCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class testInnovConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		testInnovConfigurationLoader();
		~testInnovConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
