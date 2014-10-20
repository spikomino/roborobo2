/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */





#ifndef NEATTESTWORLDOBSERVER_H
#define NEATTESTWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "neattest/include/neattestSharedData.h"
#include "neattest/include/neattestSharedData.h"

//class World;

class neattestWorldObserver : public WorldObserver
{
	private:
		void updateEnvironment();
        void updateMonitoring();

	protected:
		int _generationCount;
		int _lifeIterationCount;

	public:
		neattestWorldObserver(World *world);
		~neattestWorldObserver();

		void reset();
		void step();

		int getLifeIterationCount() { return _lifeIterationCount; }

};

#endif
