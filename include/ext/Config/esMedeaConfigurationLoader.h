/*
 * esMedeaConfigurationLoader.h
 */

#ifndef ESMEDEACONFIGURATIONLOADER_H
#define	ESMEDEACONFIGURATIONLOADER_H

#include "Config/ConfigurationLoader.h"

class esMedeaConfigurationLoader : public ConfigurationLoader
{
	public:
		esMedeaConfigurationLoader();
		~esMedeaConfigurationLoader();

		WorldObserver *make_WorldObserver(World* wm) ;
		RobotWorldModel *make_RobotWorldModel();
		AgentObserver *make_AgentObserver(RobotWorldModel* wm) ;
		Controller *make_Controller(RobotWorldModel* wm) ;
};

#endif
