#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------


import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup

from os import listdir
from os.path import isfile, join, splitext

from subprocess import Popen, PIPE


from genome2graph import *
from phylo_graph import *


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
    parser.add_option("-m", "--moviefile", 
                      dest="moviefile",
                      type="string",
                      default=defaults['moviefile']   ,  
                      help="the animation filename [default "
                      +str(defaults['moviefile'])+"]")
    
    
    return parser.parse_args()


# rename files for ffmpeg needs
# in  : a list of sorted names (in video sequence) of dot files
# out : a video file.
##def dot2mp4(inlist):
 

# generate a sequence of filenames suited for ffmpeg
# in  : the length of the sequence, the filename extention 
# out : a liste of numbered filename padded with zeros
def generate_fname_seq(n, ext):
    l = []
    chars = len(str(n))
    for i in  xrange(n):
        fname = str(i)+'.'+ext
        fname = fname.zfill(chars+len(ext)+1)
        l.append(fname)
    return l

  


# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['file']      = 'out.log'
    defaults_opts['path']      = '../logs'
    defaults_opts['moviefile'] = 'out.mp4'
    (options, args) = read_options(defaults_opts)

    # make a phylogenetic tree
    pt = create_phylo_tree(options.file, True)
    
    path = nx.shortest_path(pt, 2, 120000)
    print path
    


    
    # print generate_fname_seq(100, 'png')
    G = process_graph('../logs/0000-0000120000.gen')

    
    H = graph_from_graph(G, '../logs/0002-0000000002.gen')

  
    
    nx.write_dot(G, 'g.dot')
    nx.write_dot(H, 'h.dot')
        
    dot2png('g.dot', 'g.png')
    dot2png('h.dot', 'h.png')
