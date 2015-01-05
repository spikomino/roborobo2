#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
from optparse import OptionParser
from optparse import OptionGroup

from genome2graph import *

# Process arguments
def read_options(defaults):
    parser = OptionParser()
    parser.add_option("-f", "--infile", 
                      dest="infile",
                      type="string",
                      default=defaults['infile']   ,  
                      help="the genome file [default "
                      +str(defaults['infile'])+"]")
    parser.add_option("-o", "--outfile", 
                      dest="outfile",
                      type="string",
                      default=defaults['outfile']   ,  
                      help="the filename for the dotfile [default "
                      +str(defaults['outfile'])+"]")

    return parser.parse_args()

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['infile']    = 'test.gen'
    defaults_opts['outfile']   = None 
    (options, args) = read_options(defaults_opts)

    # Create weight matrix 
    m = process_weight_matrix(options.infile)
    
    # print the matrix 
    if options.outfile == None :
        print m
        
  
   
