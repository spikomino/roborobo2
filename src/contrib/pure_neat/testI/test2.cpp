#include "pure_neat/genome.h"
#include "pure_neat/network.h"
#include "pure_neat/gene.h"
#include "pure_neat/innov.h"

#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <chrono>
#include <random>

using namespace std;
using namespace PURENEAT;

int main(int argc, char* argv[])
{
    srand (time(NULL));


    // obtain a time-based seed:
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();


    load_neat_params ("forag.ne", false);
    int nbI = 2, nbO = 2;

    int nbR = 2;

    std::vector<int> shuf(nbR,0);

    for(int i = 0; i < nbR ; i++)
        shuf[i] = i;


    char * fname = new char[10];
    char* temp=new char[3];

    std::vector<Genome*> genomes,genomesAux;
    std::vector<Network*> nets;
    std::vector<int>  geneCounters;
    std::vector<int>  nodeCounters;

    //Creation of initial genomes
    for(int i = 0; i < nbR; i++)
    {
        Genome* g = new Genome(i, nbI,nbO,-1);//inputs,outputs, idRobot(common genes at creation)

        Network* n = g -> genesis();

        genomes.push_back(g);
        nets.push_back(n);

        strcpy( fname, "r" );
        sprintf(temp,"%d", i);
        strcat(fname,temp);
        strcat(fname, "-");
        strcat(fname,std::to_string(g -> genome_id).c_str());

        g -> print_to_filename(fname);
        nodeCounters.push_back(nbI + nbO + 1);
        geneCounters.push_back(n -> linkcount() +1);
    }

    //genomes[0] = genomes[0] ->mutate(1.0,0,1 * 100 + 0,nodeCounters[0],geneCounters[0]);
    genomes[0] -> mutate_add_node(0,nodeCounters[0],geneCounters[0],20);
    genomes[0] -> genome_id = 100;

    strcpy( fname, "r" );
    sprintf(temp,"%d", 0);
    strcat(fname,temp);
    strcat(fname, "-");
    strcat(fname,std::to_string(genomes[0] -> genome_id).c_str());

    genomes[0] -> print_to_filename(fname);

    //genomes[1] = genomes[1] ->mutate(1.0,1,1 * 100 + 1,nodeCounters[1],geneCounters[1]);
    //genomes[1] ->mutate_link_weights(1.0);
    genomes[1] -> mutate_add_node(1,nodeCounters[1],geneCounters[1],20);
    genomes[1]->genome_id = 101;

    strcpy( fname, "r" );
    sprintf(temp,"%d", 1);
    strcat(fname,temp);
    strcat(fname, "-");
    strcat(fname,std::to_string(genomes[1] -> genome_id).c_str());

    genomes[1] -> print_to_filename(fname);


    //X-over
    genomes[0] = genomes[0]->mate_multipoint(genomes[1], 2 * 100 + 0,1,1);

    strcpy( fname, "r" );
    sprintf(temp,"%d", 0);
    strcat(fname,temp);
    strcat(fname, "-");
    strcat(fname,std::to_string(genomes[0] -> genome_id).c_str());

    genomes[0] -> print_to_filename(fname);


    /*   int nbEpochs = 5;
    //Loop "generations"
    for(int i = 0; i < nbEpochs; i++)
    {
        //std::cout << "Generation: " << i + 1 <<std::endl;
        //Loop robots
        for(int k = 0; k < nbR; k++)
        {        for(int k = 0; k < nbR; k++)
            {
                //X-over
                genomes[k] = genomes[k]->mate_multipoint(genomes[(k + 1) % nbR], (i + 1) * 100 + k,1,1);

                strcpy( fname, "r" );
                sprintf(temp,"%d", k);
                strcat(fname,temp);
                strcat(fname, "-");
                strcat(fname,std::to_string(genomes[k] -> genome_id).c_str());

                genomes[k] -> print_to_filename(fname);
            }


            //X-over
            genomes[k] = genomes[k]->mate_multipoint(genomes[(k + 1) % nbR], (i + 1) * 100 + k,1,1);


            //std::cout << "  Robot: " << k << " -> ";
            //sigma idR idGenome nodeCounter geneCounter
            genomes[k] = (genomes[k])->mutate(1.0,k,(i + 1) * 100 + k,nodeCounters[k],geneCounters[k]);

            strcpy( fname, "r" );
            sprintf(temp,"%d", k);
            strcat(fname,temp);
            strcat(fname, "-");
            strcat(fname,std::to_string((genomes[k]) -> genome_id).c_str());

            (genomes[k]) -> print_to_filename(fname);

        }


        //Permutation of genomes. It emulates genomes moving from one robot to another
        //i.e. broadcast and selection
         shuffle (shuf.begin(), shuf.end(), std::default_random_engine(seed));
        std::cout << "Communications:    ";


        for(int k = 0; k < nbR; k++)
        {
            std::cout << shuf[k]<< " -> " << k << "  //  ";
            genomesAux.push_back(genomes[shuf[k]]);
        }
        std::cout << "\n\n\n";
        genomes = genomesAux;
        genomesAux.clear();
    }
*/
    /*
    //Test individual mutations
    std::vector<int> mut(5);
    int mutL = 10;
    int mutN = 0;
    int mutW = 0;
    double power = 1.0;
    int mutTog = 0;
    int mutReen = 0;
    mut[0] = mutL; mut[1] = mutN; mut[2] = mutW; mut[3] = mutTog; mut[4] = mutReen;

    //Loop robots
    for(int k = 0; k < nbR; k++)
    {
        //Loop mutation types
        for(unsigned int i = 0; i < mut.size(); i++)
        {
            //Loop specific mutation repetitions
            for(int j = 0; j < mut[i]; j++)
            {
                switch(i)
                {
                case 0:
                    (genomes[k]) -> mutate_add_link(k,geneCounters[k],20);
                    nets[k] = (genomes[k]) -> genesis();
                    std::cout << "add link" << " - ";
                    break;
                case 1:
                    (genomes[k])-> mutate_add_node(k,nodeCounters[k],geneCounters[k], 20);
                    nets[k] = (genomes[k])-> genesis();
                    std::cout << "add node" << " - ";
                    break;
                case 2:
                    (genomes[k])-> mutate_link_weights(power);
                    nets[k] = (genomes[k])-> genesis();
                    std::cout << "changed weights" << " - ";
                    break;
                case 3:
                    (genomes[k])-> mutate_toggle_enable(1);//Number of genes to be toggled
                    nets[k] = (genomes[k])-> genesis();
                    std::cout << "toggle enable" << " - ";
                    break;
                case 4:
                    (genomes[k])-> mutate_gene_reenable();
                    nets[k] = (genomes[k])-> genesis();
                    std::cout << "reenable" << " - ";
                    break;
                }
                strcpy( fname, "r" );
                sprintf(temp,"%d", k);
                strcat(fname,temp);
                strcat(fname, "-");
                strcat(fname,std::to_string((genomes[k]) -> genome_id).c_str());

                (genomes[k]) -> print_to_filename(fname);
                (genomes[k]) -> genome_id += 100;
            }

        }
         std::cout << std::endl;
    }
*/
    delete[] fname;
}
