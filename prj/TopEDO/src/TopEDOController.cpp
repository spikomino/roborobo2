/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "TopEDO/include/TopEDOController.h"
#include "TopEDO/include/TopEDOWorldObserver.h"

#include "World/World.h"
#include "Utilities/Misc.h"
#include <math.h>
#include <string>

#include <neuralnetworks/MLP.h>
#include <neuralnetworks/Perceptron.h>
#include <neuralnetworks/Elman.h>

#include <neat/genome.h>
#include "neat/genomeAdapted.h"

using namespace Neural;
using namespace NEAT;

TopEDOController::TopEDOController( RobotWorldModel *wm )
{
  
  _wm = wm;
  
  nn = NULL;
  
  // evolutionary engine
  
  _minValue = -1.0;
  _maxValue = 1.0;
  
  _currentSigma = TopEDOSharedData::gSigmaRef;


  NEAT::load_neat_params("src/contrib/neat/p2nv.ne",false);

  
  resetRobot();

  
  // behaviour
  
  _iteration = 0;
  
  _birthdate = 0;
  
  
  _wm->updateLandmarkSensor();
  
  _wm->setAlive(true);
  _wm->setRobotLED_colorValues(255, 0, 0);
 
}

TopEDOController::~TopEDOController()
{

  delete nn;
  nn = NULL;
}

void TopEDOController::reset()
{
  _currentFitness = 0.0;
  _behavior.clear();
}

void TopEDOController::resetRobot()
{
  _nbInputs = 1; // Bias constant input (1.0)


  if ( gExtendedSensoryInputs ) 
    {
       _nbInputs += ( 1 ) * _wm->_cameraSensorsNb; // CoupledSwitch 
    }
        
  _nbInputs += _wm->_cameraSensorsNb; // proximity sensors
    

  _nbOutputs = 2;


  // Inputs, outputs, 0 hidden neurons, fully connected. Start with Simple Perceptron 
 _genome = new GenomeAdapted(_nbInputs,_nbOutputs,0,0); 

 _genome->setIdTrace(_wm->getId());
 _genome->setMom(-1);
 _genome->setDad(-1);
 _genome->genome_id = _wm->getId();
 
  _genome->mutate_link_weights(1.0,1.0,COLDGAUSSIAN);
  

  createNN();



  std::string filename = "logs/net" + std::to_string(gWorld->getIterations()) + "-" + std::to_string(_wm -> getId()) + ".dot";
  nn->drawNetGraphViz(filename);  
  if ( gVerbose )
    std::cout << std::flush ;


  
  setNewGenomeStatus(true);
  _genomesList.clear();
  _fitnessList.clear();
  

  //TOFIX NEAT-like innovation number and node id FOR THIS ROBOT
  innovNum = (double) nn->linkcount();
  nodeId = 1 + _nbInputs + _nbOutputs;

  //std::cout << "NbIn: " << _nbInputs << " - NbOut: " << _nbOutputs << " - NbGenes: " << nn->linkcount() << std::endl ;
  
}



