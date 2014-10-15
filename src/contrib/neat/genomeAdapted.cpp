/*
  Extension by inheritance to default neat/genome.cpp
It includes specific GenomeAdapted id's for the current genome and its parents
This can allow to rebuild and trace back the lineage of a given genome

*/
#include "neat/genomeAdapted.h"

#include <iostream>
#include <cmath>
#include <sstream>
using namespace NEAT;


GenomeAdapted::GenomeAdapted(int num_in,int num_out,int num_hidden,int type)
  : super(num_in,num_out,num_hidden,type)
{
}
GenomeAdapted::GenomeAdapted(const GenomeAdapted& genome):super(genome)
{
  setIdTrace(genome.getIdTrace());
  setMom(genome.getMom());
  setDad(genome.getDad());
}
GenomeAdapted::GenomeAdapted(int id, std::vector<Trait*> t, std::vector<NNode*> n, std::vector<Gene*> g):super(id, t,  n, g)
{
}
GenomeAdapted::~GenomeAdapted()
{
  //Calls Genome's destructor AFTER the end of this method
}
int GenomeAdapted::getIdTrace() const
{
  return _idTrace;
}
int GenomeAdapted::getMom() const
{
  return _mom;
}
int GenomeAdapted::getDad() const
{
  return _dad;
}
    
Genome *GenomeAdapted::duplicate(int new_id)
{
	//Collections for the new Genome
	std::vector<Trait*> traits_dup;
	std::vector<NNode*> nodes_dup;
	std::vector<Gene*> genes_dup;

	//Iterators for the old Genome
	std::vector<Trait*>::iterator curtrait;
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;

	//New item pointers
	Trait *newtrait;
	NNode *newnode;
	Gene *newgene;
	Trait *assoc_trait;  //Trait associated with current item

	NNode *inode; //For forming a gene 
	NNode *onode; //For forming a gene
	Trait *traitptr;

	GenomeAdapted *newgenome;

	//verify();

	//Duplicate the traits
	for(curtrait=traits.begin();curtrait!=traits.end();++curtrait) {
		newtrait=new Trait(*curtrait);
		traits_dup.push_back(newtrait);
	}

	//Duplicate NNodes
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		//First, find the trait that this node points to
		if (((*curnode)->nodetrait)==0) assoc_trait=0;
		else {
			curtrait=traits_dup.begin();
			while(((*curtrait)->trait_id)!=(((*curnode)->nodetrait)->trait_id))
				++curtrait;
			assoc_trait=(*curtrait);
		}

		newnode=new NNode(*curnode,assoc_trait);

		(*curnode)->dup=newnode;  //Remember this node's old copy
		//    (*curnode)->activation_count=55;
		nodes_dup.push_back(newnode);    
	}

	//Duplicate Genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		//First find the nodes connected by the gene's link

		inode=(((*curgene)->lnk)->in_node)->dup;
		onode=(((*curgene)->lnk)->out_node)->dup;

		//Get a pointer to the trait expressed by this gene
		traitptr=((*curgene)->lnk)->linktrait;
		if (traitptr==0) assoc_trait=0;
		else {
			curtrait=traits_dup.begin();
			while(((*curtrait)->trait_id)!=(traitptr->trait_id))
				++curtrait;
			assoc_trait=(*curtrait);
		}

		newgene=new Gene(*curgene,assoc_trait,inode,onode);
		genes_dup.push_back(newgene);

	}

	//Finally, return the genome
	newgenome=new GenomeAdapted(new_id,traits_dup,nodes_dup,genes_dup);



	return newgenome;
 
}
// Duplicate this Genome to create a new one with the specified id 
// and give mom id value
GenomeAdapted *GenomeAdapted::duplicateAdapted(int new_id, int idTr)
{
  
  GenomeAdapted* result = (GenomeAdapted*) duplicate(new_id);
  
  
  result -> setIdTrace(idTr);
  result -> setMom(this -> getIdTrace());
  result -> setDad(-1);  
    
  return result;
  

}

void GenomeAdapted::setIdTrace(int id)
{
  _idTrace = id;
}
void GenomeAdapted::setMom(int idMom)
{
  _mom = idMom;
}
void GenomeAdapted::setDad(int idDad)
{
  _dad = idDad;
}

