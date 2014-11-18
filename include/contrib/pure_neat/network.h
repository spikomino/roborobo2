#ifndef _PURENETWORK_H_
#define _PURENETWORK_H_

#include <algorithm>
#include <vector>
#include "nnode.h"
#include "genome.h"

namespace PURENEAT {

  class Genome;
  
  // ----------------------------------------------------------------------- 
  // A NETWORK is a LIST of input NODEs and a LIST of output NODEs           
  //   The point of the network is to define a single entity which can evolve
  //   or learn on its own, even though it may be part of a larger framework 
  class Network 
  {
    
    friend class Genome;
    
  public:
    
    // The number of nodes in the net (-1 means not yet counted)
    int numnodes;
    //The number of links in the net (-1 means not yet counted)
    int numlinks;
    
    // A list of all the nodes
    std::vector<NNode*> all_nodes;  
    
    // Kills all nodes and links within
    void destroy();  
    // helper for above
    void destroy_helper(NNode *curnode,std::vector<NNode*> &seenlist); 
    
    void nodecounthelper(NNode *curnode,int &counter,std::vector<NNode*> &seenlist);
    void linkcounthelper(NNode *curnode,int &counter,std::vector<NNode*> &seenlist);
    
  public:
    
    // Allows Network to be matched with its Genome
    Genome *genotype;  
    
    // NNodes that input into the network
    std::vector<NNode*> inputs;  
    // Values output by the network
    std::vector<NNode*> outputs; 
    
    // Allow for a network id
    int net_id; 
    
    // This constructor allows the input and output lists to be supplied
    Network(std::vector<NNode*> in,std::vector<NNode*> out,std::vector<NNode*> all,int netid);
    
    // This constructs a net with empty input and output lists
    Network(int netid);
    
    // Copy Constructor
    Network(const Network& network);
    
    ~Network();
    
    // Puts the network back into an inactive state
    void flush();
    
    // Verify flushedness for debugging
    void flush_check();
    
    // Activates the net such that all outputs are active
    bool activate();
    
    // Add a new input node
    void add_input(NNode*);
    
    // Add a new output node
    void add_output(NNode*);
    
    // Takes an array of sensor values and loads it into SENSOR inputs ONLY
    void load_sensors(double*);
    void load_sensors(const std::vector<float> &sensvals);
    
    // Counts the number of nodes in the net if not yet counted
    int nodecount();
    
    // Counts the number of links in the net if not yet counted
    int linkcount();
    
    // This checks a POTENTIAL link between a potential in_node
    // and potential out_node to see if it must be recurrent 
    // Use count and thresh to jump out in the case of an infinite loop 
    bool is_recur(NNode *potin_node,NNode *potout_node,int &count,int thresh); 
    
    // If all output are not active then return true
    bool outputsoff();
    
    // Just print connections weights with carriage returns
    void print_links_tofile(char *filename);
    
    int max_depth();
    
  };
  
} // namespace PURENEAT

#endif
