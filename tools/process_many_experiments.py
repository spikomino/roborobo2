#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys
from os import listdir
from os.path import isfile, join
from datalog_stats import *

# draw a figure
def draw_data(exp): 
    pylab.ion()
    fig = pylab.figure(num=None, figsize=(10, 5), dpi=100)
    pylab.clf()


    ax1 = pylab.subplot2grid((2,3), (0,0), rowspan=2)
    for e in exp:
        (p, data, stats) = e
        box = map(list, zip(*data))
        #pylab.plot(pylab.mean(box, axis=1), color='black', lw=1, label='Mean')
        pylab.plot(pylab.median(box, axis=1), lw=1, label=p)


    pylab.draw()
    pylab.legend(loc='lower right')
    pylab.xlabel('Iterations')
    pylab.ylabel('Fitness')   

    ax2 = pylab.subplot2grid((2,3), (0,1))
    stats=[]
    for e in exp:
        (dir, d, s) = e 
        stats.append(s['aasf']) 

    pylab.boxplot(stats)
    pylab.xlabel('Average accumulated swarm fitness')
    pylab.ylabel('Fitness') 

    ax3 = pylab.subplot2grid((2,3), (0, 2))
    stats=[]
    for e in exp:
        (dir, d, s) = e
        stats.append(s['fbsf']) 

    pylab.boxplot(stats)
    pylab.xlabel('Fixed budget swarm fitness')
    pylab.ylabel('Fitness') 

    ax4 = pylab.subplot2grid((2,3), (1, 1))
    stats=[]
    for e in exp:
        (dir, d, s) = e
        stats.append(s['trt']) 

    pylab.boxplot(stats)
    pylab.xlabel('Time to reach target')
    pylab.ylabel('Generation') 

    ax5 = pylab.subplot2grid((2,3), (1, 2))
    stats=[]
    for e in exp:
        (dir, d, s) = e
        stats.append(s['aat']) 

    pylab.boxplot(stats)
    pylab.xlabel('Accumulated fitness above target')
    pylab.ylabel('Fitness') 


    pylab.show()
    print 'Press enter to exit'
    raw_input()

if __name__ == '__main__':
    
    if len(sys.argv) < 2 :
        print 'Usage :'+ sys.argv[0] +' exp1 exp2 ...'
        print '\t\teach exp is a directory containing datalog files'
        exit(0)

    D=[]
    for d in sys.argv[1:] :
        logs = [ f for f in listdir(d) 
                     if isfile(join(d,f)) and f.startswith('datalog') ]

        datalogs = map(lambda f: join(d,f), logs) 
        data, stats = process_experiment(datalogs)
        D.append((d, data, stats))
    
    
    draw_data(D)
