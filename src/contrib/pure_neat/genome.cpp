#include "pure_neat/genome.h"
#include "pure_neat/network.h"

#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstring>
#include <map>

using namespace PURENEAT;

Genome::Genome(int id, std::vector<NNode*> n, std::vector<Gene*> g) 
{
  genome_id=id;
  nodes=n; 
  genes=g;
  mom_id = -1;
  dad_id = -1;
}

Genome::Genome(const Genome& genome)
{
  genome_id = genome.genome_id;
  
  std::vector<NNode*>::const_iterator curnode;
  std::vector<Gene*>::const_iterator curgene;
  
  //Duplicate NNodes
  for(curnode=genome.nodes.begin();curnode!=genome.nodes.end();++curnode) 
    {
      NNode* newnode=new NNode(*curnode);
      (*curnode)->dup=newnode;  
      //Remember this node's old copy
      nodes.push_back(newnode);    
    }
  
  NNode *inode; //For forming a gene 
  NNode *onode; //For forming a gene
    
  //Duplicate Genes
  for(curgene=genome.genes.begin(); curgene!=genome.genes.end(); ++curgene) 
    {
      //First find the nodes connected by the gene's link
      
      inode=(((*curgene)->lnk)->in_node)->dup;
      onode=(((*curgene)->lnk)->out_node)->dup;
      
      Gene* newgene=new Gene(*curgene,inode,onode);
      genes.push_back(newgene);
      
    }
  mom_id = genome.mom_id;
  dad_id = genome.dad_id;
}

Genome::Genome(int id, std::ifstream &iFile) 
{

  char curword[128];  //max word size of 128 characters
  char curline[1024]; //max line size of 1024 characters
  char delimiters[] = " \n";
  
  int done=0;
  
  genome_id=id;
  
  iFile.getline(curline, sizeof(curline));
  int wordcount = getUnitCount(curline, delimiters);
  int curwordnum = 0;
  
  //Loop until file is finished, parsing each line
  while (!done) 
    {
      if (curwordnum > wordcount || wordcount == 0) 
	{
	  iFile.getline(curline, sizeof(curline));
	  wordcount = getUnitCount(curline, delimiters);
	  curwordnum = 0;
	}
      
      std::stringstream ss(curline);
      
      ss >> curword;
      
      
      //Check for end of Genome
      if (strcmp(curword,"genomeend")==0) 
	{
	  
	  ss >> curword;
	  int idcheck = atoi(curword);
	  if (idcheck!=genome_id) printf("ERROR: id mismatch in genome");
	  done=1;
	}

      //Ignore genomestart if it hasn't been gobbled yet
      else if (strcmp(curword,"genomestart")==0) 
	++curwordnum;
      //Ignore comments surrounded by - they get printed to screen
      else if (strcmp(curword,"/*")==0) 
	{
	  ss >> curword;
	  while (strcmp(curword,"*/")!=0) 
	    ss >> curword;
	}
      //Read in a node
      else if (strcmp(curword,"node")==0) 
	{
	  NNode *newnode;
	  char argline[1024];
	  curwordnum = wordcount + 1;
    
	  ss.getline(argline, 1024);
	  //Allocate the new node
	  newnode=new NNode(argline);
	  //Add the node to the list of nodes
	  nodes.push_back(newnode);
	}  
      //Read in a Gene
      else if (strcmp(curword,"gene")==0) {
	Gene *newgene;
	
	char argline[1024];
	curwordnum = wordcount + 1;
	
	ss.getline(argline, 1024);
	//Allocate the new Gene
	newgene=new Gene(argline,nodes);
	
	//Add the gene to the genome
	genes.push_back(newgene);
      }
      
    }
  
}

Genome::Genome(int num_in,int num_out) {

  //Temporary lists of nodes
  std::vector<NNode*> inputs;
  std::vector<NNode*> outputs;
  
  std::vector<NNode*>::iterator curnode1; //Node iterator1
  std::vector<NNode*>::iterator curnode2; //Node iterator2
  std::vector<NNode*>::iterator curnode3; //Node iterator3
  
  
  //For creating the new genes
  NNode *newnode;
  Gene *newgene;
  
  int count;
  int ncount;
  
  
  //Assign the id 0
  genome_id=0;
  
  //Create the inputs and outputs
  
  //Build the input nodes. Last one is bias
  for(ncount=1;ncount<=num_in;ncount++) 
    {
      if (ncount<num_in)
	newnode=new NNode(SENSOR,ncount,INPUT);
      else { 
	newnode=new NNode(SENSOR,ncount,BIAS);
      }

      //Add the node to the list of nodes
      nodes.push_back(newnode);
      inputs.push_back(newnode);
    }

  //Build the output nodes
  for(ncount=num_in+1;ncount<=num_in+num_out;ncount++) 
    {
      newnode=new NNode(NEURON,ncount,OUTPUT);
      //Add the node to the list of nodes
      nodes.push_back(newnode);
      outputs.push_back(newnode);
    }
  
  //Create the links
  //Just connect inputs straight to outputs
  
  count=1;
  //Loop over the outputs
  for(curnode1=outputs.begin();curnode1!=outputs.end();++curnode1) 
    {
      //Loop over the inputs
      for(curnode2=inputs.begin();curnode2!=inputs.end();++curnode2) 
	{
	  //Connect each input to each output
	  newgene=new Gene(0, (*curnode2), (*curnode1),false,count,0);
	  
	  //Add the gene to the genome
	  genes.push_back(newgene);	 
	  
	  count++;
	  
	}
      
    }
  mom_id = -1;
  dad_id = -1;
}

