/*
 * TopEDOConfigurationLoader.h
 */

#ifndef TOPEDOCONFIGURATIONLOADER_H
#define TOPEDOCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class TopEDOConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		TopEDOConfigurationLoader();
		~TopEDOConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
