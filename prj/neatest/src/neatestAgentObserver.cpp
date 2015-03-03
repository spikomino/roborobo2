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

static bool is_energy_item(int id){
    return (gPhysicalObjects[id]->getType()==1);
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
	
	/* if energy item */
	if(is_energy_item(targetIndex)){
	
	    /* get the controller */
	    neatestController *cont = 
		dynamic_cast <
		neatestController *
		>(gWorld->getRobot (_wm->getId())->getController ());

	    /* does the agent have room in its basket */ 
	    if(cont->stillRoomInBasket()){
	    
		/* notify the object to diapear */
		gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
	
		/* notify the robot that it picked an item  */
		cont->pickItem(targetIndex);
		
		/* store in a list and remove from */
		gPhysicalObjects[targetIndex]->unregisterObject();
		gPhysicalObjects[targetIndex]->hide();

		/*std::cout << "Robot " << cont->getId() 
			  << " picked " << targetIndex 
			  << " it has "  << cont->ItemsPicked() 
			  << " items in basket" 
			  << std::endl; */
	    }
	}

	/* if(gVerbose) */
	    /* std::cout << "\t[Robot #" + to_string(_wm->getId())  */
	    /* 	      << "\twalked upon " << targetIndex << "]"  */
	    /* 	      <<  std::endl; */
	
	
    }
}
