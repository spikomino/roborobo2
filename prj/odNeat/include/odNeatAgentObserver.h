/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef ODNEATAGENTOBSERVER_H
#define ODNEATAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "odNeat/include/odNeatSharedData.h"

#include <iomanip>

class odNeatAgentObserver : public AgentObserver
{
	public:
		odNeatAgentObserver(RobotWorldModel *wm);
		~odNeatAgentObserver();

		void reset();
		void step();

};

#endif

