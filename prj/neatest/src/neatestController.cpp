/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 * 
 */

#include <math.h>
#include <string>
#include <algorithm>

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

  _reported_fitness  = 0.0;
  _fitness           = 0.0;

  _items_collected   = 0;
  _items_forraged    = 0;
  
  _items_max         = 5;

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
    _nbInputs ++;                                  // basket capacity sensor
    _nbInputs ++;                                  // ground sensor

    _nbOutputs = 2;                                // motor output
    _nbOutputs ++ ;                                // drop  items effector 

    // Start with Simple Perceptron Inputs, outputs, 0 hidden neurons. 
    _genome = new GenomeAdapted (_nbInputs, _nbOutputs, 0, 0);
    _genome->genome_id = getId();
    _genome->setIdTrace(getId());
    _genome->setMom(-1);
    _genome->setDad(-1);
    _genome->mut_link_weights(_sigma);
    
    // create a neuro controller from this genome
    createNeuroController();
    _genome->setInnovNumber( (double) _neurocontroller->linkcount ());
    _genome->setNodeId(1 + _nbInputs + _nbOutputs);    
 
        
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
    _birthdate = gWorld->getIterations();

    /* fitness related resets */ 
    _reported_fitness = _fitness;
    _fitness = 0.0;
    _items_collected   = 0;
    _items_forraged    = 0;
  
    emptyBasket();
    emptyGenomeList();
}

