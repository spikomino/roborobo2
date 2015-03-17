#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys
from os import listdir
from os.path import isfile, join
from scipy.stats import *
from pylab import *
import brewer2mpl

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
def process_datalog(fname, key='fit'):
    d=[]
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        for o in data :
            if o.startswith(key, 1, len(key)+1):
                f = re.sub('[:\[\]]', ' ', o)
                f = float(f.split()[1])
                d.append(f)
    fh.close()
    print fname, len(d)
    return d

# read the survival rate file 
def process_srfile(fname):
    d=[]
    fh = open(fname, 'r')
    for line in fh :
        d.append(float(line))
    fh.close()
    print fname, len(d)

    return d

# statistics measures
def ave_accu_sf(data, cut=0.1):
    gen = int(cut * len(data[0])) 
    result=[]
    for d in data:
        result.append(average(d[-gen:]))
    return result

def fix_budg_sf(data, cut=0.8):
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

def stat_test(x,y, ks=False):
    if ks:
        return ks_2samp(x,y)
    else :
        return mannwhitneyu(x,y)

def stars(p):
   if p < 0.0001:
       return "****"
   elif (p < 0.001):
       return "***"
   elif (p < 0.01):
       return "**"
   elif (p < 0.05):
       return "*"
   else:
       return "-"

def perc(data_l):
    
  
    
    data = np.asarray(data_l)
    print data.shape
    median  = np.zeros(data.shape[1])
    perc_25 = np.zeros(data.shape[1])
    perc_75 = np.zeros(data.shape[1])
    for i in xrange(0, len(median)):
        median[i] = np.median(data[:, i])
        perc_25[i] = np.percentile(data[:, i], 25)
        perc_75[i] = np.percentile(data[:, i], 75)
    return median, perc_25, perc_75


# process multiple datalogs
def process_experiment(path):
    datalogs = list_datalogs(path)
    survival = list_srfiles(path)
    
   
    D=[]
    for f in datalogs:
        # default 'fit'. Possible: popsize col for mis
        D.append(process_datalog(f, 'mis')) 

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

def plot_one_curve(data, color, axis, label, quartiles=False):
    
    med, perc_25, perc_75 = perc(data)
    if quartiles :
        axis.fill_between(np.arange(0, len(med)), perc_25, perc_75,
                          alpha=0.25, linewidth=0, color=color)
    axis.plot(med, lw=2, label=label, color=color)
    
      
    axis.grid(axis='y', color="0.9", linestyle='-', linewidth=1)
    axis.spines['top'].set_visible(False)
    axis.spines['right'].set_visible(False)
    axis.spines['left'].set_visible(False)
    axis.get_xaxis().tick_bottom()
    axis.get_yaxis().tick_left()
    axis.tick_params(axis='x', direction='out')
    axis.tick_params(axis='y', length=0)
    for spine in axis.spines.values():
        spine.set_position(('outward', 5))
    axis.set_axisbelow(True)

