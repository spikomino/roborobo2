/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#include "Redouane/include/RedouaneAgentObserver.h"
#include "World/World.h"
#include "Utilities/Misc.h"
#include "RoboroboMain/roborobo.h"
#include "Redouane/include/RedouaneController.h"
#include <cmath>
#include "Redouane/include/RedouaneWorldObserver.h"
#include <string>


RedouaneAgentObserver::RedouaneAgentObserver( RobotWorldModel *wm )
{
    _wm = (RobotWorldModel*)wm;

}

RedouaneAgentObserver::~RedouaneAgentObserver()
{
	// nothing to do.
}

void RedouaneAgentObserver::reset()
{
	// nothing to do.
}

void RedouaneAgentObserver::step()
{
    // * update energy if needed
    if ( gEnergyLevel && _wm->isAlive() )
        {
            _wm->substractEnergy(1);
            assert( _wm->getEnergyLevel() >= 0 );
            if ( _wm->getEnergyLevel() == 0 )
                _wm->setAlive(false);
        }

    // * send callback messages to objects touched or walked upon.
    
    // through distance sensors
    for( int i = 0 ; i < _wm->_cameraSensorsNb; i++)
    {
        int targetIndex = _wm->getObjectIdFromCameraSensor(i);
        
        if ( PhysicalObject::isInstanceOf(targetIndex) )   // sensor ray bumped into a physical object
        {
            targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
            //std::cout << "[DEBUG] Robot #" << _wm->getId() << " touched " << targetIndex << "\n";
            gPhysicalObjects[targetIndex]->isTouched(_wm->getId());
        }
    }
    
    // through floor sensor
    int targetIndex = _wm->getGroundSensorValue();
    if ( PhysicalObject::isInstanceOf(targetIndex) ) // ground sensor is upon a physical object (OR: on a place marked with this physical object footprint, cf. groundsensorvalues image)
    {
        targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
        //std::cout << "[DEBUG] #" << _wm->getId() << " walked upon " << targetIndex << "\n";
        gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
    }
}
