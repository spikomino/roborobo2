/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 */


#ifndef ESMEDEAWORLDOBSERVER_H
#define ESMEDEAWORLDOBSERVER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"

#include "Observers/WorldObserver.h"

class esMedeaWorldObserver : public WorldObserver{

private: 
     
    int _lifeIterationCount;
    int _generationCount;


protected:

    

    
public:
    esMedeaWorldObserver( World *__world );
    ~esMedeaWorldObserver();
    
    void reset();
    void step();
  
    
    int getLifeIterationCount() { return _lifeIterationCount; }
    int generationCount() { return _generationCount; }
};

#endif

