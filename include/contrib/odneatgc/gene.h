#ifndef _ODNEATGCGENE_H_
#define _ODNEATGCGENE_H_

#include "odneatgc/link.h"
#include "odneatgc/nnode.h"
#include "odneatgc/innov.h"

namespace ODNEATGC
{

  class Gene 
  {
  public:
    
    Link *lnk;
    innov innovation_num;
    
    bool enable;  //When this is off the Gene is disabled
    
    //Construct a gene with no trait
    Gene(double w,NNode *inode,NNode *onode,bool recur,innov innov);
    
    //Construct a gene off of another gene as a duplicate
    Gene(Gene *g,NNode *inode,NNode *onode);
    
    //Construct a gene from a file spec given nodes
    Gene(const char *argline, std::vector<NNode*> &nodes);
    
    // Copy Constructor
    Gene(const Gene& gene);
    
    ~Gene();
    
    //Print gene to a file- called from Genome
    void print_to_file(std::ostream &outFile);
  };
  
} // namespace ODNEATGC


#endif
