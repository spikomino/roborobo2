#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys, pylab 
import matplotlib.gridspec as gridspec

from datalog_stats import *



# draw a figure
def draw_data(data, stats): 
    pylab.ion()
    fig = pylab.figure(num=None, figsize=(10, 5), dpi=100)
    pylab.clf()

    ax1 = pylab.subplot2grid((2,3), (0,0), rowspan=2)
    for d in data:
        pylab.plot(d, lw=.2, linestyle="-", label='', color='0.2')
    box = map(list, zip(*data))
    pylab.plot(pylab.mean(box, axis=1), color='black', lw=2, label='Mean')
    pylab.plot(pylab.median(box, axis=1), color='red', lw=2, label='Median')
    pylab.draw()
    pylab.legend(loc='upper left')
    pylab.xlabel('Iterations')
    pylab.ylabel('Fitness')   

    ax2 = pylab.subplot2grid((2,3), (0,1))
    pylab.xticks([])
    pylab.boxplot(stats['aasf'])
    pylab.xlabel('Average accumulated swarm fitness')
    pylab.ylabel('Fitness') 

    ax3 = pylab.subplot2grid((2,3), (0, 2))
    pylab.xticks([])
    pylab.boxplot(stats['fbsf'])
    pylab.xlabel('Fixed budget swarm fitness')
    pylab.ylabel('Fitness') 

    ax4 = pylab.subplot2grid((2,3), (1, 1))
    pylab.xticks([])
    pylab.boxplot(stats['trt'])
    pylab.xlabel('Time to reach target')
    pylab.ylabel('Generation') 

    ax5 = pylab.subplot2grid((2,3), (1, 2))
    pylab.xticks([])
    pylab.boxplot(stats['aat'])
    pylab.xlabel('Accumulated fitness above target')
    pylab.ylabel('Fitness') 

    pylab.show()

    print 'Press enter to exit'
    raw_input()


if __name__ == '__main__':
    
    if len(sys.argv) < 2 :
        print 'Usage :'+ sys.argv[0] +' datalog1 datalog2 ...'
        exit(0)

    # compute raw statistics 
    D, S = process_experiment(sys.argv[1:])
    
    # plot all
    draw_data(D,S)
    
