#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import sys

from datalog_stats import *


    
if len(sys.argv) < 2 :
    print 'Usage :'+ sys.argv[0] +' exp1 exp2 ...'
    print '\t\teach exp is a directory containing datalog files'
    exit(0)

D=[]
for d in sys.argv[1:] :
    data, survival = process_experiment(d)
    D.append((d, data, survival))
        
stats = compute_stats(D)

draw_data(D,stats, runs=False, tex=False, sig=True)
