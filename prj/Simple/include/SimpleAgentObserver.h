#ifndef SIMPLEAGENTOBSERVER_H
#define SIMPLEAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "Simple/include/SimpleSharedData.h"

#include <iomanip>

class SimpleAgentObserver : public AgentObserver
{
	public:
		SimpleAgentObserver(RobotWorldModel *wm);
		~SimpleAgentObserver();

		void reset();
		void step();

};

#endif

