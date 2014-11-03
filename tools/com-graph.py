#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import pylab
import networkx as nx
from optparse import OptionParser
from optparse import OptionGroup
from collections import deque

colors=['green','yellow']
c=0;

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
    p=nx.spring_layout(g)
    nx.draw(g,p)
    pylab.draw()
    
def add_iteration_graph(g, q, i):
    global c, colors
    c = (c+1) % len(colors)
    cur_col = colors[c]
    
    root=str(i)
    for d in q :
        n0 = d[1]+d[0]
        g.add_node(n0)
        g.node[n0]['color'] = cur_col
        g.add_edge(root, n0)

        for n in d[3:]: 
            node = "R"+n+d[0]
            g.add_node(node)
            g.node[node]['color'] = cur_col
            g.add_edge(n0, node)
    q.clear();

def process(parameters):
   
    # If we draw things ... prepare    
    if parameters.win_out:
        pylab.ion()
        fig = pylab.figure(num=None, figsize=(5, 5), dpi=100)
        pylab.show()
        
    # create the graph
    G=nx.DiGraph()
    Q=deque()

    # read the first line 
    fh = open(parameters.file, 'r')
    line = fh.readline()
    data = line.split()
    Q.append(data)
    
    it = data[0]
    G.add_node(str(it))
    G.node[str(it)]['color'] = 'gray'
    for line in fh :
        data = line.split()
        if data[0] != it :
            add_iteration_graph(G, Q, it)
            if parameters.win_out:
                draw(G) 
                print 'Press enter to step'
                raw_input()
            G.add_node(str(data[0]))
            G.node[str(data[0])]['color'] = 'gray'
            G.add_edge(it, str(data[0]))
            it = data[0]
        Q.append(data)
        add_iteration_graph(G, Q, it)

    fh.close()   
    nx.write_dot(G, "out.dot")

    if parameters.win_out:
        print 'Press enter to end'
        raw_input()

# If run directly (toplevel)
if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['win_out'] = False
    defaults_opts['file']    = 'com.log'
    (options, args) = read_options(defaults_opts)
        
    process(options)
