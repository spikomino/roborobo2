#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import networkx as nx
import os
from subprocess import Popen, PIPE

# process trait of genes (not implemented)
# in  : a graph object and the splited line from the genome file
#       called exclusively by process_graph 
def process_trait(g, d):
    return 

# add a node in the network
# in  : a graph object and the splited line from the genome file
#       called exclusively by process_graph 
def process_node(g, d):
    shapes = ['circle', 'box', 'diamond', 'doublecircle']
    colors = ['yellow', 'gray', 'darkorchid', 'cyan']    
    n = int(d[1])
    g.add_node(n)
    g.node[n]['trait'] = d[2] # ? 
    g.node[n]['type']  = d[3] # 0=>output, 1=>input
    g.node[n]['lbl']   = d[4] # 0=>hidden, 1=>input, 2=>output, 3=>bias
    g.node[n]['shape'] = shapes[ int(d[4]) ]
    g.node[n]['color'] = colors[ int(d[4]) ]

# add a link in the network
# in  : a graph object and the splited line from the genome file
#       called exclusively by process_graph 
def process_gene(g, d):
    n0 = d[2]
    n1 = d[3]
    w  = d[4]
    g.add_edge(n0, n1, weight=float(w))
    g.edge[n0][n1]['trait']    =d[1] # ?
    g.edge[n0][n1]['recurent'] =d[5] # 0/1 yes/no
    g.edge[n0][n1]['inov_num'] =d[6] # int
    g.edge[n0][n1]['mut_num']  =d[7] # float
    g.edge[n0][n1]['enable']   =d[8] # 0/1 yes/no
    g.edge[n0][n1]['style'] = 'solid'
    g.edge[n0][n1]['color'] = 'black'
    if d[8]=='0':
        g.edge[n0][n1]['style'] = 'dashed' 
        g.edge[n0][n1]['color'] = 'red'

# Reads a neat genome filename and greate the coresponding graph
# in  : a filename of the genome (neat format)
# out : a graph object of the genome
def process_graph(fname):
    G=nx.DiGraph()
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data[0] == 'genomestart' :
            gid = data[1]
            G.graph['name'] = os.path.splitext(fname)[0] # remove extention
        elif data[0] == 'trait' :
            process_trait(G,data)
        elif data[0] == 'node' : 
            process_node(G,data)
        elif data[0] == 'gene' :  
            process_gene(G,data)
        elif data[0] == 'genomeend' :
            if gid != data[1] : 
                print 'Error: mismatch genome end tag'
                return None
        else :
            print data
            print 'Error: unexpected token'
            return None
    fh.close()   
    return G

# Create a graph from a larger by hiding nodes and edges not fname
# in  : a graph object (the larger one) and a genome filename (the smaller) 
# out : the graph object
def graph_from_graph(G, fname):
    
    # read the new genome
    small = process_graph(fname)

    # copy the larger graph 
    new = G.copy() 

    # enable all that was disabled
    for v in small.edges_iter():
        (n1,n2) = v
        new.edge[n1][n2]['style'] = small.edge[n1][n2]['style']
        new.edge[n1][n2]['color'] = small.edge[n1][n2]['color']
    
    # make nodes not in small invisible 
    for n in [v for v in G if v not in small ] :
        new.node[n]['style'] = 'invis'
    # make edges not in small invisible 
    for v in [e for e in G.edges_iter() if e not in small.edges()]:
        (n1,n2) = v
        new.edge[n1][n2]['style'] = 'invis'
    return new

################################################################################
# phylogenetic related functions
################################################################################

# extract the genome form a line and put it in the right list
# in  : a line from the evolution log
# out : a tuple (rob_id, id_trace, mom, dad)
def process_robot_entry(d):
    if d[6] != '][Genome:' :
        return (None, None, None, None)
    rob     = int( d[7].split('=')[1]) 
    idtrace = int( d[8].split('=')[1]) 
    mom     = int( d[9].split('=')[1])
    dad     = int(d[10].split('=')[1])
    return (rob,idtrace, mom, dad)

