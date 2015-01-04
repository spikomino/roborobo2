#include "odneat/gene.h"
#include "odneat/genome.h"
#include "odneat/innov.h"

#include <iostream>
#include <string.h>
#include <sstream>
#include <time.h>
using namespace ODNEAT;

Gene::Gene(double w, NNode *inode, NNode *onode, bool recur, innov innovNum)
{
  lnk = new Link(w, inode, onode, recur);
  innovation_num = innovNum;
  
  enable = true;
}


Gene::Gene(Gene *g,NNode *inode,NNode *onode) 
{
  lnk=new Link((g->lnk)->weight,inode,onode,(g->lnk)->is_recurrent);
  innovation_num=g->innovation_num;
  enable=g->enable;
}

Gene::Gene(const char *argline, std::vector<NNode*> &nodes) 
{
  //Gene parameter holders
/*  time_t inodeSec;
  long inodeNsec;

  time_t onodeSec;
  long onodeNsec;

  time_t geneSec;
  long geneNsec;*/

  //Set time zone to UTC
  char *tz;

  tz = getenv("TZ");
  setenv("TZ", "", 1);
  tzset();

  NNode *inode;
  NNode *onode;
  double weight;
  int recur;
  
  std::vector<NNode*>::iterator curnode;
  
  char inSec[30], inNsec[30];
  char outSec[30], outNsec[30];
  char geneSecS[30], geneNsecS[30];

  innov innovNodeIn, innovNodeOut;

  //Get the gene parameters
  
  std::stringstream ss(argline);
  ss >> inSec >> inNsec >> outSec >> outNsec >> weight >> recur >> geneSecS >> geneNsecS >> enable;

  struct tm tm;
  strptime(inSec, "%a-%m-%d-%Y-%H:%M:%S", &tm);

  innovNodeIn.timestamp.tv_sec= timegm(&tm);
  innovNodeIn.timestamp.tv_nsec= atol(inNsec);

  struct tm tm2;
  strptime(outSec, "%a-%m-%d-%Y-%H:%M:%S", &tm2);

  innovNodeOut.timestamp.tv_sec= mktime(&tm2);
  innovNodeOut.timestamp.tv_nsec= atol(outNsec);

  struct tm tm3;
  strptime(geneSecS, "%a-%m-%d-%Y-%H:%M:%S", &tm3);
  innovation_num.timestamp.tv_sec = mktime(&tm3);
  innovation_num.timestamp.tv_nsec = atol(geneNsecS);





  //Get a pointer to the input node
  curnode=nodes.begin();
  while(!(((*curnode)->node_id)==innovNodeIn))
    ++curnode;
  inode=(*curnode);
  
  //Get a pointer to the output node
  curnode=nodes.begin();
  while(!(((*curnode)->node_id)==innovNodeOut))
    ++curnode;
  onode=(*curnode);
  
  lnk=new Link(weight,inode,onode,recur);
  if (tz)
      setenv("TZ", tz, 1);
  else
      unsetenv("TZ");
  tzset();
}

Gene::Gene(const Gene& gene)
{
	innovation_num = gene.innovation_num;
	enable = gene.enable;

	lnk = new Link(*gene.lnk);
}

Gene::~Gene() {
	delete lnk;
}

void Gene::print_to_file(std::ostream &outFile) 
{
    time_t timeStamp1 = (lnk->in_node)->node_id.timestamp.tv_sec;
    struct tm *t1 = gmtime(&timeStamp1);
    char time1[30];

    /*time1= asctime(t1);
    time1[strlen(time1)-1] = '\0';*/

    strftime(time1, 30, "%a-%m-%d-%Y-%H:%M:%S", t1);

    char tempbuf[128];

    sprintf(tempbuf, "gene %s %.9ld ",
            &(time1[0]), (lnk->in_node)->node_id.timestamp.tv_nsec);
    outFile << tempbuf;

    time_t timeStamp2 = (lnk->out_node)->node_id.timestamp.tv_sec;
    struct tm *t2 = gmtime(&timeStamp2);

    char time2[30];
    /*time2 = asctime(t2);
    time2[strlen(time2)-1] = '\0';*/

    strftime(time2, 30, "%a-%m-%d-%Y-%H:%M:%S", t2);

    char tempbuf2[128];

    sprintf(tempbuf2, "%s %.9ld ",
            &(time2[0]), (lnk->out_node)->node_id.timestamp.tv_nsec);
    outFile << tempbuf2;

    t1 = gmtime(&(innovation_num.timestamp.tv_sec));
    /*time1 = asctime(t1);
    time1[strlen(time1)-1] = '\0';*/

    strftime(time1, 30, "%a-%m-%d-%Y-%H:%M:%S", t1);

    char tempbuf3[128];
    sprintf(tempbuf3, "%f %d %s %.9ld %d \n", (lnk->weight), (lnk->is_recurrent),
            time1, innovation_num.timestamp.tv_nsec, enable);
    outFile << tempbuf3;
/*
  std::cout << "Here are: " << ctime(&((lnk->in_node)->node_id.timestamp.tv_sec)) << " secs "<< std::endl;
  outFile<<"gene ";
  outFile<< ctime(&((lnk->in_node)->node_id.timestamp.tv_sec)) <<" ";
  outFile<<(lnk->in_node)->node_id.timestamp.tv_nsec<<" ";
  outFile<<ctime(&((lnk->out_node)->node_id.timestamp.tv_sec))<<" ";
  outFile<<(lnk->out_node)->node_id.timestamp.tv_nsec<<" ";
  outFile<<(lnk->weight)<<" ";
  outFile<<(lnk->is_recurrent)<<" ";
  outFile<<ctime(&(innovation_num.timestamp.tv_sec))<<" ";
  outFile<<innovation_num.timestamp.tv_nsec<<" ";
  outFile<<enable<<std::endl;*/
}
