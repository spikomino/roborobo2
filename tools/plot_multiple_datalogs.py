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
    datalogs = list_datalogs(d)
    data, stats = process_experiment(datalogs)
    D.append((d, data, stats))
        
draw_data(D, runs=False, tex=False)
