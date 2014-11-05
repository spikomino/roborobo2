#include "Simple/include/SimpleSharedData.h"

std::string SimpleSharedData::gEvolutionLogFile = "";
std::ofstream SimpleSharedData::gEvoLog;

double SimpleSharedData::gSigma = 0.0; 

int SimpleSharedData::gIteration = 0;
int SimpleSharedData::gEvalTime = 0; 
int SimpleSharedData::gListenTime = 200;

bool SimpleSharedData::gPropertiesLoaded = false; 
