#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup
from genome2graph import *


def read_options(defaults):
    parser = OptionParser()
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
    defaults_opts['path']      = 'test'
    (options, args) = read_options(defaults_opts)

    # read the directory 
    fl = []
    for f in os.listdir(options.path):
        if f.endswith('.gen'):
            fl.append(f)
    fl.sort()

    # cretae the animation files 
    genome_animation(fl, options.path)
