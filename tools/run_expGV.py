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
template_file = 'gcForWithOdNTune.properties'
#template_file = 'odNForTune.properties'
#template_file = 'gcNavWithOdNTune.properties
#template_file = 'gcNoUpNavWithOdNTune.properties'
#template_file = 'gcNavWithOdNTune.properties'
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


var_parameter = 'gSelectionPressure'
var_values = [ '0.00', '0.25', '0.50', '0.75', '1.00' ]

parameters={
    'gInitialNumberOfRobots': 600,
    'gNbOfPhysicalObjects': 0,
    'gSigmaRef': 0.5,
    'gFitnessFunction': 0,  # 0: locomotion, 1: collection, 2: forraging 
    'gControllerType': 1,   # 0 = NEAT, 1 = FFNN
    'gEvaluationTime': 200,
    'gMaxIt'         : 20000, 
    'gBatchMode'     : 'true'}

 
count=1
for p in var_values :



    # add the experiments parameters
    with open(dst_path, 'a') as file:
        for k in parameters.keys() :
            line = k+' = '+str(parameters[k])+'\n'
            file.write(line)
        line = var_parameter+' = '+p+'\n'
        file.write(line)
            
    # create single instances file (fed to parallel)
    with open(p, 'w') as file:
        for r in xrange(1,nb_exec+1):
            rnd = random.random() * 5.0
            line = 'sleep '+str(rnd)+' ;;  roborobo -l '+ dst_path + ' > '+log_dir+'/%03d.log'%(r)+'\n'
            file.write(line)
        
    # prepare the survival rate script
    #logs = list_logfiles(log_dir+'/sp_'+p)
    logs = map(lambda f: log_dir+'/sp_'+p+'/%03d.log'%(f), xrange(1,nb_exec+1) )
    with open(sr_extraction_list, 'a') as file:
        for  l in logs :
            line='python tools/'+sr_extraction_script+' -f '+l+' > '+l+'.sr\n' 
            file.write(line)

    # append to the global run script
    with open(global_script, 'a') as file:
        line = '/bin/cat '+ p +' | /usr/bin/parallel\n' + '/bin/mkdir '+ log_dir +'/sp_'+ p +'\n' + '/bin/mv '+ log_dir +'/*.log '+ log_dir +'/*.txt '+ log_dir +'/sp_'+ p +'\n' #+ 'python '+home+'/bin/send_sms.py -t \'Experiment '+str(count)+'/'+str(len(var_values))+' done\'\n'
        file.write(line)
    count +=1

# append the survival rate script to the run script        
with open(global_script, 'a') as file:
    line = '/bin/cat '+sr_extraction_list+' | /usr/bin/parallel\n' 
    file.write(line) 














