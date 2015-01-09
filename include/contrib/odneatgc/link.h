#ifndef _ODNEATGCLINK_H_
#define _ODNEATGCLINK_H_

#include "odneatgc/nnode.h"

namespace ODNEATGC
{
  
  class NNode;
  // ----------------------------------------------------------------------- 
  // A LINK is a connection from one node to another with an associated weight 
  // It can be marked as recurrent 
  // Its parameters are made public for efficiency 
  class Link 
  {
  public: 
    // Weight of connection
    double weight; 
    // NNode inputting into the link
    NNode *in_node; 
    // NNode that the link affects
    NNode *out_node; 
    //Flag indicating the network if this link is recurrent
    bool is_recurrent;
    
    Link(double w,NNode *inode,NNode *onode,bool recur);
    
    // For when you don't know the connections yet
    Link(double w);
    
    // Copy Constructor
    Link(const Link& link);
    
  };
  
} // namespace ODNEATGC

#endif
