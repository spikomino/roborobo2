/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef TESTMEDEAAGENTOBSERVER_H
#define TESTMEDEAAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "TestMedea/include/TestMedeaSharedData.h"

#include <iomanip>

class TestMedeaAgentObserver : public AgentObserver
{
	public:
		TestMedeaAgentObserver(RobotWorldModel *wm);
		~TestMedeaAgentObserver();

		void reset();
		void step();

};

#endif