Genome::~Genome() 
{
  std::vector<NNode*>::iterator curnode;
  std::vector<Gene*>::iterator curgene;
  
  for(curnode=nodes.begin();curnode!=nodes.end();++curnode) 
    delete (*curnode);
    
  for(curgene=genes.begin();curgene!=genes.end();++curgene) 
    delete (*curgene);
 
  
}

Network *Genome::genesis(int id) 
{
  std::vector<NNode*>::iterator curnode; 
  std::vector<Gene*>::iterator curgene;
  NNode *newnode;
  
  Link *curlink;
  Link *newlink;
  
    //Inputs and outputs will be collected here for the network
  //All nodes are collected in an all_list- 
  //this will be used for later safe destruction of the net
  std::vector<NNode*> inlist;
  std::vector<NNode*> outlist;
  std::vector<NNode*> all_list;
  
  //Gene translation variables
  NNode *inode;
  NNode *onode;
  
  //The new network
  Network *newnet;
	
  //Create the nodes
  for(curnode=nodes.begin();curnode!=nodes.end();++curnode) 
    {
      newnode=new NNode((*curnode)->type,(*curnode)->node_id);
      
      //Check for input or output designation of node
      if (((*curnode)->gen_node_label)==INPUT) 
	inlist.push_back(newnode);
      if (((*curnode)->gen_node_label)==BIAS) 
	inlist.push_back(newnode);
      if (((*curnode)->gen_node_label)==OUTPUT)
	outlist.push_back(newnode);
      
      //Keep track of all nodes, not just input and output
      all_list.push_back(newnode);
      
      //Have the node specifier point to the node it generated
      (*curnode)->analogue=newnode;
      
    }
  
  //Create the links by iterating through the genes
  for(curgene=genes.begin();curgene!=genes.end();++curgene) 
    {
      //Only create the link if the gene is enabled
      if (((*curgene)->enable)==true) {
	curlink=(*curgene)->lnk;
	inode=(curlink->in_node)->analogue;
	onode=(curlink->out_node)->analogue;

	//NOTE: This line could be run through a recurrency check if desired
	newlink=new Link(curlink->weight,inode,onode,curlink->is_recurrent);
	
	(onode->incoming).push_back(newlink);
	(inode->outgoing).push_back(newlink);
	
      }
    }
  
  //Create the new network
  newnet=new Network(inlist,outlist,all_list,id);
  
  //Attach genotype and phenotype together
  newnet->genotype=this;
  phenotype=newnet;
  
  return newnet;
  
}

bool Genome::verify() 
{
  std::vector<NNode*>::iterator curnode;
  std::vector<Gene*>::iterator curgene;
  std::vector<Gene*>::iterator curgene2;
  NNode *inode;
  NNode *onode;
  
  int last_id;
  
  //Check each gene's nodes
  for(curgene=genes.begin();curgene!=genes.end();++curgene) 
    {
      inode=((*curgene)->lnk)->in_node;
      onode=((*curgene)->lnk)->out_node;
      
      //Look for inode
      curnode=nodes.begin();
      while((curnode!=nodes.end())&&
	    ((*curnode)!=inode))
	++curnode;
      
      if (curnode==nodes.end()) 
	return false;
      
      //Look for onode
      curnode=nodes.begin();
      while((curnode!=nodes.end())&&
	    ((*curnode)!=onode))
	++curnode;
      
      if (curnode==nodes.end()) 
	return false;
      
      
    }
  
  //Check for NNodes being out of order
  last_id=0;
  for(curnode=nodes.begin();curnode!=nodes.end();++curnode) 
    {
      if ((*curnode)->node_id<last_id) 
	return false;
            
      last_id=(*curnode)->node_id;
    }
  
  
  //Make sure there are no duplicate genes
  for(curgene=genes.begin();curgene!=genes.end();++curgene) 
    {
      for(curgene2=genes.begin();curgene2!=genes.end();++curgene2) 
	{
	  if (((*curgene)!=(*curgene2))&&
	      ((((*curgene)->lnk)->is_recurrent)==(((*curgene2)->lnk)->is_recurrent))&&
	      ((((((*curgene2)->lnk)->in_node)->node_id)==((((*curgene)->lnk)->in_node)->node_id))&&
	       (((((*curgene2)->lnk)->out_node)->node_id)==((((*curgene)->lnk)->out_node)->node_id)))) 
	    return false;
	}
    }

  return true;
}

