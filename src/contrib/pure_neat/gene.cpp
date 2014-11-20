#include "pure_neat/gene.h"

#include <iostream>
#include <sstream>
using namespace PURENEAT;

Gene::Gene(double w, NNode *inode, NNode *onode, bool recur, double innov)
{
  lnk = new Link(w, inode, onode, recur);
  innovation_num = innov;
  
  enable = true;
}


Gene::Gene(Gene *g,NNode *inode,NNode *onode) 
{
  lnk=new Link((g->lnk)->weight,inode,onode,(g->lnk)->is_recurrent);
  innovation_num=g->innovation_num;
  enable=g->enable;
}

Gene::Gene(const char *argline, std::vector<NNode*> &nodes) 
{
  //Gene parameter holders
  int inodenum;
  int onodenum;
  NNode *inode;
  NNode *onode;
  double weight;
  int recur;
  
  std::vector<NNode*>::iterator curnode;
  
  //Get the gene parameters
  
  std::stringstream ss(argline);
  ss >>  inodenum >> onodenum >> weight >> recur >> innovation_num >> enable;
  
  //Get a pointer to the input node
  curnode=nodes.begin();
  while(((*curnode)->node_id)!=inodenum)
    ++curnode;
  inode=(*curnode);
  
  //Get a pointer to the output node
  curnode=nodes.begin();
  while(((*curnode)->node_id)!=onodenum)
    ++curnode;
  onode=(*curnode);
  
  lnk=new Link(weight,inode,onode,recur);
}

Gene::Gene(const Gene& gene)
{
	innovation_num = gene.innovation_num;
	enable = gene.enable;

	lnk = new Link(*gene.lnk);
}

Gene::~Gene() {
	delete lnk;
}

void Gene::print_to_file(std::ostream &outFile) 
{
  outFile<<"gene ";
  outFile<<(lnk->in_node)->node_id<<" ";
  outFile<<(lnk->out_node)->node_id<<" ";
  outFile<<(lnk->weight)<<" ";
  outFile<<(lnk->is_recurrent)<<" ";
  outFile<<innovation_num<<" ";
  outFile<<enable<<std::endl;
}
