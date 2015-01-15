#ifndef ODNEATRANDOMINNOV_H
#define ODNEATRANDOMINNOV_H

#include <iostream>
namespace ODNEATGRANDOM
{
//Struct for innovation numbers of topological innovations (nodes and links)
//in a distributed setup. In this case, a single number is assigned
//in a random way
struct innov
{
  int gc;
  bool operator ==(const innov& innov2)
    {
        return (gc == innov2.gc);
    }
  bool operator <(const innov& innov2)
    {
      if(gc <= innov2.gc)
          return true;
      else
          return false;      
    }
  bool operator >(const innov& innov2)
    {
      if(gc > innov2.gc)
          return true;
      else
          return false;      
    }
};
} //namespace ODNEATGRANDOM
#endif // INNOV_H
