#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup
from genome2graph import *

# python phylo_graph.py -f test/evolution.log -d test/evolution-phylo.dot -p test/evolution-phylo.png


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
    parser.add_option("-f", "--infile", 
                      dest="infile",
                      type="string",
                      default=defaults['infile']   ,  
                      help="the evolution log file [default "
                      +str(defaults['infile'])+"]")
    parser.add_option("-d", "--dotfile", 
                      dest="dotfile",
                      type="string",
                      default=defaults['dotfile']   ,  
                      help="the filename for the dotfile [default "
                      +str(defaults['dotfile'])+"]")
    parser.add_option("-p", "--pngfile", 
                      dest="pngfile",
                      type="string",
                      default=defaults['pngfile']   ,  
                      help="the file to save the png file [default "
                      +str(defaults['pngfile'])+"]")
    parser.add_option("-g", "--path", 
                      dest="path",
                      type="string",
                      default=defaults['path']   ,  
                      help="the path to genome files [default "
                      +str(defaults['path'])+"]")

    return parser.parse_args()


# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out']   = False
    defaults_opts['infile']    = 'test/out.log'
    defaults_opts['dotfile']   = 'test/out-pylo.dot'
    defaults_opts['pngfile']   = 'test/out-pylo.png'
    defaults_opts['path']      = 'test'
    (options, args) = read_options(defaults_opts)
    
  
    # create the phylogenetic tree
    g = create_phylo_tree(options.infile,
                          dotfile=options.dotfile, 
                          pngfile=options.pngfile,
                          genpath=options.path)
    
     
    # If we draw things ... prepare    
    if options.win_out: 
        pylab.ion()
        fig = pylab.figure(num=None, figsize=(5, 5), dpi=100)
        pylab.show()
        pylab.clf()
        p=nx.circular_layout(g)
        nx.draw(g)
        pylab.draw()
        print 'Press enter to exit'
        raw_input()

   
