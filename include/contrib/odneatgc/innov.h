#ifndef ODNEATGCINNOV_H
#define ODNEATGCINNOV_H

#include <iostream>
namespace ODNEATGC
{
//Struct for innovation numbers of topological innovations (nodes and links)
//in a distributed setup.
//It uses distributed genetic clocks updated on reception. It stores a pair of integers
//(idR: id of the robot creating the innovation, gc: gene counter in that robot)
//gc is supposed to be sent along with a genome when broadcasting it, in order to
//update the receiver gc and thus avoiding gene counter collisions as much as possible
struct innov
{
  int idR;
  int gc;
  bool operator ==(const innov& innov2)
    {
        return ((idR == innov2.idR) && (gc == innov2.gc));
    }
  bool operator <(const innov& innov2)
    {
      if(gc < innov2.gc)
          return true;
      if(gc > innov2.gc)
          return false;

      return (idR < innov2.idR);
    }
  bool operator >(const innov& innov2)
    {
      if(gc > innov2.gc)
          return true;
      if(gc < innov2.gc)
          return false;

      return (idR > innov2.idR);
    }
};
} //namespace ODNEATGC
#endif // INNOV_H
