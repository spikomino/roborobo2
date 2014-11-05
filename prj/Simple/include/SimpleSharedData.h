#ifndef SIMPLESHAREDDATA_H
#define SIMPLESHAREDDATA_H
#include <string>
#include <fstream>

class SimpleSharedData {
	
 public: 
  
  // -----
  static std::string gEvolutionLogFile;
  static std::ofstream gEvoLog;
  
  static double gSigma; 
  static int gEvalTime; 
  static int gListenTime;

  static int gIteration;

  static bool gPropertiesLoaded;

  
};


#endif
