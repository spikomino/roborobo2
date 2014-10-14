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
    
// Duplicate this Genome to create a new one with the specified id 
// and give mom id value
GenomeAdapted* GenomeAdapted::duplicate(int new_id, int idTr)//,int mom, int dad)
{
  GenomeAdapted* result =(GenomeAdapted*)(((Genome*)this)->duplicate(new_id));
  result -> setIdTrace(idTr);
  //result -> setMom(this -> getIdTrace());
  //result -> setDad(-1);
    
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

