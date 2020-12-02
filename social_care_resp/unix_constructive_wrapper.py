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



def main():
    # Solving parameters:
    
    quality_measure = 'ait_h'
    ds_skill_type = 'strictly-shared'
    max_time_seconds = 10
    verbose_level = 10
    create_python_plots = False
    create_html_website = False
    results_file_name = 'hhcrsp_results.txt'
    run_in_parallel = False
    parallel_workers = 3 # Only works if previous is true
    random_seed = -1 # Only if run_in_parallel = False. -1 is a true random seed, otherwise set to this value
    
    # Set the default options:
    options_vector = hhc.default_options_vector()

    options_vector[6] = 1.0 # Nurse order change active (In GRASP, between calls)
    options_vector[7] = 1.0 # performPathRelinking
    options_vector[9] = 1.0 # performPathRelinking for every solution
    options_vector[8] = 30.0 # Solutions in pool
    options_vector[3] = 0.0 # Nurse order change active (neighbourhood in local search)
    options_vector[1] = 0.0 # Two-opt active

    options_vector[4] = 0.01 # -   GRASP: Delta low
    options_vector[5] = 0.6 # -   GRASP: Delta range

    # For our own format, load driving matrix from disk. If not, it is saved after the first execution.
    # File name should be: self.name + '_' + 'driving' + '_times.npy',
    # where self.name = current_file.split('.')[0]
    load_from_disk = False 

    # Choosing file to run. All files in file_list will be run in a loop.
    # Need to set instance_type, directory and a file list (even if only with one file)

    instance_type = 'ait_h' # Options are: 'excel', 'mankowska', 'ait_h', 'tsp', 'braekers'
    file_list = []
    directory = '.'
    if instance_type == 'excel':
        # directory = '..\\..\\Instances\\test_instances\\'
        directory = '..\\..\\Instances\\'
        instance_name = 'random40_VRP_frozen_various_depots.xlsx'
        file_list = [instance_name]

    elif instance_type == 'mankowska':
        # #MATFILE
        directory = '/home/clf1v16/instances/hhcrsp/mankowska/'
        # directory_mat = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Instances\Mankowska\all_mk\\'
        file_list_mat = ['saved_InstanzCPLEX_HCSRP_10_1.mat', 'saved_InstanzCPLEX_HCSRP_10_2.mat', 'saved_InstanzCPLEX_HCSRP_10_3.mat', 'saved_InstanzCPLEX_HCSRP_10_4.mat', 'saved_InstanzCPLEX_HCSRP_10_5.mat', 'saved_InstanzCPLEX_HCSRP_10_6.mat', 'saved_InstanzCPLEX_HCSRP_10_7.mat', 'saved_InstanzCPLEX_HCSRP_10_8.mat', 'saved_InstanzCPLEX_HCSRP_10_9.mat', 'saved_InstanzCPLEX_HCSRP_10_10.mat', 'saved_InstanzCPLEX_HCSRP_25_1.mat', 'saved_InstanzCPLEX_HCSRP_25_2.mat', 'saved_InstanzCPLEX_HCSRP_25_3.mat', 'saved_InstanzCPLEX_HCSRP_25_4.mat', 'saved_InstanzCPLEX_HCSRP_25_5.mat', 'saved_InstanzCPLEX_HCSRP_25_6.mat', 'saved_InstanzCPLEX_HCSRP_25_7.mat', 'saved_InstanzCPLEX_HCSRP_25_8.mat', 'saved_InstanzCPLEX_HCSRP_25_9.mat', 'saved_InstanzCPLEX_HCSRP_25_10.mat', 'saved_InstanzCPLEX_HCSRP_50_1.mat', 'saved_InstanzCPLEX_HCSRP_50_2.mat', 'saved_InstanzCPLEX_HCSRP_50_3.mat', 'saved_InstanzCPLEX_HCSRP_50_4.mat', 'saved_InstanzCPLEX_HCSRP_50_5.mat', 'saved_InstanzCPLEX_HCSRP_50_6.mat', 'saved_InstanzCPLEX_HCSRP_50_7.mat', 'saved_InstanzCPLEX_HCSRP_50_8.mat', 'saved_InstanzCPLEX_HCSRP_50_9.mat', 'saved_InstanzCPLEX_HCSRP_50_10.mat', 'saved_InstanzCPLEX_HCSRP_75_1.mat', 'saved_InstanzCPLEX_HCSRP_75_2.mat', 'saved_InstanzCPLEX_HCSRP_75_3.mat', 'saved_InstanzCPLEX_HCSRP_75_4.mat', 'saved_InstanzCPLEX_HCSRP_75_5.mat', 'saved_InstanzCPLEX_HCSRP_75_6.mat', 'saved_InstanzCPLEX_HCSRP_75_7.mat', 'saved_InstanzCPLEX_HCSRP_75_8.mat', 'saved_InstanzCPLEX_HCSRP_75_9.mat', 'saved_InstanzCPLEX_HCSRP_75_10.mat']
        # file_list_mat = ['saved_InstanzCPLEX_HCSRP_10_8.mat']

        # #VNS
        # directory_vns = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Instances\Mankowska\all_mk\\'
        # file_list_vns =  ['InstanzVNS_HCSRP_100_1.txt', 'InstanzVNS_HCSRP_100_2.txt','InstanzVNS_HCSRP_100_3.txt','InstanzVNS_HCSRP_100_4.txt','InstanzVNS_HCSRP_100_5.txt','InstanzVNS_HCSRP_100_6.txt','InstanzVNS_HCSRP_100_7.txt','InstanzVNS_HCSRP_100_8.txt','InstanzVNS_HCSRP_100_9.txt','InstanzVNS_HCSRP_100_10.txt','InstanzVNS_HCSRP_200_1.txt','InstanzVNS_HCSRP_200_2.txt','InstanzVNS_HCSRP_200_3.txt','InstanzVNS_HCSRP_200_4.txt','InstanzVNS_HCSRP_200_5.txt','InstanzVNS_HCSRP_200_6.txt','InstanzVNS_HCSRP_200_7.txt','InstanzVNS_HCSRP_200_8.txt','InstanzVNS_HCSRP_200_9.txt','InstanzVNS_HCSRP_200_10.txt','InstanzVNS_HCSRP_300_1.txt','InstanzVNS_HCSRP_300_2.txt','InstanzVNS_HCSRP_300_3.txt','InstanzVNS_HCSRP_300_4.txt','InstanzVNS_HCSRP_300_5.txt','InstanzVNS_HCSRP_300_6.txt','InstanzVNS_HCSRP_300_7.txt','InstanzVNS_HCSRP_300_8.txt','InstanzVNS_HCSRP_300_9.txt','InstanzVNS_HCSRP_300_10.txt']
        file_list_vns =  []

        # directory = directory_mat
        file_list = file_list_mat + file_list_vns

    elif instance_type == 'ait_h':
        directory = '/home/clf1v16/instances/hhcrsp/ait_h/'
        # file_list = ['18-4-s-2a', '18-4-s-2b', '18-4-s-2c', '18-4-s-2d', '18-4-m-2a', '18-4-m-2b', '18-4-m-2c', '18-4-m-2d', '18-4-m-2e', '18-4-l-2a', '18-4-l-2b', '18-4-l-2c', '18-4-l-2d', '18-4-l-2e', '45-10-s-3a', '45-10-s-2a', '45-10-s-3b', '45-10-s-2b', '45-10-s-3c', '45-10-m-4', '45-10-m-2a', '45-10-m-2b', '45-10-m-3', '45-10-l-2a', '45-10-l-2b', '45-10-l-3', '45-10-l-4', '73-16-s-2a', '73-16-s-3', '73-16-s-2b', '73-16-m-3a', '73-16-m-2', '73-16-m-3b', '73-16-l-2', '73-16-l-3', '73-16-l-4', '73-16-l-5']
        # file_list = ['73-16-s-2a', '73-16-s-3', '73-16-s-2b', '73-16-m-3a', '73-16-m-2', '73-16-m-3b', '73-16-l-2', '73-16-l-3', '73-16-l-4', '73-16-l-5']
        # file_list = ['45-10-s-3c']
        file_list = ['45-10-l-2b']
        
    elif instance_type == 'braekers':
        directory = 'C:\\Users\\clf1u13\\Docs\\01.HHCRSP\\Instances\\Braekers\\bihcrsp_instances\\'
        fName = 'bihcrsp_33.txt'

    elif instance_type == 'tsp':
        directory = 'C:\\Users\\clf1u13\\Downloads\\SolomonPotvinBengio\\'
        file_list = allTSPTW = ['rc_201.1.txt','rc_202.3.txt','rc_204.1.txt','rc_205.4.txt','rc_207.2.txt','rc_201.2.txt','rc_202.4.txt','rc_204.2.txt','rc_206.1.txt','rc_207.3.txt','rc_201.3.txt','rc_203.1.txt','rc_204.3.txt','rc_206.2.txt','rc_207.4.txt','rc_201.4.txt','rc_203.2.txt','rc_205.1.txt','rc_206.3.txt','rc_208.1.txt','rc_202.1.txt','rc_203.3.txt','rc_205.2.txt','rc_206.4.txt','rc_208.2.txt','rc_202.2.txt','rc_203.4.txt','rc_205.3.txt','rc_207.1.txt','rc_208.3.txt']
        print('TSP support needs to be reviewed')
        return 0

    ###########################################################################################
    ###########################################################################################
    ###########################################################################################
    ###									                 	 								###
    ###									  START CODE    	 								###
    ###									                 	 								###
    ###########################################################################################
    ###########################################################################################
    ###########################################################################################
    big_m = 10000000

    # Pack all options in a tuple:
    options_list = ['', instance_type, quality_measure, ds_skill_type,
                    max_time_seconds, verbose_level, load_from_disk, False, options_vector]

    f = open(results_file_name, 'a')


    for file_idx, current_file in enumerate(file_list):
        
        print('Running file number {0}, "{1}"'.format(file_idx, current_file), end=' ')
        if run_in_parallel:
            print('on {0} processes of {1} seconds each.'.format(parallel_workers, max_time_seconds))
        else:
            print('for a maximum of {0} seconds.'.format(max_time_seconds))

        options_list[0] = directory + current_file

        # For MK instances, we need to check if it is one of the VNS files or not:
        if instance_type == 'mankowska':
            options_list[7] = current_file in file_list_vns
            # if options_list[7]:
                # directory = directory_vns
            # else:
                # directory = directory_mat

        random_seed_values = np.zeros(1, dtype=np.int32)
        if not run_in_parallel:
            if random_seed < 0:
                random_seed_values[0] = np.random.randint(low=0, high=big_m)
        else:
            random_seed_values = np.random.randint(low=0, high=big_m, size=parallel_workers)



        if run_in_parallel:
            solved_instances = Parallel(n_jobs=parallel_workers)(delayed(worker_task)(options_list, r_seed) for r_seed in random_seed_values)
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
            i = worker_task(options_list, random_seed_values[0])



        quality = i.Cquality

        print('Finshed.\nQuality: ' + str(quality))

        # Need to re-read the file...
        # i = prepare_instance(options_list[0], instance_type, quality_measure, ds_skill_type,
        # 				max_time_seconds, verbose_level, load_from_disk, options_list[-1])

        i.post_process_solution()
        i.full_solution_report(report=0, doPlots=False)
        print('Those were results for ' + str(current_file))

        f = open(results_file_name, 'a')
        f.write(current_file + '\t' + str(i.Cquality))
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

