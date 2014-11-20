#include "pure_neat/gene.h"
#include "pure_neat/genome.h"

#include <iostream>
#include <sstream>
using namespace PURENEAT;

Gene::Gene(double w, NNode *inode, NNode *onode, bool recur, int innovNum, int idR)
{
  lnk = new Link(w, inode, onode, recur);
  innov innovClock;
  innovClock.gc = innovNum;
  innovClock.idR = idR;
  innovation_num = innovClock;
  
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
  int inodeIdR, inodeGc;
  int onodeIdR, onodeGc;
  NNode *inode;
  NNode *onode;
  double weight;
  int recur;
  
  std::vector<NNode*>::iterator curnode;
  
  //Get the gene parameters
  
  std::stringstream ss(argline);
  ss >>  inodeIdR >> inodeGc >> onodeIdR >> onodeGc >> weight >> recur >> innovation_num.idR >> innovation_num.gc >> enable;
  
  innov innovNodeIn, innovNodeOut;
  innovNodeIn.idR = inodeIdR;
  innovNodeIn.gc = inodeGc;

  innovNodeOut.idR = onodeIdR;
  innovNodeOut.gc = onodeGc;

  //Get a pointer to the input node
  curnode=nodes.begin();
  while(!(((*curnode)->node_id)==innovNodeIn))
    ++curnode;
  inode=(*curnode);
  
  //Get a pointer to the output node
  curnode=nodes.begin();
  while(!(((*curnode)->node_id)==innovNodeOut))
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
  outFile<<(lnk->in_node)->node_id.idR<<" ";
  outFile<<(lnk->in_node)->node_id.gc<<" ";
  outFile<<(lnk->out_node)->node_id.idR<<" ";
  outFile<<(lnk->out_node)->node_id.gc<<" ";
  outFile<<(lnk->weight)<<" ";
  outFile<<(lnk->is_recurrent)<<" ";
  outFile<<innovation_num.idR<<" ";
  outFile<<innovation_num.gc<<" ";
  outFile<<enable<<std::endl;
}
