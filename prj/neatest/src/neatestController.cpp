/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 * 
 */

#include <math.h>
#include <string>

#include "World/World.h"
#include "Utilities/Misc.h"

#include "neat/genome.h"
#include "neat/genomeAdapted.h"

#include "neatest/include/neatestController.h"
#include "neatest/include/neatestWorldObserver.h"

using namespace NEAT;
void print_genome(GenomeAdapted*);
void emptyBasket();

int get_lifetime(){
    return dynamic_cast <neatestWorldObserver*> 
	(gWorld->getWorldObserver())->getLifeIterationCount();  
}

bool lifeTimeOver(){
    return dynamic_cast <neatestWorldObserver*> 
	(gWorld->getWorldObserver())->getLifeIterationCount() 
	>= neatestSharedData::gEvaluationTime - 1;
}



neatestController::neatestController(RobotWorldModel * wm){
  _wm              = wm;
  _iteration       = 0;
  _birthdate       = 0;
  _prev_fitness    = 0.0;
  _fitness         = 0.0;
  _items           = 0;
  _neurocontroller = NULL;
  _sigma           = neatestSharedData::gSigmaRef;

  _wm->setAlive(true);
  initRobot();
  
}

neatestController::~neatestController (){
    delete _neurocontroller;
    _neurocontroller = NULL;
}

void neatestController::initRobot (){
    // setup the number of input and outputs 
    _nbInputs = 1;		                   // bias 
    if (gExtendedSensoryInputs)
	_nbInputs += (1) * _wm->_cameraSensorsNb;  // object sensor
    _nbInputs += _wm->_cameraSensorsNb;	           // proximity sensors
    _nbOutputs = 2;                                // motor output

    // Start with Simple Perceptron Inputs, outputs, 0 hidden neurons. 
    _genome = new GenomeAdapted (_nbInputs, _nbOutputs, 0, 0);
    _genome->genome_id = getId();
    _genome->setIdTrace(getId());
    _genome->setMom(-1);
    _genome->setDad(-1);
    
    // create a neuro controller from this genome
    createNeuroController();
    _genome->setInnovNumber( (double) _neurocontroller->linkcount ());
    _genome->setNodeId(1 + _nbInputs + _nbOutputs);    
    _genome->mut_link_weights(_sigma);
        
    // clear the genome list 
    emptyGenomeList();

    if (gVerbose){
	std::cout << "[initRobot] " 
		  << "id="  << _wm->getId() << " "
		  << "in="  << _nbInputs    << " "
		  << "out=" << _nbOutputs 
		  << std::endl;
	printRobot();
	print_genome(_genome);
	save_genome();
    }
}

void neatestController::createNeuroController (){
  if (_neurocontroller != NULL)
    delete _neurocontroller;
  _neurocontroller = _genome->genesis(_wm->_id);
}

void neatestController::reset(){
    
    /* fitness related resets */ 
    _prev_fitness = _fitness;
    _fitness = 0.0;
    emptyBasket();
    _birthdate = gWorld->getIterations();
    emptyGenomeList();
}

void neatestController::step(){
  _iteration++;
  if(_wm->isAlive()){
      stepBehaviour(); // execure the neuro controller
      updateFitness();
      broadcast();     // broadcast genome to neighbors
  }
  if(lifeTimeOver()){
      stepEvolution (); // select, mutate, replace
      
      if (gVerbose){
	  save_genome();
	  printAll();
      }
      reset();          // reset fitness and neurocontroller
  }
}

// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################
bool is_energy_item(int id){
    return (gPhysicalObjects[id-gPhysicalObjectIndexStartOffset]->getType()==1);
} 

/*
 * Step the neuro controller and execute command
 *
 */
