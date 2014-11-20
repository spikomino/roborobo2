#ifndef _PUREGENOME_H_
#define _PUREGENOME_H_

#include <vector>
#include "innov.h"
#include <algorithm>


namespace PURENEAT 
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

    //int node_id;
    //double innovNumber;
    innov node_id;
    innov innovNumber;

    std::vector<NNode*> nodes; //List of NNodes for the Network
    std::vector<Gene*> genes; //List of innovation-tracking genes

    Network *phenotype; //Allows Genome to be matched with its Network
    
    innov get_last_node_id(); //Return id of final NNode in Genome
    innov get_last_gene_innovnum(); //Return last innovation number in Genome
        
    // Copy constructor
    Genome(const Genome& genome);

    //Special constructor which spawns off an input file
    //This constructor assumes that some routine has already read in GENOMESTART
    Genome(int id, std::ifstream &iFile);
    
    //Special constructor that creates a Genome:
    //Fully linked, no hidden nodes
    Genome(int num_in,int num_out, int idR);
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
    Genome *mutate(float sigma, int idRobot ,int idNewGenome, int &nodeId, int &innovNum);
    
    // Add Gaussian noise to all linkweights with variance power ^ 2
    void mutate_link_weights(double power);
    
    // toggle genes on or off 
    void mutate_toggle_enable(int times);
    
    // Find first disabled gene and enable it 
    void mutate_gene_reenable();
    
    // These last kinds of mutations return false if they fail
    //   They can fail under certain conditions,  being unable
    //   to find a suitable place to make the mutation.
    //   Generally, if they fail, they can be called again if desired. 
    
    // Mutate genome by adding a node respresentation 
    bool mutate_add_node(int idR,int &curnode_id,int &curinnov, int tries);
    
    // Mutate the genome by adding a new link between 2 random NNodes 
    bool mutate_add_link(int idR, int &curinnov,int tries);
    
    
    // ****** MATING METHODS ***** 
    
    // This method mates this Genome with another Genome g.  
    //   For every point in each Genome, where each Genome shares
    //   the innovation number, the Gene is chosen randomly from 
    //   either parent.  If one parent has an innovation absent in 
    //   the other, the baby will inherit the innovation 
    //   Interspecies mating leads to all genes being inherited.
    //   Otherwise, excess genes come from most fit parent.
    Genome *mate_multipoint(Genome *g,int genomeid,double fitness1, double fitness2);
    
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
    extern double     recur_prob; // Prob. that a link mutation which doesn't have to be recurrent will be made recurrent 
    extern double     mutate_only_prob; // Prob. of a non-mating reproduction 
    extern double     mutate_link_weights_prob;
    extern double     mutate_toggle_enable_prob;
    extern double     mutate_gene_reenable_prob;
    extern double     mutate_add_node_prob;
    extern double     mutate_add_link_prob;
    extern double     mate_multipoint_prob;     
    extern double     mate_only_prob; // Prob. of mating without mutation 
    extern double     recur_only_prob;  // Probability of forcing selection of ONLY links that are naturally recurrent 
    extern int        newstructure_tries;  // Number of tries mutate_add_link or mutate_add_node will attempt to find an open link
    
  }
} // namespace PURENEAT

extern "C"
{
  
  double randfloat();
  int randposneg();
  int randint(int x, int y);
  int getUnitCount(const char *string,const char *set);
  double fsigmoid(double activesum,double slope);
  double gaussrand();
  bool load_neat_params(const char *filename, bool output);
}
#endif
