/**
 * @file
 * @author Leo Cazenille <leo.cazenille@upmc.fr>
 *
 *
 */

#include <neuralnetworks/MLP.h>
#include <neuralnetworks/NeuralNetwork.h>
#include <sstream>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace Neural;


/* --------------------- MLP --------------------- */

MLP::MLP(std::vector<double>& weights,
		unsigned int nbInputs,
		unsigned int nbOutputs,
		bool activeBiais,
		bool onlyUseBiaisForFirstHiddenLayer,
		double biaisValue) :
		LayeredNeuralNetwork(weights, nbInputs, nbOutputs, activeBiais, onlyUseBiaisForFirstHiddenLayer, biaisValue) {
	// ...
}


MLP::MLP(std::vector<double>& weights,
		unsigned int nbInputs,
		unsigned int nbOutputs,
		std::vector<unsigned int>& nbNeuronsPerLayer,
		bool activeBiais,
		bool onlyUseBiaisForFirstHiddenLayer,
		double biaisValue) :
		LayeredNeuralNetwork(weights, nbInputs, nbOutputs, nbNeuronsPerLayer, activeBiais, onlyUseBiaisForFirstHiddenLayer, biaisValue) {
	// ...
}



MLP::MLP(MLP const& other) : LayeredNeuralNetwork(other) {
	// ...
}

MLP::~MLP() {
	// ...
}


std::string MLP::toString() const {
	return LayeredNeuralNetwork::toString();
}


void MLP::step() {

	std::vector<double> prec = _inputs;
	unsigned int weightsIndice = 0;
	std::vector<double> tmp;

	// Verify that the number of layers is correct
//	if(_nbNeuronsPerLayer.size() < 2)
//		throw NeuralNetworkException("MLP must have at least 2 layers : input and output");
//	if(_nbNeuronsPerLayer[0] != _inputs.size())
//		throw NeuralNetworkException("nbNeuronsPerLayer has an incorrect number of inputs neurons (first layer)");
//	if(_nbNeuronsPerLayer[_nbNeuronsPerLayer.size() - 1] == 0)
//		throw NeuralNetworkException("nbNeuronsPerLayer has an incorrect number of output neurons (output layer)");

//	unsigned int nbBiais = 0;
//	if(_activeBiais)
//		nbBiais = 1;

	for(unsigned int k = 0; k < _nbNeuronsPerLayer.size() - 1; k++) {

		unsigned int const nbOutputs = _nbNeuronsPerLayer[k + 1];

		// Do Stuffs ! (base neural computation)
		tmp = std::vector<double>(nbOutputs, 0.0);
		for(unsigned int i = 0; i < prec.size(); i++) {
			for(unsigned int j = 0; j < nbOutputs; j++) {
				tmp[j] += prec[i] * _weights[weightsIndice++];
			}
		}

		// Do computation of the biais
		if(_activeBiais && (k==0 || !_onlyUseBiaisForFirstHiddenLayer))
			for(unsigned int j = 0; j < nbOutputs; j++)
				//tmp[j] += tanh(_weights[weightsIndice++]) * .08;
				tmp[j] += tanh(_weights[weightsIndice++]) * _biaisValue;

		// Tanh activation function
		for(unsigned int i = 0; i < nbOutputs; i++)
			tmp[i] = tanh(tmp[i]);

		prec = tmp;
	}

	_outputs = tmp;
}


unsigned int MLP::computeRequiredNumberOfWeights() {
	unsigned int res = 0;
	unsigned int nbBiais = 0;
	if(_activeBiais)
		nbBiais = 1;

	if(_nbNeuronsPerLayer.size() <= 2) {
		return (_nbInputs + nbBiais) * _nbOutputs;
	} else {
		res += (_nbInputs + nbBiais) * _nbNeuronsPerLayer[1];
		if(_onlyUseBiaisForFirstHiddenLayer)
			nbBiais = 0;
		for(size_t i = 1; i < _nbNeuronsPerLayer.size() - 1; i++) {
			res += (_nbNeuronsPerLayer[i] + nbBiais) * _nbNeuronsPerLayer[i + 1];
		}
		return res;
	}
}


std::string MLP::getNNTypeName() {
	return "MLP";
}

MLP* MLP::clone() const {
	return new MLP(*this);
}

void MLP::saveFile(int id, std::string& fname) {
    

    std::ofstream ofile;

    ofile.open(fname);

    unsigned int weightsIndice = 0;
    unsigned int source_off    = 1;
    unsigned int target_off    = 1;

    

    ofile << "genomestart " << id << std::endl;

    for(unsigned int k = 0; k < _nbNeuronsPerLayer.size() - 1; k++) {
		
	target_off += _nbNeuronsPerLayer[k];
	
	unsigned int const nbInputs =  _nbNeuronsPerLayer[k];
	unsigned int const nbOutputs = _nbNeuronsPerLayer[k + 1];
	
	/* k -> k+1 */
	for(unsigned int i=0; i < nbInputs; i++) {
	    
	    int n1 = source_off + i;
	    if(k == 0)
		ofile << "node " <<  n1 << " 0 1 1" << std::endl;
	    else
		ofile << "node " <<  n1 << " 0 1 0" << std::endl;

	    for(unsigned int j = 0; j < nbOutputs; j++) {
		int n2 = target_off + j ;
		if(_activeBiais && (k==0 || !_onlyUseBiaisForFirstHiddenLayer))
		    n2++;
		
		// gene 1 1 14 2.94319 0 1 0 1
		// trait, n1, n2, w, recur, inov, mut_num, enabled 
		ofile << "gene 1 " << n1 << " " << n2 << " "
		      <<   _weights[weightsIndice++] 
		      << " 0 1 0 1" << std::endl;
	    }
	}
	source_off += nbInputs;

	// bias -> k+1
	if(_activeBiais && (k==0 || !_onlyUseBiaisForFirstHiddenLayer)){
	    int n1 = source_off  ;
	    ofile << "node " << n1 << " 0 1 3" << std::endl;

	    for(unsigned int j = 0; j < nbOutputs; j++){
		int n2 = target_off + j +1;

		ofile << "gene 1 " << n1 << " " <<n2 << " "
			  <<   _weights[weightsIndice++]
			  << " 0 1 0 1" << std::endl;
	    }
	    source_off++;
	    target_off++;
	}
    }
    
    for (unsigned int i = 0; i < _nbOutputs; i++)
	ofile << "node " <<  source_off++ << " 0 1 2" << std::endl;
    
    ofile << "genomeend " << id << std::endl;

    ofile.close();
}
