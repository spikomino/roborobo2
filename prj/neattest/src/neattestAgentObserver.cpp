/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#include "neattest/include/neattestAgentObserver.h"
#include "World/World.h"
#include "Utilities/Misc.h"
#include "RoboroboMain/roborobo.h"
#include "neattest/include/neattestController.h"
#include <cmath>
#include "neattest/include/neattestWorldObserver.h"
#include <string>


neattestAgentObserver::neattestAgentObserver( RobotWorldModel *wm ){
    _wm = (RobotWorldModel*)wm;
}

neattestAgentObserver::~neattestAgentObserver(){
    // nothing to do.
}

void neattestAgentObserver::reset(){
    // nothing to do.
}

void neattestAgentObserver::step(){
    // * update energy if needed
    if ( gEnergyLevel && _wm->isAlive() ){
	_wm->substractEnergy(1);
	assert( _wm->getEnergyLevel() >= 0 );
	if ( _wm->getEnergyLevel() == 0 )
	    _wm->setAlive(false);
    }

    // * send callback messages to objects touched or walked upon.
    
    // through distance sensors
    for( int i = 0 ; i < _wm->_cameraSensorsNb; i++){
        int targetIndex = _wm->getObjectIdFromCameraSensor(i);
        
	// sensor ray bumped into a physical object
        if ( PhysicalObject::isInstanceOf(targetIndex) ) {
            targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
            //std::cout << "[DEBUG] Robot #" << _wm->getId() << " touched " << targetIndex << "\n";
            gPhysicalObjects[targetIndex]->isTouched(_wm->getId());
        }
    }
    
    // through floor sensor
    int targetIndex = _wm->getGroundSensorValue();
    
    // ground sensor is upon a physical object 
    // (OR: on a place marked with this physical object footprint, 
    // cf. groundsensorvalues image)
    if ( PhysicalObject::isInstanceOf(targetIndex) ) {
        targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
        //std::cout << "[DEBUG] #" << _wm->getId() << " walked upon " << targetIndex << "\n";
        gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
    }
}