void TopEDOController::step()
{
  _iteration++;
  stepEvolution();
 
  stepBehaviour();
 
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void TopEDOController::stepBehaviour()
{

  // ---- Build inputs ----
  
  std::vector<double>* inputs = new std::vector<double>(_nbInputs);
  int inputToUse = 0;
 
  (*inputs)[inputToUse++] = 1.0;

  // distance sensors
  for(int i  = 0; i < _wm->_cameraSensorsNb; i++)
    {
      (*inputs)[inputToUse] = _wm->getDistanceValueFromCameraSensor(i) / _wm->getCameraSensorMaximumDistanceValue(i);
      inputToUse++;
      
      if ( gExtendedSensoryInputs ) 
        {
	  int objectId = _wm->getObjectIdFromCameraSensor(i);
          
	  // input: physical object? which type?
            if ( PhysicalObject::isInstanceOf(objectId) )
	      {
		int nbOfTypes = 5;//PhysicalObjectFactory::getNbOfTypes();
		 for ( int i = 4 ; i != nbOfTypes ; i++ )
		  {
                    if (i  == ( gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset]->getType()))
		      {
			(*inputs)[inputToUse] = 1; // match
		      }
                    else
		      (*inputs)[inputToUse] = 0;
		      inputToUse++;
		  }
	      }
            else
	      {
                // not a physical object.Should still fill in the inputs (with zeroes)
		int nbOfTypes = 5;
		//int nbOfTypes = PhysicalObjectFactory::getNbOfTypes();
                for ( int i = 4 ; i != nbOfTypes ; i++ )
		  {
                    (*inputs)[inputToUse] = 0;
		      inputToUse++;
		  }
	      }
	    
	}
    }
    

    
    // ---- compute and read out ----
    
    
  nn->load_sensors((&(*inputs)[0]));
        
  if (!(nn->activate())) 
    {
      std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
      exit(-1);
    } 
  std::vector<double> outputs;
  for(std::vector<NNode*>::iterator out_iter=nn->outputs.begin(); out_iter != nn->outputs.end() ; out_iter++)
    {
      outputs.push_back((*out_iter)->activation);
    }

  _wm->_desiredTranslationalValue = outputs[0];
  //Rotational velocity in [-1,+1]
  _wm->_desiredRotationalVelocity = 2*(outputs[1] - 0.5); 
  
  // normalize to motor interval values
    _wm->_desiredTranslationalValue = _wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity = _wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
    
    _currentFitness += updateFitness(*inputs,outputs);

    // storeBehavior(*inputs,outputs);

    delete (inputs);
}
void TopEDOController::storeBehavior(std::vector<double> in,std::vector<double> out)
{
  std::pair<std::vector<double>,std::vector<double> > currentBehav;
  currentBehav = std::make_pair(in,out);
  _behavior.push_back(currentBehav);
  
}

float TopEDOController::updateFitness(std::vector<double> in,std::vector<double> out)
{
  float deltaFit = 0.0;
  int targetIndex = _wm->getGroundSensorValue();
 
  if ( PhysicalObject::isInstanceOf(targetIndex) ) 
    {
      deltaFit += 1.0;
    }
    
  return deltaFit;
}

float TopEDOController::dist(float x1, float y1, float x2, float y2)
{
  float result = 0.0;
  result = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  return result;
}

void TopEDOController::createNN()
{
  if ( nn != NULL ) 
    delete nn;
  
  switch ( TopEDOSharedData::gControllerType )
    {
    case 0:
      {
	nn = _genome->genesis(_wm->_id);
	break;
      }
    default: // default: no controller
      std::cerr << "[ERROR] gController type unknown (value: " << TopEDOSharedData::gControllerType << ").\n";
      exit(-1);
    };
}


unsigned int TopEDOController::computeRequiredNumberOfWeights()
{
  unsigned int res = nn->linkcount();
  return res;
}

// ################ ######################## ################
// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################
// ################ ######################## ################

void TopEDOController::stepEvolution()
{
  // broadcasting genome : robot broadcasts its genome to all neighbors (contact-based wrt proximity sensors)
  if  ( gRadioNetwork )
    {
      broadcastGenome();
    }
  
  // * lifetime ended: replace genome (if possible)
  if( dynamic_cast<TopEDOWorldObserver*>(gWorld->getWorldObserver())->getLifeIterationCount() >= TopEDOSharedData::gEvaluationTime-1 )
    {
     
      //GENERATION ID-ROBOT FITNESS
      std::cout << (gWorld->getIterations() / TopEDOSharedData::gEvaluationTime ) << " "  << _wm->getId() << " " << _currentFitness << " " << _genome -> getIdTrace() << " " << _genome -> getMom() << std::endl;
      
      //Output for tracing genome lineage
      //std::cout << (gWorld->getIterations() / TopEDOSharedData::gEvaluationTime ) << " " << _genome->previous_id << " " << _genome->genome_id << " " << () << std::endl;
      
      //Log .dot file for NN drawing
      /*if((gWorld->getIterations()/TopEDOSharedData::gEvaluationTime) > 200)
	nn->drawNetGraphViz("logs/gen" + std::to_string((gWorld->getIterations()/TopEDOSharedData::gEvaluationTime))  + "r" + std::to_string(_wm->getId()) + ".dot");
      */
      
      loadNewGenome();
      
    }

  
  if ( getNewGenomeStatus() ) // check for new NN parameters
    {
      reset();
      setNewGenomeStatus(false);
    }
}

