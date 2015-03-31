#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------
import pylab
import networkx as nx
import os, progressbar
from subprocess import Popen, PIPE


def rid_from_fname(fname):
    return int(fname[:4])

def gid_from_fname(fname):
    return int((fname.split('-')[1])[:-4]) 
    
def gen_from_fname(fname):
    id = rid_from_fname(fname)
    tr = gid_from_fname(fname)
    return (tr-id)/10000

def fname_from_gen_and_rob(g,r):
    return "{:0>4d}".format(r)+'-'+"{:0>10d}".format(g*10000+r)+'.gen'



def gid_from_gen_and_rob(g,r):
    return int("{:0>10d}".format(g*10000+r))

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
    g.node[n]['style'] = 'filled'

# add a link in the network
# in  : a graph object and the splited line from the genome file
#       called exclusively by process_graph 
def process_gene(g, d):
    n0 = int(d[2])
    n1 = int(d[3])
    w  = float(d[4])
    g.add_edge(n0, n1, weight=w)
    g.edge[n0][n1]['trait']    =d[1] # ?
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

# Reads a neat genome file and return the number of elements in the network 
#   inputs, bias, hidden, outputs.
# in  : a filename of the genome (neat format)
# out : a tupple ( inputs, bias, hidden, outputs)
def genome_elements_file(fname):

    nb_in  = 0
    nb_out = 0 
    nb_hid = 0
    nb_bia = 0

    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data[0] == 'genomestart' :
            gid = data[1]
     
        elif data[0] == 'node' : 
            if data[4]   == '2' :
                nb_out = nb_out +1
            elif data[4] == '1':
                nb_in  = nb_in +1
            elif data[4] == '3' :
                nb_bia = nb_bia +1 
            elif data[4] == '0' :
                nb_hid = nb_hid +1    

        elif data[0] == 'genomeend' :
            if gid != data[1] : 
                print 'Error: mismatch genome end tag'
                return None
    fh.close()

    return (nb_in, nb_out, nb_hid, nb_bia)

# Reads a neat genome file and return the liste of the specified element
#    inputs, bias, hidden, outputs.
# in  : a filename of the genome (neat format), 
#     : a string the name of the elements ([in], out, bia, hid)
# out : a list of ( inputs, bias, hidden, outputs)
def genome_elements_list_file(fname, elem='in'):
    res=[]
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data[0] == 'genomestart' :
            gid = data[1]
     
        elif data[0] == 'node' : 
            if   data[4] == '0' and elem == 'hid' :
                res.append(int(data[1]))
            elif data[4] == '1' and elem == 'in' :
                res.append(int(data[1]))
            elif data[4] == '2' and elem == 'out' :
                res.append(int(data[1]))
            elif data[4] == '3' and elem == 'bia' :
                res.append(int(data[1]))     

        elif data[0] == 'genomeend' :
            if gid != data[1] : 
                print 'Error: mismatch genome end tag'
                return None
    fh.close()
    return res

# Reads a graph and return the number of elements in the network 
#   inputs, bias, hidden, outputs.
# in  : a graph object
# out : a tupple ( inputs, bias, hidden, outputs)
def genome_elements(g):
    nb_hid = len([v for v in g if g.node[v]['lbl'] == '0'])
    nb_in  = len([v for v in g if g.node[v]['lbl'] == '1'])
    nb_out = len([v for v in g if g.node[v]['lbl'] == '2'])
    nb_bia = len([v for v in g if g.node[v]['lbl'] == '3'])
    return (nb_in, nb_out, nb_hid, nb_bia)
    
# Reads a graph  and return the liste of the specified element
#    inputs, bias, hidden, outputs.
# in  : a graph object,
#     : a string the name of the elements ([in], out, bia, hid)
# out : a list of ( inputs, bias, hidden, outputs)
def genome_elements_list(g, elem='in'):
    if elem == 'hid' :
        return  [v for v in g if g.node[v]['lbl'] == '0']
    if elem == 'in' :
        return  [v for v in g if g.node[v]['lbl'] == '1']
    if elem == 'out' :
        return  [v for v in g if g.node[v]['lbl'] == '2']
    if elem == 'bia' :
        return  [v for v in g if g.node[v]['lbl'] == '3']



def sigmoid_neat(a): # [0,1] steep
        return (1.0/(1.0+(pylab.np.exp(-(4.924273*a)))));   

# computes the output of a neuron 
def compute_neuron(g, n, inputs, func=sigmoid_neat):

    # input 
    if g.node[n]['lbl'] == '1' or g.node[n]['lbl'] == '3':
        return inputs[n-1];
    s=0
    for i in g.predecessors(n) :
        s = s + compute_neuron(g, i, inputs, func) * g[i][n]['weight']
    return func(s)


