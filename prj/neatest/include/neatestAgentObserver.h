/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#ifndef NEATESTAGENTOBSERVER_H
#define NEATESTAGENTOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/AgentObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "neatest/include/neatestSharedData.h"

class neatestAgentObserver : public AgentObserver{


public:
    neatestAgentObserver(RobotWorldModel *wm);
    ~neatestAgentObserver();
    
    void reset();
    void step();  
};

#endif

