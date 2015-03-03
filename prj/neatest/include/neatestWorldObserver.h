/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */





#ifndef NEATESTWORLDOBSERVER_H
#define NEATESTWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "neatest/include/neatestSharedData.h"
#include "neatest/include/neatestSharedData.h"

#include <list>

//class World;

class neatestWorldObserver : public WorldObserver{

private:
    std::list<int> _pickedItems;
    
    void updateEnvironment();
    void updateMonitoring();
    
protected:
    int _generationCount;
    int _lifeIterationCount;
    
public:
    neatestWorldObserver(World *world);
    ~neatestWorldObserver();
    
    void reset();
    void step();
    
    int getLifeIterationCount() { return _lifeIterationCount; }
		
};

#endif
