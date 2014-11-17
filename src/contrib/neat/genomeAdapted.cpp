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


GenomeAdapted::GenomeAdapted(int num_in,
			     int num_out,
			     int num_hidden,
			     int type) : super(num_in,
					       num_out,
					       num_hidden,
					       type){
}

GenomeAdapted::GenomeAdapted(const GenomeAdapted& genome):super(genome){
    _idTrace     = genome.getIdTrace();
    _mom         = genome.getMom();
    _dad         = genome.getDad();
    _nodeId      = genome.getNodeId();
    _innovNumber = genome.getInnovNumber();
}

GenomeAdapted::GenomeAdapted(int id, 
			     std::vector<Trait*> t, 
			     std::vector<NNode*> n, 
			     std::vector<Gene*> g) : super(id, t,  n, g){
}

GenomeAdapted::~GenomeAdapted() {
  //Calls Genome's destructor AFTER the end of this method
}





// Duplicate this Genome to create a new one with the specified id 
GenomeAdapted *GenomeAdapted::duplicate(){
    
    //Collections for the new Genome
    std::vector<Trait*> traits_dup;
    std::vector<NNode*> nodes_dup;
    std::vector<Gene*>  genes_dup;
    
    //Iterators for the old Genome
    std::vector<Trait*>::iterator curtrait;
    std::vector<NNode*>::iterator curnode;
    std::vector<Gene*>::iterator  curgene;
    
    //New item pointers
    Trait *newtrait;
    NNode *newnode;
    Gene  *newgene;
    Trait *assoc_trait;  //Trait associated with current item
    
    NNode *inode; //For forming a gene 
    NNode *onode; //For forming a gene
    Trait *traitptr;
  
    //Duplicate the traits
    for(curtrait=traits.begin(); curtrait!=traits.end(); ++curtrait) {
	newtrait=new Trait(*curtrait);
	traits_dup.push_back(newtrait);
    }

    //Duplicate NNodes
    for(curnode=nodes.begin(); curnode!=nodes.end(); ++curnode) {
	//First, find the trait that this node points to
	if (((*curnode)->nodetrait)==0)
	    assoc_trait=0;
	else {
	    curtrait=traits_dup.begin();
	    while(((*curtrait)->trait_id)!=(((*curnode)->nodetrait)->trait_id))
		++curtrait;
	    assoc_trait=(*curtrait);
	}
	newnode=new NNode(*curnode,assoc_trait);
	(*curnode)->dup=newnode;  //Remember this node's old copy
	nodes_dup.push_back(newnode);    
    }
    
    //Duplicate Genes
    for(curgene=genes.begin(); curgene!=genes.end(); ++curgene) {
	//First find the nodes connected by the gene's link	
	inode=(((*curgene)->lnk)->in_node)->dup;
	onode=(((*curgene)->lnk)->out_node)->dup;
	
	//Get a pointer to the trait expressed by this gene
	traitptr=((*curgene)->lnk)->linktrait;
	if (traitptr==0) 
	    assoc_trait=0;
	else {
	    curtrait=traits_dup.begin();
	    while(((*curtrait)->trait_id)!=(traitptr->trait_id))
		++curtrait;
	    assoc_trait=(*curtrait);
	}
	newgene=new Gene(*curgene,assoc_trait,inode,onode);
	genes_dup.push_back(newgene);
    }

    // Finally, create a copy of the genome
    GenomeAdapted *newgenome = new GenomeAdapted(genome_id, 
						 traits_dup, 
						 nodes_dup, 
						 genes_dup);
    
    newgenome->_idTrace     = _idTrace;
    newgenome->_mom         = _mom;
    newgenome->_dad         = _dad;
    newgenome->_nodeId      = _nodeId;
    newgenome->_innovNumber = _innovNumber;
    
    return newgenome;
}

GenomeAdapted* GenomeAdapted::mutate(float sigma, int idRobot, int newId){

    return mutate(sigma,idRobot, newId, _nodeId, _innovNumber);

}

