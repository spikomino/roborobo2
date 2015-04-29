#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import sys
import numpy as np
from datalog_Paper import *


    
if len(sys.argv) < 2 :
    print 'Usage :'+ sys.argv[0] +' exp1 exp2 ...'
    print '\t\teach exp is a directory containing datalog files'
    exit(0)

maxFitness = 0.0

for d in sys.argv[1:] :
    aux = extract_max(d)
    if(aux > maxFitness) :
        maxFitness = aux

D=[]
for d in sys.argv[1:] :
    data, stats, species, sizes, stNN = process_experiment(d,maxFitness)
    D.append((d, data, stats, species, sizes,stNN))


#Tests comparing the two first experiments
mannWhitney = {}
(name1,data1,st1,sp1,sizes1,stNN1) = D[0]
(name2,data2,st2,sp2,sizes2,stNN2) = D[1]


#mannWhitney['aasf'] = stat_test(st1['aasf'],st2['aasf'],True)
#mannWhitney['fbsf'] = stat_test(st1['fbsf'],st2['fbsf'],True)
#mannWhitney['trt'] = stat_test(st1['trt'],st2['trt'],True)
#mannWhitney['aat'] = stat_test(st1['aat'],st2['aat'],True)

#mannWhitney['avgAccNN'] = stat_test(stNN2['avgAcc'],stNN2['avgAcc'],True)

mannWhitney['aasf'] = stat_test(st1['aasf'],st2['aasf'])
mannWhitney['fbsf'] = stat_test(st1['fbsf'],st2['fbsf'])
mannWhitney['trt'] = stat_test(st1['trt'],st2['trt'])
mannWhitney['aat'] = stat_test(st1['aat'],st2['aat'])

mannWhitney['avgAccNN'] = stat_test(stNN2['avgAcc'],stNN2['avgAcc'])

text = ""
#text = str(mannWhitney)
#print(text)
#text = 'Experiments ' +name1 + 'and '+ name2 + ' statistical comparisons \n' + text + ''

#f = open('workfile', 'w')
#print >>f, D
       
draw_data(D,text, runs=True, tex=False)
