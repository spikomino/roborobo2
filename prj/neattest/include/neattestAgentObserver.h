/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef NEATTESTAGENTOBSERVER_H
#define NEATTESTAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "neattest/include/neattestSharedData.h"

#include <iomanip>

class neattestAgentObserver : public AgentObserver
{
	public:
		neattestAgentObserver(RobotWorldModel *wm);
		~neattestAgentObserver();

		void reset();
		void step();

};

#endif