# Execute the network for one input 
# in  : a graph object (the network)
# out : a list of outputs
# NOTE : works only on FF networks
def execute_mlp(g, inputs):
    outputs = []
    out_nodes = genome_elements_list(g, 'out')
    for o in out_nodes :
        s= compute_neuron(g, o, inputs, sigmoid_neat)
        outputs.append(s)
    
    outputs[0] = 2.0 * (outputs[0]-0.5);
    outputs[1] = 2.0 * (outputs[1]-0.5);
    return outputs





    
# Reads a neat genome filename and greate the coresponding weight matrix
# in  : a filename of the genome (neat format)
# out : a list of list (weight matrix) [inputs][outputs]
# NOTE : Does not work for MLP only for perceptron
def process_weight_matrix(fname):
    
    nb_in  = 0
    nb_out = 0 
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data[0] == 'genomestart' :
            gid = data[1]
     
        elif data[0] == 'node' : 
            if data[4] == '2' :
                nb_out = nb_out +1
            elif data[4] == '1' or  data[4] == '3' :
                nb_in = nb_in +1
                 
        elif data[0] == 'genomeend' :
            if gid != data[1] : 
                print 'Error: mismatch genome end tag'
                return None
    fh.close()

    m =  [[0 for x in xrange(nb_out)] for x in xrange(nb_in)]
   
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data[0] == 'genomestart' :
            gid = data[1]
     
        elif data[0] == 'gene' : 
            n0 = int(data[2])-1
            n1 = int(data[3])-1 - nb_in
            
            w  = float(data[4])
            
            m[n0][n1] = w
            
        elif data[0] == 'genomeend' :
            if gid != data[1] : 
                print 'Error: mismatch genome end tag'
                return None
    fh.close()
    
    return m

# compute the distance between 2 weight matrices
# in  : 2 matrices (see above)
# out : a float the distance between the matrices (Frobenius norm)
def weight_matrix_dist(m1, m2):
    
    sum = 0.0
    for i in xrange(len(m1)):
        for j in xrange(len(m1[0])):
            sum = sum + (m1[i][j] - m2[i][j]) * (m1[i][j] - m2[i][j]) 
    return pylab.sqrt(sum)




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
    gstart = 13 # depends on the log format  

    if d[gstart] != '][Genome:' :
        return (None, None, None, None)
    rob     = int(d[gstart+1].split('=')[1]) 
    idtrace = int(d[gstart+2].split('=')[1]) 
    mom     = int(d[gstart+3].split('=')[1])
    dad     = int((d[gstart+4].split('=')[1]).split(']')[0] )
    
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
def trac_genome(gl, id, G, col, genpath=None):

    #extrac the robot number from id -> to read the gen file name
    nbrobsize =len(str(len(gl.keys())))
    id_robid = int(str(id)[-nbrobsize:])
    
    for i in gl :
        for g in gl[i] :
            (tr, m, d) = g
            if m == id :               
                G.add_node(tr)
                tr_robid = int(str(tr)[-nbrobsize:])
                G.node[tr]['agent'] = i
                G.node[tr]['id']    = tr
                G.node[tr]['mom']   = m
                G.node[tr]['dad']   = d
                G.node[tr]['color'] = col[i%len(col)]
                G.add_edge(id, tr)
                if genpath != None :
                    id_fname = "{:0>4d}".format(id_robid)+'-'+\
                        "{:0>10d}".format(id)+'.gen'
                    tr_fname = "{:0>4d}".format(tr_robid)+'-'+\
                        "{:0>10d}".format(tr)+'.gen'
                    #print id_fname, tr_fname
                    w =weight_matrix_dist(
                        process_weight_matrix(genpath+'/'+id_fname),
                        process_weight_matrix(genpath+'/'+tr_fname))
                    G.edge[id][tr]['label'] = "{0: 2.5f}".format(w) 
                trac_genome(gl, tr, G, col, genpath)


# Crete a phylogenetic tree from tracing mom ids
# in  : log file in the correct format see above
# out : the graphe og the phylo tree (root nodes are the initial genes)
def create_phylo_tree(fname, dotfile=None, pngfile=None, genpath=None): 
    colors = ['chartreuse', 'chocolate', 'cadetblue', 'cornflowerblue', 'cyan',
              'darkorange', 'darkviolet', 'deeppink']
  
    # create a graph
    G=nx.DiGraph()        

    # fill the lists with all the genes 
    gl = process_file(fname)

    # create the phylo-tree
    pbar = progressbar.ProgressBar(maxval=len(gl.keys())).start()
    i=0
    for n in gl.keys():
        pbar.update(i+1)
        i = i+1
        G.add_node(n) # the root node (the initial gene)
        G.node[n]['agent'] = n
        G.node[n]['id']    = n
        G.node[n]['color'] = colors[n%len(colors)]
        trac_genome(gl, n, G, colors, genpath)

    pbar.finish()

    # write the file 
    if dotfile != None :
        print 'Writing %s ...'%(dotfile)
        nx.write_dot(G, dotfile)
        print 'done'
    if pngfile != None :
        print 'Writing %s ...'%(pngfile)
        dot2png(dotfile, pngfile)
        print 'done'

    return G


