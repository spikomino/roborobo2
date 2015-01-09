/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef ODNEATGCAGENTOBSERVER_H
#define ODNEATGCAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "odNeatGC/include/odNeatGCSharedData.h"

#include <iomanip>

class odNeatGCAgentObserver : public AgentObserver
{
	public:
		odNeatGCAgentObserver(RobotWorldModel *wm);
		~odNeatGCAgentObserver();

		void reset();
		void step();

};

#endif

