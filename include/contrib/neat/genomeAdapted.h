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
    int _idTrace;
    int _mom;
    int _dad;

    int nodeId;
    double innovNumber;
    
  public:
    
    GenomeAdapted(int num_in,int num_out,int num_hidden,int type);
    GenomeAdapted(const GenomeAdapted& genome);
    GenomeAdapted(int id, std::vector<Trait*> t, std::vector<NNode*> n, std::vector<Gene*> g);
    ~GenomeAdapted();
    int getIdTrace() const;
    int getMom() const;
    int getDad() const;
    
    // Duplicate this Genome to create a new one with the specified id 
<<<<<<< HEAD
    GenomeAdapted* duplicate(int idRobot);
    GenomeAdapted* mutate(float sigma, int idRobot ,int idNewGenome, int &nodeId, double &innovNumber);
=======
    GenomeAdapted* duplicate(int new_id, int idTr);
    GenomeAdapted* mutate(float sigma, int idRobot ,int newId, int &nodeId, double &innovNumber);
    GenomeAdapted* mutate(float sigma, int idRobot ,int newId);

>>>>>>> 7a0087de0d96507bc7694c9b9beaa30035204fac
    void setIdTrace(int id);
    void setMom(int idMom);
    void setDad(int idDad);

    void setNodeId(int id){nodeId=id;};
    void setInnovNumber(double n){innovNumber=n;};

  };
} // namespace NEAT

#endif
