/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 */
 
#ifndef ESMEDEAAGENTOBSERVER_H
#define ESMEDEAAGENTOBSERVER_H 

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "WorldModels/RobotWorldModel.h"
#include "Observers/AgentObserver.h"

class esMedeaAgentObserver : public AgentObserver
{
	public:
		esMedeaAgentObserver( );
		esMedeaAgentObserver( RobotWorldModel *__wm );
		~esMedeaAgentObserver();
				
		void reset();
		void step();
		
};


#endif