def plot_boxplot(stats, colors, axis, labels, sig=False):
    
    bp = axis.boxplot(stats)

    for i in range(0, len(bp['boxes'])):
        bp['boxes'][i].set_color(colors[i])
        # we have two whiskers!
        bp['whiskers'][i*2].set_color(colors[i])
        bp['whiskers'][i*2 + 1].set_color(colors[i])
        bp['whiskers'][i*2].set_linewidth(2)
        bp['whiskers'][i*2 + 1].set_linewidth(2)
        # top and bottom fliers
        # (set allows us to set many parameters at once)
        bp['fliers'][i * 2].set(markerfacecolor=colors[i],
                                marker='o', alpha=0.75, markersize=6,
                                markeredgecolor='none')
        bp['fliers'][i * 2 + 1].set(markerfacecolor=colors[i],
                                    marker='o', alpha=0.75, markersize=6,
                                    markeredgecolor='none')
        bp['medians'][i].set_color('black')
        bp['medians'][i].set_linewidth(3)
        # and 4 caps to remove
        for c in bp['caps']:
            c.set_linewidth(0)

    for i in range(len(bp['boxes'])):
        box = bp['boxes'][i]
        box.set_linewidth(0)
        boxX = []
        boxY = []
        for j in range(5):
            boxX.append(box.get_xdata()[j])
            boxY.append(box.get_ydata()[j])
            boxCoords = zip(boxX,boxY)
            boxPolygon = Polygon(boxCoords, facecolor = colors[i], linewidth=0)
            axis.add_patch(boxPolygon)


    
    axis.set_xticklabels(labels)
    axis.spines['top'].set_visible(False)
    axis.spines['right'].set_visible(False)
    axis.spines['left'].set_visible(False)
    axis.get_xaxis().tick_bottom()
    axis.get_yaxis().tick_left()
    axis.tick_params(axis='x', direction='out')
    axis.tick_params(axis='y', length=0)
    axis.grid(axis='y', color="0.9", linestyle='-', linewidth=1)
    axis.set_axisbelow(True)


    # stat test 
    if sig :
        for i in xrange(len(stats)) :
            for j in xrange(i+1,len(stats)) :
                y_max = max(concatenate((stats[i], stats[j])))
                y_min = min(concatenate((stats[i], stats[j])))
                z,p = stat_test(stats[i], stats[j])

                axis.annotate("", xy=(i+1, y_max), xycoords='data',
                             xytext=(j+1, y_max), textcoords='data',
                             arrowprops=dict(arrowstyle="-", ec='#aaaaaa',
                                             connectionstyle="bar,fraction=0.1"))
                axis.text((j-i)/2.0 + j, y_max + abs(y_max - y_min)*0.1, stars(p*2.0),
                         horizontalalignment='center',
                         verticalalignment='center')
                
         




def draw_data(exp, runs=False, tex=False): 
    font = {'family' : 'serif', 'size'   : 6}
    if tex :
        matplotlib.rc('text', usetex=True)
    matplotlib.rc('font', **font)
    bmap = brewer2mpl.get_map('Set2', 'qualitative', 7)
    colors = bmap.mpl_colors
   

    figure(num=None, figsize=(10, 5), dpi=100)
    clf()

    # median Fitness
    ax1 = subplot2grid((2,3), (0,0))
    c=0
    for e in exp:
        (n, data, stats, survival) = e
        plot_one_curve(data, colors[c], ax1,  re.sub('[_/]', '', n), runs)
        c=c+1
    ax1.set_title('Median swarm fitness over time (%d runs)'%(len(data)))
    ax1.legend(loc='lower right')
    ax1.set_xlabel('Generations')
    ax1.set_ylabel('Fitness')   

    # Median Lineage survival rate
    ax11 = subplot2grid((2,3), (1,0))
    c=0
    for e in exp:
        (n, data, stats, survival) = e
        plot_one_curve(survival, colors[c], ax11,  re.sub('[_/]', '', n), runs)
        c=c+1
    ax11.set_title('Genetic lines over time (%d runs)'%(len(data)))
    ax11.legend(loc='upper right')
    ax11.set_xlabel('Generations')
    ax11.set_ylabel('Rate of survival')  


    # average accumulated swarm fitness
    ax2 = subplot2grid((2,3), (0,1))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e 
        stats.append(s['aasf']) 
        l.append(re.sub('[_/]', '', n))
        
        plot_boxplot(stats, colors, ax2, l, sig=False)
        ax2.set_title('Average accumulated swarm fitness')

    # Fix budget swarm fitness
    ax3 = subplot2grid((2,3), (0, 2))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e
        stats.append(s['fbsf']) 
        l.append(re.sub('[_/]', '', n))
    plot_boxplot(stats, colors, ax3, l, sig=False)
    ax3.set_title('Fixed budget swarm fitness')
    

    # Time to reach target 
    ax4 = subplot2grid((2,3), (1, 1))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e
        stats.append(s['trt']) 
        l.append(re.sub('[_/]', '', n))
    plot_boxplot(stats, colors, ax4, l, sig=False)
    ax4.set_title('Time to reach target')

    # accumulated fitness above target
    ax5 = subplot2grid((2,3), (1, 2))
    stats=[]
    l=[]
    for e in exp:
        (n, d, s, r) = e
        stats.append(s['aat']) 
        l.append(re.sub('[_/]', '', n))
    plot_boxplot(stats, colors, ax5, l, sig=False)
    ax5.set_title('Accumulated fitness above target')
    

    draw()
    show()


