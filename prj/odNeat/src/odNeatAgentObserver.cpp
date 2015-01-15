/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#include "odNeat/include/odNeatAgentObserver.h"
#include "World/World.h"
#include "Utilities/Misc.h"
#include "RoboroboMain/roborobo.h"
#include "odNeat/include/odNeatController.h"
#include <cmath>
#include "odNeat/include/odNeatWorldObserver.h"
#include <string>


odNeatAgentObserver::odNeatAgentObserver( RobotWorldModel *wm )
{
    _wm = (RobotWorldModel*)wm;

}

odNeatAgentObserver::~odNeatAgentObserver()
{
	// nothing to do.
}

void odNeatAgentObserver::reset()
{
	// nothing to do.
}

void odNeatAgentObserver::step()
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

    //Phototaxis: add energy to robot if it's at an energy point
    if(odNeatSharedData::gFitness == 0)
    {
        int targetIndex = _wm->getGroundSensorValue();

        if ( PhysicalObject::isInstanceOf(targetIndex) ) // ground sensor is upon a physical object (OR: on a place marked with this physical object footprint, cf. groundsensorvalues image)
        {
            targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
            //std::cout << "[DEBUG] #" << _wm->getId() << " walked upon " << targetIndex << "\n";
            gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
            /* notify the robot that it picked an item  */
            odNeatController *cont =
                    dynamic_cast <
                    odNeatController *
                    >(gWorld->getRobot (_wm->getId())->getController ());
            cont->gatherEnergy();
        }
    }
    else
    {
        // through floor sensor
        int targetIndex = _wm->getGroundSensorValue();
        if ( PhysicalObject::isInstanceOf(targetIndex) ) // ground sensor is upon a physical object (OR: on a place marked with this physical object footprint, cf. groundsensorvalues image)
        {
            targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
            //std::cout << "[DEBUG] #" << _wm->getId() << " walked upon " << targetIndex << "\n";
            gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
            // notify the robot that it picked an item
            /*odNeatController *cont =
                dynamic_cast <
                odNeatController *
                >(gWorld->getRobot (_wm->getId())->getController ());
            cont->pickItem();*/
        }
    }


}
