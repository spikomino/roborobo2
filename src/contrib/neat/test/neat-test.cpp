

#include <iostream>
#include <iomanip>
#include <vector>
#include "neat/neat.h"
#include "neat/genomeAdapted.h"

using namespace std;
using namespace NEAT;

int main (int argc, char** argv){
    int nb_mut=40;
    int nb_inputs=5;
    int nb_outputs=2;
    double sigma = .5;
    double innovNumber;
    int nodeId= nb_inputs+nb_outputs+1;
   
    load_neat_params ("explo.ne", true);
    
    GenomeAdapted* genome = new GenomeAdapted(nb_inputs, nb_outputs, 0, 0);
    genome->mutate_link_weights (1.0, 1.0, COLDGAUSSIAN);
    genome->setIdTrace(0);
    genome->setNodeId(nodeId);
    innovNumber=0;
    genome->setInnovNumber(innovNumber);
   
    char fname[128];
    snprintf(fname, 127, "%04d-%010d.gen", 0, genome->getIdTrace());
    std::ofstream oFile(fname);
    genome->print_to_file(oFile);
    oFile.close();

    for(int i=1; i<=nb_mut; i++){

	genome = genome->mutate(sigma,
				genome->getIdTrace(), 
				i);
	snprintf(fname, 127, "%04d-%010d.gen", 0, genome->getIdTrace());
	std::ofstream oFile(fname);
	genome->print_to_file(oFile);
	oFile.close();
	
    }

    
}
