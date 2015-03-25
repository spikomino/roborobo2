#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

from ffnet import ffnet, mlgraph, savenet, loadnet, exportnet
import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup

from genome2graph import *

# Process arguments
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
                      help="the genome file [default "
                      +str(defaults['infile'])+"]")
    return parser.parse_args()

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out']   = False
    defaults_opts['infile']    = 'test.gen'
    (options, args) = read_options(defaults_opts)

    # Create the graph object of the genome
    g = process_graph(options.infile)
     
    print 'Nodes '+str(len(g.nodes()))+':', g.nodes()
    print 'Edges '+str(len(g.edges()))+':', g.edges() 

    e = genome_elements(g) 
    
    print 'Elements:', e[1]

    # test values
    inputs = [
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  # obs
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,  # itm
        1.0, # basket
        1.0, 1.0, 1.0, # nest (rgb) 
        1.0, 1.0, # landmark dist, orientation
        1.0 # bias
        ]
  
    inputs = [
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # obs
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # itm
        1.0, # basket
        0.0, 1.0, 0.0, # nest (rgb) 
        0.0, 1.0, # landmark dist, orientation
        1.0 # bias
        ]
  



    print inputs

    outputs = execute_mlp(g, inputs)
    print outputs

    



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

   