void Genome::print_to_file(std::ostream &outFile)
{
  std::vector<NNode*>::iterator curnode;
  std::vector<Gene*>::iterator curgene;
  
  
  outFile<<"genomestart "<<genome_id<<std::endl;
  
  //Output the nodes
  for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
    (*curnode)->print_to_file(outFile);
  }
  
  //Output the genes
  for(curgene=genes.begin();curgene!=genes.end();++curgene) {
    (*curgene)->print_to_file(outFile);
  }
  
  
  outFile << "genomeend " << genome_id << std::endl << std::endl << std::endl;
}

void Genome::print_to_filename(char *filename) 
{
  std::ofstream oFile(filename);
  print_to_file(oFile);
  oFile.close();
}

int Genome::get_last_node_id() 
{
  return ((*(nodes.end() - 1))->node_id)+1;
}

double Genome::get_last_gene_innovnum() 
{
  return ((*(genes.end() - 1))->innovation_num)+1;
}

Genome *Genome::duplicate() 
{
  //Collections for the new Genome
  std::vector<NNode*> nodes_dup;
  std::vector<Gene*> genes_dup;
  
  //Iterators for the old Genome
  std::vector<NNode*>::iterator curnode;
  std::vector<Gene*>::iterator curgene;
  
  //New item pointers
  NNode *newnode;
  Gene *newgene;
  
  NNode *inode; //For forming a gene 
  NNode *onode; //For forming a gene
  
  Genome *newgenome;
  
  //verify();
  
  //Duplicate NNodes
  for(curnode=nodes.begin();curnode!=nodes.end();++curnode) 
    {
      
      newnode=new NNode(*curnode);
      //Remember this node's old copy
      (*curnode)->dup=newnode;  
      
      nodes_dup.push_back(newnode);    
    }

  //Duplicate Genes
  for(curgene=genes.begin();curgene!=genes.end();++curgene) {
    //First find the nodes connected by the gene's link
    inode=(((*curgene)->lnk)->in_node)->dup;
    onode=(((*curgene)->lnk)->out_node)->dup;
    
    newgene=new Gene(*curgene,inode,onode);
    genes_dup.push_back(newgene);
    
  }
  
  //Finally, return the genome
  newgenome=new Genome(this->genome_id,nodes_dup,genes_dup);
  
  newgenome->mom_id = this->mom_id;
  newgenome->dad_id = this->dad_id;
  
  return newgenome;
  
}

Genome *Genome::mutate(float sigma, int idRobot ,int idNewGenome, int &nodeId, double &innovNum)
{
  std::map<int, Genome*>::iterator curorg;

  Genome *new_genome;	//For holding baby's genes

  double mut_power = weight_mut_power;

  new_genome = this -> duplicate ();

  new_genome->genome_id = idNewGenome;
  new_genome->mom_id = this->genome_id;
  new_genome->dad_id = -1;  
  
  //Choose the mutation depending on probabilities  
  if (randfloat () < mutate_add_node_prob)
    {
      //Innovation numbers as pairs <idRobot, genecounter>
      if(!(new_genome->mutate_add_node(new_genome->node_id,new_genome->innovNumber)))
	{
	  std::cerr << "[FAILURE] No neural node added. This is not always a problem" << std::endl;
	  // exit(-1);		  
	}
    }   
  else 
    {
      if (randfloat () < mutate_add_link_prob) 
	{
	  if (!(new_genome->mutate_add_link(new_genome->innovNumber,
					    newlink_tries)))
	    {
	      std::cerr << "[FAILURE] No link added. This is not always a problem" << std::endl;
	      //exit(-1);		  
	    }
	}
      
      //NOTE:links CANNOT be added directly after a node  because the phenotype
      // will not be appropriately altered to reflect the change
      else
	{
	  //If we didn't do a structural mutation, we do the other kinds
	  if (randfloat () < mutate_link_weights_prob)
	    {
	      new_genome->mutate_link_weights (mut_power, 1.0, GAUSSIAN);
	    }
	  if (randfloat () < mutate_toggle_enable_prob)
	    {
	      new_genome->mutate_toggle_enable (1);
	    }
	  if (randfloat () < mutate_gene_reenable_prob)
	    {
	      new_genome->mutate_gene_reenable ();
	    }
	}
    }
  
  return new_genome;
}

