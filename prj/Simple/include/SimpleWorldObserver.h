#ifndef SIMPLEWORLDOBSERVER_H
#define SIMPLEWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "Simple/include/SimpleSharedData.h"
#include "Simple/include/SimpleSharedData.h"

//class World;

class SimpleWorldObserver : public WorldObserver
{
	private:
		void updateEnvironment();
		void updateMonitoring();

	protected:
		int _generationCount;
		int _lifeIterationCount;

	public:
		SimpleWorldObserver(World *world);
		~SimpleWorldObserver();

		void reset();
		void step();

		int getLifeIterationCount() { return _lifeIterationCount; }
		int getGenerationCount() { return _generationCount; }
};

#endif
