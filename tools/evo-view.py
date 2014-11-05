#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup

from os import listdir
from os.path import isfile, join, splitext

from subprocess import Popen, PIPE


# process files of the form 
#

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


# this function creates a dictionary with num robots lists 
# like this one with 3 robots {0: [0], 1: [1], 2: [2]}
def create_genome_lists(fname):
    G = {}
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
       
        if data != [] and data[0] == '[initRobot]' :
            # read the id of the robot (which = genome id)
            gid = int(data[1].split('=')[1]) 
            G[gid] = [gid]
    fh.close()   
    return G


# extract the genome form a line and put it in the right list
def process_robot_entry(d):
    if d[6] != '][Genome:' :
        return (None, None))
    idtrace = int(d[8].split('=')[1]) 
    mom     = int(d[9].split('=')[1]) 
    return (idtrace, mom)


# read the log file and fill the genome lists.
def process_file(fname, G):
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and  data[0] == '[Robot:' :
            (id, mom) = process_robot_entry(data)
            if id != None :
                
    fh.close()   
    


# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out']   = False
    defaults_opts['file']      = 'out.log'
    defaults_opts['path']      = None
    (options, args) = read_options(defaults_opts)


    
    G = create_genome_lists(options.file)
    print G
