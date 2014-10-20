/*
 * neattestConfigurationLoader.h
 */

#ifndef NEATTESTCONFIGURATIONLOADER_H
#define NEATTESTCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class neattestConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		neattestConfigurationLoader();
		~neattestConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
