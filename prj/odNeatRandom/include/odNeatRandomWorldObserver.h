/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */





#ifndef ODNEATRANDOMWORLDOBSERVER_H
#define ODNEATRANDOMWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "odNeatRandom/include/odNeatRandomSharedData.h"
#include <set>


class odNeatRandomWorldObserver : public WorldObserver
{
	private:
		void updateEnvironment();
        void updateMonitoring();

	protected:
		int _generationCount;
		int _lifeIterationCount;
        int _countGeneClockCollisions;

	public:
		odNeatRandomWorldObserver(World *world);
		~odNeatRandomWorldObserver();
        std::set<int> usedGeneCounters;
        void incrementCollisions();
		void reset();
		void step();

		int getLifeIterationCount() { return _lifeIterationCount; }
		int getGenerationCount() { return _generationCount; }
};

#endif
