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



def process_trait(g, d):
    return 

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
            G.graph['name'] = splitext(fname)[0] # remove extention
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


################################################################################

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
    parser.add_option("-a", "--animation", 
                      dest="animation",
                      type="string",
                      default=defaults['animation']   ,  
                      help="the animation filename [default "
                      +str(defaults['animation'])+"]")
    
    
    return parser.parse_args()

def draw(g) :
    pylab.clf()
    p=nx.circular_layout(g)
    nx.draw(g,p)
    pylab.draw()

def list_files_in_path(path):
    l=[]
    for f in listdir(path) :
        if isfile(join(path,f)) and f.endswith('.gen'):
            l.append(join(path,f) )
    l.sort()
    return l

def process_path(l):
    gl = []
    for g in l:
        gl.append(process_graph(g))
    return gl;

def save_graph_dot(g):
    nx.write_dot(g, g.graph['name']+'.dot')

def save_graphe_png(g):
    
    # need a dotfile 
    save_graph_dot(g)
    
    fin  = g.graph['name']+'.dot'
    fout = g.graph['name']+'.png'
    
    # execute dot fin | gvcolor | dot -Tpng -o fout
    p1 = Popen(["dot", fin], stdout=PIPE)
    p2 = Popen(["gvcolor"], stdin=p1.stdout, stdout=PIPE)
    p1.stdout.close()  
    p3 = Popen(["dot", "-Tpng", "-o", fout],stdin=p2.stdout,stdout=PIPE)
    p2.stdout.close()  
    p3.communicate()[0]
    

def save_graph_many_dot(l):
    for g in l:
        save_graph_dot(g)

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out']   = False
    defaults_opts['file']      = 'gen.log'
    defaults_opts['path']      = None
    defaults_opts['animation'] = None
    (options, args) = read_options(defaults_opts)
 
   
    # are we processing a directory
    if options.path != None :
        l  = list_files_in_path(options.path)
        gl = process_path(l)
        save_graph_many_dot(gl)
        

    else : # we're processing a single file 
        g = process_graph(options.file) 
        save_graph_dot(g)
      

    if options.win_out:
        print 'Press enter to exit'
        raw_input()


        
    if options.path != None and options.animation != None :
        for g in gl:
            save_graph_png(g)


        exit 


    # If we draw things ... prepare    
    if options.win_out: 
        pylab.ion()
        fig = pylab.figure(num=None, figsize=(5, 5), dpi=100)
        pylab.show()
        for g in gl:
            draw(g)
            print 'Press enter'
            raw_input()

        # execute ffmpeg -framerate 1 -pattern_type glob -i '*.png' -c:v libx264 -pix_fmt yuv420p out.mp4
        
        fin = [v.graph['id']+'.png' for v in gl ]
        l=''
        for f in fin :
            l = l+' '+f
        
            #      subprocess.call(
            #          ['ffmpeg',
            #           '-framerate', '1', 
            #           '-pattern_type','glob', '-i', "" 
            # '-c:v', 'libx264', '-pix_fmt','yuv420p', ]
            #)
            