void Genome::mutate_link_weights(double power,double rate,mutator mut_type) 
{
  std::vector<Gene*>::iterator curgene;
  double num;  //counts gene placement
  double gene_total;
  double powermod; //Modified power by gene number
  //The power of mutation will rise farther into the genome
  //on the theory that the older genes are more fit since
  //they have stood the test of time
  double randnum;
  double randchoice; //Decide what kind of mutation to do on a gene
  double endpart; //Signifies the last part of the genome
  double gausspoint;
  double coldgausspoint;
  
  bool severe;  //Once in a while really shake things up
  
  
  if (randfloat()>0.5) severe=true;
  else severe=false;
  
  //Go through all the Genes and perturb their link's weights
  num=0.0;
  gene_total=(double) genes.size();
  endpart=gene_total*0.8;
  
  powermod=1.0;
  
  
  //Loop on all genes
  for(curgene=genes.begin();curgene!=genes.end();curgene++) 
    {
      if (severe) 
	{
	  gausspoint=0.3;
	  coldgausspoint=0.1;
	}
      else if ((gene_total>=10.0)&&(num>endpart)) 
	{
	  gausspoint=0.5;  //Mutate by modification % of connections
	  coldgausspoint=0.3; //Mutate the rest by replacement % of the time
	}
      else 
	{
	  //Half the time don't do any cold mutations
	  if (randfloat()>0.5) 
	    {
	      gausspoint=1.0-rate;
	      coldgausspoint=1.0-rate-0.1;
	    }
	  else 
	    {
	      gausspoint=1.0-rate;
	      coldgausspoint=1.0-rate;
	    }
	}

      randnum=randposneg()*randfloat()*power*powermod;
      
      if (mut_type==GAUSSIAN) 
	{
	  randchoice=randfloat();
	  if (randchoice>gausspoint)
	    ((*curgene)->lnk)->weight+=randnum;
	  else if (randchoice>coldgausspoint)
	    ((*curgene)->lnk)->weight=randnum;
	}
      else if (mut_type==COLDGAUSSIAN)
	((*curgene)->lnk)->weight=randnum;
      
      //Cap the weights at 8.0 (experimental)
      if (((*curgene)->lnk)->weight > 8.0) ((*curgene)->lnk)->weight = 8.0;
      else if (((*curgene)->lnk)->weight < -8.0) ((*curgene)->lnk)->weight = -8.0;
      
      //Counter on the genes, to mutate more the genes 
      num+=1.0;
      
    } //end for loop
}

void Genome::mutate_toggle_enable(int times) 
{
	int genenum;
	int count;
	std::vector<Gene*>::iterator thegene;  //Gene to toggle
	std::vector<Gene*>::iterator checkgene;  //Gene to check
	int genecount;

	for (count=1;count<=times;count++) {

		//Choose a random genenum
		genenum=randint(0,genes.size()-1);

		//find the gene
		thegene=genes.begin();
		for(genecount=0;genecount<genenum;genecount++)
			++thegene;

		//Toggle the enable on this gene
		if (((*thegene)->enable)==true) {
			//We need to make sure that another gene connects out of the in-node
			//Because if not a section of network will break off and become isolated
			checkgene=genes.begin();
			while((checkgene!=genes.end())&&
				(((((*checkgene)->lnk)->in_node)!=(((*thegene)->lnk)->in_node))||
				(((*checkgene)->enable)==false)||
				((*checkgene)->innovation_num==(*thegene)->innovation_num)))
				++checkgene;

			//Disable the gene if it's safe to do so
			if (checkgene!=genes.end())
				(*thegene)->enable=false;
		}
		else (*thegene)->enable=true;
	}
}

void Genome::mutate_gene_reenable() {
	std::vector<Gene*>::iterator thegene;  //Gene to enable

	thegene=genes.begin();

	//Search for a disabled gene
	while((thegene!=genes.end())&&((*thegene)->enable==true))
		++thegene;

	//Reenable it
	if (thegene!=genes.end())
		if (((*thegene)->enable)==false) (*thegene)->enable=true;

}

