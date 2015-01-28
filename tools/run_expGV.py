#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, shutil, random
import sys
#from datalog_stats import *

home = os.environ["HOME"]
config_dir    = 'config'
log_dir       = 'logs'
#template_file = 'gcForWithOdNTune.properties
#template_file = 'gcNoUpForWithOdNTune.properties'
#template_file = 'rdmForWithOdNTune.properties'
#template_file = 'odNForTune.properties'
#template_file = 'gcNavWithOdNTune.properties
#template_file = 'gcNoUpWithOdNNavTune.properties'
template_file = 'rdmWithOdNNavTune.properties'
#template_file = 'odNNavTune.properties'
nb_exec       = 64
global_script = 'run-all'
sr_extraction_script = 'genome_phylo_stats.py'
sr_extraction_list = 'sr_list'

if len(sys.argv) > 1:
    template_file = sys.argv[1]


# copy the tenmplate file and update the values 
src_path = os.path.join(config_dir, template_file)

if os.path.isfile(global_script):
    os.remove(global_script)

if os.path.isfile(sr_extraction_list):
    os.remove(sr_extraction_list)

template_name=template_file.split('.')[0]

p='parallel-'+template_name

# create single instances file (fed to parallel)
with open(p, 'w') as file:
    for r in xrange(1,nb_exec+1):
        # copy template prameter file 
        dst_path = os.path.join(config_dir, 'run-%03d'%r+'-'+template_name+'.properties') 
        shutil.copy(src_path, dst_path)
        # add the experiments parameters
        with open(dst_path, 'a') as file_run:
            line = ' gEvolutionLogFile= '+log_dir+ '/%03d'%r+'-evo-'+template_name+'.log\n'
            file_run.write(line)
        rnd = random.random() * 5.0
        line = 'sleep '+str(rnd)+' ;  ./roborobo -l '+ dst_path + ' > '+log_dir+'/'+template_name+'%03d.log'%(r)+'\n'
        file.write(line)

exit(0)

