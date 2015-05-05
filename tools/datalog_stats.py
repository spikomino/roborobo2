#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys
from os import listdir
from os.path import isfile, join
from scipy.stats import *
from pylab import *
import brewer2mpl
from genome2graph import *

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
def process_datalog(fname, key='fit', norm=0):
    d=[]
    fh = open(fname, 'r')
    for line in fh :
        data = line.split()
        for o in data :
            if o.startswith(key, 1, len(key)+1):
                f = re.sub('[:\[\]]', ' ', o)
                f = float(f.split()[1])
                if norm != 0 :
                    f = f/norm
                d.append(f)
    fh.close()
    #print fname, len(d)
    return d

# read the survival rate file 
def process_srfile(fname):
    d=[]
    fh = open(fname, 'r')
    for line in fh :
        d.append(float(line))
    fh.close()
    #print fname, len(d)

    return d

# statistics measures
def ave_accu_sf(data, cut=0.25):
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

def time_reach_target(data, pers=0.75):
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
    #print data.shape
    median  = np.zeros(data.shape[1])
    perc_25 = np.zeros(data.shape[1])
    perc_75 = np.zeros(data.shape[1])
    for i in xrange(0, len(median)):
        median[i] = np.median(data[:, i])
        perc_25[i] = np.percentile(data[:, i], 25)
        perc_75[i] = np.percentile(data[:, i], 75)
    return median, perc_25, perc_75


# process multiple datalogs Fitness + boxplot
def process_experiment(path):
    datalogs = list_datalogs(path)
    survival = list_srfiles(path)
    
   
    D=[]
    for f in datalogs:
        # default 'fit'. Possible: popsize col for mis
        D.append(process_datalog(f, 'fit')) 

    R=[]
    for f in survival:
        R.append(process_srfile(f))

    S = {} 
    S['aasf'] = ave_accu_sf(D)
    S['fbsf'] = fix_budg_sf(D)
    S['trt']  = time_reach_target(D)
    S['aat']  = acc_above_target(D, S['trt']) 

    return D,S,R

# process multiple datalogs Fitness + missed + collected + foraged 
def process_experiment2(path):
    datalogs = list_datalogs(path)
    
    F=[]
    C=[]
    U=[]
    M=[]
    L=[]
    R=[]
    for f in datalogs:
        n = int((f.split('sp_')[1]).split('/')[0])
        F.append(process_datalog(f, 'fit')) 
        C.append(process_datalog(f, 'col')) 
        U.append(process_datalog(f, 'use')) 
        M.append(process_datalog(f, 'mis'))
        L.append(process_datalog(f, 'lnd'))
        R.append(process_datalog(f, 'mis')) 
        
    # compute the rate of col/drop
    for j in xrange(len(datalogs)):
        for i in xrange(len(R[j])):
            R[j][i] = R[j][i] / (C[j][i] + 1e-19)
        
    return F,C,U,M,L,R


# draw a figure

def plot_one_curve(data, color, axis, label, quartiles=False):
    
    med, perc_25, perc_75 = perc(data)
    if quartiles :
        axis.fill_between(np.arange(0, len(med)), perc_25, perc_75,
                          alpha=0.25, linewidth=0, color=color)
    axis.plot(med, lw=1, label=label, color=color)
    
      
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
                
         


# designed for foraging experiements
def draw_data2(exp, runs=False, tex=False): 
    font = {'family' : 'serif', 'size'   : 10}
    if tex :
        matplotlib.rc('text', usetex=True)
    matplotlib.rc('font', **font)
    bmap = brewer2mpl.get_map('Set2', 'qualitative', 7)
    colors = bmap.mpl_colors
   

    figure(num=None, figsize=(10, 5), dpi=100)
    clf()

    ax1 = subplot2grid((2,3), (0,0))
    ax2 = subplot2grid((2,3), (1,0))
    ax3 = subplot2grid((2,3), (0,1))
    ax4 = subplot2grid((2,3), (0,2))
    ax5 = subplot2grid((2,3), (1,1))
    ax6 = subplot2grid((2,3), (1,2))

    c=0
    for e in exp:
        (n,F,C,U,M,L,R) = e
        plot_one_curve(F, colors[c], ax1,  re.sub('[_/]', '', n), runs)
        plot_one_curve(C, colors[c], ax2,  re.sub('[_/]', '', n), runs)
        plot_one_curve(U, colors[c], ax3,  re.sub('[_/]', '', n), runs)
        plot_one_curve(M, colors[c], ax4,  re.sub('[_/]', '', n), runs)
        plot_one_curve(L, colors[c], ax5,  re.sub('[_/]', '', n), runs)
        plot_one_curve(R, colors[c], ax6,  re.sub('[_/]', '', n), runs)
        c=c+1
  

    for x in [ax1,ax2,ax3,ax4,ax5,ax6]:
        x.set_xlabel('Generations')

    ax1.set_ylabel('Fitness')
    ax2.set_ylabel('Items collected')  
    ax3.set_ylabel('Basket Usage') 
    ax4.set_ylabel('Items droped') 
    ax5.set_ylabel('Items forraged at landmark')  
    ax6.set_ylabel('Ratio missed/collected') 

    ax2.legend(loc='upper right')

    draw()
    show()




def draw_data(exp, runs=False, tex=False): 
    font = {'family' : 'serif', 'size'   : 10}
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
    print 'ax2'
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
    print 'ax3'
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
    print 'ax4'
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
    print 'ax5'
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



