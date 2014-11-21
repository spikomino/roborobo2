
#include "pure_neat/genome.h"
#include "pure_neat/network.h"
#include "pure_neat/gene.h"
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <signal.h>

using namespace std;
using namespace PURENEAT;

int main(int argc, char* argv[])
{
    load_neat_params ("forag.ne", false);
    int nbI = 2, nbO = 1;
    int idR = 0;
    Genome* g = new Genome(nbI,nbO,idR);//inputs,outputs, idRobot

    Network* n = g -> genesis();

    char * fname = new char[10];
    strcpy(fname, (std::to_string(g -> genome_id++).c_str()));
    g -> print_to_filename(fname);

    int curNodeId = nbI + nbO + 1;
    int currInnov = n -> linkcount() +1;

    //Test individual mutations
    std::vector<int> mut(5);
    int mutL = 1;
    int mutN = 0;
    int mutW = 0;
    double power = 1.0;
    int mutTog = 0;
    int mutReen = 0;
    mut[0] = mutL; mut[1] = mutN; mut[2] = mutW; mut[3] = mutTog; mut[4] = mutReen;

    for(unsigned int i = 0; i < mut.size(); i++)
    {
        for(int j = 0; j < mut[i]; j++)
        {
            switch(i)
            {
            case 0:
                g -> mutate_add_link(idR,currInnov,20);
                n = g -> genesis();
                std::cout << i << " - ";
                break;
            case 1:
                g -> mutate_add_node(idR,curNodeId,currInnov, 20);
                n = g -> genesis();
                std::cout << i << " - ";
                break;
            case 2:
                g -> mutate_link_weights(power);
                n = g -> genesis();
                std::cout << i << " - ";
                break;
            case 3:
                g -> mutate_toggle_enable(1);//Number of genes to be toggled
                n = g -> genesis();
                std::cout << i << " - ";
                break;
            case 4:
                g -> mutate_gene_reenable();
                n = g -> genesis();
                std::cout << i << " - ";
                break;
            }
            strcpy(fname, (std::to_string(g -> genome_id++).c_str()));
            g -> print_to_filename(fname);
        }
        std::cout << std::endl;
    }


    for(int i = 0; i < n->linkcount();i++ )
    {
        if((i % 2) == 0)
            ((g->genes[i])->lnk)->weight = 5.0;
        else
            ((g->genes[i])->lnk)->weight = (-1) * (i % 2) * 5.0;
    }

    strcpy(fname, (std::to_string(g -> genome_id++).c_str()));

    g -> print_to_filename(fname);
    unsigned int nbSteps = 10;

    double inputs[nbI];

    std::vector < double >outputs;
    n = g -> genesis();
    for(unsigned int i = 0; i <= nbSteps;i++)
    {
        for(int j = 0; j < nbI;j++)
        {
            inputs[j] = i * (1.0 / nbSteps);
        }
        n->load_sensors(&(inputs[0]));
        n->activate();

        for (std::vector < NNode * >::iterator out_iter = n->outputs.begin ();
             out_iter != n->outputs.end (); out_iter++)
        {
            outputs.push_back ((*out_iter)->activation);
            std::cout << (*out_iter)->activation << " - ";
        }
        std::cout << std::endl;

    }
    delete[] fname;
}