void neatestController::stepBehaviour(){
    double inputs[_nbInputs]; 
    int    inputToUse = 0;
    
    // (1)  Read inputs 
 
    /* read distance sensors  */
    for(int i = 0; i < _wm->_cameraSensorsNb; i++)
	inputs[inputToUse++] = 
	    _wm->getDistanceValueFromCameraSensor (i) /
	    _wm->getCameraSensorMaximumDistanceValue (i);
    
    /* read object sensors */
    if(gExtendedSensoryInputs)
	for(int i = 0; i < _wm->_cameraSensorsNb; i++){
	    int objectId = _wm->getObjectIdFromCameraSensor(i);

	    /* if physical object, and of correct type */
	    if(PhysicalObject::isInstanceOf(objectId)){
		if(is_energy_item(objectId))
		    inputs[inputToUse] = 1.0;  
		else
		    inputs[inputToUse] = 0.0;
		inputToUse++;
	    }
	    /* got a physical object but not interesting */
	    else
		inputs[inputToUse++] = 0.0;
	}
    
    /* bias node : neat put biases after sensors */
    inputs[inputToUse++] = 1.0; 
    
    // (2) step the neuro controller
    _neurocontroller->load_sensors(inputs);
    if (!_neurocontroller->activate()){
	std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
	exit (-1);
    }
    
    // (3) read the output
    std::vector<double> outputs;
    std::vector<NNode*>::iterator out_iter;
    for (out_iter  = _neurocontroller->outputs.begin();
	 out_iter != _neurocontroller->outputs.end(); 
	 out_iter++)
	outputs.push_back((*out_iter)->activation);
     
    // (4) execute the motor commands 
    _wm->_desiredTranslationalValue = outputs[0]; 
    _wm->_desiredRotationalVelocity = 2.0 * (outputs[1] - 0.5); /* [-1, 1] */

    /* normalize to motor interval values */
    _wm->_desiredTranslationalValue =
	_wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
	_wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
    
    // print things
    /*if(gVerbose){
	std::cout << "[Controller] "
		  << "\t[Robot #" + to_string(_wm->getId()) + "]\n"
		  << "\t\t[Inputs : " ;
	for(unsigned int i = 0; i < _nbInputs; i++){
	    std::cout << to_string(inputs[i]) + " ";
	    if ((i % _wm->_cameraSensorsNb) == 0)
		std::cout << "]" <<  std::endl << "\t\t\t[ ";
	}
	std::cout <<  std::endl;

	std::cout << "\t\t[Outputs : " ;
	std::vector<double>::iterator itr;
	for(itr = outputs.begin (); itr != outputs.end (); itr++)
	    std::cout << to_string(*itr) + " ";
	std::cout << "]"
		  << std::endl;
		  }*/

    //_wm->_desiredTranslationalValue = 0.0;
    //_wm->_desiredRotationalVelocity = 0.0;
} 


// ################ ######################## ################
// ################ COMMUNICATION  METHODS   ################
// ################ ######################## ################

void neatestController::broadcast() {
    /* Make a list of all neighbors within reach  */
    std::vector<neatestController *> neighbors;
    for (int i = 0; i < _wm->_cameraSensorsNb; i++)	{
	int targetIndex = _wm->getObjectIdFromCameraSensor (i);
	
	/* sensor ray bumped into a robot : communication is possible */
	if (targetIndex >= gRobotIndexStartOffset){
	    
	    /* convert image registering index into robot id. */
	    targetIndex = targetIndex - gRobotIndexStartOffset;
	    neatestController *targetRobotController =
		dynamic_cast <
		neatestController *
		>(gWorld->getRobot (targetIndex)->getController ());
	    
	    /* an error occured */ 
	    if(!targetRobotController){
		std:: cerr << "Error from robot "         << _wm->getId () 
			   << " : the observer of robot " << targetIndex 
			   << " is not compatible"        << std::endl;
		exit (-1);
	    }
	    
	    /* add to the list  */
	    neighbors.push_back(targetRobotController);
	}
    }
    
    /* if found neighbors, broadcast my genome */
    if(neighbors.size() > 0) {
	message msg (_genome, _wm->_fitnessValue, _sigma, _birthdate);

	/* remove duplicates */
	std::sort(neighbors.begin(), neighbors.end()); 
	auto last = std::unique(neighbors.begin(), neighbors.end());
	neighbors.erase(last, neighbors.end());
	
	/* send */
	for (const auto& c : neighbors)
	    c->storeMessage (getId(), msg);
	
	/* some screen output */
	if (gVerbose){
	    std::cout << "@"  << _iteration << " R" << getId() << " -> " ;
	    for (const auto& c : neighbors)
		std::cout << c->getId() << " ";
	    std::cout << std::endl;
	}
	/* delete neighbors list */
	neighbors.clear();
    }
}

void neatestController::storeMessage(int id, message msg){
    _glist[id] = msg;
}

void neatestController::emptyGenomeList(){
    _glist.clear();
}

// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void neatestController::stepEvolution() {
    /* store our genome in the list */
    message msg (_genome, _wm->_fitnessValue, _sigma, _birthdate);
    storeMessage(_wm->getId(), msg);
       
    /* select an offspring */
    int selected = selectRandom();
    _genome = std::get<0>(_glist[selected]);
    _sigma  = std::get<2>(_glist[selected]);
    
    /* mutate the offspring */
    int newId = _wm->getId() + 10000 * 
	(1 + (gWorld->getIterations() /
	      neatestSharedData::gEvaluationTime));
    
    switch(neatestSharedData::gControllerType) {
    case 0:
	_genome = _genome->mutate(_sigma, getId(), newId);
	break;
    case 1:
	_genome = _genome->mutate_weights(_sigma, getId(), newId);
	break;
    default:
	std::cerr << "Error unknown mutation for this controller" << std::endl;
	exit (-1);
    }
    
    /* create a new network */
    createNeuroController();

    /** there is a memory leak here. Genome is never deleted.
	Selection selects some other genome and  mutate create a new one, 
	the previous genome is not deleted. It cannot be deleted since it 
	could be selected at some other agent ???? **/
}


// update fitness for foraging 
void neatestController::updateFitness(){
    _fitness = (double) _items / (double) get_lifetime();
}

void neatestController::pickItem(){
    _items++;
}

void neatestController::emptyBasket(){
    _items = 0;
}


int  neatestController::selectRandom(){
    auto it = _glist.begin();
    std::advance(it, rand() % _glist.size());
    return it->first;
}

int neatestController::selectBest(){
    //printGenomeList() ;
    
    std::map<int, message>::iterator it = _glist.begin();
    double max_fit =  std::get<1>(it->second);
    int    best_g  =  it->first;
    for ( ; it != _glist.end(); it++){
	GenomeAdapted* g;
	double f,s;
	int b;
	std::tie (g,f,s,b) = it->second;
	if(f > max_fit){
	    max_fit = f;
	    best_g = it->first ;
	}
    }
       
    //std::cout << "best = " << best_g << std::endl; 

    return best_g;
}

// ################ ######################## ################
// ################ OUTPUT (FILES / SCREEN) METHODS #########
// ################ ######################## ################

void neatestController::printMessage(message msg){
    GenomeAdapted* g;
    double f,s;
    int b;

    std::tie (g,f,s,b) = msg;
    std::cout << "(Id="        << g->getIdTrace()
	      << " fitness="   << f
	      << " sigma="     << s
	      << " birthdate=" << b << ")";
    
    std::cout << "[Genome : (id = " << g->genome_id
	      << ", idtrace = "     << g->getIdTrace()
	      << ", mom = "         << g->getMom()
	      << ", dad = "         << g->getDad() << ")]";
}

// Save a genome (file name = robot_id-genome_id.gen)
void neatestController::save_genome(){
    char fname[128];
    snprintf(fname, 127, "logs/%04d-%010d.gen", 
	     getId(), _genome->getIdTrace());
    std::ofstream oFile(fname);
    _genome->print_to_file(oFile);
    oFile.close();
}

void print_genome(GenomeAdapted* g){
    std::cout << "[Genome: id=" << g->genome_id
	      << " idtrace="    << g->getIdTrace()
	      << " mom="        << g->getMom()
	      << " dad="        << g->getDad() << " ]";
}

void neatestController::printGenomeList(){
    std::cout << "[Genome list at agent " + to_string(getId()) + "]\n";
    std::map<int, message>::iterator it;
    for (it=_glist.begin() ; it != _glist.end(); it++){
	std::cout << "\t\t[R# " << it->first << "] " ;
	GenomeAdapted* g;
	double f,s;
	int b;
	std::tie (g,f,s,b) = it->second;
	
	print_genome(g);
	printMessage(it->second);
	std::cout << std::endl;
    }
}

void neatestController::printRobot(){
    std::cout << "[Robot: id=" + to_string(getId())
	      << " iteration=" + to_string(_iteration)
	      << " birthdate=" + to_string(_birthdate) 
	      << " fitness="   + to_string(_fitness) 
	      << " items="     + to_string(_items) 
	      << " sigma="     + to_string(_sigma) + " ]";
}

void neatestController::printAll(){
    printRobot();
    print_genome(_genome);
    std::cout << "\n";
    //std::cout << "\t";
    //printGenomeList();
}
