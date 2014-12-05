#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import sys

from datalog_stats import *


if len(sys.argv) < 2 :
    print 'Usage :'+ sys.argv[0] +' dir_to_datalogs'
    exit(0)

# compute raw statistics
datalogs = list_datalogs(sys.argv[1])

D, S = process_experiment(datalogs)

# plot all
draw_data([(sys.argv[1], D, S)], runs=True)
    
