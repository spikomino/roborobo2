#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys
import pylab 
import matplotlib.gridspec as gridspec

# read a datalog and extract the [fit:##.###] component
# in  : a file name 
# out : returns a list of fitness values. 
def process_file(fname):
    d=[]
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        for o in data :
            if o.startswith('fit', 1, 4):
                n = re.sub('[:\[\]]', ' ', o)
                n = float(n.split()[1])
                d.append(n)
    return d

def save_data(data):
    return 


# draw a figure
def draw_data(data, stats): 
    pylab.ion()
    fig = pylab.figure(num=None, figsize=(10, 5), dpi=100)
    pylab.clf()

    ax1 = pylab.subplot2grid((2,3), (0,0), rowspan=2)
    for d in data:
        pylab.plot(d, lw=.2, linestyle="-", label='', color='0.2')
    box = map(list, zip(*D))
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



# statistics measures
def ave_accu_sf(data, cut=0.1):
    gen = int(cut * len(data[0])) 
    result=[]
    for d in data:
        result.append(pylab.average(d[-gen:]))
    return result

def fix_budg_sf(data, cut=0.9):
    gen = int(cut * len(data[0])) 
    result=[]
    for d in data:
        result.append(d[gen])
    return result

def time_reach_target(data, pers=0.8):
    max_l=[]
    for d in data:
        max_l.append(max(d))
    target = pers*max(max_l)
    result=[]
    for d in data:
        for g in xrange(len(d)):
            if d[g] >= target :
                break
        result.append(g)
    return result

def acc_above_target(data, trg_gen):
    result=[]
    g=0
    for d in data :
        result.append( sum(d[trg_gen[g]-1:]) )
        g +=1
    return result




if __name__ == '__main__':
    
    if len(sys.argv) < 2 :
        print 'Usage :'+ sys.argv[0] +' datalog1 datalog2 ...'
        exit(0)

    # compute raw statistics 
    D=[]
    for f in sys.argv[1:]:
        D.append(process_file(f))
    
    # compute the stats measures 
    S = {} 
    S['aasf'] = ave_accu_sf(D)
    S['fbsf'] = fix_budg_sf(D)
    S['trt']  = time_reach_target(D)
    S['aat']  = acc_above_target(D, S['trt']) 
    
    # plot all
    draw_data(D,S)
