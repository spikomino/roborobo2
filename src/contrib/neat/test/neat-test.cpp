

#include <iostream>
#include <iomanip>
#include <vector>
#include "neat/neat.h"
#include "neat/genomeAdapted.h"

using namespace std;
using namespace NEAT;


void save_genome(GenomeAdapted* g){
    char fname[128];
    snprintf(fname, 127, "%04d-%010d.gen", 
	     g->genome_id, 
	     g->getIdTrace());
    
    std::ofstream oFile(fname);
    g->print_to_file(oFile);
    oFile.close();
}


int main (int argc, char** argv){
    int nb_mut=10;
    int nb_inputs   =10;
    int nb_outputs  =2;
    double sigma = .5;

    srand( (unsigned)time( NULL ) );
    
    load_neat_params ("explo.ne", true);

    /* create an initial genome */
    GenomeAdapted* genome = new GenomeAdapted(nb_inputs, nb_outputs, 0, 0);
    genome->genome_id = 0;
    genome->setIdTrace(0);
    genome->setNodeId(1 + nb_inputs + nb_outputs);
    genome->setMom(-1);
    genome->setDad(-1);

    genome->mutate_link_weights (1.0, 1.0, COLDGAUSSIAN);
    Network* neurocontroller = genome->genesis(genome->genome_id);
    genome->setInnovNumber((double) neurocontroller->linkcount ());

    save_genome(genome);


    /* test genome copy */
    if(0){
	GenomeAdapted* copy = genome->duplicate();
	save_genome(copy);
	exit(0);
    }
    
    /* test mutations */
    if(1){
	for(int i=1; i<=nb_mut; i++){
	    GenomeAdapted* mutated;
	    std::cout << i << " : ";
	    mutated = genome->mutate(sigma, genome->getIdTrace(), i);
	    delete genome;
	    genome = mutated;
	    save_genome(genome);
	}
	//exit(0);
    }

    
    if(1){
	GenomeAdapted* copy = genome->duplicate();
	copy->genome_id = genome->genome_id+1;
	save_genome(copy);
	exit(0);
    }
    
    
    
    

    
}
