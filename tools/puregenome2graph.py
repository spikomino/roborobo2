#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import networkx as nx
import os
from subprocess import Popen, PIPE

# add a node in the network
# in  : a graph object and the splited line from the genome file
#       called exclusively by process_graph 
def process_node(g, d):
    shapes = ['circle', 'box', 'diamond', 'doublecircle']
    colors = ['yellow', 'gray', 'darkorchid', 'cyan']    
    n = int(d[1])
    g.add_node(n)
    g.node[n]['type']  = d[3] # 0=>output, 1=>input
    g.node[n]['lbl']   = d[4] # 0=>hidden, 1=>input, 2=>output, 3=>bias
    g.node[n]['shape'] = shapes[ int(d[4]) ]
    g.node[n]['color'] = colors[ int(d[4]) ]
    g.node[n]['style'] = 'filled'

# add a link in the network
# in  : a graph object and the splited line from the genome file
#       called exclusively by process_graph 
def process_gene(g, d):
    n0 = int(d[2])
    n1 = int(d[3])
    w  = float(d[4])
    g.add_edge(n0, n1, weight=w)
    g.edge[n0][n1]['label']    = "{0: 2.3f}".format(float(d[4]))
    g.edge[n0][n1]['recurent'] =d[5] # 0/1 yes/no
    g.edge[n0][n1]['inov_num'] =d[6] # int
    g.edge[n0][n1]['mut_num']  =d[7] # float
    g.edge[n0][n1]['enable']   =d[8] # 0/1 yes/no
    g.edge[n0][n1]['style'] = 'solid'
    g.edge[n0][n1]['color'] = 'black'
    g.edge[n0][n1]['labelfontcolor'] = 'black'
    g.edge[n0][n1]['labelfontsize'] = 4

    if d[8]=='0':
        g.edge[n0][n1]['style'] = 'dashed' 
        g.edge[n0][n1]['color'] = 'red'
        g.edge[n0][n1]['labelfontcolor'] = 'red'

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

    # make nodes & edges not in small invisible 
    sn = small.nodes()
    se = small.edges()
    for n in G.nodes_iter():
        if n not in sn:
            new.node[n]['style'] = 'invis'
        else :
            new.node[n]['style'] = small.node[n]['style']
            
    for e in G.edges_iter():
        (n1,n2) = e
        if e not in se :
            new.edge[n1][n2]['style'] = 'invis'
        else :
            new.edge[n1][n2]['style'] = small.edge[n1][n2]['style']
            new.edge[n1][n2]['color'] = small.edge[n1][n2]['color']
            new.edge[n1][n2]['label'] = small.edge[n1][n2]['label']
            new.edge[n1][n2]['labelfontcolor'] = small.edge[n1][n2]['labelfontcolor']
       
    return new

################################################################################
# phylogenetic related functions
################################################################################

# extract the genome form a line and put it in the right list
# in  : a line from the evolution log
# out : a tuple (rob_id, id_trace, mom, dad)
def process_robot_entry(d):
    if d[9] != '][Genome:' :
        return (None, None, None, None)
    rob     = int( d[10].split('=')[1]) 
    idtrace = int( d[11].split('=')[1]) 
    mom     = int( d[12].split('=')[1])
    dad     = int(d[13].split('=')[1])
    
    return (rob, idtrace, mom, dad)

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
               
#Computes an associated color w.r.t the parents' colors
#def color(color1,color2)
#TODO
#    return newcolor

#TODO
#Prune extinct branches
#def prune_extinct(g,id)
#for  i in g.nodes()
#if prune_extinct

# Crete a phylogenetic tree 
# in  : log file in the correct format see above
# out : the graphe og the phylo tree (root nodes are the initial genes)
def create_phylo_tree(fname, save=False, dotfile='philo.dot'): 
    colors = ['chartreuse', 'chocolate', 'cadetblue', 'cornflowerblue', 'cyan',
              'darkorange', 'darkviolet', 'deeppink']
    #TODO check how to define hexadecimal colors for GraphViz
    colorsH = ["#7FFF00", "#D2691E", "#5F9EA0", "#6495ED", "#00FFFF",
              "#FF8C00", "#9400D3", "#F1493"]

    # create a graph
    G=nx.MultiDiGraph()        

    # fill the lists with all the genes 
    gl = process_file(fname)
    # create the phylo-tree
    for n in gl.keys():
        G.add_node(n) # the root node (the initial genome)
        G.node[n]['agent'] = n
        G.node[n]['id']    = n
        G.node[n]['color'] = colors[n%len(colors)]
        trac_genome(gl, n, G, colors)
    
    #Add dad links
    for n in G.nodes():
        #omit initial genomes, who do not have dad nor mom
        if (not(n in gl.keys()) and (G.node[n]['dad']!=-1)):
           G.add_edge(G.node[n]['dad'],G.node[n]['id'],style='dashed')
    
    id_last_epoch= max(G.nodes()) - len(gl.keys()) + 1
    nb_epochs= len(G.nodes())/len(gl.keys())

    #Prune extinct branches (i.e. only keep genomes who 
    #survive at the end and their predecessors)
    for i in range(1,nb_epochs):
        #Compute the leaves
        leaves=[n for n,d in G.out_degree().items() if d==0]
        for j in leaves:
            #Only leaves in the last generation survive
            if j < id_last_epoch:
                G.remove_node(j)


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
    
    # create the animation
    genome_animation(genomes_files, indir, outdir)


    