void TopEDOController::feedBehaviorBase(std::vector<std::pair< std::vector<double>, std::vector<double> > > b)
{
  double minDist = 1000000.0;
  int idx = -1;
  unsigned int i = 0;
  for(i = 0; i < _behBase.size() ; i++)
    {
      if(dist(_behBase[i],b) < minDist)
	{
	  minDist = dist(_behBase[i],b);
	  idx = i;
	}
    }
   if(minDist < TopEDOSharedData::gBehThresh)
    {
      _behBaseCounter[idx] +=1;
    }
  else
    {
      _behBase[i] = b;
      _behBaseCounter[i] = 1;
    }

}

double TopEDOController::dist(std::vector<std::pair< std::vector<double>, std::vector<double> > > b1, std::vector<std::pair< std::vector<double>, std::vector<double> > > b2)
{
  if(b1.size() != b2.size())
    {
      std::cout << "[ERROR] Behaviors of different sizes: b1 (" << b1.size() << "), b2 (" << b2.size() << ")"<< std::endl;
      exit(-1);
    }
  double res = 0.0;
  double aux = 0.0;
  
  for(unsigned int i = 0; i < std::min(b1.size(),b2.size()); i++)
    {
      aux = 0.0;
      
      for(unsigned int j = 0; j < b1[0].first.size(); j++)
	{
	  aux += (b1[i].first[j] - b2[i].first[j]) * (b1[i].first[j] - b2[i].first[j]);
	}
     for(unsigned int j = 0; j < b1[0].second.size(); j++)
	{
	  aux += (b1[i].second[j] - b2[i].second[j]) * (b1[i].second[j] - b2[i].second[j]);
	}
     res += sqrt(aux);
    }


  return res;
}

void TopEDOController::selectRandomGenome()
{
  if(_genomesList.size() != 0)
    {
      int randomIndex = rand()%_genomesList.size();
      std::map<int, GenomeAdapted* >::iterator it = _genomesList.begin();
      while (randomIndex !=0 )
        {
	  it ++;
	  randomIndex --;
        }
      
      _genome = (*it).second;
      
      mutate(_sigmaList[(*it).first]);
      
      setNewGenomeStatus(true);
      
      _birthdate = gWorld->getIterations();
      
      // Logging
        std::string s = std::string("");
        s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] descends from [" + std::to_string((*it).first) + "::" + std::to_string(_birthdateList[(*it).first]) + "]\n";
        gLogManager->write(s);
        gLogManager->flush();
        
        _genomesList.clear();
    }
}


void TopEDOController::storeGenome( GenomeAdapted* genome, int senderId, int senderBirthdate, float sigma, float fitness)
{
  
  //TODO: adapt storeGenome to Neat
  //08/10/14 (storeGenome Adaptedto NEAT, I think)
  _genomesList[senderId] = genome;
  _sigmaList[senderId] = sigma;
  _birthdateList[senderId] = senderBirthdate;
  _fitnessList[senderId] = fitness;
  
}


