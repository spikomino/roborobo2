#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys
import pylab 


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
                n = re.sub('[:\[\]]', ' ', o)
                n = float(n.split()[1])
                d.append(n)
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


# process multiple datalogs
def process_experiment(datalogs):
    D=[]
    for f in datalogs:
        D.append(process_datalog(f))

    S = {} 
    S['aasf'] = ave_accu_sf(D)
    S['fbsf'] = fix_budg_sf(D)
    S['trt']  = time_reach_target(D)
    S['aat']  = acc_above_target(D, S['trt']) 

    return D,S




