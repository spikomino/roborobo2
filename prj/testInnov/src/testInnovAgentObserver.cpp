/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */


#include "testInnov/include/testInnovAgentObserver.h"
#include "World/World.h"
#include "Utilities/Misc.h"
#include "RoboroboMain/roborobo.h"
#include "testInnov/include/testInnovController.h"
#include <cmath>
#include "testInnov/include/testInnovWorldObserver.h"
#include <string>


testInnovAgentObserver::testInnovAgentObserver( RobotWorldModel *wm )
{
    _wm = (RobotWorldModel*)wm;

}

testInnovAgentObserver::~testInnovAgentObserver()
{
	// nothing to do.
}

void testInnovAgentObserver::reset()
{
	// nothing to do.
}

void testInnovAgentObserver::step()
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
    if(testInnovSharedData::gFitness == 0)
    {
        int targetIndex = _wm->getGroundSensorValue();

        if ( PhysicalObject::isInstanceOf(targetIndex) ) // ground sensor is upon a physical object (OR: on a place marked with this physical object footprint, cf. groundsensorvalues image)
        {
            targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
            //std::cout << "[DEBUG] #" << _wm->getId() << " walked upon " << targetIndex << "\n";
            gPhysicalObjects[targetIndex]->isWalked(_wm->getId());
            /* notify the robot that it picked an item  */
            testInnovController *cont =
                    dynamic_cast <
                    testInnovController *
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
            /*testInnovController *cont =
                dynamic_cast <
                testInnovController *
                >(gWorld->getRobot (_wm->getId())->getController ());
            cont->pickItem();*/
        }
    }


}