bool Genome::mutate_add_node(int &curnode_id,double &curinnov) 
{
  std::vector<Gene*>::iterator thegene;  //random gene containing the original link
  int genenum;  //The random gene number
  NNode *in_node; //Here are the nodes connected by the gene
  NNode *out_node; 
  Link *thelink;  //The link inside the random gene
  
  Gene *newgene1;  //The new Genes
  Gene *newgene2;
  NNode *newnode;   //The new NNode
  
  double oldweight;  //The weight of the original link
  
  int trycount;  //Take a few tries to find an open node
  bool found;
  
  //First, find a random gene already in the genome  
  trycount=0;
  found=false;
  
  
  //Random uniform choice of genes
  while ((trycount<20)&&(!found)) 
    {
    
    //This old totally random selection is bad- splitting
    //inside something recently splitted adds little power
    //to the system (should use a gaussian if doing it this way)
    genenum=randint(0,genes.size()-1);
    
    //find the gene
    thegene=genes.begin();
    for(int genecount=0;genecount<genenum;genecount++)
      ++thegene;
	  
    //If either the gene is disabled, or it has a bias input, try again
    if (!(((*thegene)->enable==false)||
	  (((((*thegene)->lnk)->in_node)->gen_node_label)==BIAS)))
      found=true;
	  
    ++trycount;
    
  }

  //If we couldn't find anything so say goodbye
  if (!found) 
    return false;

  //Disabled the gene
  (*thegene)->enable=false;

  //Extract the link
  thelink=(*thegene)->lnk;
  oldweight=(*thegene)->lnk->weight;
  
  //Extract the nodes
  in_node=thelink->in_node;
  out_node=thelink->out_node;
  
  //Create the new NNode
  newnode=new NNode(NEURON,curnode_id++,HIDDEN);
  
  //Create the new Genes
  if (thelink->is_recurrent) 
    {
      newgene1=new Gene(1.0,in_node,newnode,true,curinnov,0);
      newgene2=new Gene(oldweight,newnode,out_node,false,curinnov+1,0);
      curinnov+=2.0;
    }
  else {
    newgene1=new Gene(1.0,in_node,newnode,false,curinnov,0);
    newgene2=new Gene(oldweight,newnode,out_node,false,curinnov+1,0);
    curinnov+=2.0;
  }

   //Now add the new NNode and new Genes to the Genome
  //Add genes in correct order
  add_gene(genes,newgene1);  
  
  add_gene(genes,newgene2);
    
  node_insert(nodes,newnode);


  return true;

} 

bool Genome::mutate_add_link(double &curinnov,int tries) 
{
  int nodenum1,nodenum2;  //Random node numbers
  std::vector<NNode*>::iterator thenode1,thenode2;  //Random node iterators
  int nodecount;  //Counter for finding nodes
  int trycount; //Iterates over attempts to find an unconnected pair of nodes
  NNode *nodep1; //Pointers to the nodes
  NNode *nodep2; //Pointers to the nodes
  std::vector<Gene*>::iterator thegene; //Searches for existing link
  bool found=false;  //Tells whether an open pair was found
  
  int recurflag; //Indicates whether proposed link is recurrent
  Gene *newgene;  //The new Gene
  
  double newweight;  //The new weight for the new link
  
  bool done;
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
  if (randfloat() < recur_only_prob) 
    do_recur=true;
  else do_recur=false;
  
  //Find the first non-sensor so that the to-node won't look at sensors as
  //possible destinations
  first_nonsensor=0;
  thenode1=nodes.begin();
  while(((*thenode1)->get_type())==SENSOR) 
    {
      first_nonsensor++;
      ++thenode1;
    }
  
  //Here is the recurrent finder loop- it is done separately
  if (do_recur) 
    {
    
      while(trycount<tries) 
	{
	  //Some of the time try to make a recur loop
	  if (randfloat()>0.5) 
	    {
	      loop_recur=true;
	    }
	  else 
	    loop_recur=false;
      
	  if (loop_recur) 
	    {
	      nodenum1=randint(first_nonsensor,nodes.size()-1);
	      nodenum2=nodenum1;
	    }
	  else 
	    {
	      //Choose random nodenums
	      nodenum1=randint(0,nodes.size()-1);
	      nodenum2=randint(first_nonsensor,nodes.size()-1);
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
		    ((*thegene)->lnk)->is_recurrent))) 
	    ++thegene;
	    
      
	  if (thegene!=genes.end())
	    trycount++;
	  else 
	    {
	      count=0;
	      recurflag=phenotype->is_recur(nodep1->analogue,nodep2->analogue,count,thresh);
	
	      //ADDED: CONSIDER connections out of outputs recurrent
          if ((nodep1->gen_node_label)==OUTPUT)
		recurflag=true;
	
	      //Make sure it finds the right kind of link (recur)
	      if (!(recurflag))
		trycount++;
	      else 
		{
		  trycount=tries;
		  found=true;
		}
	      
	    }
	  
	}
    }
  else 
    {
      //Loop to find a nonrecurrent link
      while(trycount<tries) 
	{
	  //Choose random nodenums
	  nodenum1=randint(0,nodes.size()-1);
	  nodenum2=randint(first_nonsensor,nodes.size()-1);
	  
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
	  
	  //See if a link already exists  ALSO STOP AT END OF GENES!!!!
	  thegene=genes.begin();
	  while ((thegene!=genes.end()) && 
		 ((nodep2->type)!=SENSOR) &&   //Don't allow SENSORS to get input
		 (!((((*thegene)->lnk)->in_node==nodep1)&&
		    (((*thegene)->lnk)->out_node==nodep2)&&
		    (!(((*thegene)->lnk)->is_recurrent))))) 
	    ++thegene;
	  
	  
	  if (thegene!=genes.end())
	    trycount++;
	  else 
	    {
	      count=0;
	      recurflag=phenotype->is_recur(nodep1->analogue,nodep2->analogue,count,thresh);
	      
	      //ADDED: CONSIDER connections out of outputs recurrent
          if ((nodep1->gen_node_label)==OUTPUT)
		recurflag=true;
	      
	      //Make sure it finds the right kind of link (recur or not)
	      if (recurflag)
		trycount++;
	      else 
		{
		  trycount=tries;
		  found=true;
		}
	      
	    }
      
	} //End of normal link finding loop
    }
  
  //Continue only if an open link was found
  if (found) 
    {
      //TODELETE
      std::cout << "ADDED GENE" << std::endl;
      //If it was supposed to be recurrent, make sure it gets labeled that way
      if (do_recur) recurflag=1;
      
      done=false;
      
      while(!done) 
	{
	  //If the phenotype does not exist, exit on false,print error
	  //Note: This should never happen- if it does there is a bug
	  if (phenotype==0) 
	    return false;
	    	
	  //Choose the new weight
	  newweight=randposneg()*randfloat()*1.0; 
	  
	  //Create the new gene
	newgene=new Gene(newweight,nodep1,nodep2,recurflag,curinnov,newweight);
	curinnov=curinnov+1.0;
	
	done=true;
      }
    //Now add the new Genes to the Genom in correct order
    add_gene(genes,newgene);  
    return true;
    }
  else 
    return false;
}