void TopEDOController::mutate( float sigma) // mutate within bounds.
{

  //TODO: NEAT mutations


  int count;
  std::map<int,GenomeAdapted*>::iterator curorg;
  
  int poolsize;  //The number of Organisms in the old generation

  int orgnum;  //Random variable
  int orgcount;
  GenomeAdapted *mom; //Parent Organisms
  //Genome *dad;
  GenomeAdapted *baby;  //The new Organism
  
  GenomeAdapted *new_genome;  //For holding baby's genes
  
  Network *net_analogue;  //For adding link to test for recurrency

  
  NEAT::weight_mut_power = 1.0;
  NEAT::pop_size = 1;

  //The weight mutation power is species specific depending on its age
  double mut_power=NEAT::weight_mut_power;

  //Roulette wheel variables
  //double total_fitness=0.0;
  //double marble;  //The marble will have a number between 0 and total_fitness
  //double spin;  //0Fitness total while the wheel is spinning
  
  //NOTE: added by Inaki: declaration of expected_offspring = 1
  //since each robot can have one offspring (at most, i.e. if it does not die)
  int expected_offspring = 1;

  
  poolsize=_genomesList.size()-1;

  expected_offspring = 1;

  //Create the designated number of offspring for the Species
  //one at a time
  for (count=0; count < expected_offspring ;count++) 
    {

      int newId =_wm->getId() + 10000 * ( 1 + (gWorld->getIterations() / TopEDOSharedData::gEvaluationTime )) ;

      //Debug Trap
      if (expected_offspring > NEAT::pop_size) 
	{
	  std::cerr << "[ERROR] EXPECTED OFFSPRING = " << expected_offspring << " BIGGER THAN POPULATION = " << NEAT::pop_size << std::endl;
	  exit(-1);
	}
      
      /************************************************/
      //NOTE: default NEAT method (randomly) replaced  by previous selection (best)
      mom = _genome;

      new_genome=(mom)->duplicate(count, newId);
      
      /************************************************/
      



      //First, decide whether to mate or mutate
      //If there is only one organism in the pool, then always mutate
      if ((randfloat() < NEAT::mutate_only_prob) || poolsize == 0) 
	{
	  //Choose the random parent		
	  //RANDOM PARENT CHOOSER
	  orgnum=randint(0,poolsize);
	  curorg=_genomesList.begin();
	  for(orgcount=0;orgcount<orgnum;orgcount++)
	    ++curorg;                       
	  
	  ////NOTE: (commented) Roulette Wheel functionality erased
	  
	  //Do the mutation depending on probabilities of various mutations  
	  if (randfloat()<NEAT::mutate_add_node_prob) 
	    {
	      
	      //08/10/14  Bogus variables for innovations deactivated 
	      //perRobot innovNum and 
	      //nodeId used instead
	     
	      std::vector<Innovation*> innovations;

	      if(new_genome->mutate_add_node(innovations,nodeId,innovNum))
		{
		  //std::cout << "Mutate add node " << nodeId - 1 << std::endl;
		}
	      
	    }
	  else if (randfloat()<NEAT::mutate_add_link_prob) 
	    {	     
	      //NOTE: Inaki Hack, generation is only used as a network_id
	      int generation = 0; 
	      //No further repercusion of this parameter
	      net_analogue=new_genome->genesis(generation);

	      std::vector<Innovation*> innovations;
	      if(new_genome->mutate_add_link(innovations,innovNum,NEAT::newlink_tries))
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
	      
	      if (randfloat()<NEAT::mutate_random_trait_prob) 
		{
		  //std::cout << "Mutate random trait" << std::endl;
		  new_genome->mutate_random_trait();
		}
	      if (randfloat()<NEAT::mutate_link_trait_prob) 
		{
		  //std::cout << "Mutate_link_trait" << std::endl;
		  new_genome->mutate_link_trait(1);
		}
	      if (randfloat()<NEAT::mutate_node_trait_prob) 
		{
		  //std::cout << "Mutate_node_trait" << std::endl;
		  new_genome->mutate_node_trait(1);
		}
	      if (randfloat()<NEAT::mutate_link_weights_prob) 
		{
		  //std::cout << "Mutate_link_weights" << std::endl;
		  new_genome->mutate_link_weights(mut_power,1.0,GAUSSIAN);
		}
	      if (randfloat()<NEAT::mutate_toggle_enable_prob) 
		{
		  //std::cout << "Mutate toggle enable" << std::endl;
		  new_genome->mutate_toggle_enable(1);		      
		}
	      if (randfloat()<NEAT::mutate_gene_reenable_prob) 
		{
		  //std::cout << "Mutate gene reenable" << std::endl;
		  new_genome->mutate_gene_reenable();
		}
	    }
	  
	  baby =  new_genome;
	  _genome = baby; 
	}
 
      //Otherwise we should mate 
      /*  else 
	{
	  
	  //Choose the random mom
	  orgnum=randint(0,poolsize);
	  curorg=organisms.begin();
	  for(orgcount=0;orgcount<orgnum;orgcount++)
	    ++curorg;
	  
	  ////NOTE: (commented) Roulette Wheel functionality erased
	  	  
	  mom=(*curorg);         
	  
	  //Choose random dad
	  
	  if ((randfloat()>NEAT::interspecies_mate_rate)) 
	    {
	      //Mate within Species
	      
	      orgnum=randint(0,poolsize);
	      curorg=organisms.begin();
	      for(orgcount=0;orgcount<orgnum;orgcount++)
		++curorg;
	      
	      ////NOTE: (commented) Roulette Wheel functionality erased
	      
	      dad=(*curorg);
	    }
	  else 
	    {
	      
	      //Mate outside Species  
	      randspecies=this;
	      
	      //Select a random species
	      giveup=0;  //Give up if you cant find a different Species
	      while((randspecies==this)&&(giveup<5)) 
		{
		  
		  //Choose a random species tending towards better species
		  randmult=gaussrand()/4;
		  if (randmult>1.0) randmult=1.0;
		  //This tends to select better species
		  randspeciesnum=(int) floor((randmult*(sorted_species.size()-1.0))+0.5);
		  cursp=(sorted_species.begin());
		  for(spcount=0;spcount<randspeciesnum;spcount++)
		    ++cursp;
		  randspecies=(*cursp);
		  
		  ++giveup;
		}
	      
	      //New way: Make dad be a champ from the random species
	      dad=(*((randspecies->organisms).begin()));
	      
	      outside=true;	
	    }
	  
	  //Perform mating based on probabilities of differrent mating types
	  if (randfloat()<NEAT::mate_multipoint_prob) 
	    { 
	      new_genome=(mom->gnome)->mate_multipoint(dad->gnome,count,mom->orig_fitness,dad->orig_fitness,outside);
	    }
	  else if (randfloat()<(NEAT::mate_multipoint_avg_prob/(NEAT::mate_multipoint_avg_prob+NEAT::mate_singlepoint_prob))) 
	    {
	      new_genome=(mom->gnome)->mate_multipoint_avg(dad->gnome,count,mom->orig_fitness,dad->orig_fitness,outside);
	    }
	  else 
	    {
	      new_genome=(mom->gnome)->mate_singlepoint(dad->gnome,count);
	    }
	  
	  mate_baby=true;
	  
	  //Determine whether to mutate the baby's Genome
	  //This is done randomly or if the mom and dad are the same organism
	  if ((randfloat() > NEAT::mate_only_prob) 
	      || ((dad->gnome)->genome_id == (mom->gnome)->genome_id) 
	      || (((dad->gnome)->compatibility(mom->gnome)) == 0.0))
	    {
	      
	      //Do the mutation depending on probabilities of 
	      //various mutations
	      if (randfloat() < NEAT::mutate_add_node_prob) 
		{
		  new_genome->mutate_add_node(pop->innovations,pop->cur_node_id,pop->cur_innov_num);
		  //  std::cout<<"mutate_add_node: "<<new_genome<<std::endl;
		  mut_struct_baby=true;
		}
	      else if (randfloat() < NEAT::mutate_add_link_prob) 
		{
		  net_analogue=new_genome->genesis(generation);
		  new_genome->mutate_add_link(pop->innovations,pop->cur_innov_num,NEAT::newlink_tries);
		  delete net_analogue;
		  //std::cout<<"mutate_add_link: "<<new_genome<<std::endl;
		  mut_struct_baby=true;
		}
	      else 
		{
		  //Only do other mutations when not doing sturctural mutations
		  
		  if (randfloat() < NEAT::mutate_random_trait_prob) 
		    {
		      new_genome->mutate_random_trait();
		      //std::cout<<"..mutate random trait: "<<new_genome<<std::endl;
		    }
		  if (randfloat() < NEAT::mutate_link_trait_prob) 
		    {
		      new_genome->mutate_link_trait(1);
		      //std::cout<<"..mutate link trait: "<<new_genome<<std::endl;
		    }
		  if (randfloat() < NEAT::mutate_node_trait_prob) 
		    {
		      new_genome->mutate_node_trait(1);
		      //std::cout<<"mutate_node_trait: "<<new_genome<<std::endl;
		    }
		  if (randfloat() < NEAT::mutate_link_weights_prob) 
		    {
		      new_genome->mutate_link_weights(mut_power,1.0,GAUSSIAN);
		      //std::cout<<"mutate_link_weights: "<<new_genome<<std::endl;
		    }
		  if (randfloat() < NEAT::mutate_toggle_enable_prob) 
		    {
		      new_genome->mutate_toggle_enable(1);
		      //std::cout<<"mutate_toggle_enable: "<<new_genome<<std::endl;
		    }
		  if (randfloat()<NEAT::mutate_gene_reenable_prob) 
		    {
		      new_genome->mutate_gene_reenable(); 
		      //std::cout<<"mutate_gene_reenable: "<<new_genome<<std::endl;
		    }
		}
		    //Create the baby
	      baby=new Organism(0.0,new_genome,generation);
	      
	    }
	  else 
	    {
	      //Create the baby without mutating first
	      baby=new Organism(0.0,new_genome,generation);
	    }
	  
	    }*/
      
      //Add the baby to its proper Species
      //If it doesn't fit a Species, create a new one
	    
      /*baby->mut_struct_baby=mut_struct_baby;
	baby->mate_baby=mate_baby;*/
      
      /*curspecies=(pop->species).begin();
      if (curspecies==(pop->species).end())
	{
	  //Create the first species
	  newspecies=new Species(++(pop->last_species),true);
	  (pop->species).push_back(newspecies);
	  newspecies->add_Organism(baby);  //Add the baby
	  baby->species=newspecies;  //Point the baby to its species
	} 
      else 
	{
	  comporg=(*curspecies)->first();
	  found=false;
	  while((curspecies != (pop->species).end()) && (!found)) 
	    {	
	      if (comporg==0) 
		{
		  //Keep searching for a matching species
		  ++curspecies;
		  if (curspecies != (pop->species).end())
		    comporg=(*curspecies)->first();
		}
	      else if (((baby->gnome)->compatibility(comporg->gnome)) < NEAT::compat_threshold) 
		{
		  //Found compatible species, so add this organism to it
		  (*curspecies)->add_Organism(baby);
		  baby->species=(*curspecies);  //Point organism to its species
		  found=true;  //Note the search is over
		}
	      else 
		{
		  //Keep searching for a matching species
		  ++curspecies;
		  if (curspecies != (pop->species).end()) 
		    comporg=(*curspecies)->first();
		}
	    }		
	  //If we didn't find a match, create a new species
	  if (found==false) 
	    {
	      newspecies=new Species(++(pop->last_species),true);
	      //std::std::cout<<"CREATING NEW SPECIES "<<pop->last_species<<std::std::endl;
	      (pop->species).push_back(newspecies);
	      newspecies->add_Organism(baby);  //Add the baby
	      baby->species=newspecies;  //Point baby to its species
	    }
	  
	    } //end else (if not the first species)*/
      
    }//end for expected_offspring


    //ERASED Old Mutation method (mEDEA)
  

    createNN();
   
    // Logging
    std::string s = std::string("");
    s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] [sigma=" + std::to_string(_currentSigma) + "]\n";
  gLogManager->write(s);
  gLogManager->flush();
}



