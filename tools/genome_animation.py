#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup
from genome2graph import *


def read_options(defaults):
    parser = OptionParser()
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
    parser.add_option("-s", "--start", 
                      dest="start",
                      type="int",
                      default=defaults['start']   ,  
                      help="the start node [default "
                      +str(defaults['start'])+"]")
    parser.add_option("-e", "--end", 
                      dest="end",
                      type="int",
                      default=defaults['end']   ,  
                      help="the end node [default "
                      +str(defaults['end'])+"]")


    return parser.parse_args()

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['file']      = 'out.log'
    defaults_opts['path']      = 'test'
    defaults_opts['start']     = 0
    defaults_opts['end']       = 0
    (options, args) = read_options(defaults_opts)

    start_genome = options.start
    end_genome   = options.end
    
    # make a phylogenetic tree end extract the lineage of interest 
    phylo_tree = create_phylo_tree(options.file, False)
    lineage = nx.shortest_path(phylo_tree, start_genome, end_genome)

    # cretae the animation files 
    lineage_animation(lineage, options.path)
