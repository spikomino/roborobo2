/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef TESTAGENTOBSERVER_H
#define TESTAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "Test/include/TestSharedData.h"

#include <iomanip>

class TestAgentObserver : public AgentObserver
{
	public:
		TestAgentObserver(RobotWorldModel *wm);
		~TestAgentObserver();

		void reset();
		void step();

};

#endif