# create multiple png files suited for an animation
# in  : an orderd sequence of genome files, the genomes dir &  the output dir
# out : png files 
def genome_animation(genomes_files, indir, outdir=None):

    # by default the same 
    if outdir == None :
        outdir = indir
    
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





    
################################################################################
# Communication graph related functions
################################################################################

# adds a node to indicate the iteration number
# in : the graph, the node 
def add_com_root(g, n):
    g.add_node(n)
    g.node[n]['label'] = 'iteration '+n[1:]
    g.node[n]['color'] = 'gray'
    g.node[n]['style'] = 'filled'

# add a 2 nodes to the grah (2 agent that comunicate)    
# in  : a graph object the data line from the log and a color string
def add_com_entry(g, d, c):
    root = d[0]
    n1 = d[1]+d[0]
    g.add_node(n1)
    g.add_edge(root, n1)
    g.edge[root][n1]['style'] = 'dashed'
    
    g.node[n1]['label'] = d[1]
    g.node[n1]['color'] = c
    g.node[n1]['style'] = 'filled'
   
    n2 = 'R'+d[3]+d[0]            
    g.add_node(n2)
    g.node[n2]['label'] = 'R'+d[3]
    g.node[n2]['color'] = c
    g.node[n2]['style'] = 'filled'
    g.add_edge(n1, n2)
    
    
# Create the communication graph (genome exchange between agents)
# in  : and evolution log 
# out : a graph object
#       Will parse lines : @ R# -> #
def make_com_graph(fname):
    colors=['green','yellow']
    cur_color=0;

    # create the graph
    G=nx.DiGraph()
    fh = open(fname, 'r')

    # make the root node (the first iteration)
    last_iteration = None
    for line in fh :
        data = line.split()
        
        if len(data)>0 and data[0][:1] == '@': # if comm line 
            if data[0] not in G.nodes():
                add_com_root(G, data[0])
                G.add_edge(last_iteration, data[0])
                last_iteration = data[0]
                cur_color = (cur_color+1) % len(colors)
            add_com_entry(G, data, colors[cur_color])
    fh.close()   
    return G
       
# Generate a timeline of exchanges from a list of robot exchanges 
# in  : a dictionary of exchange lists (see add_event bellow)
# out : a graph object
def generate_event_graph(gl):
    # get the latest event (to set the size of the graph) 
    last = -1
    for r in gl :
        if last < gl[r][-1][0] :
            last =  gl[r][-1][0]
            i = r
    # the number of lines in graph
    num_robots = len(gl)

    g = nx.DiGraph()

    # construct the time lines 
    for r in gl :
        n1='R'+str(r)
        g.add_node(n1)
        g.node[n1]['label'] = n1
        
        g.node[n1]['color']='white'
        g.node[n1]['style']='filled'
        for e in xrange(1, last+1):
            n2 = str(e)+'@'+str(r)
            g.add_node(n2)
            g.node[n2]['label']=e
            g.node[n2]['color']='white'
            g.node[n2]['style']='filled'
            g.add_edge(n1,n2)
            n1 = n2

    # add the exchange events 
    for r in gl :
        for e in gl[r]:
            n = str(e[0])+'@'+str(r)
            t = str(e[0])+'@'+str(e[1])
            g.node[n]['color'] = 'green'
            g.node[t]['color'] = 'green'
            g.add_edge(n,t)
    return g

# adds an event to the dictionary 
def add_event(gl, d):
    r = int(d[1][1:]) # robot
    t = int(d[3])     # target
    i = int(d[0][1:]) # iteration

    # if robot not in list 
    if r not in gl :
        gl[r] = []
    
    gl[r].append((i,t))
    if t not in gl :
        gl[t] = []

# Create the communication graph (as process time lines)
# in  : and evolution log 
# out : a graph object
#       Will parse lines : @ R# -> #
def make_event_graph(fname):
    colors=['green','yellow']
    cur_color=0;

    # create the graph
    GL = {}
    fh = open(fname, 'r')
    # make the root node (the first iteration)
    last_iteration = None
    for line in fh :
        data = line.split()
        if len(data)>0 and data[0][:1] == '@': # if comm line 
            add_event(GL, data)
    fh.close()   

    G = generate_event_graph(GL)
    return G
       

            
          
           

    
