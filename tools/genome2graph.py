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
