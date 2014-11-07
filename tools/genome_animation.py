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
    
    return parser.parse_args()

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['file']      = 'evolution.log'
    defaults_opts['path']      = 'test'
    (options, args) = read_options(defaults_opts)

    start_genome = 1
    end_genome = 110002
    
    # make a phylogenetic tree end extract the lineage of interest 
    phylo_tree = create_phylo_tree(options.file, False)
    lineage = nx.shortest_path(phylo_tree, start_genome, end_genome)

    # cretae the animation files 
    lineage_animation(lineage, options.path)
