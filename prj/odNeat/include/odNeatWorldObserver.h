/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */





#ifndef ODNEATWORLDOBSERVER_H
#define ODNEATWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "odNeat/include/odNeatSharedData.h"
#include "odNeat/include/odNeatSharedData.h"

//class World;

class odNeatWorldObserver : public WorldObserver
{
	private:
		void updateEnvironment();
        void updateMonitoring();

	protected:
		int _generationCount;
		int _lifeIterationCount;

	public:
		odNeatWorldObserver(World *world);
		~odNeatWorldObserver();

		void reset();
		void step();

		int getLifeIterationCount() { return _lifeIterationCount; }
		int getGenerationCount() { return _generationCount; }
};

#endif