def prepare_instance(file_to_run,
                     instance_type,
                     quality_measure='mankowska',
                     ds_skill_type='strictly-shared',
                     max_time_seconds=-1,
                     verbose_level=-1,
                     load_from_disk=False,
                     mkVNS=False,
                     options_vector=[]):

    if (max_time_seconds < 0 or verbose_level < 0):
        print('Invalid parameters: ')
        print('max_time_seconds = ', max_time_seconds)
        print('verbose_level = ', verbose_level)
        return -1
    
    i = hhc.INSTANCE()

    # i.name = file_to_run.split('.')[-2]
    i.full_file_name = file_to_run
    i.name = os.path.basename(file_to_run)
    i.qualityMeasure = quality_measure
    print('Skill type was', i.DSSkillType)
    i.DSSkillType = ds_skill_type
    print('Skill type now is', ds_skill_type)
    i.MAX_TIME_SECONDS = max_time_seconds
    i.verbose = verbose_level
    i.loadFromDisk = load_from_disk
    print('max time seconds ' + str(i.MAX_TIME_SECONDS))

    # Generate the instance from the file:
    if instance_type == 'excel':
        i.read_excel(file_to_run)

    elif instance_type == 'mankowska':
        quality_measure = 'mankowska'
        i.DSSkillType = 'strictly-shared'
        if mkVNS:
            i = Mk.generate_Mk(i, matfile=file_to_run, filetype='large_vns')
        else:
            i = Mk.generate_Mk(i, file_to_run)

    elif instance_type == 'ait_h':
        quality_measure = 'ait_h'
        i = Mk.ait_h_to_c(file_to_run, i)

    elif instance_type == 'tsp':
        i = Mk.generate_Mk(i, matfile=file_to_run, filetype='tsptw')
        # i = Mk.generate_Mk(tspLib=True)
        print('TSP format implementation needs review')
        return(-1)

    elif instance_type == 'braekers':
        i.read_braekers(file_to_run)
        # i.generate_hampshire_euclidean()
        # i.generate_hampshire2()




    # Set lambdas:
    i.lambda_1 = 1
    i.lambda_2 = 1
    i.lambda_3 = 1
    i.lambda_4 = 1
    i.lambda_5 = 1
    i.lambda_6 = 10
    print('Skill type still is', i.DSSkillType)
    # Initialise the rest:
    print('BEFORE INIT ', i.MAX_TIME_SECONDS)
    i.init()
    if (len(options_vector) == 100):
        i.algorithmOptions = options_vector
    else:
        i.algorithmOptions = hhc.default_options_vector()

    if quality_measure == 'ait_h':
        i.algorithmOptions[0] = 0.0
    elif quality_measure == 'mankowska':
        i.algorithmOptions[0] = 1.0


    i.MAX_TIME_SECONDS = max_time_seconds
    return i

def worker_task(options_tuple, rSeed):

    # Unpack all the options:
    file_to_run, instance_type, quality_measure, ds_skill_type,\
    max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector = options_tuple

    i = prepare_instance(file_to_run, instance_type, quality_measure, ds_skill_type,
                         max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector)

    saved_od_data = i.od[0][0]
    i.solve(randomSeed=rSeed, printAllCallData=False)
    i.solve = []
    i.lib = []
    i.fun = []
    i.Cquality = -1*i.od[0][0]
    i.od[0][0] = saved_od_data
    # print('WORKER QUALITY ' + str(i.c_quality))
    # quality = i.full_solution_report(doPlots=False, report=0)
    # print('Another quality ' + str(quality))
    # print('i.od[0][0]  ' + str(i.od[0][0]))
    return i


if __name__ == '__main__':
    main()
