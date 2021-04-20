import ctypes
import numpy as np
from numpy.ctypeslib import ndpointer
import matplotlib.pyplot as plt
import copy
import time
# import random
import os

from joblib import Parallel, delayed

# import tkinter
# from tkinter import messagebox
# import tkMessageBox # Python 2.x version

# Own modules:
import mankowska_data as Mk
import python_resources as pr
import instance_handler as hhc

# NOTE: THIS NEEDS TO BE CHANGED TO BE SIMILAR TO CONSTRUCTIVE_WRAPPER.PY

def solver(opt_obj, instance_file, options_vector=[]):
    # Solving parameters:
    
    quality_measure = opt_obj.quality_measure
    ds_skill_type = opt_obj.ds_skill_type
    max_time_seconds = opt_obj.max_time_seconds
    verbose_level = opt_obj.verbose_level
    create_python_plots = opt_obj.create_python_plots
    create_html_website = opt_obj.create_html_website
    results_file_name = opt_obj.results_file_name
    run_in_parallel = opt_obj.run_in_parallel
    parallel_workers = opt_obj.parallel_workers
    random_seed = opt_obj.random_seed
    file_idx = opt_obj.file_idx
    # Set the default options:

    if len(options_vector) < 1:
        print('ERROR!')
        # exit(-32423)



    # For our own format, load driving matrix from disk. If not, it is saved after the first execution.
    # File name should be: self.name + '_' + 'driving' + '_times.npy',
    # where self.name = instance_file.split('.')[0]
    load_from_disk = opt_obj.load_from_disk

    # Choosing file to run. All files in file_list will be run in a loop.
    # Need to set instance_type, directory and a file list (even if only with one file)

    instance_type = opt_obj.instance_type
    mankowska_vns = False
    if instance_type == 'mankowska-vns':
        instance_type = 'mankowska'
        mankowska_vns = True
    elif instance_type == 'mankowska-mat':
        instance_type = 'mankowska'
        mankowska_vns = True
    
    ###########################################################################################
    ###########################################################################################
    ###########################################################################################
    ###                                                                                     ###
    ###                                   START CODE                                        ###
    ###                                                                                     ###
    ###########################################################################################
    ###########################################################################################
    ###########################################################################################
    big_m = 10000000

    # Pack all options in a tuple:
    options_list = ['', instance_type, quality_measure, ds_skill_type,
                    max_time_seconds, verbose_level, load_from_disk, False, options_vector]

    f = open(results_file_name, 'a')

    
    
    print('Running file number {0}, "{1}"'.format(file_idx, instance_file), end=' ')
    if run_in_parallel:
        print('on {0} processes of {1} seconds each.'.format(parallel_workers, max_time_seconds))
    else:
        print('for a maximum of {0} seconds.'.format(max_time_seconds))

    options_list[0] = instance_file

    # For MK instances, we need to check if it is one of the VNS files or not:
    if instance_type == 'mankowska':
        options_list[7] = mankowska_vns

    random_seed_values = np.zeros(1, dtype=np.int32)
    if not run_in_parallel:
        if random_seed < 0:
            random_seed_values[0] = np.random.randint(low=0, high=big_m)
        else:
            random_seed_values[0] = random_seed
    else:
        random_seed_values = np.random.randint(low=0, high=big_m, size=parallel_workers)

    if run_in_parallel:
        solved_instances = Parallel(n_jobs=parallel_workers)(delayed(hhc.worker_task)(options_list, r_seed) for r_seed in random_seed_values)
        bQual = np.inf
        bInd = -1
        for idx,ii in enumerate(solved_instances):
            if(ii.Cquality < bQual):
                bInd = idx
                bQual = ii.Cquality
            print('\t> Run ' + str(idx) + ' quality:\t' + str(ii.Cquality))


        print('Best quality is: ' + str(bQual))
        i = solved_instances[bInd]
    else:
        i = hhc.worker_task(options_list, random_seed_values[0])



    quality = i.Cquality

    print('Finshed.\nQuality: ' + str(quality))

    # Need to re-read the file...
    # i = prepare_instance(options_list[0], instance_type, quality_measure, ds_skill_type,
    #               max_time_seconds, verbose_level, load_from_disk, options_list[-1])

    i.post_process_solution()
    i.full_solution_report(report=0, doPlots=False)
    print('Those were results for ' + str(instance_file))

    f = open(results_file_name, 'a')
    f.write(instance_file + '\t' + str(i.Cquality))
    f.write("\t" + str(i.totalTravelTime))
    f.write("\t" + str(i.totalTardiness))
    f.write("\t" + str(i.maxTardiness) + "\n")
    f.close()
    # i.simple_solution_plot(filename=currentFile + 'plot.png')
    # print('Saving to: ' + currentFile + 'coolfig.png')
    # i.simple_solution_plot(filename=(currentFile + 'coolfig.png'))
    # plt.show()
    # plt.show()
    # print("Returned this...")
    print('About to plot')
    if create_python_plots:
        # i.full_solution_report(report=1)
        i.simple_solution_plot()
        plt.show()

    # result = messagebox.askquestion("Results", "Generate website?", icon='question')
    # if result == "yes":
    if create_html_website:
        print('Generating website...')
        i.solution_to_website()
        print('Done.')
    
    print('\n=======================================================')
    
    return quality