GenomeAdapted *GenomeAdapted::mutate(float sigma, 
				     int idRobot, 
				     int idNewGenome, 
				     int &nodeId, 
				     double &innovNum) {

  


  // duplication this into a genome to mutate
  GenomeAdapted *new_genome = this -> duplicate ();

  new_genome->_idTrace = idNewGenome;
  new_genome->_mom     = this->_idTrace ;
  new_genome->_dad     = -1;  
  
  //Decide whether to mate or mutate
  //If there is only one genome in the list, then always mutate
  
  //if(randfloat() < NEAT::mutate_only_prob){
  //Choose the mutation depending on probabilities  
 
  /* add a node */
  if (randfloat () < NEAT::mutate_add_node_prob){
      //08/10/14  Bogus variables for innovations deactivated 
      //perRobot innovNum and nodeId used instead
      std::vector < Innovation * > innovations;
      
      if (new_genome->mutate_add_node (innovations,
				       new_genome->_nodeId,
				       new_genome->_innovNumber))
	  
	  std::cout << "Mutation: Node added" <<std::endl; 
  }   
  else if (randfloat () < NEAT::mutate_add_link_prob) {
      
      std::vector < Innovation * >innovations;
    
      if (new_genome->mutate_add_link(innovations,
				      new_genome->_innovNumber,
				      NEAT::newlink_tries))
	  
	  std::cout << "Mutation: Link added" <<std::endl; 
  }
  
      //NOTE:links CANNOT be added directly after a node  because the phenotype
      // will not be appropriately altered to reflect the change
  else{
      //If we didn't do a structural mutation, we do the other kinds
      if (randfloat () < NEAT::mutate_link_weights_prob){
	  new_genome->mut_link_weights (sigma);
	  std::cout << "Mutation: link weight changed" <<std::endl; 
      }
      if (randfloat () < NEAT::mutate_toggle_enable_prob){
	  new_genome->mut_toggle_enable ();
	  std::cout << "Mutation: toggle a link" <<std::endl; 
      }
      if (randfloat () < NEAT::mutate_gene_reenable_prob){
	  new_genome->mut_gene_reenable ();
	  std::cout << "Mutation: reenable a link" <<std::endl; 
      }
  }
  
  
  return new_genome;
}


// Reanable a random gene
bool GenomeAdapted::mut_gene_reenable() {

    /* list all disabled genes */
    std::vector<Gene*> disabled; 
    for(const auto& g : genes)	
	if(! g->enable)
	    disabled.push_back(g);

    /* reenable a random gene  if possible */
    if(disabled.size() > 0){
	auto it = disabled.begin();
	std::advance(it, rand() % disabled.size());
	(*it)->enable = true ;
	return true;
    }
    return false;
}

bool GenomeAdapted::mut_toggle_enable() {
    /* choose a random gene  */
    auto it = genes.begin();
    std::advance(it, rand() % genes.size());
   
    /* Toggle the enable on this gene */
    if ( ! (*it)->enable ){
	(*it)->enable = true ;
	return true;
    }
    else {
	//We need to make sure that another gene connects out of the in-node
	//Because if not a section of network will break off and become isolated

	std::vector<Gene*>::iterator checkgene = genes.begin();
	while( checkgene!=genes.end() &&
	       ((((*checkgene)->lnk)->in_node != ((*it)->lnk)->in_node) ||
		(*checkgene)->enable ==false ||
		((*checkgene)->innovation_num == (*it)->innovation_num)))
	    ++checkgene;
	
	//Disable the gene if it's safe to do so
	if (checkgene!=genes.end()){
	    (*it)->enable=false;
	    return true;
	}
    }
    return false;
}

// gaussian random adapted from: 
// http://www.design.caltech.edu/erik/Misc/Gaussian.html
double gaussian(double m, double s) {
    /* mean m, standard deviation s */
    double x1, x2, w, y1;
    static double y2;
    static int use_last = 0;
    
    if (use_last){		        /* use value from previous call */
	y1 = y2;
	use_last = 0;
    }
    else{
	do {
	    // Uniform from interval [0,1) 
	    //  ((double)rand()/(1.0+(double)RAND_MAX)) 
	
	    x1 = 2.0 * ((double)rand()/(1.0+(double)RAND_MAX))  - 1.0;
	    x2 = 2.0 * ((double)rand()/(1.0+(double)RAND_MAX))  - 1.0;
	    w = x1 * x1 + x2 * x2;
	} while ( ( w >= 1.0 ) || ( w == 0.0 ) );
	
	w = sqrt( (-2.0 * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;
	use_last = 1;
    }
    
    return( m + y1 * s );
}


void GenomeAdapted::mut_link_weights(double sigma) {
    double _maxValue =  1.0;
    double _minValue = -1.0; 

    for(const auto& g : genes){		
	if (!g->frozen){
	    double value = (g->lnk)->weight + gaussian(0,sigma);
	    
	    // bouncing upper/lower bounds
	    if(value < _minValue){
		double range = _maxValue - _minValue;
		double overflow = - ( (double)value - _minValue );
		overflow = overflow - 2*range * (int)( overflow / (2*range) );
		if ( overflow < range )
		    value = _minValue + overflow;
		else // overflow btw range and range*2
		    value = _minValue + range - (overflow-range);
	    }
	    else if ( value > _maxValue ){
		double range = _maxValue - _minValue;
		double overflow = (double)value - _maxValue;
		overflow = overflow - 2*range * (int)( overflow / (2*range) );
		if ( overflow < range )
		    value = _maxValue - overflow;
		else // overflow btw range and range*2
		    value = _maxValue - range + (overflow-range);
	    }
     
	    (g->lnk)->weight = value;
	}
    }



}