//Adds a new gene that has been created through a mutation in the
//*correct order* into the list of genes in the genome
void Genome::add_gene(std::vector<Gene*> &glist,Gene *g) 
{
  std::vector<Gene*>::iterator curgene;

  double inum=g->innovation_num;
  
  curgene=glist.begin();
  
  while ((curgene!=glist.end())&&
	 (((*curgene)->innovation_num)<inum)) 
    ++curgene;
  
  glist.insert(curgene,g);

}


void Genome::node_insert(std::vector<NNode*> &nlist,NNode *n) 
{
  std::vector<NNode*>::iterator curnode;
  int id=n->node_id;
  
  curnode=nlist.begin();
  while ((curnode!=nlist.end())&&
	 (((*curnode)->node_id)<id)) 
    ++curnode;
  
  nlist.insert(curnode,n);
  
}

Genome *Genome::mate_multipoint(Genome *g,int genomeid,double fitness1,double fitness2) 
{
  //The baby Genome will contain these new NNodes and Genes
  std::vector<NNode*> newnodes;   
  std::vector<Gene*> newgenes;    
  Genome *new_genome;
  
  //Checks for link duplication
  std::vector<Gene*>::iterator curgene2;  
  
  //Iterators for moving through the two parents' genes
  std::vector<Gene*>::iterator p1gene;
  std::vector<Gene*>::iterator p2gene;
  
  //Innovation numbers for genes inside parents' Genomes
  double p1innov;  
  double p2innov;

  Gene *chosengene = NULL;  //Gene chosen for baby to inherit

//NNodes connected to the chosen Gene
  NNode *inode;  
  NNode *onode;
  NNode *new_inode;
  NNode *new_onode;

  //For checking if NNodes exist already 
  std::vector<NNode*>::iterator curnode;  

  //Set to true if we want to disabled a chosen gene
  bool disable;  

  disable=false;
  Gene *newgene;

  //Tells if the first genome (this one) has better fitness or not
  bool p1better; 

  bool skip;

  //Figure out which genome is better
  //The worse genome should not be allowed to add extra structural baggage
  //If they are the same, use the smaller one's disjoint and excess genes only
  if (fitness1>fitness2) 
    p1better=true;
  else if (fitness1==fitness2) 
    {
      if (genes.size()<(g->genes.size()))
	p1better=true;
      else p1better=false;
    }
  else 
    p1better=false;
	
  //Make sure all sensors and outputs are included
  for(curnode=(g->nodes).begin();curnode!=(g->nodes).end();++curnode) 
    {
      if ((((*curnode)->gen_node_label)==INPUT)||
	  (((*curnode)->gen_node_label)==BIAS)||
	  (((*curnode)->gen_node_label)==OUTPUT)) 
	{
	  //Create a new node off the sensor or output
	  new_onode=new NNode((*curnode));
	  
	  //Add the new node
	  node_insert(newnodes,new_onode);
	  
	}
      
    }

  //Now move through the Genes of each parent until both genomes end
  p1gene=genes.begin();
  p2gene=(g->genes).begin();
  while(!((p1gene==genes.end())&&
	  (p2gene==(g->genes).end()))) 
    {
      //Default to not skipping a chosen gene
      skip=false;  
	  
      if (p1gene==genes.end()) 
	{
	  chosengene=*p2gene;
	  ++p2gene;
	  //Skip excess from the worse genome
	  if (p1better) skip=true;  
	}
      else if (p2gene==(g->genes).end()) 
	{
	  chosengene=*p1gene;
	  ++p1gene;
	  //Skip excess from the worse genome
	  if (!p1better) skip=true; 
	}
      else 
	{
	  //Extract current innovation numbers
	  p1innov=(*p1gene)->innovation_num;
	  p2innov=(*p2gene)->innovation_num;
	  
	  if (p1innov==p2innov) 
	    {
	      if (randfloat()<0.5) 
		{
		  chosengene=*p1gene;
		}
	      else 
		{
		  chosengene=*p2gene;
		}
	      
	      //If one is disabled, the corresponding gene in the offspring
	      //will likely be disabled
	      if ((((*p1gene)->enable)==false)||
		  (((*p2gene)->enable)==false)) 
		if (randfloat()<0.75) disable=true;
	      
	      ++p1gene;
	      ++p2gene;
	      
	    }
	    else if (p1innov<p2innov) 
	      {
		chosengene=*p1gene;
		++p1gene;
		
		if (!p1better) skip=true;
	      }
	    else if (p2innov<p1innov) 
	      {
		chosengene=*p2gene;
		++p2gene;
		if (p1better) skip=true;
	      }
	}
	  
	  
      //Check to see if the chosengene conflicts with an already chosen gene
      //i.e. do they represent the same link    
      curgene2=newgenes.begin();
      while ((curgene2!=newgenes.end())&&
	     (!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
		(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&((((*curgene2)->lnk)->is_recurrent)== (((chosengene)->lnk)->is_recurrent)) ))&&
	     (!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&
		(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
		(!((((*curgene2)->lnk)->is_recurrent)))&&
		(!((((chosengene)->lnk)->is_recurrent))) )))
	++curgene2;
		  
      //Links conflicts, abort adding
      if (curgene2!=newgenes.end()) skip=true;  
      
      if (!skip) 
	{
	  //Now add the chosengene to the baby
	  
	  //Next check for the nodes, add them if not in the baby Genome already
	  inode=(chosengene->lnk)->in_node;
	  onode=(chosengene->lnk)->out_node;
	    
	    //Check for inode in the newnodes list
	    if (inode->node_id<onode->node_id) 
	      {
		//inode before onode
	      
		//Checking for inode's existence
		curnode=newnodes.begin();
		while((curnode!=newnodes.end())&&
		      ((*curnode)->node_id!=inode->node_id)) 
		  ++curnode;
	      
	      if (curnode==newnodes.end()) 
		{
		  //Here we know the node doesn't exist so we have to add it
		  new_inode=new NNode(inode);
		  node_insert(newnodes,new_inode);
		}
	      else 
		{
		  new_inode=(*curnode);
		}

	      //Checking for onode's existence
	      curnode=newnodes.begin();
	      while((curnode!=newnodes.end())&&
		    ((*curnode)->node_id!=onode->node_id)) 
		++curnode;
	      if (curnode==newnodes.end()) 
		{
		  //Here we know the node doesn't exist so we have to add it
		  new_onode=new NNode(onode);
		  
		  node_insert(newnodes,new_onode);
		}
	      else 
		{
		  new_onode=(*curnode);
		}
	      
	      }
	    //If the onode has a higher id than the inode we want to add it first
	    else 
	      {
		//Checking for onode's existence
		curnode=newnodes.begin();
		while((curnode!=newnodes.end())&&
		      ((*curnode)->node_id!=onode->node_id)) 
		  ++curnode;
		if (curnode==newnodes.end()) 
		  {
		    //Here we know the node doesn't exist so we have to add it
		    new_onode=new NNode(onode);

		    node_insert(newnodes,new_onode);
		  }
		else 
		  {
		    new_onode=(*curnode);
		    
		  }
	      
		//Checking for inode's existence
		curnode=newnodes.begin();
		while((curnode!=newnodes.end())&&
		      ((*curnode)->node_id!=inode->node_id)) 
		  ++curnode;
		if (curnode==newnodes.end()) 
		  {
		    //Here we know the node doesn't exist so we have to add it
		    new_inode=new NNode(inode);
		    
		    node_insert(newnodes,new_inode);
		  }
		else 
		  {
		    new_inode=(*curnode);
		    
		  }
	      } //End NNode checking section- NNodes are now in new Genome
	    
	    //Add the Gene
	    
	    newgene=new Gene(chosengene,new_inode,new_onode);
	    if (disable) 
	      {
		newgene->enable=false;
		disable=false;
	      }
	    newgenes.push_back(newgene);
	}
      
    }
	
  new_genome=new Genome(genomeid,newnodes,newgenes);
  
  //Return the baby Genome
  return (new_genome);
  
}

