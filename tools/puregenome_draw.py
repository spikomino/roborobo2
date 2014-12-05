#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup

from puregenome2graph import *

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
    return parser.parse_args()

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out']   = False
    defaults_opts['infile']    = 'test.gen'
    defaults_opts['dotfile']   = None
    defaults_opts['pngfile']   = None
    (options, args) = read_options(defaults_opts)

    # Create the graph object of the genome
    g = process_graph(options.infile)
    
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

   
