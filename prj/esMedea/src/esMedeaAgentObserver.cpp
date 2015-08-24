/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 */

#include "esMedea/include/esMedeaAgentObserver.h"
#include "esMedea/include/esMedeaController.h"
#include "esMedea/include/esMedeaWorldObserver.h"
#include "World/World.h"

esMedeaAgentObserver::esMedeaAgentObserver( RobotWorldModel *__wm ){
	_wm = (RobotWorldModel*) __wm;
}

esMedeaAgentObserver::~esMedeaAgentObserver(){

}

void esMedeaAgentObserver::reset() {

}


static bool is_energy_item(int id){
    return (gPhysicalObjects[id]->getType()==1);
} 

void esMedeaAgentObserver::step() {

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
	    esMedeaController *cont = 
		dynamic_cast <
		esMedeaController *
		>(gWorld->getRobot (_wm->getId())->getController ());

	    /* does the agent have room in its basket */ 
	    if(cont->stillRoomInBasket()){
	    
		/* notify the object to diapear */
		gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
	
		/* notify the robot that it picked an item  */
		cont->pickItem(targetIndex);
		

		/* TODO : bug in object picking 
		   items can be picked by multiple robots 
		 */



		/*std::cout << "Robot " << cont->getId() 
			  << " picked " << targetIndex 
			  << " it has "  << cont->getBasket().size() 
			  << " items in basket" 
			  << std::endl;*/
	    }
	}

	/* if(gVerbose) */
	    /* std::cout << "\t[Robot #" + to_string(_wm->getId())  */
	    /* 	      << "\twalked upon " << targetIndex << "]"  */
	    /* 	      <<  std::endl; */
	
	
    }
}

