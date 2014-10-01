/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */





#ifndef PRJMATESELECTIONWORLDOBSERVER_H
#define PRJMATESELECTIONWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Observers/Observer.h"
#include "Observers/WorldObserver.h"
#include "WorldModels/RobotWorldModel.h"
#include "PrjMateSelection/include/PrjMateSelectionSharedData.h"
#include "PrjMateSelection/include/PrjMateSelectionSharedData.h"

//class World;

class PrjMateSelectionWorldObserver : public WorldObserver
{
	private:
		void updateEnvironment();
        void updateMonitoring();

	protected:
		int _generationCount;
		int _lifeIterationCount;

	public:
		PrjMateSelectionWorldObserver(World *world);
		~PrjMateSelectionWorldObserver();

		void reset();
		void step();

		int getLifeIterationCount() { return _lifeIterationCount; }

};

#endif