# def prepare_instance(file_to_run,
#                      instance_type,
#                      quality_measure='mankowska',
#                      ds_skill_type='strictly-shared',
#                      max_time_seconds=-1,
#                      verbose_level=-1,
#                      load_from_disk=False,
#                      mkVNS=False,
#                      options_vector=[]):

#     if (max_time_seconds < 0 or verbose_level < 0):
#         print('Invalid parameters: ')
#         print('max_time_seconds = ', max_time_seconds)
#         print('verbose_level = ', verbose_level)
#         return -1
    
#     i = hhc.INSTANCE()

#     # i.name = file_to_run.split('.')[-2]
#     i.full_file_name = file_to_run
#     i.name = os.path.basename(file_to_run)
#     i.qualityMeasure = quality_measure
#     print('Skill type was', i.DSSkillType)
#     i.DSSkillType = ds_skill_type
#     print('Skill type now is', ds_skill_type)
#     i.MAX_TIME_SECONDS = max_time_seconds
#     i.verbose = verbose_level
#     i.loadFromDisk = load_from_disk
#     print('max time seconds ' + str(i.MAX_TIME_SECONDS))

#     # Generate the instance from the file:
#     if instance_type == 'excel':
#         i.read_excel(file_to_run)
#     elif instance_type == 'pfile':
#         i = i.unpickle_instance(file_to_run)
#         i.DSSkillType = 'strictly-shared'
#         if len(i.capabilityOfDoubleServices) > 0:
#             # print('i.capabilityOfDoubleServices\n', i.capabilityOfDoubleServices)
#             # print('i.capabilityOfDoubleServices type\n', type(i.capabilityOfDoubleServices))
#             i.capabilityOfDoubleServices = np.ascontiguousarray(i.capabilityOfDoubleServices.reshape(-1))
#             # print('reshaped')
#     elif instance_type == 'mankowska':
#         quality_measure = 'mankowska'
#         i.DSSkillType = 'strictly-shared'
#         if mkVNS:
#             i = Mk.generate_Mk(i, matfile=file_to_run, filetype='large_vns')
#         else:
#             i = Mk.generate_Mk(i, file_to_run)

#     elif instance_type == 'ait_h':
#         quality_measure = 'ait_h'
#         i = Mk.ait_h_to_c(file_to_run, i)

#     elif instance_type == 'tsp':
#         i = Mk.generate_Mk(i, matfile=file_to_run, filetype='tsptw')
#         # i = Mk.generate_Mk(tspLib=True)
#         print('TSP format implementation needs review')
#         return(-1)

#     elif instance_type == 'braekers':
#         i.read_braekers(file_to_run)
#         # i.generate_hampshire_euclidean()
#         # i.generate_hampshire2()




#     # Set lambdas:
#     i.lambda_1 = 1
#     i.lambda_2 = 1
#     i.lambda_3 = 1
#     i.lambda_4 = 1
#     i.lambda_5 = 1
#     i.lambda_6 = 10
#     print('Skill type still is', i.DSSkillType)
#     # Initialise the rest:
#     print('BEFORE INIT ', i.MAX_TIME_SECONDS)
#     i.init()
#     if (len(options_vector) == 100):
#         i.algorithmOptions = options_vector
#     else:
#         i.algorithmOptions = hhc.default_options_vector()

#     if quality_measure == 'ait_h':
#         i.algorithmOptions[0] = 0.0
#     elif quality_measure == 'mankowska':
#         i.algorithmOptions[0] = 1.0

#     return i

# def worker_task(options_tuple, rSeed):

#     # Unpack all the options:
#     file_to_run, instance_type, quality_measure, ds_skill_type,\
#     max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector = options_tuple

#     i = prepare_instance(file_to_run, instance_type, quality_measure, ds_skill_type,
#                          max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector)

#     saved_od_data = i.od[0][0]
#     i.solve(randomSeed=rSeed, printAllCallData=False)
#     i.solve = []
#     i.lib = []
#     i.fun = []
#     i.Cquality = -1*i.od[0][0]
#     i.od[0][0] = saved_od_data
#     # print('WORKER QUALITY ' + str(i.c_quality))
#     # quality = i.full_solution_report(doPlots=False, report=0)
#     # print('Another quality ' + str(quality))
#     # print('i.od[0][0]  ' + str(i.od[0][0]))
#     return i


if __name__ == '__main__':
    print('This file is for launching from target-runner-hhcrsp.bat, not standalone version (see constructive_wrapper instead)')