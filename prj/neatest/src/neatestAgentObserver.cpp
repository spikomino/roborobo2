/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#include "neatest/include/neatestAgentObserver.h"
#include "World/World.h"
#include "Utilities/Misc.h"
#include "RoboroboMain/roborobo.h"
#include "neatest/include/neatestController.h"
#include <cmath>
#include "neatest/include/neatestWorldObserver.h"
#include <string>


neatestAgentObserver::neatestAgentObserver( RobotWorldModel *wm ){
    _wm = (RobotWorldModel*)wm;
}

neatestAgentObserver::~neatestAgentObserver(){
    // nothing to do.
}

void neatestAgentObserver::reset(){
    // nothing to do.
}

void neatestAgentObserver::step(){
    /* if(gVerbose) */
    /*   std::cout << "[Observer]\n"; */
    
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
	    gPhysicalObjects[targetIndex]->isTouched(_wm->getId());

            /* if(gVerbose) */
	    /* 	std::cout << "\t[Robot #" + to_string(_wm->getId())  */
	    /* 		  << "\tTouched " << targetIndex << "]"  */
	    /* 		  <<  std::endl; */
	}
    }
    
    // through floor sensor
    int targetIndex = _wm->getGroundSensorValue();

    // ground sensor is upon a physical object 
    // (OR: on a place marked with this physical object footprint, 
    // cf. groundsensorvalues image)
    if ( PhysicalObject::isInstanceOf(targetIndex) ) {
        targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
	gPhysicalObjects[targetIndex]->isWalked(_wm->getId());

	/* if(gVerbose) */
	    /* std::cout << "\t[Robot #" + to_string(_wm->getId())  */
	    /* 	      << "\twalked upon " << targetIndex << "]"  */
	    /* 	      <<  std::endl; */
	
	
    }
}