void TopEDOController::broadcastGenome()
{  
  // only if agent is active (ie. not just revived) and deltaE>0.
  if ( _wm->isAlive() == true )
    {
        for( int i = 0 ; i < _wm->_cameraSensorsNb; i++)
        {
            int targetIndex = _wm->getObjectIdFromCameraSensor(i);
	    
            // sensor ray bumped into a robot : communication is possible
            if ( targetIndex >= gRobotIndexStartOffset )   
            {
	      // convert image registering index into robot id.
                targetIndex = targetIndex - gRobotIndexStartOffset;
                
                TopEDOController* targetRobotController = dynamic_cast<TopEDOController*>(gWorld->getRobot(targetIndex)->getController());
                
                if ( ! targetRobotController )
                {
                    std::cerr << "Error from robot " << _wm->getId() << " : the observer of robot " << targetIndex << " is not compatible" << std::endl;
                    exit(-1);
                }
                
                float dice = float(rand()%100) / 100.0;
                float sigmaSendValue = _currentSigma;
                
                if ( dice <= TopEDOSharedData::gProbaMutation )
                {
                    dice = float(rand() %100) / 100.0;
                    if ( dice < 0.5 )
                    {
                        sigmaSendValue = _currentSigma * ( 1 + TopEDOSharedData::gUpdateSigmaStep ); // increase sigma
                        
                        if (sigmaSendValue > TopEDOSharedData::gSigmaMax)
                        {
                            sigmaSendValue = TopEDOSharedData::gSigmaMax;
                        }
                    }
                    else
                    {
                        sigmaSendValue = _currentSigma * ( 1 - TopEDOSharedData::gUpdateSigmaStep ); // decrease sigma
                        
                        if ( sigmaSendValue < TopEDOSharedData::gSigmaMin )
                        {
                            sigmaSendValue = TopEDOSharedData::gSigmaMin;
                        }
                    }
                }
                
		// other agent stores my genome.
		
                targetRobotController->storeGenome(_genome, _wm->getId(), _birthdate, sigmaSendValue, _currentFitness);
		
            }
        }
    }
}

