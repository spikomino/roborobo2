/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef ODNEATRANDOMAGENTOBSERVER_H
#define ODNEATRANDOMAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "odNeatRandom/include/odNeatRandomSharedData.h"

#include <iomanip>

class odNeatRandomAgentObserver : public AgentObserver
{
	public:
		odNeatRandomAgentObserver(RobotWorldModel *wm);
		~odNeatRandomAgentObserver();

		void reset();
		void step();

};

#endif

