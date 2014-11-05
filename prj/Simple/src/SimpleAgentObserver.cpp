#include "Simple/include/SimpleAgentObserver.h"
#include "World/World.h"
#include "Utilities/Misc.h"
#include "RoboroboMain/roborobo.h"
#include "Simple/include/SimpleController.h"
#include <cmath>
#include "Simple/include/SimpleWorldObserver.h"
#include <string>


SimpleAgentObserver::SimpleAgentObserver( RobotWorldModel *wm )
{
    _wm = (RobotWorldModel*)wm;

}
SimpleAgentObserver::~SimpleAgentObserver()
{
}
void SimpleAgentObserver::reset()
{
}

void SimpleAgentObserver::step()
{
    for( int i = 0 ; i < _wm->_cameraSensorsNb; i++)
    {
        int targetIndex = _wm->getObjectIdFromCameraSensor(i);
        
        if ( PhysicalObject::isInstanceOf(targetIndex) ) 
        {
            targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
            gPhysicalObjects[targetIndex]->isTouched(_wm->getId());
        }
    }
    
    int targetIndex = _wm->getGroundSensorValue();
    if ( PhysicalObject::isInstanceOf(targetIndex) ) 
    {
        targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
        gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
    }
}
