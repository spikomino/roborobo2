/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef REDOUANEAGENTOBSERVER_H
#define REDOUANEAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "Redouane/include/RedouaneSharedData.h"

#include <iomanip>

class RedouaneAgentObserver : public AgentObserver
{
	public:
		RedouaneAgentObserver(RobotWorldModel *wm);
		~RedouaneAgentObserver();

		void reset();
		void step();

};

#endif