# extract the performance of ONE ROBOT durring its lifetime.
# in  : the outpout log NOT the datalog
#       the robot id 
# out : a tupple (fitness, collected, forraged, dropped) 
def extract_data_by_rob(fname, rid, inputs=None, path=None):
    fit=[]
    col=[]
    frg=[]
    drp=[]
    usg=[]
    out=[]

    # extract the fitness into the lists 
    fh = open(fname, 'r')
    gen=0
    for line in fh :
        data = line.split()
        if data!=[] and data[0]=='[Robot:' and int(data[1].split('=')[1])==rid:
            fit.append(float(data[4].split('=')[1]))
            tmp=data[7].split('=')[1]
            col.append(int(tmp.split('/')[0]))
            frg.append(int(data[8].split('=')[1]))
            drp.append(int(data[9].split('=')[1]))
            usg.append(float(data[12].split('=')[1]))

            # execute network with inputs
            if inputs != None:
                gname=fname_from_gen_and_rob(gen, int(rid))
                gen = gen+1
                g = process_graph(path+'/'+gname)
                outputs = execute_mlp(g, inputs)
                out.append(outputs) 


    fh.close()
    return (fit,col,frg,drp, usg, out)

def draw_robot_stats(stats, tex=False):
    font = {'family' : 'serif', 'size'   : 6}
    if tex :
        matplotlib.rc('text', usetex=True)
    matplotlib.rc('font', **font)
    bmap = brewer2mpl.get_map('Set2', 'qualitative', 7)
    colors = bmap.mpl_colors
   

    (fit,col,frg,drp,usg,out) = stats

    figure(num=None, figsize=(10, 5), dpi=100)
    clf()
    
    # fitness 
    ax1 = subplot2grid((2,3), (0,0))
    ax1.plot(fit, lw=1, label='fit', color=colors[0])
    ax1.grid(axis='y', color="0.9", linestyle='-', linewidth=1)
    ax1.spines['top'].set_visible(False)
    ax1.spines['right'].set_visible(False)
    ax1.spines['left'].set_visible(False)
    ax1.get_xaxis().tick_bottom()
    ax1.get_yaxis().tick_left()
    ax1.tick_params(axis='x', direction='out')
    ax1.tick_params(axis='y', length=0)
    for spine in ax1.spines.values():
        spine.set_position(('outward', 5))
    ax1.set_axisbelow(True)
    ax1.set_xlabel('Generations')
    ax1.set_ylabel('Fitness')
    
    # items
    ax2 = subplot2grid((2,3), (0,1))
    #ax2.plot(col, lw=1, label='col', color=colors[0])
    #ax2.plot(frg, lw=1, label='forraged', color=colors[1])
    ax2.plot(drp, lw=1, label='dropped', color=colors[2])
    ax2.grid(axis='y', color="0.9", linestyle='-', linewidth=1)
    ax2.spines['top'].set_visible(False)
    ax2.spines['right'].set_visible(False)
    ax2.spines['left'].set_visible(False)
    ax2.get_xaxis().tick_bottom()
    ax2.get_yaxis().tick_left()
    ax2.tick_params(axis='x', direction='out')
    ax2.tick_params(axis='y', length=0)
    for spine in ax2.spines.values():
        spine.set_position(('outward', 5))
    ax2.set_axisbelow(True)
    ax2.legend(loc='upper right')
    ax2.set_xlabel('Generations')
    ax2.set_ylabel('Items')

    # items
    ax5 = subplot2grid((2,3), (0,2))
    ax5.plot(usg, lw=1, label='basket usage', color=colors[0])
    ax5.grid(axis='y', color="0.9", linestyle='-', linewidth=1)
    ax5.spines['top'].set_visible(False)
    ax5.spines['right'].set_visible(False)
    ax5.spines['left'].set_visible(False)
    ax5.get_xaxis().tick_bottom()
    ax5.get_yaxis().tick_left()
    ax5.tick_params(axis='x', direction='out')
    ax5.tick_params(axis='y', length=0)
    for spine in ax5.spines.values():
        spine.set_position(('outward', 5))
    ax5.set_axisbelow(True)
    ax5.legend(loc='upper right')
    ax5.set_xlabel('Generations')
    ax5.set_ylabel('Rate')





    # network activations 

    if out != [] :

        O = map(list, zip(*out))
        ax3 = subplot2grid((2,3), (1,0))
   
        ax3.plot(O[0], lw=1, label='L', color=colors[0])
        ax3.plot(O[1], lw=1, label='R', color=colors[1])
        ax3.grid(axis='y', color="0.9", linestyle='-', linewidth=1)
        ax3.spines['top'].set_visible(False)
        ax3.spines['right'].set_visible(False)
        ax3.spines['left'].set_visible(False)
        ax3.get_xaxis().tick_bottom()
        ax3.get_yaxis().tick_left()
        ax3.tick_params(axis='x', direction='out')
        ax3.tick_params(axis='y', length=0)
        for spine in ax3.spines.values():
            spine.set_position(('outward', 5))
        ax3.set_axisbelow(True)
        ax3.legend(loc='upper right')
        ax3.set_xlabel('Generations')
        ax3.set_ylabel('Items')
    

        ax4 = subplot2grid((2,3), (1,1))
        ax4.plot(O[2], lw=1, label='D', color=colors[0])
        ax4.grid(axis='y', color="0.9", linestyle='-', linewidth=1)
        ax4.spines['top'].set_visible(False)
        ax4.spines['right'].set_visible(False)
        ax4.spines['left'].set_visible(False)
        ax4.get_xaxis().tick_bottom()
        ax4.get_yaxis().tick_left()
        ax4.tick_params(axis='x', direction='out')
        ax4.tick_params(axis='y', length=0)
        for spine in ax4.spines.values():
            spine.set_position(('outward', 5))
        ax4.set_axisbelow(True)
        ax4.legend(loc='upper right')
        ax4.set_xlabel('Generations')
        ax4.set_ylabel('Items')
    
    



    draw()
    show()


    