void TopEDOController::loadNewGenome()
{
  // Logging
  std::string s = "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] [energy:" +  std::to_string(_wm->getEnergyLevel()) + "] [genomeList:" + std::to_string(_genomesList.size()) + "]\n";
  gLogManager->write(s);
  gLogManager->flush();
  
  _genomesList[_wm->getId()] = _genome;
  _fitnessList[_wm->getId()] = _currentFitness;
  _sigmaList[_wm->getId()] = _currentSigma;
  _birthdateList[_wm->getId()] = _birthdate;

  int selected;
      
    if (_genomesList.size() > 0)
      {
	
	selected = selectBest(_fitnessList);
	_genome = _genomesList[selected];
	_currentFitness = _fitnessList[selected];

	//ERASED Adaptation of sigma in mEDEA 


      mutate(_currentSigma);

      _currentFitness = 0.0;
      setNewGenomeStatus(true);
      _birthdate = gWorld->getIterations();
      _genomesList.clear();
      _fitnessList.clear();
      _sigmaList.clear();
      _birthdateList.clear();


      //  _wm->setRobotLED_colorValues(rand()%255, 0, 0);
    }
  
  // log the genome
  
  s = std::string("");
  s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] new_genome: ";

  //TODO: adapt printing to NEAT genomes
  /* for(unsigned int i=0; i<_genome.size(); i++)
    {
	  s += std::to_string(_genome[i]) + " ";	
	  }*/
  s += "\n";
  gLogManager->write(s);
  gLogManager->flush();
     
}

int TopEDOController::selectBest(std::map<int,float> lFitness)
{
  int idx = -1;
  float bestFit = -1000000;
  std::map<int, float>::iterator it;
  for(it = lFitness.begin(); it != lFitness.end(); it++ )
    {
      if(it->second > bestFit)
	{
	  bestFit = it->second;
	  idx = it->first;
	}
      
    }
  return idx;
}
