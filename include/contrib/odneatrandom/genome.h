#ifndef _ODNEATRANDOMGENOME_H_
#define _ODNEATRANDOMGENOME_H_

#include <vector>
#include "odneatrandom/innov.h"
#include <algorithm>

namespace ODNEATGRANDOM
{
  class Gene;
  class NNode;
  class Network;

  enum mutator 
  {
    GAUSSIAN = 0,
  };

  //----------------------------------------------------------------------- 
  //A Genome is the primary source of genotype information used to create   
  //a phenotype.  It contains 3 major constituents:                         
  //  1) A list of Traits                                                 
  //  2) A list of NNodes pointing to a Trait from (1)                      
  //  3) A list of Genes with Links that point to Traits from (1)           
  //(1) Reserved parameter space for future use
  //(2) NNode specifications                                                
  //(3) Is the primary source of innovation in the evolutionary Genome.     
  //    Each Gene in (3) has a marker telling when it arose historically.   
  //    Thus, these Genes can be used to speciate the population, and the   
  //    list of Genes provide an evolutionary history of innovation and     
  //    link-building.
  
  class Genome 
  {

  public:
    int genome_id;
    int mom_id;
    int  dad_id;

    int species;

    int nbFitnessUpdates;//average on how many energy measures

    std::vector<NNode*> nodes; //List of NNodes for the Network
    std::vector<Gene*> genes; //List of innovation-tracking genes

    Network *phenotype; //Allows Genome to be matched with its Network
    
    // Copy constructor
    Genome(const Genome& genome);
    Genome& operator=(const Genome& genome);
    //Special constructor which spawns off an input file
    //This constructor assumes that some routine has already read in GENOMESTART
    Genome(int id, std::ifstream &iFile);
    
    //Special constructor that creates a Genome:
    //Fully linked, no hidden nodes
    //with default innovation numbers
    Genome(int idGenome,int num_in,int num_out);
    Genome(int id, std::vector<NNode*> n, std::vector<Gene*> g);

      //Destructor kills off all lists (including the trait vector)
    ~Genome();
    
    //Generate a network phenotype from this Genome with specified id
    Network *genesis();
    
    // Dump this genome to specified file
    void print_to_file(std::ostream &outFile);
    
    // Wrapper for print_to_file above
    void print_to_filename(char *filename);
    
    // Duplicate this Genome to create a new one 
    Genome *duplicate();
    
    // For debugging: A number of tests can be run on a genome to check its
    // integrity
    // Note: Some of these tests do not indicate a bug, but rather are meant
    // to be used to detect specific system states
    bool verify();
    
    // ******* MUTATORS *******
    
    //Launch all mutations and returns the corresponding mutated genome
    //Check out properties file, section NEAT - paramerters for the probabilities
    Genome *mutate(float sigma, int idNewGenome, int &nodeId, int &genecounter);
    
    // Add Gaussian noise to all linkweights with variance power ^ 2
    void mutate_link_weights(double power);
    
    double capWeights(double w);

    // toggle genes on or off 
    void mutate_toggle_enable(int times);
    
    // Find first disabled gene and enable it 
    void mutate_gene_reenable();
    
    // These last kinds of mutations return false if they fail
    //   They can fail under certain conditions,  being unable
    //   to find a suitable place to make the mutation.
    //   Generally, if they fail, they can be called again if desired. 
    
    // Mutate genome by adding a node
    bool mutate_add_node(int tries,int &nodeId, int &genecounter);
    
    // Mutate the genome by adding a new link between 2 random NNodes 
    bool mutate_add_link(int tries,int &genecounter);
    
    
    // ****** MATING METHODS ***** 
    
    // This method mates this Genome with another Genome g.  
    //   For every point in each Genome, where each Genome shares
    //   the innovation number, the Gene is chosen randomly from 
    //   either parent.  If one parent has an innovation absent in 
    //   the other, the baby will inherit the innovation 
    //   Interspecies mating leads to all genes being inherited.
    //   Otherwise, excess genes come from most fit parent.
    Genome *mate_multipoint(Genome *g,int genomeid,double fitness1, double fitness2);
    
    double dissimilarity(Genome *g);

    // Return number of non-disabled genes 
    int extrons();
    
  protected:
    //Inserts a NNode into a given ordered list of NNodes in order
    void node_insert(std::vector<NNode*> &nlist, NNode *n);
    
    //Adds a new gene that has been created through a mutation in the
    //*correct order* into the list of genes in the genome
    void add_gene(std::vector<Gene*> &glist,Gene *g);
    
  };
  
  extern "C"
  {
    //Global variables: external definition    
    extern double     mutateOnlyProbOdNRandom; // Prob. of a non-mating reproduction
    extern double     mutateLinkWeightsProbOdNRandom;
    extern double     mutateToggleEnableProbOdNRandom;
    extern double     mutateGeneReenableProbOdNRandom;
    extern double     mutateAddNodeProbOdNRandom;
    extern double     mutateAddLinkProbOdNRandom;
    extern double     mateOnlyProbOdNRandom; // Prob. of mating without mutation
    extern double     recurOnlyProbOdNRandom;  // Probability of forcing selection of ONLY links that are naturally recurrent
    extern int        newStructureTriesOdNRandom;  // Number of tries mutate_add_link or mutate_add_node will attempt to find an open link

    extern double     coefEOdNRandom;
    extern double     coefDOdNRandom;
    extern double     coefWOdNRandom;

    extern double     rangeWOdNRandom;
    
  }
} // namespace ODNEATGRANDOM

extern "C"
{
  
  double randFloatOdNRandom();
  int randPosNegOdNRandom();
  int randIntOdNRandom(int x, int y);
  int getUnitCountsOdNRandom(const char *string,const char *set);

  double fSigmoidOdNRandom(double activesum,double slope);
  double gaussRandOdNRandom();
  bool load_odneatrandom_params(const char *filename, bool output);
}
#endif
