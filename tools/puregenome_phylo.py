#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup
from puregenome2graph import *

# python phylo_graph.py -f test/evolution.log -d test/evolution-phylo.dot -p test/evolution-phylo.png


# process files of the form 
#Note that there are some added columns

#[Robot: id=39 iteration=464200 birthdate=463999 fitness=0.015625 items=3 sigma=0.300000 nodeId=2320 geneId=4637 ][Genome: id=39 idtrace=23210039 mom=23200043 dad=-1 ]
#[Robot: id=32 iteration=464200 birthdate=463999 fitness=0.005025 items=1 sigma=0.300000 nodeId=2320 geneId=4637 ][Genome: id=32 idtrace=23210032 mom=23200032 dad=-1 ]
#[Robot: id=36 iteration=464200 birthdate=463999 fitness=0.015625 items=3 sigma=0.300000 nodeId=2320 geneId=4637 ][Genome: id=36 idtrace=23210036 mom=23200043 dad=-1 ]
#[Robot: id=44 iteration=464200 birthdate=463999 fitness=0.010989 items=0 sigma=0.300000 nodeId=2320 geneId=4637 ][Genome: id=44 idtrace=23210044 mom=23200034 dad=-1 ]
#[Robot: id=43 iteration=464200 birthdate=463999 fitness=0.015625 items=1 sigma=0.300000 nodeId=2319 geneId=4635 ][Genome: id=43 idtrace=23210043 mom=23200039 dad=-1 ]
#[Robot: id=26 iteration=464200 birthdate=463999 fitness=0.026316 items=3 sigma=0.300000 nodeId=2320 geneId=4637 ][Genome: id=26 idtrace=23210026 mom=23200002 dad=-1 ]


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
    parser.add_option("-x", "--xover", 
                      dest="xover",
                      action="store_true",                      
                      default=defaults['xover'],  
                      help="Flag for adding dad links to phylogenetic tree")
    parser.add_option("-c", "--prune", 
                      dest="prune",
                      action="store_true",                      
                      default=defaults['prune'],  
                      help="Flag for pruning extinct branches in phylogenetic tree")
    return parser.parse_args()


# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out']   = False
    defaults_opts['infile']    = 'evolution.log'
    defaults_opts['dotfile']   = None
    defaults_opts['pngfile']   = None
    defaults_opts['xover']     = False
    defaults_opts['prune']     = False
    (options, args) = read_options(defaults_opts)
    
  
    # create the phylogenetic tree
    g = create_phylo_tree(options.infile,xover=options.xover,prune=options.prune)
    
    # dot ? png ? window ?
    if options.dotfile != None :
        nx.write_dot(g, options.dotfile)
        if options.pngfile != None :
            dot2png(options.dotfile, options.pngfile)
   
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

   
