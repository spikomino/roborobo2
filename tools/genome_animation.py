#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------


import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup

from os import listdir
from os.path import isfile, join, splitext

from subprocess import Popen, PIPE, call


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



  


# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['file']      = 'out.log'
    defaults_opts['path']      = '../logs'
    defaults_opts['moviefile'] = 'out.mp4'
    (options, args) = read_options(defaults_opts)

    start_genome = 1
    end_genome = 110002
    tmpdir = '/tmp'
    movie_filename = 'out.mp4'
    frame_rate = 1
    
    # make a phylogenetic tree end extract the lineage of interest 
    phylo_tree = create_phylo_tree(options.file, True)
    lineage = nx.shortest_path(phylo_tree, start_genome, end_genome)
    print lineage
    
    # read the genome filenames and put then in a list
    genomes_files =[]
    for g in lineage :
        name=str(g)+'.gen'
        name=name.zfill(10+4)
        for f in listdir(options.path) :
            if f.endswith(name):
                genomes_files.append(f)
   
    # create dot files 
    last_genome_file = genomes_files[-1]
    last_genome = process_graph(options.path+'/'+last_genome_file)
    for gf in genomes_files :
        graph_of_g = graph_from_graph(last_genome, options.path+'/'+gf)
        nx.write_dot(graph_of_g, options.path+'/'+gf[:-4]+'.dot')

    # create png files 
    png_filenames = generate_fname_seq(len(genomes_files), 'png')
    i=0
    for gf in genomes_files :
        dot2png(options.path+'/'+gf[:-4]+'.dot', tmpdir+'/'+png_filenames[i])
        i=i+1
    zpad = len(str(i)) # the size needed below int ffmpeg -i format  
    
    # make movie
    # ffmpeg -framerate 1 -i /tmp/%02d.png -c:v libx264 -pix_fmt yuv420p out.mp4
    cmd_line = [
        'ffmpeg',
        '-framerate '+str(frame_rate)+' -i '+tmpdir+'/'+'%0'+str(zpad)+'d.png -y  -loglevel debug -c:v libx264 -pix_fmt yuv420p '+movie_filename]
    
    print cmd_line
    
    #call(cmd_line)
        
    # clean up by removing dot & png files
    i=0
    for gf in genomes_files :
        #os.remove(options.path+'/'+gf[:-4]+'.dot')
        #os.remove(tmpdir+'/'+png_filenames[i])
        i=i+1

    
