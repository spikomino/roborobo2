#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import sys

from datalog_stats import *


if len(sys.argv) < 2 :
    print 'Usage :'+ sys.argv[0] +' dir_to_datalogs'
    exit(0)


#datalogs = list_datalogs(sys.argv[1])
#survival = list_srfiles(path)

#print datalogs

#D=[]
#for f in datalogs:
#    D.append(process_datalog(f))



# compute raw statistics
D, S, R = process_experiment(sys.argv[1])


# plot all
draw_data([(sys.argv[1], D, S, R)], runs=True)
    