# read the number of robots & creates a dictionary with num robots lists 
# like this one with 3 robots {0: [(tuples), ()], 1: [], 2: []}
# in  : a log file name
# out : a dictionary with robots id as keys to lists to tuple 
def process_file(fname):
    G = {}
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and data[0] == '[initRobot]' :
            # read the id of the robot (which is genome id)
            gid = int(data[1].split('=')[1]) 
            G[gid] = []
    fh.close()   

    # fill the lists 
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and  data[0] == '[Robot:' :
            (r, id, mom, dad) = process_robot_entry(data)
            if r != None :
                G[r].append((id, mom, dad))
    fh.close()   
    return G

# trac the offspring of a gene and add them to the graph 
def trac_genome(gl, id, G, col):
    for i in gl :
        for g in gl[i] :
            (tr, m, d) = g
            if m == id :               
                G.add_node(tr)
                G.node[tr]['agent'] = i
                G.node[tr]['id']    = tr
                G.node[tr]['mom']   = m
                G.node[tr]['dad']   = d
                G.node[tr]['color'] = col[i%len(col)]
                G.add_edge(id, tr)
                trac_genome(gl, tr, G, col)


# Crete a phylogenetic tree 
# in  : log file in the correct format see above
# out : the graphe og the phylo tree (root nodes are the initial genes)
def create_phylo_tree(fname, save=False, dotfile='philo.dot'): 
    colors = ['chartreuse', 'chocolate', 'cadetblue', 'cornflowerblue', 'cyan',
              'darkorange', 'darkviolet', 'deeppink']
  
    # create a graph
    G=nx.DiGraph()        

    # fill the lists with all the genes 
    gl = process_file(fname)

    # create the phylo-tree
    for n in gl.keys():
        G.add_node(n) # the root node (the initial gene)
        G.node[n]['agent'] = n
        G.node[n]['id']    = n
        G.node[n]['color'] = colors[n%len(colors)]
        trac_genome(gl, n, G, colors)
    
    # write the file 
    if save :
        nx.write_dot(G, dotfile)
    
    return G

################################################################################
# Animation related functions
################################################################################

# Convert a DOF file to PNG image 
#   ->  execute: dot dotfile | gvcolor | dot -Tpng -o pngfile
# in : the dot filename and the png filename
def dot2png(dotfile, pngfile):
    p1 = Popen(["dot", dotfile], stdout=PIPE)
    p2 = Popen(["gvcolor"], stdin=p1.stdout, stdout=PIPE)
    p1.stdout.close()  
    p3 = Popen(["dot", "-Tpng", "-o", pngfile],stdin=p2.stdout,stdout=PIPE)
    p2.stdout.close()  
    p3.communicate()[0]
    
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

# create multiple png files suited for an animation
# in  : a phylogenetic line, the genomes dir &  the output dir
# out : png files 
def lineage_animation(phylo_line, indir, outdir=None):

    # by default the same 
    if outdir == None :
        outdir = indir
    
    # read the genome filenames and put then in a list
    genomes_files =[]
    for g in phylo_line :
        name=str(g)+'.gen'
        name=name.zfill(10+4)
        for f in os.listdir(indir) :
            if f.endswith(name):
                genomes_files.append(f)
    
    # create dot files 
    last_genome_file = genomes_files[-1]
    last_genome = process_graph(indir+'/'+last_genome_file)
    for gf in genomes_files :
        graph_of_g = graph_from_graph(last_genome, indir+'/'+gf)
        nx.write_dot(graph_of_g, outdir+'/'+gf[:-4]+'.dot')
        
    # create png files 
    png_filenames = generate_fname_seq(len(genomes_files), 'png')
    i=0
    for gf in genomes_files :
        dot2png(outdir+'/'+gf[:-4]+'.dot', outdir+'/'+png_filenames[i])
        i=i+1

    # print the command line to create the movie 
    zpad = len(str(i)) 
    cmd_line = 'ffmpeg -framerate 1 -i '+outdir+'/'+'%0'+str(zpad)+'d.png  -c:v libx264 -pix_fmt yuv420p '+outdir+'/out.mp4';
    print 'To create the movies run the following command:'
    print cmd_line

    # scratchapd no meant to be used
    # to clean up from the script (removing dot & png files)
    #i=0
    #for gf in genomes_files :
    #os.remove(options.path+'/'+gf[:-4]+'.dot')
    #os.remove(tmpdir+'/'+png_filenames[i])
    #i=i+1

        
    
