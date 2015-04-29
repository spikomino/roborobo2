/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */





#ifndef REDOUANEWORLDOBSERVER_H
#define REDOUANEWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "Redouane/include/RedouaneSharedData.h"
#include "Redouane/include/RedouaneSharedData.h"

//class World;

class RedouaneWorldObserver : public WorldObserver
{
	private:
		void updateEnvironment();
        void updateMonitoring();

	protected:
		int _generationCount;
		int _lifeIterationCount;

	public:
		RedouaneWorldObserver(World *world);
		~RedouaneWorldObserver();

		void reset();
		void step();

		int getLifeIterationCount() { return _lifeIterationCount; }

};

#endif
