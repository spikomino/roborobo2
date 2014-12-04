#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, re, sys
import pylab 

# read a datalog and extract the [fit:##.###] component
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

def draw_raw_data(data): 
    for d in D:
        pylab.plot(d, linewidth=1, linestyle="-", label='', color='0.5')

    box = map(list, zip(*D))
    
    pylab.plot(pylab.mean(box, axis=1), color='black', lw=2, label='Mean')
    pylab.plot(pylab.median(box, axis=1), color='r', lw=2, label='Median')

    pylab.draw()
        


if __name__ == '__main__':
    
    if len(sys.argv) < 2 :
        print 'Usage :'+ sys.argv[0] +' datalog1 datalog2 ...'
        exit(0)

    D=[]
    for f in sys.argv[1:]:
        D.append(process_file(f))
        
    pylab.ion()
    fig = pylab.figure(num=None, figsize=(7, 7), dpi=100)
    pylab.show()
    pylab.clf()

    draw_raw_data(D)

    pylab.legend(loc='upper left')
    pylab.xlabel('Iterations')
    pylab.ylabel('Fitness')
   

    print 'Press enter to exit'
    raw_input()
