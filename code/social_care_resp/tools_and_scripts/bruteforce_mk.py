import ctypes
import numpy as np
from numpy.ctypeslib import ndpointer
import matplotlib.pyplot as plt
import copy
import time
from datetime import datetime
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
    
    # 'default' quality will use the default for the type of instance (if it comes from the literature, for example)
    quality_measure = 'paper' # 'default', 'paper', 'mankowska', 'ait_h', 'workload_balance'
    ds_skill_type = 'strictly-shared'
    max_time_seconds = 5
    verbose_level = 1
    create_python_plots = False
    create_html_website = False
    results_file_name = 'hhcrsp_results_test.txt'
    run_in_parallel = False
    parallel_workers = 1 # Only works if previous is true
    random_seed = -1 # Only if run_in_parallel = False. -1 is a true random seed, otherwise set to this value
    # random_seed = 5265489

    # Set the default options:
    options_vector = hhc.default_options_vector()
    # Short irace run:
    # do_twopt no_change_ls no_change_grasp pr_strategy pr_direction    sols_in_pool grasp_dl grasp_dr rcl_strategy
    # 1            1               1           3            2           20           0.50     0.48            2       
    options_vector[6] = 1.0 # Nurse order change active (In GRASP, between calls)
    options_vector[7] = 1.0 # deprecated
    options_vector[9] = 3.0 # PR_STRATEGY
    options_vector[10] = 2.0 # RCL Strategy
    options_vector[11] = 2.0 # PR_DIRECTION
    options_vector[8] = 20.0 # Solutions in pool
    options_vector[3] = 0.0 # Nurse order change active (neighbourhood in local search)
    options_vector[1] = 1.0 # Two-opt active
    options_vector[4] = 0.5 # -   GRASP: Delta low
    options_vector[5] = 0.48 # -   GRASP: Delta range
    options_vector[12] = 0 # -   Use gap (1) or precedence (0)


    options_vector[99] = 0 # -   print all inputs

    # For our own format, load driving matrix from disk. If not, it is saved after the first execution.
    # File name should be: self.name + '_' + 'driving' + '_times.npy',
    # where self.name = current_file.split('.')[0]
    load_from_disk = False 

    # Choosing file to run. All files in file_list will be run in a loop.
    # Need to set instance_type, directory and a file list (even if only with one file)

    instance_type = 'mankowska' # Options are: 'excel', 'mankowska', 'ait_h', 'pfile', 'tsp', 'braekers'
    file_list = []
    directory = '.'

    # print('DEBUG mimic ait h')
    # options_vector[50] = 1 # 1 if tardiness and overtime are infeasible, 0 if feasible
    # options_vector[51] = -0.3*60 # alpha_1 Travel time
    # options_vector[52] = 0.0 # alpha_2 Waiting time
    # options_vector[53] = 0.0 # alpha_3 Tardiness
    # options_vector[54] = 0.0 # alpha_4 Overtime
    # options_vector[55] = 0.00 # alpha_5 Workload balance
    # options_vector[56] = -1 # alpha_6 Preference score
    # options_vector[57] = 0.0 # Max tardiness (allowed)
    # print('remove above')


    # options_vector[55] = 0.00 # alpha_5 Workload balance




    if instance_type == 'excel':
        # directory = '..\\..\\Instances\\test_instances\\'
        # directory = '..\\..\\Instances\\'
        directory = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Instances'
        instance_name = 'random40_VRP_frozen_various_depots.xlsx'
        file_list = [instance_name]

    elif instance_type == 'pfile':
        directory = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Code\HHCRSP\testbed_1rep'
        # file_list = ['first_set_0.p', 'first_set_1.p', 'first_set_2.p', 'first_set_3.p', 'first_set_4.p', 'first_set_5.p', 'first_set_6.p', 'first_set_7.p', 'first_set_8.p', 'first_set_9.p']
        # file_list =  hhc.get_all_files_in_directory(directory, '.p')
        file_list = ['n10_sh_tw10l_ds10_de10_0.p']



        # file_list = [f for r, d, f in os.walk(directory)][0]
        # exit(-3)
        # file_list = ['default_set_' + str(i) + '.p' for i in range(100)]
        # print('Running files: ', file_list)

    elif instance_type == 'mankowska':
        print('DEBUG mimic mankowska')
        options_vector[50] = 0 # 1 if tardiness and overtime are infeasible, 0 if feasible
        options_vector[51] = -1/3*60 # alpha_1 Travel time
        options_vector[52] = 0.0 # alpha_2 Waiting time
        options_vector[53] = -1/3*60 # alpha_3 Tardiness
        options_vector[54] = 0.0 # alpha_4 Overtime
        options_vector[55] = 0.00 # alpha_5 Workload balance
        options_vector[56] = 0 # alpha_6 Preference score
        options_vector[57] = -1/3*60 # Max tardiness (allowed)
        print('remove above')
        # #MATFILE
        file_list_mat = []
        file_list_vns =  []

        directory_mat = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Instances\Mankowska\all_mk\\'
        # file_list_mat = ['saved_InstanzCPLEX_HCSRP_10_1.mat', 'saved_InstanzCPLEX_HCSRP_10_2.mat', 'saved_InstanzCPLEX_HCSRP_10_3.mat', 'saved_InstanzCPLEX_HCSRP_10_4.mat', 'saved_InstanzCPLEX_HCSRP_10_5.mat', 'saved_InstanzCPLEX_HCSRP_10_6.mat', 'saved_InstanzCPLEX_HCSRP_10_7.mat', 'saved_InstanzCPLEX_HCSRP_10_8.mat', 'saved_InstanzCPLEX_HCSRP_10_9.mat', 'saved_InstanzCPLEX_HCSRP_10_10.mat', 'saved_InstanzCPLEX_HCSRP_25_1.mat', 'saved_InstanzCPLEX_HCSRP_25_2.mat', 'saved_InstanzCPLEX_HCSRP_25_3.mat', 'saved_InstanzCPLEX_HCSRP_25_4.mat', 'saved_InstanzCPLEX_HCSRP_25_5.mat', 'saved_InstanzCPLEX_HCSRP_25_6.mat', 'saved_InstanzCPLEX_HCSRP_25_7.mat', 'saved_InstanzCPLEX_HCSRP_25_8.mat', 'saved_InstanzCPLEX_HCSRP_25_9.mat', 'saved_InstanzCPLEX_HCSRP_25_10.mat', 'saved_InstanzCPLEX_HCSRP_50_1.mat', 'saved_InstanzCPLEX_HCSRP_50_2.mat', 'saved_InstanzCPLEX_HCSRP_50_3.mat', 'saved_InstanzCPLEX_HCSRP_50_4.mat', 'saved_InstanzCPLEX_HCSRP_50_5.mat', 'saved_InstanzCPLEX_HCSRP_50_6.mat', 'saved_InstanzCPLEX_HCSRP_50_7.mat', 'saved_InstanzCPLEX_HCSRP_50_8.mat', 'saved_InstanzCPLEX_HCSRP_50_9.mat', 'saved_InstanzCPLEX_HCSRP_50_10.mat', 'saved_InstanzCPLEX_HCSRP_75_1.mat', 'saved_InstanzCPLEX_HCSRP_75_2.mat', 'saved_InstanzCPLEX_HCSRP_75_3.mat', 'saved_InstanzCPLEX_HCSRP_75_4.mat', 'saved_InstanzCPLEX_HCSRP_75_5.mat', 'saved_InstanzCPLEX_HCSRP_75_6.mat', 'saved_InstanzCPLEX_HCSRP_75_7.mat', 'saved_InstanzCPLEX_HCSRP_75_8.mat', 'saved_InstanzCPLEX_HCSRP_75_9.mat', 'saved_InstanzCPLEX_HCSRP_75_10.mat']
        # file_list_mat += ['saved_InstanzCPLEX_HCSRP_10_1.mat', 'saved_InstanzCPLEX_HCSRP_10_2.mat', 'saved_InstanzCPLEX_HCSRP_10_3.mat', 'saved_InstanzCPLEX_HCSRP_10_4.mat', 'saved_InstanzCPLEX_HCSRP_10_5.mat', 'saved_InstanzCPLEX_HCSRP_10_6.mat', 'saved_InstanzCPLEX_HCSRP_10_7.mat', 'saved_InstanzCPLEX_HCSRP_10_8.mat', 'saved_InstanzCPLEX_HCSRP_10_9.mat', 'saved_InstanzCPLEX_HCSRP_10_10.mat']
        # file_list_mat += ['saved_InstanzCPLEX_HCSRP_25_1.mat', 'saved_InstanzCPLEX_HCSRP_25_2.mat', 'saved_InstanzCPLEX_HCSRP_25_3.mat', 'saved_InstanzCPLEX_HCSRP_25_4.mat', 'saved_InstanzCPLEX_HCSRP_25_5.mat', 'saved_InstanzCPLEX_HCSRP_25_6.mat', 'saved_InstanzCPLEX_HCSRP_25_7.mat', 'saved_InstanzCPLEX_HCSRP_25_8.mat', 'saved_InstanzCPLEX_HCSRP_25_9.mat', 'saved_InstanzCPLEX_HCSRP_25_10.mat']
        # file_list_mat += ['saved_InstanzCPLEX_HCSRP_50_1.mat', 'saved_InstanzCPLEX_HCSRP_50_2.mat', 'saved_InstanzCPLEX_HCSRP_50_3.mat', 'saved_InstanzCPLEX_HCSRP_50_4.mat', 'saved_InstanzCPLEX_HCSRP_50_5.mat', 'saved_InstanzCPLEX_HCSRP_50_6.mat', 'saved_InstanzCPLEX_HCSRP_50_7.mat', 'saved_InstanzCPLEX_HCSRP_50_8.mat', 'saved_InstanzCPLEX_HCSRP_50_9.mat', 'saved_InstanzCPLEX_HCSRP_50_10.mat']
        # file_list_mat += ['saved_InstanzCPLEX_HCSRP_75_1.mat', 'saved_InstanzCPLEX_HCSRP_75_2.mat', 'saved_InstanzCPLEX_HCSRP_75_3.mat', 'saved_InstanzCPLEX_HCSRP_75_4.mat', 'saved_InstanzCPLEX_HCSRP_75_5.mat', 'saved_InstanzCPLEX_HCSRP_75_6.mat', 'saved_InstanzCPLEX_HCSRP_75_7.mat', 'saved_InstanzCPLEX_HCSRP_75_8.mat', 'saved_InstanzCPLEX_HCSRP_75_9.mat', 'saved_InstanzCPLEX_HCSRP_75_10.mat']

        # file_list_mat += ['saved_InstanzCPLEX_HCSRP_25_8.mat']
        file_list_mat += ['saved_InstanzCPLEX_HCSRP_10_8.mat']
        # file_list_mat += ['saved_InstanzCPLEX_HCSRP_10_2.mat']
        # #VNS
        directory_vns = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Instances\Mankowska\all_mk\\'
        # file_list_vns =  ['InstanzVNS_HCSRP_100_1.txt', 'InstanzVNS_HCSRP_100_2.txt','InstanzVNS_HCSRP_100_3.txt','InstanzVNS_HCSRP_100_4.txt','InstanzVNS_HCSRP_100_5.txt','InstanzVNS_HCSRP_100_6.txt','InstanzVNS_HCSRP_100_7.txt','InstanzVNS_HCSRP_100_8.txt','InstanzVNS_HCSRP_100_9.txt','InstanzVNS_HCSRP_100_10.txt','InstanzVNS_HCSRP_200_1.txt','InstanzVNS_HCSRP_200_2.txt','InstanzVNS_HCSRP_200_3.txt','InstanzVNS_HCSRP_200_4.txt','InstanzVNS_HCSRP_200_5.txt','InstanzVNS_HCSRP_200_6.txt','InstanzVNS_HCSRP_200_7.txt','InstanzVNS_HCSRP_200_8.txt','InstanzVNS_HCSRP_200_9.txt','InstanzVNS_HCSRP_200_10.txt','InstanzVNS_HCSRP_300_1.txt','InstanzVNS_HCSRP_300_2.txt','InstanzVNS_HCSRP_300_3.txt','InstanzVNS_HCSRP_300_4.txt','InstanzVNS_HCSRP_300_5.txt','InstanzVNS_HCSRP_300_6.txt','InstanzVNS_HCSRP_300_7.txt','InstanzVNS_HCSRP_300_8.txt','InstanzVNS_HCSRP_300_9.txt','InstanzVNS_HCSRP_300_10.txt']
        file_list_vns =  []
        # file_list_vns +=  ['InstanzVNS_HCSRP_100_1.txt', 'InstanzVNS_HCSRP_100_2.txt','InstanzVNS_HCSRP_100_3.txt','InstanzVNS_HCSRP_100_4.txt','InstanzVNS_HCSRP_100_5.txt','InstanzVNS_HCSRP_100_6.txt','InstanzVNS_HCSRP_100_7.txt','InstanzVNS_HCSRP_100_8.txt','InstanzVNS_HCSRP_100_9.txt','InstanzVNS_HCSRP_100_10.txt']
        # file_list_vns +=  ['InstanzVNS_HCSRP_200_1.txt','InstanzVNS_HCSRP_200_2.txt','InstanzVNS_HCSRP_200_3.txt','InstanzVNS_HCSRP_200_4.txt','InstanzVNS_HCSRP_200_5.txt','InstanzVNS_HCSRP_200_6.txt','InstanzVNS_HCSRP_200_7.txt','InstanzVNS_HCSRP_200_8.txt','InstanzVNS_HCSRP_200_9.txt','InstanzVNS_HCSRP_200_10.txt']
        # file_list_vns +=  ['InstanzVNS_HCSRP_300_1.txt','InstanzVNS_HCSRP_300_2.txt','InstanzVNS_HCSRP_300_3.txt','InstanzVNS_HCSRP_300_4.txt','InstanzVNS_HCSRP_300_5.txt','InstanzVNS_HCSRP_300_6.txt','InstanzVNS_HCSRP_300_7.txt','InstanzVNS_HCSRP_300_8.txt','InstanzVNS_HCSRP_300_9.txt','InstanzVNS_HCSRP_300_10.txt']

        # file_list_vns +=  ['InstanzVNS_HCSRP_300_1.txt']
        # file_list_vns +=  ['InstanzVNS_HCSRP_100_3.txt']

        directory = directory_mat
        file_list = file_list_mat + file_list_vns

    elif instance_type == 'ait_h':
        options_vector[12] = 1
        directory = 'C:\\Users\\clf1v16\\docs_offline\\01.HHCRSP\\Instances\\Ait_Haddadene\\adapted\\'
        print(os.getcwd())
        # directory = os.path.join(os.getcwd(), '..', 'Instances', 'Ait_Haddadene', 'adapted')
        # directory = '..\\Instances\\Ait_Haddadene\\adapted\\'
        # file_list = ['18-4-s-2a', '18-4-s-2b', '18-4-s-2c', '18-4-s-2d', '18-4-m-2a', '18-4-m-2b', '18-4-m-2c', '18-4-m-2d', '18-4-m-2e', '18-4-l-2a', '18-4-l-2b', '18-4-l-2c', '18-4-l-2d', '18-4-l-2e', '45-10-s-3a', '45-10-s-2a', '45-10-s-3b', '45-10-s-2b', '45-10-s-3c', '45-10-m-4', '45-10-m-2a', '45-10-m-2b', '45-10-m-3', '45-10-l-2a', '45-10-l-2b', '45-10-l-3', '45-10-l-4', '73-16-s-2a', '73-16-s-3', '73-16-s-2b', '73-16-m-3a', '73-16-m-2', '73-16-m-3b', '73-16-l-2', '73-16-l-3', '73-16-l-4', '73-16-l-5']
        # file_list = ['18-4-s-2a', '18-4-s-2b', '18-4-s-2c', '18-4-s-2d', '18-4-m-2a', '18-4-m-2b', '18-4-m-2c', '18-4-m-2d', '18-4-m-2e', '18-4-l-2a', '18-4-l-2b', '18-4-l-2c', '18-4-l-2d', '18-4-l-2e']
        # file_list = ['45-10-s-3a', '45-10-s-2a', '45-10-s-3b', '45-10-s-2b', '45-10-s-3c', '45-10-m-4', '45-10-m-2a', '45-10-m-2b', '45-10-m-3', '45-10-l-2a', '45-10-l-2b', '45-10-l-3', '45-10-l-4']
        file_list = ['73-16-s-2a', '73-16-s-3', '73-16-s-2b', '73-16-m-3a', '73-16-m-2', '73-16-m-3b', '73-16-l-2', '73-16-l-3', '73-16-l-4', '73-16-l-5']
        # file_list = ['45-10-s-3c']
        # file_list = ['18-4-s-2a']
        # file_list = ['73-16-s-3']
        
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
    f.write('------------------------------------------------------------\n')
    f.write('Date: ' + str(datetime.now()) + ', running ' + str(len(file_list)) +  ' files.\n')
    f.write("Instance\tCquality")
    f.write("\ttotalTravelTime")
    f.write("\ttotalTardiness")
    f.write("\tmaxTardiness")
    f.write("\telapsed_time\n")
    f.close()

    for file_idx, current_file in enumerate(file_list):

        print('\n=======================================================')
        print('Running file number {0}, "{1}"'.format(file_idx, current_file), end=' ')

        stt_time = time.perf_counter()
        if run_in_parallel:
            print('on {0} processes of {1} seconds each.'.format(parallel_workers, max_time_seconds))
        else:
            print('for a maximum of {0} seconds.'.format(max_time_seconds))

        options_list[0] = os.path.join(directory, current_file)

        # For MK instances, we need to check if it is one of the VNS files or not:
        if instance_type == 'mankowska':
            options_list[7] = current_file in file_list_vns
            if options_list[7]:
                directory = directory_vns
            else:
                directory = directory_mat

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
            file_to_run, instance_type, quality_measure, ds_skill_type,\
            max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector = options_list

            i = hhc.prepare_instance(file_to_run, instance_type, quality_measure, ds_skill_type,
                                 max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector)

            saved_od_data = i.od[0][0]

            thisSolMatrix = np.array(
[[-1, -1, -1,  1,  2,  3, -1, -1,  4,  0, -1, -1.],
 [-1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1.],
 [ 3,  5,  4, -1, -1, -1,  2,  0, -1, -1,  1,  6.]]
                                    )
            i.solMatrix = thisSolMatrix


            i.solve(randomSeed=random_seed_values[0], printAllCallData=0)
            i.solve = []
            i.lib = []
            i.fun = []
            i.Cquality = -1*i.od[0][0]
            i.od[0][0] = saved_od_data
            print("FINISHED, QUALITY: ", i.Cquality)
            exit(-2)



        quality = i.Cquality

        print('Finished.\nQuality: ' + str(quality))

        # Need to re-read the file...
        # i = prepare_instance(options_list[0], instance_type, quality_measure, ds_skill_type,
        # 				max_time_seconds, verbose_level, load_from_disk, options_list[-1])

        i.post_process_solution()
        i.full_solution_report(report=0, doPlots=create_python_plots)

        end_time = time.perf_counter()
        elapsed_time = end_time - stt_time
        print('Those were results for ' + str(current_file) + ' running for ' + str(np.round(elapsed_time, 1)) + ' seconds.')
        f = open(results_file_name, 'a')
        f.write(current_file + '\t' + str(i.Cquality))
        f.write("\t" + str(i.totalTravelTime))
        f.write("\t" + str(i.totalTardiness))
        f.write("\t" + str(i.maxTardiness))
        f.write("\t" + str(elapsed_time) + "\n")
        f.close()
        # i.simple_solution_plot(filename=currentFile + 'plot.png')
        # print('Saving to: ' + currentFile + 'coolfig.png')
        # i.simple_solution_plot(filename=(currentFile + 'coolfig.png'))
        # plt.show()
        # plt.show()
        # print("Returned this...")
        if create_python_plots:
            print('About to plot')
            # i.full_solution_report(report=1)
            i.simple_solution_plot()
            plt.show()

        # result = messagebox.askquestion("Results", "Generate website?", icon='question')
        # if result == "yes":
        if create_html_website:
            print('Generating website...')
            i.full_solution_report(report=0)
            i.solution_to_website()
            print('Done.')
        
        print('\n\n=======================================================')
        print('=======================================================')
        print('=======================================================\n\n')

    f = open(results_file_name, 'a')
    f.write('\nRun finished at: ' + str(datetime.now()))
    f.write('\n------------------------------------------------------------\n')
    f.close()

if __name__ == '__main__':
    repetitions = 1
    for i in range(repetitions):
        main()