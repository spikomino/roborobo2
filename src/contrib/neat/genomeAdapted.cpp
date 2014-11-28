/*
  Extension by inheritance to default neat/genome.cpp
It includes specific GenomeAdapted id's for the current genome and its parents
This can allow to rebuild and trace back the lineage of a given genome

*/
#include "neat/genomeAdapted.h"

#include <iostream>
#include <cmath>
#include <sstream>
#include <cfloat>

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


GenomeAdapted *GenomeAdapted::mutate(float sigma, 
				     int idRobot, 
				     int idNewGenome) {
  
  // duplication this into a genome to mutate
  GenomeAdapted *new_genome = this -> duplicate ();
  new_genome->_idTrace = idNewGenome;
  new_genome->_mom     = this->_idTrace ;
  new_genome->_dad     = -1;  
  
 

  //Choose the mutation depending on probabilities  
 
  /* add a node */
  if (randfloat () < NEAT::mutate_add_node_prob){
      new_genome->mut_add_node();
      //	  std::cout << "Mutation: Node added" <<std::endl; 
  }
  /* add a link */
  else if (randfloat () < NEAT::mutate_add_link_prob){
      new_genome->mut_add_link(NEAT::newlink_tries);
      //std::cout << "Mutation: Link added" <<std::endl; 
  }
  
  /* If we didn't do a structural mutation, we do the other kinds */
  else{
      if (randfloat () < NEAT::mutate_link_weights_prob){
	  new_genome->mut_link_weights(sigma);
	  //std::cout << "Mutation: link weight changed" <<std::endl; 
      }
      if (randfloat () < NEAT::mutate_toggle_enable_prob)
	  new_genome->mut_toggle_enable();
      // std::cout << "Mutation: toggle a link" <<std::endl; 
 
      if (randfloat () < NEAT::mutate_gene_reenable_prob)
	  new_genome->mut_gene_reenable();
      //std::cout << "Mutation: reenable a link" <<std::endl; 
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

// a wrapper to mutate only FFNN
GenomeAdapted *GenomeAdapted::mutate_weights(float sigma, 
				     int idRobot, 
				     int idNewGenome) {
    
    GenomeAdapted *new_genome = this -> duplicate ();
    new_genome->_idTrace = idNewGenome;
    new_genome->_mom     = this->_idTrace ;
    new_genome->_dad     = -1;  
    
    new_genome->mut_link_weights(sigma);
    return new_genome;
}

void GenomeAdapted::mut_link_weights(double sigma) {
    for(const auto& g : genes)	
	if (!g->frozen)
	    (g->lnk)->weight = (g->lnk)->weight + gaussian(0,sigma);
}


bool GenomeAdapted::mut_add_node() {
    NNode *in_node; //Here are the nodes connected by the gene
    NNode *out_node; 
    Link  *thelink;  //The link inside the random gene

    Gene  *newgene1;  //The new Genes
    Gene  *newgene2;
    NNode *newnode;   //The new NNode
    Trait *traitptr;  //The original link's trait
    
   
    double oldweight;  //The weight of the original link
    
    int trycount;  //Take a few tries to find an open node
    bool found;
    
    //First, find a random gene that is enabled and not a bias 
    trycount=0;
    found=false;
    Gene *the_gene;
    while ( trycount<20 && !found ) {
	auto it = genes.begin();
	std::advance(it, rand() % genes.size());
	if ((*it)->enable && (((*it)->lnk)->in_node)->gen_node_label !=BIAS){
	    found=true;
	    the_gene = *it;
	}
	++trycount;
    }
    
    //If we couldn't find anything so say goodbye
    if (!found) 
	return false;
    
    //Disabled the gene
    the_gene->enable=false;
    
    //Extract the link and the nodes the gene link
    thelink   = the_gene->lnk;
    oldweight = the_gene->lnk->weight;
    in_node   = thelink->in_node;
    out_node  = thelink->out_node;
    
	    
    
    //Get the old link's trait
    traitptr = thelink->linktrait;
    
    //Create the new NNode
    //By convention, it will point to the first trait
    newnode=new NNode(NEURON, _nodeId++, HIDDEN);
    newnode->nodetrait=(*(traits.begin()));
    
    //Create the new Genes
  
    newgene1 = new Gene(traitptr, 1.0, in_node, newnode, thelink->is_recurrent,
		      _innovNumber, 0);
    newgene2 = new Gene(traitptr, oldweight, newnode, out_node, false,
		      _innovNumber+1, 0);
    
    _innovNumber += 2.0;
    
    add_gene(genes,newgene1);  //Add genes in correct order
    add_gene(genes,newgene2);
    node_insert(nodes,newnode);
    
    return true;
    
} 



bool GenomeAdapted::mut_add_link(int tries) {

    int nodenum1,nodenum2;  //Random node numbers
    std::vector<NNode*>::iterator thenode1,thenode2;  //Random node iterators
    int nodecount;  //Counter for finding nodes
    int trycount; //Iterates over attempts to find an unconnected pair of nodes
    NNode *nodep1; //Pointers to the nodes
    NNode *nodep2; //Pointers to the nodes
    std::vector<Gene*>::iterator thegene; //Searches for existing link
    bool found=false;  //Tells whether an open pair was found
    std::vector<Innovation*>::iterator theinnov; //For finding a historical match
    int recurflag; //Indicates whether proposed link is recurrent
    Gene *newgene;  //The new Gene
    
    int traitnum;  //Random trait finder
    std::vector<Trait*>::iterator thetrait;
    
    double newweight;  //The new weight for the new link

    bool do_recur;
    bool loop_recur;
    int first_nonsensor;

    //These are used to avoid getting stuck in an infinite loop checking
    //for recursion
    //Note that we check for recursion to control the frequency of
    //adding recurrent links rather than to prevent any paricular
    //kind of error
    int thresh=(nodes.size())*(nodes.size());
    int count=0;

    //Make attempts to find an unconnected pair
    trycount=0;


    //Decide whether to make this recurrent
    do_recur=false;
    if (randfloat()<NEAT::recur_only_prob) 
	do_recur=true;
   

    //Find the first non-sensor so that the to-node won't look at sensors as
    //possible destinations
    first_nonsensor=0;
    thenode1=nodes.begin();
    while(((*thenode1)->get_type())==SENSOR) {
	first_nonsensor++;
	++thenode1;
    }

    //Here is the recurrent finder loop- it is done separately
    if (do_recur) {

	while(trycount<tries) {

	    //Some of the time try to make a recur loop
	    loop_recur=false;
	    if (randfloat()>0.5) 
		loop_recur=true;

	    if (loop_recur) {
		nodenum1 = randint(first_nonsensor,nodes.size()-1);
		nodenum2 = nodenum1;
	    }
	    else {
		//Choose random nodenums
		nodenum1 = randint(0,nodes.size()-1);
		nodenum2 = randint(first_nonsensor,nodes.size()-1);
	    }

	    //Find the first node
	    thenode1=nodes.begin();
	    for(nodecount=0;nodecount<nodenum1;nodecount++)
		++thenode1;

	    //Find the second node
	    thenode2=nodes.begin();
	    for(nodecount=0;nodecount<nodenum2;nodecount++)
		++thenode2;

	    nodep1=(*thenode1);
	    nodep2=(*thenode2);

	    //See if a recur link already exists  ALSO STOP AT END OF GENES!!!!
	    thegene=genes.begin();
	    while ((thegene!=genes.end()) && 
		   ((nodep2->type)!=SENSOR) &&   //Don't allow SENSORS to get input
		   (!((((*thegene)->lnk)->in_node==nodep1)&&
		      (((*thegene)->lnk)->out_node==nodep2)&&
		      ((*thegene)->lnk)->is_recurrent))) {
		++thegene;
	    }

	    if (thegene!=genes.end())
		trycount++;
	    else {
		count=0;
		recurflag=phenotype->is_recur(nodep1->analogue,
					      nodep2->analogue,
					      count, thresh);

		//ADDED: CONSIDER connections out of outputs recurrent
		if (nodep1->gen_node_label == OUTPUT)
		    recurflag=true;

		//Exit if the network is faulty (contains an infinite loop)
		//NOTE: A loop doesn't really matter
		//if (count>thresh) {
		//  cout<<"LOOP DETECTED DURING A RECURRENCY CHECK"<<std::endl;
		//  return false;
		//}

		//Make sure it finds the right kind of link (recur)
		if (!(recurflag))
		    trycount++;
		else {
		    trycount=tries;
		    found=true;
		}
	    }
	}
    }
    else {
	//Loop to find a nonrecurrent link
	while(trycount<tries) {

	    //cout<<"TRY "<<trycount<<std::endl;

	    //Choose random nodenums
	    nodenum1 = randint(0, nodes.size()-1);
	    nodenum2 = randint(first_nonsensor, nodes.size()-1);

	    //Find the first node
	    thenode1 = nodes.begin();
	    for(nodecount=0; nodecount<nodenum1; nodecount++)
		++thenode1;

	    //cout<<"RETRIEVED NODE# "<<(*thenode1)->node_id<<std::endl;

	    //Find the second node
	    thenode2 = nodes.begin();
	    for(nodecount=0; nodecount<nodenum2; nodecount++)
		++thenode2;

	    nodep1=(*thenode1);
	    nodep2=(*thenode2);

	    //See if a link already exists  ALSO STOP AT END OF GENES!!!!
	    thegene=genes.begin();
	    while ((thegene!=genes.end()) && 
		   ((nodep2->type)!=SENSOR) &&   //Don't allow SENSORS to get input
		   (!((((*thegene)->lnk)->in_node==nodep1)&&
		      (((*thegene)->lnk)->out_node==nodep2)&&
		      (!(((*thegene)->lnk)->is_recurrent))))) {
		++thegene;
	    }

	    if (thegene!=genes.end())
		trycount++;
	    else {

		count=0;
		recurflag=phenotype->is_recur(nodep1->analogue,
					       nodep2->analogue,
					       count, thresh);

		//ADDED: CONSIDER connections out of outputs recurrent
		if(nodep1->gen_node_label == OUTPUT)
		    recurflag=true;

		//Exit if the network is faulty (contains an infinite loop)
		if (count>thresh) {
		    //cout<<"LOOP DETECTED DURING A RECURRENCY CHECK"<<std::endl;
		    //return false;
		}

		//Make sure it finds the right kind of link (recur or not)
		if (recurflag)
		    trycount++;
		else {
		    trycount=tries;
		    found=true;
		}

	    }

	} //End of normal link finding loop
    }

    //Continue only if an open link was found
    if (found) {


	//If it was supposed to be recurrent, make sure it gets labeled that way
	if (do_recur)
	    recurflag=1;


	//If the phenotype does not exist, exit on false,print error
	//Note: This should never happen- if it does there is a bug
	if (phenotype==0) {
	    //cout<<"ERROR: Attempt to add link to genome with no phenotype"<<std::endl;
	    return false;
	}
	
	//Useful for debugging
	//cout<<"nodep1 id: "<<nodep1->node_id<<std::endl;
	//cout<<"nodep1: "<<nodep1<<std::endl;
	//cout<<"nodep1 analogue: "<<nodep1->analogue<<std::endl;
	//cout<<"nodep2 id: "<<nodep2->node_id<<std::endl;
	//cout<<"nodep2: "<<nodep2<<std::endl;
	//cout<<"nodep2 analogue: "<<nodep2->analogue<<std::endl;
	//cout<<"recurflag: "<<recurflag<<std::endl;
	
	//NOTE: Something like this could be used for time delays,
	//      which are not yet supported.  However, this does not
	//      have an application with recurrency.
	//If not recurrent, randomize recurrency
	//if (!recurflag) 
	//  if (randfloat()<recur_prob) recurflag=1;
	
	//Choose a random trait
	traitnum=randint(0,(traits.size())-1);
	thetrait=traits.begin();
	
	//Choose the new weight
	//newweight=(gaussrand())/1.5;  //Could use a gaussian
	newweight=randposneg()*randfloat()*1.0; //used to be 10.0
	
	//Create the new gene
	newgene=new Gene(((thetrait[traitnum])),
			 newweight,
			 nodep1,
			 nodep2,
			 recurflag,
			 _innovNumber,
			 newweight);
	
	
	
	_innovNumber = _innovNumber+1.0;
	

 
	//Now add the new Genes to the Genome
	//genes.push_back(newgene);  //Old way - could result in out-of-order innovation numbers in rtNEAT
	add_gene(genes,newgene);  //Adds the gene in correct order
	
	
	return true;
    }
    return false;
    

}
