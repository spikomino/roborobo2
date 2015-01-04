#ifndef ODNEATINNOV_H
#define ODNEATINNOV_H
#include <time.h>
#include <iostream>
namespace ODNEAT
{
//Struct for innovation numbers of topological innovations (nodes and links)
//As in odNEAT, it uses timestamps as historical markers
struct innov
{
timespec timestamp;

bool operator ==(const innov& innov2)
{
    return ((timestamp.tv_sec == innov2.timestamp.tv_sec) && (timestamp.tv_nsec == innov2.timestamp.tv_nsec));
}
bool operator <(const innov& innov2)
{
    if((timestamp.tv_sec < innov2.timestamp.tv_sec)
            || ((timestamp.tv_sec == innov2.timestamp.tv_sec)
                && (timestamp.tv_nsec < innov2.timestamp.tv_nsec)))
        return true;
    else
        return false;
}
bool operator >(const innov& innov2)
{
    if((timestamp.tv_sec > innov2.timestamp.tv_sec)
            || ((timestamp.tv_sec == innov2.timestamp.tv_sec)
                && (timestamp.tv_nsec > innov2.timestamp.tv_nsec)))
        return true;
    else
        return false;
}
friend std::ostream& operator << (std::ostream& os, const innov& innov2print)
{
    time_t tt = innov2print.timestamp.tv_sec;
    os << ctime(&tt) << " " << innov2print.timestamp.tv_nsec;
    return os;
}
};
} //namespace ODNEAT
#endif // INNOV_H
