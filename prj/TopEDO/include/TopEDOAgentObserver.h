/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef TOPEDOAGENTOBSERVER_H
#define TOPEDOAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "TopEDO/include/TopEDOSharedData.h"

#include <iomanip>

class TopEDOAgentObserver : public AgentObserver
{
	public:
		TopEDOAgentObserver(RobotWorldModel *wm);
		~TopEDOAgentObserver();

		void reset();
		void step();

};

#endif