int Genome::extrons() 
{
  std::vector<Gene*>::iterator curgene;
  int total=0;
  
  for(curgene=genes.begin();curgene!=genes.end();curgene++) 
    if ((*curgene)->enable) 
      ++total;
    
  return total;
}

void print_Genome_tofile(Genome *g,const char *filename) 
{
  std::string file = "";
  file += filename;
  
  std::ofstream oFile(file.c_str());
  
  g->print_to_file(oFile);
  
  oFile.close();
}

//Random Functions 
int randposneg() 
{
  if (rand()%2) 
    return 1; 
  else 
    return -1;
}
    
int randint(int x,int y) 
{
  return rand()%(y-x+1)+x;
}

double randfloat() 
{
  return rand() / (double) RAND_MAX;        
}
double gaussrand() 
{
  static int iset=0;
  static double gset;
  double fac,rsq,v1,v2;
  
  if (iset==0) 
    {
      do 
	{
	  v1=2.0*(randfloat())-1.0;
	  v2=2.0*(randfloat())-1.0;
	  rsq=v1*v1+v2*v2;
	} while (rsq>=1.0 || rsq==0.0);
      fac=sqrt(-2.0*log(rsq)/rsq);
      gset=v1*fac;
      iset=1;
      return v2*fac;
    }
  else 
    {
      iset=0;
      return gset;
    }
}

