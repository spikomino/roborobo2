#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, shutil

config_dir    = 'config'
log_dir       = 'logs'
template_file = 'template.properties'
nb_exec       = 30

parameters={
    'gSelectionPressure': 0.0,
    'gInitialNumberOfRobots': 300,
    'gNbOfPhysicalObjects': 0,
    'gSigmaRef': 0.3,
    'gFitnessFunction': 0,  # 0: locomotion, 1: collection, 2: forraging 
    'gControllerType': 1,   # 0 = NEAT, 1 = FFNN
    'gEvaluationTime': 500,
    'gMaxIt'         : 125000 }

# copy the tenmplate file and update the values 
src_path = os.path.join(config_dir, template_file)
dst_path = os.path.join(config_dir, 'foo-0.properties') 
shutil.copy(src_path, dst_path)

with open(dst_path, 'a') as file:
    for k in parameters.keys() :
        line = k+' = '+str(parameters[k])+'\n'
        file.write(line)


# run many instances 
for r in xrange(1,nb_exec+1):
    print 'roborobo -l '+ dst_path + ' > '+log_dir+'/%03d.log'%(r)
    

        
