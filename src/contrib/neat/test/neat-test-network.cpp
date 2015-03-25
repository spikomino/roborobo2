

#include <iostream>
#include <iomanip>
#include <vector>
#include "neat/neat.h"
#include "neat/genome.h"

using namespace std;
using namespace NEAT;


void save_genome(Genome* g){
    std::ofstream oFile("out.gen");
    g->print_to_file(oFile);
    oFile.close();
}


// test values
double inputs1[] = {
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  // obs
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, // itm
    1.0, // basket
    1.0, 1.0, 1.0, //  nest (rgb) 
    0.0, 0.0, // landmark dist, orientation
    1.0 // bias
};

double inputs0[] = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  // obs
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, // itm
    1.0, // basket
    0.0, 1.0, 0.0, //  nest (rgb) 
    0.0, 1.0, // landmark dist, orientation
    1.0  // bias
};

int main (int argc, char** argv){


    load_neat_params ("explo.ne", true);

    // read the genome file 
    std::ifstream iFile(argv[1]);
    Genome* genome = new Genome(17,iFile); 
    iFile.close();

    std::vector<NNode*> nodes = genome->nodes;
    int nbInputs = 0;
    int nbOutpus = 0;
    for (const auto& n : genome->nodes)
	if(n->type == SENSOR)
	    nbInputs++;
    printf("Read a genome with %d inputs\n", nbInputs);
    
	
    // create the coresponding  network 
    Network* neurocontroller = genome->genesis(genome->genome_id);

    

    neurocontroller->load_sensors(inputs0);
    if (!neurocontroller->activate()){
	std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
	exit (-1);
    }
    printf("output : ");
    // (3) read the output
    std::vector<double> outputs;
    std::vector<NNode*>::iterator out_iter;
    for (out_iter  = neurocontroller->outputs.begin();
	 out_iter != neurocontroller->outputs.end(); 
	 out_iter++)

	printf(" %f", (*out_iter)->activation);

     printf("\n");


    // save the genome to out.gen
    save_genome(genome);


    
    
    
    

    
}