# Crete a phylogenetic tree based on matrix similarity
# in  : log file in the correct format see above
# out : the graphe og the phylo tree (root nodes are the initial genes)
def create_phylo_tree_dist(genpath, dotfile=None, pngfile=None): 
    colors = ['chartreuse', 'chocolate', 'cadetblue', 'cornflowerblue', 'cyan',
              'darkorange', 'darkviolet', 'deeppink']
  
    # create a graph
    G=nx.DiGraph()        

    # read the directory (list the genome files)
    fl = []
    for f in os.listdir(genpath):
        if f.endswith('.gen'):
            fl.append(f)
    fl.sort()
  
    last = fl[-1]
  
    maxgen = gen_from_fname(last)
    nbrob = rid_from_fname(last)

    M =  [[0 for x in xrange(nbrob)] for x in xrange(maxgen)]
   
    for g in xrange(maxgen) :
        for r1 in xrange(nbrob) :
            r1f = fname_from_gen_and_rob(g,r1)
            m1 = process_weight_matrix(genpath+'/'+r1f)
            min = float("inf")
            min_rob = -1
            distances = []
            for r2 in xrange(nbrob): 
                if r1 != r2 :
                    r2f = fname_from_gen_and_rob(g+1,r2)
                    m2 = process_weight_matrix(genpath+'/'+r2f)
                    d = weight_matrix_dist(m1, m2)
                    distances.append(d)
                    if d < min :
                        min = d
                        min_rob = r2
            M[g][r1] = (min_rob, min, gid_from_gen_and_rob(g+1,min_rob), 
                        distances)

    # create the graph  
    for r in xrange(nbrob) :       
        G.add_node(r) # the root node (the initial gene)
        G.node[r]['agent'] = r
        G.node[r]['id']    = r
        G.node[r]['color'] = colors[r%len(colors)]
        
    for g in xrange(maxgen) :    
        for r in xrange(nbrob) :
            (mr, d, mr_id, D) = M[g][r]
            print r, mr_id, d, mr, D
            n1 = gid_from_gen_and_rob(g,r)
            n2 = mr_id
            
            G.add_node(n1)
            G.node[n1]['color'] = colors[r%len(colors)]
            G.add_edge(n1, n2)
            G.edge[n1][n2]['label'] = "{0: 2.5f}".format(d)
 
    
    # write the file 
    if dotfile != None :
        print 'Writing %s ...'%(dotfile)
        nx.write_dot(G, dotfile)
        print 'done'
    if pngfile != None :
        print 'Writing %s ...'%(pngfile)
        dot2png(dotfile, pngfile)
        print 'done'

    return G

# modified survival rate computation based on genome id at each generation
# in  : a log file.
# out : the number of different genomes at each generation 
def compute_survival_rate_by_genome_id(fname):

    G = {} # dictionary whose keys are iteration number at each generation 
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and data[0] == '[Robot:' :
            # read the iteration number 
            itr = int(data[2].split('=')[1]) 
            G[itr] = []
    fh.close()   

    # compute the number of genomes 
    num_genomes=0
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and data[0] == '[initRobot]' :
            num_genomes=num_genomes+1
    fh.close()   


    def read_robot_line(d):
        if d[12] != '][Genome:' :
            return (None, None)
        gid = int(d[13].split('=')[1]) 
        itr = int(d[2].split('=')[1]) 
        return (gid, itr)

    # fill the lists 
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        if data != [] and  data[0] == '[Robot:' :
            (id,it) = read_robot_line(data)
            if id != None :
                G[it].append(id)
    fh.close()   

    R = {}
    for g in G.keys() :
        R[g] = len(list(set(G[g]))) / float(num_genomes)


    return R



# Compute survival rate
# in  : a phylo genetic graph
# out : a list of survival rates at each generation
def compute_survival_rate(G):

    # get the statring and ending nodes 
    S = [n for n,d in G.in_degree().items() if d==0]
    E = [n for n,d in G.out_degree().items() if d==0]
    S.sort()
    E.sort()

    # get the longest paths length from the sources 
    LP = {}
    for s in S:
        longest=-1;
        for e in E:
            if nx.has_path(G, s, e):
                l = nx.shortest_path_length(G, s, e) + 1
                if l >= longest :
                    longest = l
        LP[s] = l
 
    # compute the survival rate
    # start from the root nodes until down 
    result = []
    for lv in xrange(max(LP.values())) :
        count = 0
        for s in S:
            if LP[s] > lv :
                count += 1
        result.append(float(count)/float(len(S)))

    return result

    
    



    
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
       

            
          
           

    
