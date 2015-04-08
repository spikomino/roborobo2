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
    F,C,Fr,M,P,R = process_experiment2(d)
    D.append((d,F,C,Fr,M,P,R))

    if len(F) == 1:
        draw_data2(D, runs=False, tex=False)
    else:
        draw_data2(D, runs=True, tex=False)
