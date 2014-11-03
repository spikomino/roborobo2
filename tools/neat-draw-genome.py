#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup
from collections import deque


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
    return parser.parse_args()

def draw(g) :
    pylab.clf()

    #in_nodes     = [v for v in g if g.node[v]['lbl']==1]
    #out_nodes    = [v for v in g if g.node[v]['lbl']==2]
    #bias_nodes   = [v for v in g if g.node[v]['lbl']==3]
    #hidden_nodes = [v for v in g if g.node[v]['lbl']==0]
                
    p=nx.circular_layout(g)

    #nx.draw_networkx_nodes(g, p, nodelist=in_nodes,     node_color='r')
    #nx.draw_networkx_nodes(g, p, nodelist=out_nodes,    node_color='g')
    #nx.draw_networkx_nodes(g, p, nodelist=bias_nodes,   node_color='b')
    #nx.draw_networkx_nodes(g, p, nodelist=hidden_nodes, node_color='w')
    #nx.draw_networkx_edges(g,p)
   
    nx.draw(g,p)
    
    pylab.draw()




def process_trait(g, d):
    return 

def process_node(g, d):
    shapes = ['circle', 'box', 'diamond', 'doublecircle']
    
    n = d[1]
    g.add_node(n)
    g.node[n]['trait'] = d[2] # ? 
    g.node[n]['type']  = d[3] # 0=>output, 1=>input
    g.node[n]['lbl'] = d[4] # 0=>hidden, 1=>input, 2=>output, 3=>bias
    g.node[n]['shape'] = shapes[ int(d[4]) ]

    
def process_gene(g, d):
    n0 = d[2]
    n1 = d[3]
    w  = d[4]
    g.add_edge(n0, n1, weight=w)
    g.edge[n0][n1]['trait']    =d[1] # ?
    g.edge[n0][n1]['recurent'] =d[5] # 0/1 yes/no
    g.edge[n0][n1]['inov_num'] =d[6] # int
    g.edge[n0][n1]['mut_num']  =d[7] # float
    g.edge[n0][n1]['enable']   =d[8] # 0/1 yes/no
    if d[8]=='0':
        g.edge[n0][n1]['style'] = 'dashed' 
    

def process(parameters):
   
    # If we draw things ... prepare    
    if parameters.win_out:
        pylab.ion()
        fig = pylab.figure(num=None, figsize=(5, 5), dpi=100)
        pylab.show()
        
    # create the graph
    G=nx.DiGraph()
    
    fh = open(parameters.file, 'r')
    for line in fh :
        data = line.split()
        if data[0] == 'genomestart' :
            gid = data[1]
            G.graph['id'] = data[1]
        elif data[0] == 'trait' :
            process_trait(G,data)
        elif data[0] == 'node' : 
            process_node(G,data)
        elif data[0] == 'gene' :  
            process_gene(G,data)
        elif data[0] == 'genomeend' :
            if gid != data[1] : 
                print 'Error: mismatch genome end tag'
                exit(1)
            if parameters.win_out:
                draw(G) 
        else :
            print data
            print 'Error: unexpected token'

   
    
    fh.close()   
    nx.write_dot(G, "gen.dot")

    if parameters.win_out:
        print 'Press enter to end'
        raw_input()

# If run directly (toplevel)
if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out'] = False
    defaults_opts['file']    = 'gen.log'
    (options, args) = read_options(defaults_opts)
        
    process(options)
