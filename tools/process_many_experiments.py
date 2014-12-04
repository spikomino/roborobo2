#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys, pylab
from os import listdir
from os.path import isfile, join
from datalog_stats import *

# draw a figure
def draw_data(exp, tex=False): 
    font = {'family' : 'serif', 'size'   : 8}
    if tex :
        pylab.matplotlib.rc('text', usetex=True)
    pylab.matplotlib.rc('font', **font)

    pylab.ion()
    fig = pylab.figure(num=None, figsize=(10, 5), dpi=100)
    pylab.clf()


    ax1 = pylab.subplot2grid((2,3), (0,0), rowspan=2)
    for e in exp:
        (n, data, stats) = e
        box = map(list, zip(*data))
        ax1.plot(pylab.median(box, axis=1), lw=1, label=re.sub('[_/]', '', n))
    ax1.set_title('Median swarm fitness over time (%d runs)'%(len(data)))
    ax1.ticklabel_format(style='sci', scilimits=(0,0), axis='y')
    ax1.legend(loc='lower right')
    ax1.set_xlabel('Generations')
    ax1.set_ylabel('Fitness')   

    ax2 = pylab.subplot2grid((2,3), (0,1))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s) = e 
        stats.append(s['aasf']) 
        l.append(re.sub('[_/]', '', n))
    ax2.boxplot(stats)
    ax2.set_title('Average accumulated swarm fitness')
    ax2.ticklabel_format(style='sci', scilimits=(0,0), axis='y')
    ax2.set_xticklabels=l

    ax3 = pylab.subplot2grid((2,3), (0, 2))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s) = e
        stats.append(s['fbsf']) 
        l.append(re.sub('[_/]', '', n))
    ax3.boxplot(stats)
    ax3.set_title('Fixed budget swarm fitness')
    ax3.ticklabel_format(style='sci', scilimits=(0,0), axis='y')

    ax4 = pylab.subplot2grid((2,3), (1, 1))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s) = e
        stats.append(s['trt']) 
        l.append(re.sub('[_/]', '', n))
    ax4.boxplot(stats)
    ax4.set_title('Time to reach target')
    
    ax5 = pylab.subplot2grid((2,3), (1, 2))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s) = e
        stats.append(s['aat']) 
        l.append(re.sub('[_/]', '', n))
    ax5.boxplot(stats)
    ax5.set_title('Accumulated fitness above target')
    ax5.ticklabel_format(style='sci', scilimits=(0,0), axis='y')

    pylab.draw()
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
    
    
    draw_data(D, tex=False)
