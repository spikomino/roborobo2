#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys, pylab, progressbar 
from os import listdir
from os.path import isfile, join

# find datalog in a directory coresponfing to one experiment
# in  : a directory path
# out : a tuple of name and a list of file name (name, [datalog, ...])
def list_datalogs(path):
    logs = [ f for f in listdir(path) 
             if isfile(join(path,f)) and f.startswith('datalog') ]

    datalogs = map(lambda f: join(path,f), logs) 
    return datalogs
    
def list_logfiles(path):
    files = [ f for f in listdir(path) 
              if isfile(join(path,f)) and f.endswith('.log') ]

    logs = map(lambda f: join(path,f), files) 
    return logs

def list_srfiles(path):
    files = [ f for f in listdir(path) 
              if isfile(join(path,f)) and f.endswith('.log.sr') ]

    logs = map(lambda f: join(path,f), files) 
    return logs

# read a datalog and extract the [fit:##.###] component
# in  : a file name 
# out : returns a list of fitness values. 
def process_datalog(fname):
    d=[]
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        for o in data :
            if o.startswith('fit', 1, 4):
                f = re.sub('[:\[\]]', ' ', o)
                f = float(f.split()[1])
                d.append(f)
    fh.close()
    return d

# read the survival rate file 
def process_srfile(fname):
    d=[]
    fh = open(fname, 'r')
    for line in fh :
        d.append(float(line))
    fh.close()
    return d

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

def time_reach_target(data, pers=0.9):
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

# process multiple datalogs
def process_experiment(path):
    datalogs = list_datalogs(path)
    survival = list_srfiles(path)
    
    D=[]
    for f in datalogs:
        D.append(process_datalog(f))

    R=[]
    for f in survival:
        R.append(process_srfile(f))

    S = {} 
    S['aasf'] = ave_accu_sf(D)
    S['fbsf'] = fix_budg_sf(D)
    S['trt']  = time_reach_target(D)
    S['aat']  = acc_above_target(D, S['trt']) 

    return D,S,R

# draw a figure
def draw_data(exp, runs=False, tex=False): 
    font = {'family' : 'serif', 'size'   : 6}
    if tex :
        pylab.matplotlib.rc('text', usetex=True)
    pylab.matplotlib.rc('font', **font)

    fig = pylab.figure(num=None, figsize=(10, 5), dpi=100)
    pylab.clf()

    # median Fitness
    ax1 = pylab.subplot2grid((2,3), (0,0))
    for e in exp:
        (n, data, stats, survival) = e
        if runs :
            for d in data:
                ax1.plot(d, lw=.1, linestyle="-", label='', color='0.2')
        
        box = map(list, zip(*data))
        ax1.plot(pylab.median(box, axis=1), lw=1, label=re.sub('[_/]', '', n))
    ax1.set_title('Median swarm fitness over time (%d runs)'%(len(data)))
    ax1.ticklabel_format(style='sci', scilimits=(0,0), axis='y')
    ax1.legend(loc='lower right')
    ax1.set_xlabel('Generations')
    ax1.set_ylabel('Fitness')   

    # Median Lineage survival rate
    ax11 = pylab.subplot2grid((2,3), (1,0))
    for e in exp:
        (n, data, stats, survival) = e
        if runs :
            for d in survival:
                ax11.plot(d, lw=.1, linestyle="-", label='', color='0.2')
        
        box = map(list, zip(*survival))
        ax11.plot(pylab.median(box, axis=1), lw=1, label=re.sub('[_/]', '', n))
    ax11.set_title('Median number of genetic lines over time (%d runs)'%(len(data)))
    ax11.ticklabel_format(style='sci', scilimits=(0,0), axis='y')
    ax11.legend(loc='upper right')
    ax11.set_xlabel('Generations')
    ax11.set_ylabel('Rate of survival')  

    # average accumulated swarm fitness
    ax2 = pylab.subplot2grid((2,3), (0,1))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e 
        stats.append(s['aasf']) 
        l.append(re.sub('[_/]', '', n))
    ax2.boxplot(stats)
    ax2.set_title('Average accumulated swarm fitness')
    ax2.ticklabel_format(style='sci', scilimits=(0,0), axis='y')
    ax2.set_xticklabels=l

    # Fix budget swarm fitness
    ax3 = pylab.subplot2grid((2,3), (0, 2))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e
        stats.append(s['fbsf']) 
        l.append(re.sub('[_/]', '', n))
    ax3.boxplot(stats)
    ax3.set_title('Fixed budget swarm fitness')
    ax3.ticklabel_format(style='sci', scilimits=(0,0), axis='y')

    # Time to reach target 
    ax4 = pylab.subplot2grid((2,3), (1, 1))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e
        stats.append(s['trt']) 
        l.append(re.sub('[_/]', '', n))
    ax4.boxplot(stats)
    ax4.set_title('Time to reach target')

    # accumulated fitness above target
    ax5 = pylab.subplot2grid((2,3), (1, 2))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e
        stats.append(s['aat']) 
        l.append(re.sub('[_/]', '', n))
    ax5.boxplot(stats)
    ax5.set_title('Accumulated fitness above target')
    ax5.ticklabel_format(style='sci', scilimits=(0,0), axis='y')

    pylab.draw()
    pylab.show()
