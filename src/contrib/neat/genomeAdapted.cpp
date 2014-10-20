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

GenomeAdapted *GenomeAdapted::mutate(float sigma, int idRobot ,int newId, int &nodeId, double &innovNum) 
{
  std::map<int, GenomeAdapted*>::iterator curorg;

  GenomeAdapted *new_genome;	//For holding baby's genes

  Network *net_analogue;	//For adding link to test for recurrency

  double mut_power = NEAT::weight_mut_power;



  //NOTE: default NEAT method (randomly) replaced  by previous selection (best)
  // which has been stored in _genome
  new_genome = this -> duplicate (idRobot, newId);

  //Decide whether to mate or mutate
  //If there is only one genome in the list, then always mutate
  if ((randfloat () < NEAT::mutate_only_prob))
    {
      //Choose the mutation depending on probabilities  
      if (randfloat () < NEAT::mutate_add_node_prob)
	{
	  //08/10/14  Bogus variables for innovations deactivated 
	  //perRobot innovNum and nodeId used instead
	  std::vector < Innovation * >innovations;

	  if (new_genome->mutate_add_node (innovations, nodeId, innovNum))
	    {
	      //std::cout << "Mutate add node " << nodeId - 1 << std::endl;
	    }

	}
      else if (randfloat () < NEAT::mutate_add_link_prob)
	{
	  //Inaki Hack: generation is only used as a network_id
	  int generation = 0;
	  //No further repercusion of this parameter
	  net_analogue = new_genome->genesis (generation);

	  std::vector < Innovation * >innovations;
	  if (new_genome->mutate_add_link (innovations, innovNum,
					   NEAT::newlink_tries))
	    {
	      //std::cout << "Mutate add link" << std::endl;
	    }
	  delete net_analogue;

	}
      //NOTE:links CANNOT be added directly after a node  because the phenotype
      // will not be appropriately altered to reflect the change
      else
	{
	  //If we didn't do a structural mutation, we do the other kinds
	  if (randfloat () < NEAT::mutate_link_weights_prob)
	    {
	      //std::cout << "Mutate_link_weights" << std::endl;
	      new_genome->mutate_link_weights (mut_power, 1.0, GAUSSIAN);
	    }
	  if (randfloat () < NEAT::mutate_toggle_enable_prob)
	    {
	      //std::cout << "Mutate toggle enable" << std::endl;
	      new_genome->mutate_toggle_enable (1);
	    }
	  if (randfloat () < NEAT::mutate_gene_reenable_prob)
	    {
	      //std::cout << "Mutate gene reenable" << std::endl;
	      new_genome->mutate_gene_reenable ();
	    }
	}

    }

  return new_genome;
}

// Duplicate this Genome to create a new one with the specified id 
// and give mom id value
GenomeAdapted *GenomeAdapted::duplicate(int new_id, int idTr )
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
	newgenome -> setIdTrace(idTr);
	newgenome -> setMom(this -> getIdTrace());
	newgenome -> setDad(-1);  
		
	return newgenome;
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

