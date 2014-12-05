#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------------

import os, shutil


home = os.environ["HOME"]
config_dir    = 'config'
log_dir       = 'logs'
template_file = 'template.properties'
nb_exec       = 28
global_script = 'run-all'

var_parameter = 'gSelectionPressure'
var_values = [ '0.00', '0.25', '0.50', '0.75', '1.00' ]

parameters={
    'gInitialNumberOfRobots': 300,
    'gNbOfPhysicalObjects': 0,
    'gSigmaRef': 0.3,
    'gFitnessFunction': 0,  # 0: locomotion, 1: collection, 2: forraging 
    'gControllerType': 1,   # 0 = NEAT, 1 = FFNN
    'gEvaluationTime': 200,
    'gMaxIt'         : 20000 }

# copy the tenmplate file and update the values 
src_path = os.path.join(config_dir, template_file)
 
if os.path.isfile(global_script):
    os.remove(global_script)


for p in var_values :

    # copy template prameter file 
    dst_path = os.path.join(config_dir, 'foo-'+p+'.properties') 
    shutil.copy(src_path, dst_path)
    # add experiments parameters
    with open(dst_path, 'a') as file:
        for k in parameters.keys() :
            line = k+' = '+str(parameters[k])+'\n'
            file.write(line)
        line = var_parameter+' = '+p+'\n'
        file.write(line)

    # create single instances file (fed to parallel)
    with open(p, 'w') as file:
        for r in xrange(1,nb_exec+1):
            line = 'roborobo -l '+ dst_path + ' > '+log_dir+'/%03d.log'%(r)+'\n'
            file.write(line)

        
    # append to the global run script
    with open(global_script, 'a') as file:
        line = '/bin/cat '+ p +' | /usr/bin/parallel\n' + '/bin/mkdir '+ log_dir +'/sp_'+ p +'\n' + '/bin/mv '+ log_dir +'/*.log '+ log_dir +'/*.txt '+ log_dir +'/sp_'+ p +'\n' + home+'/bin/sms -t \'Experiment '+p+' done\'\n'
        file.write(line)


    
