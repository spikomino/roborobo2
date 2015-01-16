#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup
from genome2graph import *


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
    defaults_opts['dotfile']   = 'test/out-pylo2.dot'
    defaults_opts['pngfile']   = 'test/out-pylo2.png'
    defaults_opts['path']      = 'test'
    (options, args) = read_options(defaults_opts)
    
  
    # create the phylogenetic tree
    g = create_phylo_tree_dist(options.path,
                               dotfile=options.dotfile, 
                               pngfile=options.pngfile)
    
     
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

   