void neatestController::step(){
  _iteration++;
  if(_wm->isAlive()){
      stepBehaviour(); // execure the neuro controller
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

void neatestController::pickItem(){
    _items_collected++;
}

void neatestController::emptyBasket(){
    _items_collected = 0;
}

void neatestController::dropItem(int n){
    if(_items_collected - n <= 0)
	_items_collected = 0;
    else
	_items_collected -= n;
}

/*
 * Step the neuro controller and execute command
 *
 */
void neatestController::stepBehaviour(){
    double inputs[_nbInputs]; 
    int    inputToUse = 0;
    const  int L = 0; /* left / right motor*/
    const  int R = 1;
    const  int D = 2; /* drop effector */
    
    /* floreano fitness related atribute */
    double lv, rv, md; /* tran rot velocities and min dist */


    // (1)  Read inputs 
 
    /* read distance sensors  [0 -> 1 ] (1 = touching object)  */
    for(int i = 0; i < _wm->_cameraSensorsNb; i++)
	inputs[inputToUse++] = 1 - (
	    _wm->getDistanceValueFromCameraSensor (i) /
	    _wm->getCameraSensorMaximumDistanceValue (i));

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
     
    /* get the most activated obstacle sensor for floreano fitness*/
    md =-10.0;
    for(int i = 0; i < _wm->_cameraSensorsNb; i++)
	if(md < inputs[i] && 
	   gExtendedSensoryInputs && inputs[i+_wm->_cameraSensorsNb] < 1.0)
	    md = inputs[i];
    
    /* Basket capacity sensor */
    double activation = (double) _items_collected / (double) _items_max; 
    inputs[inputToUse++] = activation;

    /* ground sensor */
    const int max_activation = 255*256*256 + 255*256 + 255; 
    double ground = _wm->getGroundSensorValue() / (double) max_activation;
    inputs[inputToUse++] = ground;
    /* are we on a green nest ? */
    const int nest_color = 255*256; 
    bool at_nest= _wm->getGroundSensorValue() == nest_color; 

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
    
    /* outputs => rot & trans */ 
    //_wm->_desiredTranslationalValue = outputs[0]; 
    //_wm->_desiredRotationalVelocity = 2.0 * (outputs[1] - 0.5); /* [-1, 1] */

    /* output = L & R velocity (differential drive) */ 
    
    /* store velocities for floreano fitness */
    lv = outputs[L]-0.5;
    rv = outputs[R]-0.5;

    outputs[L] = 2.0 * (outputs[L]-0.5); /* rescale to [-1, 1] */
    outputs[R] = 2.0 * (outputs[R]-0.5);
  
    _wm->_desiredTranslationalValue =  (outputs[R] + outputs[L]) / 2.0 ;
    _wm->_desiredRotationalVelocity =  outputs[R] - outputs[L] ;

    /* normalize to motor interval values */
    _wm->_desiredTranslationalValue =
    	_wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
    	_wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
   
    /* drop items */ 
    int droped = (int) (outputs[D] * _items_collected);
    dropItem(droped);
       
    /* (5) update the fitness function */
    switch(neatestSharedData::gFitnessFunction) {

    case 0: /* locomotion */
	_fitness += (fabs(lv) + fabs(rv)) * 
	    (1.0 - sqrt(fabs(lv - rv))) * 
	    (1.0 - md) ;
	break;

    case 1: /* Collection */
	_fitness = (double) _items_collected / (double) get_lifetime();
	break;

    case 2: /* Forraging */ 
	if(at_nest)
	    _items_forraged += droped;
	else
	    _items_forraged -= droped;
	    
	_fitness = (double) _items_forraged / (double) get_lifetime(); 
	break;
    default:
	std::cerr << "Error unknown fitness function" << std::endl;
	exit (-1);
    }
   

    // print things
    if(gVerbose){
	std::cout << "[Controller] "
		  << "\t[Robot #" + to_string(_wm->getId()) + "]\n"
		  << "\t\t[lv="   << lv 
		  << " rv="       << rv 
		  << " max_sens=" << md 
		  << "]" << std::endl 
		  << "\t\tInputs :[ " ;
	for(unsigned int i = 1; i <= _nbInputs; i++){
	    std::cout << to_string(inputs[i-1]) + " ";
	    if ((i % _wm->_cameraSensorsNb) == 0)
		std::cout << "]" <<  std::endl << "\t\t\t[ ";
	}
	std::cout <<  " ]" << std::endl;

	std::cout << "\t\tOutputs :[ " ;
	std::vector<double>::iterator itr;
	for(itr = outputs.begin (); itr != outputs.end (); itr++)
	    std::cout << to_string(*itr) + " ";
	std::cout << "]"
		  << std::endl;

	

    }

    

    /* _wm->_desiredTranslationalValue = 0.0; */
    /* _wm->_desiredRotationalVelocity = 0.0; */

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
	message msg (_genome, _fitness, _sigma, _birthdate);

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
    message msg (_genome, _fitness, _sigma, _birthdate);
    storeMessage(_wm->getId(), msg);
       
    /* select an offspring */
    int selected = select(neatestSharedData::gSelectionPressure);
    _genome = std::get<0>(_glist[selected]);
    _sigma  = std::get<2>(_glist[selected]);
    
    /* mutate the offspring */
    int newId = _wm->getId() + 10000 * 
	(1 + (gWorld->getIterations() /
	      neatestSharedData::gEvaluationTime));
    
    switch(neatestSharedData::gControllerType) {
    case 0: /* neat*/
	_genome = _genome->mutate(_sigma, getId(), newId);
	break;
    case 1: /* FFNN*/
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



int neatestController::select(double sp){
    /* the size of the tournament */
    int inspected = sp * (double) _glist.size();
   
    /* shuffle indexes */
    vector<int> v;
    for(auto i: _glist)
	v.push_back(i.first);
    std::random_shuffle(v.begin(), v.end());
    
    /* get the best from the inspected */
    double max_fit =  std::get<1>(_glist[v[0]]);
    int    best_g  =  v[0];
    int    j=1; /* index in v */
    for (int i=1 ; i<inspected; i++){
	double f  = std::get<1>(_glist[v[j]]);
	if(f > max_fit){
	    max_fit = f;
	    best_g = v[j] ;
	}
	j++;
    } 
    
    /* printGenomeList() ;
    std::cout << "\t\tShuffled :[" << v[0];
    for (int i=1 ; i<inspected ; i++)
	std::cout << ", " << v[i];
    std::cout << "] "  
    << "selected = " << best_g << std::endl; */

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
    std::cout << "[Robot: id="         <<  getId() 
	      << " iteration="         <<  _iteration
	      << " birthdate="         <<  _birthdate
	      << " fitness="           <<  _fitness
	      << " reported fitness="  <<  _reported_fitness
	      << " collected="         <<  _items_collected << "/" << _items_max
	      << " forraged="          <<  _items_forraged 
	      << " sigma="             <<  _sigma 
	      << " ]";
}

void neatestController::printAll(){
    printRobot();
    print_genome(_genome);
    std::cout << "\n";
    //std::cout << "\t";
    //printGenomeList();
}
