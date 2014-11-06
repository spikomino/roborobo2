#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup

# python evo-view.py -f ../logs/out.log
# process files of the form 


#[Robot: id=0 iteration=20 birthdate=0 fitness=0.000000 sigma=0.000000 ][Genome: id=0 idtrace=10000 mom=2 dad=-1 ]
#[Robot: id=2 iteration=20 birthdate=0 fitness=0.000000 sigma=0.000000 ][Genome: id=2 idtrace=10002 mom=0 dad=-1 ]
#[Robot: id=1 iteration=20 birthdate=0 fitness=0.000000 sigma=0.000000 ][Genome: id=1 idtrace=10001 mom=1 dad=-1 ]
#[gen:1;pop:3]
#[Robot: id=2 iteration=40 birthdate=19 fitness=0.000000 sigma=0.000000 ][Genome: id=2 idtrace=20002 mom=10000 dad=-1 ]
#[Robot: id=1 iteration=40 birthdate=19 fitness=0.000000 sigma=0.000000 ][Genome: id=1 idtrace=20001 mom=10001 dad=-1 ]
#[Robot: id=0 iteration=40 birthdate=19 fitness=0.000000 sigma=0.000000 ][Genome: id=0 idtrace=20000 mom=10000 dad=-1 ]
#[gen:2;pop:3]
#[Robot: id=0 iteration=60 birthdate=39 fitness=0.000000 sigma=0.000000 ][Genome: id=0 idtrace=30000 mom=20002 dad=-1 ]
#[Robot: id=2 iteration=60 birthdate=39 fitness=0.000000 sigma=0.000000 ][Genome: id=2 idtrace=30002 mom=20000 dad=-1 ]
#[Robot: id=1 iteration=60 birthdate=39 fitness=0.000000 sigma=0.000000 ][Genome: id=1 idtrace=30001 mom=20001 dad=-1 ]
#[gen:3;pop:3]
#[Robot: id=2 iteration=80 birthdate=59 fitness=0.000000 sigma=0.000000 ][Genome: id=2 idtrace=40002 mom=30002 dad=-1 ]
#[Robot: id=1 iteration=80 birthdate=59 fitness=0.000000 sigma=0.000000 ][Genome: id=1 idtrace=40001 mom=30001 dad=-1 ]
#[Robot: id=0 iteration=80 birthdate=59 fitness=0.000000 sigma=0.000000 ][Genome: id=0 idtrace=40000 mom=30000 dad=-1 ]





def read_options(defaults):
    parser = OptionParser()
    parser.add_option("-w", "--win-output", 
                      dest="win_out",
                      default=defaults['win_out']  ,
                      action="store_true",  
                      help="display in a window very slow [default "
                      +str(defaults['win_out'])+"]") 
    parser.add_option("-f", "--file", 
                      dest="file",
                      type="string",
                      default=defaults['file']   ,  
                      help="the log file  [default "
                      +str(defaults['file'])+"]")
    parser.add_option("-p", "--path", 
                      dest="path",
                      type="string",
                      default=defaults['path']   ,  
                      help="the path to genome files [default "
                      +str(defaults['path'])+"]")
    
    return parser.parse_args()




# extract the genome form a line and put it in the right list
# in  : a line from the evolution log
# out : a tuple (rob_id, id_trace, mom, dad)
def process_robot_entry(d):
    if d[6] != '][Genome:' :
        return (None, None, None, None)
    rob     = int( d[7].split('=')[1]) 
    idtrace = int( d[8].split('=')[1]) 
    mom     = int( d[9].split('=')[1])
    dad     = int(d[10].split('=')[1])
    return (rob,idtrace, mom, dad)

# read the number of robots & creates a dictionary with num robots lists 
# like this one with 3 robots {0: [(tuples), ()], 1: [], 2: []}
# in  : a log file name
# out : a dictionary with robots id as keys to lists to tuple 
def process_file(fname):
    G = {}
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and data[0] == '[initRobot]' :
            # read the id of the robot (which is genome id)
            gid = int(data[1].split('=')[1]) 
            G[gid] = []
    fh.close()   

    # fill the lists 
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and  data[0] == '[Robot:' :
            (r, id, mom, dad) = process_robot_entry(data)
            if r != None :
                G[r].append((id, mom, dad))
    fh.close()   
    return G

# trac the offspring of a gene and add them to the graph 
def trac_genome(gl, id, G, col):
   
    for i in gl :
        for g in gl[i] :
            (tr, m, d) = g
            if m == id :               
                G.add_node(tr)
                G.node[tr]['agent'] = i
                G.node[tr]['id']    = tr
                G.node[tr]['mom']   = m
                G.node[tr]['dad']   = d
                G.node[tr]['color'] = col[i%len(col)]
                G.add_edge(id, tr)
                trac_genome(gl, tr, G, col)


# Crete a phylogenetic tree 
# in  : log file in the correct format see above
# out : the graphe og the phylo tree (root nodes are the initial genes)
def create_phylo_graph(fname, save=False): 
    colors = ['chartreuse', 'chocolate', 'cadetblue', 'cornflowerblue', 'cyan',
              'darkorange', 'darkviolet', 'deeppink']
  
    # create a graph
    G=nx.DiGraph()        

    # fill the lists with all the genes 
    gl = process_file(fname)
    
    # create the phylo-tree
    for n in gl.keys():
        G.add_node(n) # the root node (the initial gene)
        G.node[n]['agent'] = n
        G.node[n]['id']    = n
        G.node[n]['color'] = colors[n%len(colors)]
        trac_genome(gl, n, G, colors)
    
    # write the file 
    if save :
        nx.write_dot(G, os.path.splitext(fname)[0]+'-phylo.dot')
    
    return G
   


# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out']   = False
    defaults_opts['file']      = 'out.log'
    defaults_opts['path']      = None
    (options, args) = read_options(defaults_opts)

  
    # fill the lists with all the genes 
    g = create_phylo_graph(options.file, True)
    
    