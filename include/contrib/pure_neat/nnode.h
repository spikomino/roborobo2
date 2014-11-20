#ifndef _PURENNODE_H_
#define _PURENNODE_H_

#include <algorithm>
#include <vector>

namespace PURENEAT 
{
  
  enum nodetype 
  {
    NEURON = 0,
    SENSOR = 1
  };
  
  enum nodeplace 
  {
    HIDDEN = 0,
    INPUT = 1,
    OUTPUT = 2,
    BIAS = 3
  };

  enum functype 
  {
    SIGMOID = 0
  };
  
  class Link;
  
  class Network;
  
  // ----------------------------------------------------------------------- 
  // A NODE is either a NEURON or a SENSOR.  
  //   - If it's a sensor, it can be loaded with a value for output
  //   - If it's a neuron, it has a list of its incoming input signals (List<Link> is used) 
  // Use an activation count to avoid flushing
  class NNode 
  {

    friend class Network;
    friend class Genome;
    friend class GenomeAdapted;
  protected:
    
    int activation_count;  // keeps track of which activation the node is currently in

    NNode *dup;       // Used for Genome duplication
    
    NNode *analogue;  // Used for Gene decoding
    
  public:
    functype ftype; // type is either SIGMOID ..or others that can be added
    nodetype type; // type is either NEURON or SENSOR 
    
    double activesum;  // The incoming activity before being processed 
    double activation; // The total activation entering the NNode 
    bool active_flag;  // To make sure outputs are active
      
    std::vector<Link*> incoming; // A list of pointers to incoming weighted signals from other nodes
    std::vector<Link*> outgoing;  // A list of pointers to links carrying this node's signal
    
    int node_id;  // A node can be given an identification number for saving in files
    
    nodeplace gen_node_label;  // Used for genetic marking of nodes

    NNode(nodetype ntype,int nodeid);
    
    NNode(nodetype ntype,int nodeid, nodeplace placement);

    NNode(NNode *n);

    // Construct the node out of a file specification using given list of traits
    NNode (const char *argline);
    
    // Copy Constructor
    NNode (const NNode& nnode);

    ~NNode();
    
    // Just return activation for step
    double get_active_out();
    
    // Returns the type of the node, NEURON or SENSOR
    const nodetype get_type();
    
    // Allows alteration between NEURON and SENSOR.  Returns its argument
    nodetype set_type(nodetype);
    
    // If the node is a SENSOR, returns true and loads the value
    bool sensor_load(double);

    // Recursively deactivate backwards through the network
    void flushback();
    
    // Verify flushing for debugging
    void flushback_check(std::vector<NNode*> &seenlist);
    
    // Print the node to a file
    void print_to_file(std::ostream &outFile);
    
    // Returns the gene that created the node
    NNode *get_analogue();
    
    //Find the greatest depth starting from this neuron at depth d
    int depth(int d,Network *mynet); 
    
  };
  
  
} // namespace PURENEAT

#endif
