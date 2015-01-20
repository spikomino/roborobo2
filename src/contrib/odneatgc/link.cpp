#include "odneatgc/link.h"

using namespace ODNEATGC;

Link::Link(double w,NNode *inode,NNode *onode,bool recur) {
	weight=w;
	in_node=inode;
	out_node=onode;
	is_recurrent=recur;
}

Link::Link(double w) {
	weight=w;
	in_node=out_node=0;  
	is_recurrent=false;
}

Link::Link(const Link& link)
{
	weight = link.weight;
	in_node = link.in_node;
	out_node = link.out_node;
	is_recurrent = link.is_recurrent;
}
Link::~Link()
{
    //Delete link

}
