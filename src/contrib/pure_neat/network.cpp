#include "pure_neat/network.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "pure_neat/genome.h"


using namespace PURENEAT;

Network::Network(std::vector<NNode*> in,std::vector<NNode*> out,std::vector<NNode*> all,int netid) 
{
  inputs=in;
  outputs=out;
  all_nodes=all;
  numnodes=-1;
  numlinks=-1;
  net_id=netid;
}

Network::Network(int netid) 
{
  numnodes=-1;
  numlinks=-1;
  net_id=netid;
}

Network::Network(const Network& network)
{
  std::vector<NNode*>::const_iterator curnode;
  
  // Copy all the inputs
  for(curnode = network.inputs.begin(); curnode != network.inputs.end(); ++curnode) {
    NNode* n = new NNode(**curnode);
    inputs.push_back(n);
    all_nodes.push_back(n);
  }
  
  // Copy all the outputs
  for(curnode = network.outputs.begin(); curnode != network.outputs.end(); ++curnode) {
    NNode* n = new NNode(**curnode);
    outputs.push_back(n);
    all_nodes.push_back(n);
  }

  numnodes = network.numnodes;
  numlinks = network.numlinks;
  net_id = network.net_id;
}

Network::~Network() 
{
  // Kill off all the nodes and links
  destroy();  

}

// Puts the network back into an initial state
void Network::flush() 
{
  std::vector<NNode*>::iterator curnode;
  
  for(curnode=outputs.begin();curnode!=outputs.end();++curnode) 
    {
      (*curnode)->flushback();
    }
}

// Debugger: Checks network state
void Network::flush_check() 
{
  std::vector<NNode*>::iterator curnode;
  std::vector<NNode*>::iterator location;
  std::vector<NNode*> seenlist;  //List of nodes not to doublecount
  
  for(curnode=outputs.begin();curnode!=outputs.end();++curnode) 
    {    
      location= std::find(seenlist.begin(),seenlist.end(),(*curnode));
      if (location==seenlist.end()) 
	{
	  seenlist.push_back(*curnode);
	  (*curnode)->flushback_check(seenlist);
	}
    }
}

// If all output are not active then return true
bool Network::outputsoff() 
{
  std::vector<NNode*>::iterator curnode;
  
  for(curnode=outputs.begin();curnode!=outputs.end();++curnode) 
    {
      if (((*curnode)->activation_count)==0) return true;
    }
  
  return false;
}

// Activates the net such that all outputs are active
// Returns true on success;
bool Network::activate() 
{
  std::vector<NNode*>::iterator curnode;
  std::vector<Link*>::iterator curlink;
  double add_amount;  //For adding to the activesum
  bool onetime; //Make sure we at least activate once
  int abortcount=0;  //Used in case the output is somehow truncated from the network
  
  
  //Keep activating until all the outputs have become active 
  //(This only happens on the first activation, because after that they
  // are always active)
  onetime=false;
  
  while(outputsoff()||!onetime) 
    {
      
      ++abortcount;
    
      if (abortcount==20) 
	{
	  return false;
	}
      
      // For each node, compute the sum of its incoming activation 
      for(curnode=all_nodes.begin();curnode!=all_nodes.end();++curnode) 
	{
	  //Ignore SENSORS
	  if (((*curnode)->type)!=SENSOR) 
	    {
	      (*curnode)->activesum=0;
	      //This will tell us if it has any active inputs
	      (*curnode)->active_flag=false;  
	      
	      //For each incoming connection, add its activity to the activesum 
	      for(curlink=((*curnode)->incoming).begin();curlink!=((*curnode)->incoming).end();++curlink) 
		{
		  add_amount=((*curlink)->weight)*(((*curlink)->in_node)->get_active_out());
		  if ((((*curlink)->in_node)->active_flag)||
		      (((*curlink)->in_node)->type==SENSOR)) (*curnode)->active_flag=true;
		  (*curnode)->activesum+=add_amount;
		  		  
		} //End for over incoming links
	      
	    } //End if (((*curnode)->type)!=SENSOR) 
	  
	} //End for over all nodes
      
      // Now activate all the non-sensor nodes off their incoming activation 
      for(curnode=all_nodes.begin();curnode!=all_nodes.end();++curnode) 
	{
	  
	  if (((*curnode)->type)!=SENSOR) 
	    {
	      //Only activate if some active input came in
	      if ((*curnode)->active_flag) 
		{
          //Now run the net activation through an activation function
		  //Sigmoidal activation- see comments under fsigmoid
		  if ((*curnode)->ftype==SIGMOID)
		    (*curnode)->activation=fsigmoid((*curnode)->activesum,4.924273);  
		  else
		    {
		      std::cerr << "[ERROR] No valid activation function defined" << std::endl;
		      exit(-1);		  
		    }
		  //Increment the activation_count
		  //First activation cannot be from nothing!!
		  (*curnode)->activation_count++;
		}
	    }
	}
      
      onetime=true;
    }
  return true;  
}

// Add an input
void Network::add_input(NNode *in_node) {
	inputs.push_back(in_node);
}

// Add an output
void Network::add_output(NNode *out_node) {
	outputs.push_back(out_node);
}