double fsigmoid(double activesum,double slope) {
	//NON-SHIFTED STEEPENED
	return (1/(1+(exp(-(slope*activesum))))); 
}

int getUnitCount(const char *string, const char *set)
{
  int count = 0;
  short last = 0;
  while(*string)
    {
      last = *string++;
      
      for(int i =0; set[i]; i++)
	{
	  if(last == set[i])
	    {
	      count++;
	      last = 0;
	      break;
	    }   
	}
    }
  if(last)
    count++;
  return count;
} 
  

bool load_neat_params(const char *filename, bool output) 
{
std::ifstream paramFile(filename);

if(!paramFile) {
return false;
}
char curword[128];

// **********LOAD IN PARAMETERS*************** //
if(output)
printf("NEAT READING IN %s", filename);

 paramFile>>curword;
 paramFile>>weight_mut_power;

 paramFile>>curword;
 paramFile>>recur_prob;

 paramFile>>curword;
 paramFile>>mutate_only_prob;
 
 paramFile>>curword;
 paramFile>>mutate_link_weights_prob;
 
 paramFile>>curword;
 paramFile>>mutate_toggle_enable_prob;
 
 paramFile>>curword;
 paramFile>>mutate_gene_reenable_prob;
 
 paramFile>>curword;
 paramFile>>mutate_add_node_prob;
 
 paramFile>>curword;
 paramFile>>mutate_add_link_prob;
 	
 paramFile>>curword;
 paramFile>>mate_multipoint_prob;
 
 paramFile>>curword;
 paramFile>>mate_only_prob;
 
 paramFile>>curword;
 paramFile>>recur_only_prob;
 
 paramFile>>curword;
 paramFile>>newlink_tries;
 
 
 if(output) 
   {
     printf("weight_mut_power=%f\n",weight_mut_power);
     printf("recur_prob=%f\n",recur_prob);
     printf("mutate_only_prob=%f\n",mutate_only_prob);
     printf("mutate_link_weights_prob=%f\n",mutate_link_weights_prob);
     printf("mutate_toggle_enable_prob=%f\n",mutate_toggle_enable_prob);
     printf("mutate_gene_reenable_prob=%f\n",mutate_gene_reenable_prob);
     printf("mutate_add_node_prob=%f\n",mutate_add_node_prob);
     printf("mutate_add_link_prob=%f\n",mutate_add_link_prob);
     printf("mate_multipoint_prob=%f\n",mate_multipoint_prob);
     printf("mate_only_prob=%f\n",mate_only_prob);
     printf("recur_only_prob=%f\n",recur_only_prob);
     printf("newlink_tries=%d\n",newlink_tries);
   }
	paramFile.close();
	return true;
}


double     weight_mut_power = 0; // The power of a linkweight mutation 
double     recur_prob = 0; // Prob. that a link mutation which doesn't have to be recurrent will be made recurrent 
double     mutate_only_prob = 0; // Prob. of a non-mating reproduction 
double     mutate_link_weights_prob = 0;
double     mutate_toggle_enable_prob = 0;
double     mutate_gene_reenable_prob = 0;
double     mutate_add_node_prob = 0;
double     mutate_add_link_prob = 0;
double     mate_multipoint_prob = 0;     
double     mate_only_prob = 0; // Prob. of mating without mutation 
double     recur_only_prob = 0;  // Probability of forcing selection of ONLY links that are naturally recurrent 
int     newlink_tries = 0;  // Number of tries mutate_add_link will attempt to find an open link 
