/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */





#ifndef ODNEATGCWORLDOBSERVER_H
#define ODNEATGCWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "odNeatGC/include/odNeatGCSharedData.h"


//class World;

class odNeatGCWorldObserver : public WorldObserver
{
	private:
		void updateEnvironment();
        void updateMonitoring();

	protected:
		int _generationCount;
		int _lifeIterationCount;

	public:
		odNeatGCWorldObserver(World *world);
		~odNeatGCWorldObserver();

		void reset();
		void step();

		int getLifeIterationCount() { return _lifeIterationCount; }
		int getGenerationCount() { return _generationCount; }
};

#endif
