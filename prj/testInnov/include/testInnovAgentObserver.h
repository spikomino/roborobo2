/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef TESTINNOVAGENTOBSERVER_H
#define TESTINNOVAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "testInnov/include/testInnovSharedData.h"

#include <iomanip>

class testInnovAgentObserver : public AgentObserver
{
	public:
		testInnovAgentObserver(RobotWorldModel *wm);
		~testInnovAgentObserver();

		void reset();
		void step();

};

#endif

