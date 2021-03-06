/*
Extension by inheritance to default neat/genome.h.
It includes specific Genome id's for the current genome and its parents
This can allow to rebuild and trace back the lineage of a given genome
*/
#ifndef _GENOMEADAPTED_H_
#define _GENOMEADAPTED_H_

#include <vector>
#include "gene.h"
#include "innovation.h"
#include "genome.h"
#include <map>
namespace NEAT 
{
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
  
  class GenomeAdapted : public NEAT::Genome{
    
  private:
    typedef NEAT::Genome super;
    int    _idTrace;
    int    _mom;
    int    _dad;
    int    _nodeId;
    double _innovNumber;
    
  public:
    
    GenomeAdapted(int num_in,int num_out,int num_hidden,int type);
    GenomeAdapted(const GenomeAdapted& genome);
    GenomeAdapted(int id, 
		  std::vector<Trait*> t, 
		  std::vector<NNode*> n, 
		  std::vector<Gene*> g);

    ~GenomeAdapted();

    int     getIdTrace() const     { return _idTrace; };
    int     getMom() const         { return _mom; };
    int     getDad() const         { return _dad; };
    int     getNodeId() const      { return _nodeId; };
    double  getInnovNumber() const { return _innovNumber; };
    
    void setIdTrace    (int i)   { _idTrace = i; };
    void setMom        (int m)   { _mom = m; };
    void setDad        (int d)   { _dad = d; };
    void setNodeId     (int n)   { _nodeId = n; };
    void setInnovNumber(double n){ _innovNumber = n; };


    // Duplicate this Genome to create a new one with the specified id 
    GenomeAdapted* duplicate();
    GenomeAdapted* mutate(float sigma, int idRobot, int newId);
    GenomeAdapted *mutate_weights(float sigma, int idRobot, int newId);

    bool mut_gene_reenable();
    bool mut_toggle_enable();
    void mut_link_weights(double sigma); 
    bool mut_add_node();
    bool mut_add_link(int tries);
    void init_weights(double range);
    
  };
} // namespace NEAT

#endif
