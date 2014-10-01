/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef PRJMATESELECTIONAGENTOBSERVER_H
#define PRJMATESELECTIONAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "PrjMateSelection/include/PrjMateSelectionSharedData.h"

#include <iomanip>

class PrjMateSelectionAgentObserver : public AgentObserver
{
	public:
		PrjMateSelectionAgentObserver(RobotWorldModel *wm);
		~PrjMateSelectionAgentObserver();

		void reset();
		void step();

};

#endif

