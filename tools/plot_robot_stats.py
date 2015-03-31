#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import sys

from datalog_stats import *

from optparse import OptionParser
from optparse import OptionGroup


# Process arguments
def read_options(defaults):
    parser = OptionParser()
    parser.add_option("-f", "--infile", 
                      dest="infile",
                      type="string",
                      default=defaults['infile']   ,  
                      help="the out log file [default "
                      +str(defaults['infile'])+"]")
    parser.add_option("-p", "--path", 
                      dest="path",
                      type="string",
                      default=defaults['path']   ,  
                      help="the genomes path [default "
                      +str(defaults['path'])+"]")
    parser.add_option("-i", "--robot", 
                      dest="robot",
                      type="int",
                      default=defaults['robot']   ,  
                      help="the robot id [default "
                      +str(defaults['robot'])+"]")
    return parser.parse_args()

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['infile']  = '../logs/out.log'
    defaults_opts['path']    = '../logs'
    defaults_opts['robot']   = 0

    (options, args) = read_options(defaults_opts)





    inputs = [
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # obs
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # itm
        1.0, # basket
        0.0, 1.0, 0.0, # nest (rgb) 
        1.0, 0.5, # landmark dist, orientation
        1.0 # bias
        ]
  

# extract raw statistics
stats = extract_data_by_rob(options.infile, options.robot, None, options.path)

# plot all
draw_robot_stats(stats)

