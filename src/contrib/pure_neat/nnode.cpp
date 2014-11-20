#include "pure_neat/nnode.h"
#include "pure_neat/link.h"
#include "pure_neat/network.h"
#include <iostream>
#include <sstream>
using namespace PURENEAT;

NNode::NNode(nodetype ntype,int nodeid)
{
	active_flag=false;
	activesum=0;
	activation=0;
	type=ntype; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=nodeid;
	ftype=SIGMOID;
	gen_node_label=HIDDEN;
	dup=0;
	analogue=0;
}

NNode::NNode(nodetype ntype,int nodeid, nodeplace placement)
{
	active_flag=false;
	activesum=0;
	activation=0;
	type=ntype; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=nodeid;
	ftype=SIGMOID;
	gen_node_label=placement;
	dup=0;
	analogue=0;
}

NNode::NNode(NNode *n)
{
	active_flag=false;
	activation=0;
	type=n->type; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=n->node_id;
	ftype=SIGMOID;
	gen_node_label=n->gen_node_label;
	dup=0;
	analogue=0;
}

NNode::NNode (const char *argline)
{
  activesum=0;
  
  std::stringstream ss(argline);

  int nodety, nodepl;
  ss >> node_id >> nodety >> nodepl;
  type = (nodetype)nodety;
  gen_node_label = (nodeplace)nodepl;

}

// This one might be incomplete
NNode::NNode (const NNode& nnode)
{
	active_flag = nnode.active_flag;
	activesum = nnode.activesum;
	activation = nnode.activation;
	type = nnode.type; //NEURON or SENSOR type
	activation_count = nnode.activation_count; //Inactive upon creation
	node_id = nnode.node_id;
	ftype = nnode.ftype;
	gen_node_label = nnode.gen_node_label;
	dup = nnode.dup;
	analogue = nnode.dup;
}

NNode::~NNode() 
{
  std::vector<Link*>::iterator curlink;
	//Kill off all incoming links
    for(curlink=incoming.begin();curlink!=incoming.end();++curlink)
    {
		delete (*curlink);
	}
}

//Returns the type of the node, NEURON or SENSOR
const nodetype NNode::get_type()
{
	return type;
}

//Allows alteration between NEURON and SENSOR.  Returns its argument
nodetype NNode::set_type(nodetype newtype)
{
	type=newtype;
	return newtype;
}

//If the node is a SENSOR, returns true and loads the value
bool NNode::sensor_load(double value)
{
    if (type==SENSOR)
    {
		activation_count++;  //Puts sensor into next time-step
		activation=value;
		return true;
	}
    else
        return false;
}

// Note: NEAT keeps track of which links are recurrent and which
// are not even though this is unnecessary for activation.
// It is useful to do so for 2 other reasons: 
// 1. It makes networks visualization of recurrent networks possible
// 2. It allows genetic control of the proportion of connections
//    that may become recurrent


// Return activation currently in node, if it has been activated
double NNode::get_active_out()
{
	if (activation_count>0)
		return activation;
	else return 0.0;
}

// This recursively flushes everything leading into and including this NNode, 
//including recurrencies
void NNode::flushback() 
{
  std::vector<Link*>::iterator curlink;
  
  //A sensor should not flush black
  if (type!=SENSOR) 
    {
      
      if (activation_count>0) 
        {
            activation_count=0;
            activation=0;
        }
      
      //Flush back recursively
      for(curlink=incoming.begin();curlink!=incoming.end();++curlink) 
        {
            //Flush the link itself (For future learning parameters possibility)
            if ((((*curlink)->in_node)->activation_count>0))
                ((*curlink)->in_node)->flushback();
        }
    }
  else
    {
        //Flush the SENSOR
        activation_count=0;
        activation=0;
    }
  
}

// This recursively checks everything leading into and including this NNode, 
// including recurrencies
// Useful for debugging
void NNode::flushback_check(std::vector<NNode*> &seenlist)
{
  std::vector<Link*>::iterator curlink;
  
  std::vector<Link*> innodes=incoming;
  std::vector<NNode*>::iterator location;
  
  if (!(type==SENSOR))
    {
    
        if (activation_count>0)
         {
              std::cout<<"ALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
         }
    
        if (activation>0)
          {
             std::cout<<"ALERT: "<<this<<" has activation  "<<activation<<std::endl;
          }

        for(curlink=innodes.begin();curlink!=innodes.end();++curlink)
            {
                location = std::find(seenlist.begin(),seenlist.end(),((*curlink)->in_node));
                if (location==seenlist.end())
                {
                    seenlist.push_back((*curlink)->in_node);
                    ((*curlink)->in_node)->flushback_check(seenlist);
                }
            }
    
    }
  else
    {
        //Flush_check the SENSOR
        std::cout<<"sALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
        std::cout<<"sALERT: "<<this<<" has activation  "<<activation<<std::endl;
    
    if (activation_count>0)
        {
            std::cout<<"ALERT: "<<this<<" has activation count "<<activation_count<<std::endl;
        }
    
    if (activation>0)
        {
            std::cout<<"ALERT: "<<this<<" has activation  "<<activation<<std::endl;
        }

    }
  
}

// Returns the gene that created the node
NNode *NNode::get_analogue()
{
	return analogue;
}


void NNode::print_to_file(std::ostream &outFile)
{
	char tempbuf[128];
	sprintf(tempbuf, "node %d ", node_id);
	outFile << tempbuf;

	char tempbuf2[128];
	sprintf(tempbuf2, "%d %d\n", type, gen_node_label);
	outFile << tempbuf2;
}

//Find the greatest depth starting from this neuron at depth d
int NNode::depth(int d, Network *mynet)
{
  std::vector<Link*> innodes=this->incoming;
  std::vector<Link*>::iterator curlink;
  int cur_depth; //The depth of the current node
  int max=d; //The max depth

  if (d>100)
    {
        return 10;
    }

  //Base Case
  if ((this->type)==SENSOR)
    return d;
  //Recursion
  else
    {

    for(curlink=innodes.begin();curlink!=innodes.end();++curlink)
        {
            cur_depth=((*curlink)->in_node)->depth(d+1,mynet);
            if (cur_depth>max)
                 max=cur_depth;
        }
  
    return max;

  } //end else

}
