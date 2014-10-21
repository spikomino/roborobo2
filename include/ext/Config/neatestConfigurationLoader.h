/*
 * neatestConfigurationLoader.h
 */

#ifndef NEATESTCONFIGURATIONLOADER_H
#define NEATESTCONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"


class neatestConfigurationLoader : public ConfigurationLoader
{
	private:

	public:
		neatestConfigurationLoader();
		~neatestConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};



#endif