// Takes an array of sensor values and loads it into SENSOR inputs ONLY
void Network::load_sensors(double *sensvals) 
{
  std::vector<NNode*>::iterator sensPtr;
  
  for(sensPtr=inputs.begin();sensPtr!=inputs.end();++sensPtr) 
    {
      //only load values into SENSORS (not BIASes)
      if (((*sensPtr)->type)==SENSOR) {
	(*sensPtr)->sensor_load(*sensvals);
	sensvals++;
      }
    }
}

void Network::load_sensors(const std::vector<float> &sensvals) 
{
  
  std::vector<NNode*>::iterator sensPtr;
  std::vector<float>::const_iterator valPtr;
  
  for(valPtr = sensvals.begin(), sensPtr = inputs.begin(); sensPtr != inputs.end() && valPtr != sensvals.end(); ++sensPtr, ++valPtr) {
    //only load values into SENSORS (not BIASes)
    if (((*sensPtr)->type)==SENSOR) {
      (*sensPtr)->sensor_load(*valPtr);
    }
  }
}

// The following two methods recurse through a network from outputs
// down in order to count the number of nodes and links in the network.
// This can be useful for debugging genotype->phenotype spawning 
// (to make sure their counts correspond)

int Network::nodecount() {
	int counter=0;
	std::vector<NNode*>::iterator curnode;
	std::vector<NNode*>::iterator location;
	std::vector<NNode*> seenlist;  //List of nodes not to doublecount

	for(curnode=outputs.begin();curnode!=outputs.end();++curnode) {

        location = std::find(seenlist.begin(),seenlist.end(),(*curnode));
		if (location==seenlist.end()) {
			counter++;
			seenlist.push_back(*curnode);
			nodecounthelper((*curnode),counter,seenlist);
		}
	}

	numnodes=counter;

	return counter;

}

void Network::nodecounthelper(NNode *curnode,int &counter,std::vector<NNode*> &seenlist) {
	std::vector<Link*> innodes=curnode->incoming;
	std::vector<Link*>::iterator curlink;
	std::vector<NNode*>::iterator location;

	if (!((curnode->type)==SENSOR)) {
		for(curlink=innodes.begin();curlink!=innodes.end();++curlink) {
            location= std::find(seenlist.begin(),seenlist.end(),((*curlink)->in_node));
			if (location==seenlist.end()) {
				counter++;
				seenlist.push_back((*curlink)->in_node);
				nodecounthelper((*curlink)->in_node,counter,seenlist);
			}
		}

	}

}

int Network::linkcount() {
	int counter=0;
	std::vector<NNode*>::iterator curnode;
	std::vector<NNode*> seenlist;  //List of nodes not to doublecount

	for(curnode=outputs.begin();curnode!=outputs.end();++curnode) {
		linkcounthelper((*curnode),counter,seenlist);
	}

	numlinks=counter;

	return counter;

}

void Network::linkcounthelper(NNode *curnode,int &counter,std::vector<NNode*> &seenlist) {
	std::vector<Link*> inlinks=curnode->incoming;
	std::vector<Link*>::iterator curlink;
	std::vector<NNode*>::iterator location;

    location = std::find(seenlist.begin(),seenlist.end(),curnode);
	if ((!((curnode->type)==SENSOR))&&(location==seenlist.end())) {
		seenlist.push_back(curnode);

		for(curlink=inlinks.begin();curlink!=inlinks.end();++curlink) {
			counter++;
			linkcounthelper((*curlink)->in_node,counter,seenlist);
		}

	}

}

// Destroy will find every node in the network and 
// delete them one by one.  Since deleting a node deletes its incoming
// links, all nodes and links associated with a network will be destructed
void Network::destroy() 
{
  std::vector<NNode*>::iterator curnode;
  
  // Erase all nodes from all_nodes list 
  for(curnode=all_nodes.begin();curnode!=all_nodes.end();++curnode) 
    {
      delete (*curnode);
    }
}

// This checks a POTENTIAL link between a potential in_node and 
//potential out_node to see if it must be recurrent 
bool Network::is_recur(NNode *potin_node,NNode *potout_node,int &count,int thresh) 
{
  std::vector<Link*>::iterator curlink;
    
  ++count;  //Count the node as visited
  
  if (count>thresh) 
    {
      //Short out the whole thing- loop detected
      return false;  
    }

  if (potin_node==potout_node) return true;
  else {
    //Check back on all links...
    for(curlink=(potin_node->incoming).begin();curlink!=(potin_node->incoming).end();curlink++) 
      {
      //But skip links that are already recurrent
      //(We want to check back through the forward flow of signals only
      if (!((*curlink)->is_recurrent)) 
	{
	  if (is_recur((*curlink)->in_node,potout_node,count,thresh)) return true;
	}
    }
    return false;
  }
}

//Find the maximum number of neurons between an ouput and an input
int Network::max_depth() 
{
  std::vector<NNode*>::iterator curoutput; //The current output we are looking at
  int cur_depth; //The depth of the current node
  int max=0; //The max depth
  
  for(curoutput=outputs.begin();curoutput!=outputs.end();curoutput++) 
    {
      cur_depth=(*curoutput)->depth(0,this);
      if (cur_depth>max) max=cur_depth;
    }
  
  return max;
}
