#-----------------------#
# CARROT - CARe ROuting Tool
# constructive_wrapper.py
# main file to run the program
# 01/11/2020
#-----------------------#

import os
import time
import numpy as np
import pandas as pd
from datetime import datetime
from tkinter import *

# Own modules:
import instance_handler_dfs as ihd
import tools_and_scripts.get_data_dfs as gdd
import tools_and_scripts.tkinter_userinput as tui
import tools_and_scripts.week_counter as wc

def main():
    
    # Get user input variables using tkinter entry box:
    # area, tw_interval, wb_balance, quality_measure, max_time_seconds, create_html_website, create_python_plots, codepoint_directory, input_filename, date_selected = tui.get_user_input_variables()
    area = 'Hampshire'
    tw_interval = 15
    wb_balance = 1
    quality_measure = 'default' # default = paper
    max_time_seconds = 60
    create_html_website = False
    create_python_plots = False
    input_filename = r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/CarrotAll/data/abicare/2021.07.15 - UOS - Clients - Carers - Rota - Anonymised.xlsx'
    codepoint_directory = r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/CarrotAll/data/codepo_gb'
    date_selected = pd.Timestamp("2021-07-27")
    instance_type = 'ait_h' # Can be either 'abicare' or 'ait_h'
    # random_seed = 13027 # Should random seed be added to tkinter box?
    # random_seed = -1 # Only if run_in_parallel = False. -1 is a true random seed, otherwise set to this value

    # Convert date_selected from datetime_date to Timestamp
    # planning_date = pd.Timestamp(date_selected)

    # Create options vector
    options_vector = ihd.default_options_vector() 

    if instance_type == 'abicare':
        random_seed = 13027 # Should random seed be added to tkinter box?

        planning_date = date_selected
        
        # Obtain day of week index of planning_date for both 2-week and 8-week schedules
        week_period2 = 2
        week_period8 = 8
        dayindex_2weeks = wc.calculate_cycle_day(planning_date, week_period2)
        dayindex_8weeks = wc.calculate_cycle_day(planning_date, week_period8)

        # Create dataframes for clients and carers using information from input excel file:
        client_df, nurseshift_df, nurseday_df = gdd.get_info_create_dfs(area, tw_interval, planning_date, dayindex_2weeks, dayindex_8weeks, print_statements=False, filename=input_filename, foldername=codepoint_directory)
    
        # Start time of program
        start_time_program = time.perf_counter()

        # Main function of program: create the instance from the client and carer dataframes, and call the C program to run GRASP-VNS
        inst = ihd.create_solve_inst(client_df, nurseshift_df, nurseday_df, planning_date, options_vector, wb_balance, quality_measure, max_time_seconds, random_seed)

        # End time of program
        end_time_program = time.perf_counter()
        elapsed_time = end_time_program - start_time_program

        # Fill jobObjs and carerObjs lists with solution data
        inst.full_solution_report(report=0, doPlots=create_python_plots)

        # Create map of solution in website
        if create_html_website:
            inst.solution_to_website_dst(add_plots=create_python_plots)    

        # Put results in results_area_date.txt file
        inst.output_results_file(elapsed_time)
        
        # Output solution into client_df and save as csv
        inst.solution_df_csv(client_df)

    elif instance_type == 'ait_h':
        options_vector[12] = 1
        directory = r'./Instances/literature/ait_h/'
        print(os.getcwd())
        # directory = os.path.join(os.getcwd(), '..', 'Instances', 'Ait_Haddadene', 'adapted')
        # directory = '..\\Instances\\Ait_Haddadene\\adapted\\'

        file_list = ['18-4-m-2e']
        results_file_name = 'paper_18_4_m_2e_getopt_newWBcoeff-2000-5-5.txt'
        # results_file_name = 'aith_45_10_m_4_getopt24.txt'

        # file_list = ['18-4-s-2a', '18-4-s-2b', '18-4-s-2c', '18-4-s-2d'] # 18-4-s (4 files)
        # file_list = ['18-4-m-2a', '18-4-m-2b', '18-4-m-2c', '18-4-m-2d', '18-4-m-2e'] # 18-4-m (5 files)
        # file_list = ['18-4-l-2a', '18-4-l-2b', '18-4-l-2c', '18-4-l-2d', '18-4-l-2e'] # 18-4-l (5 files)

        # file_list = ['45-10-s-2a', '45-10-s-2b', '45-10-s-3a', '45-10-s-3b', '45-10-s-3c'] # 45-10-s (5 files)
        # file_list = ['45-10-m-2a', '45-10-m-2b', '45-10-m-3', '45-10-m-4'] # 45-10-m (4 files)
        # file_list = ['45-10-l-2a', '45-10-l-2b', '45-10-l-3', '45-10-l-4'] # 45-10 l (4 files)

        # file_list = ['73-16-s-2a', '73-16-s-2b', '73-16-s-3'] # 73-16-s (3 files)
        # file_list = ['73-16-m-2', '73-16-m-3a', '73-16-m-3b'] # 73-16-m (3 files)
        # file_list = ['73-16-l-2', '73-16-l-3', '73-16-l-4', '73-16-l-5'] # 73-16-l (4 files)

        # quality_measure = 'ait_h' # 'default', 'paper', 'mankowska', 'ait_h', 'workload_balance'
        quality_measure = 'paper' # 'default', 'paper', 'mankowska', 'ait_h', 'workload_balance'
        ds_skill_type = 'strictly-shared'
        max_time_seconds = 60
        verbose_level = 1
        create_python_plots = False
        create_html_website = False
        # results_file_name = 'aith_18_4_m_2b_getopt.txt'
        load_from_disk = False 
        run_in_parallel = False
        parallel_workers = 1 # Only works if previous is true
        random_seed = -1 # Only if run_in_parallel = False. -1 is a true random seed, otherwise set to this value
        big_m = 10000000

        options_list = ['', instance_type, quality_measure, ds_skill_type, max_time_seconds, verbose_level, load_from_disk, False, options_vector]

        f = open(results_file_name, 'a')
        f.write('Date: ' + str(datetime.now()) + ', running ' + str(len(file_list)) +  ' files.\n')
        f.write('------------------------------------------------------------\n')
        f.close()

        for file_idx, current_file in enumerate(file_list):

            print('\n=======================================================')
            print('Running file number {0}, "{1}"'.format(file_idx, current_file), end=' ')
            print('for a maximum of {0} seconds.'.format(max_time_seconds))

            options_list[0] = os.path.join(directory, current_file)

            random_seed_values = np.zeros(1, dtype=np.int32)
            if random_seed < 0:
                random_seed_values[0] = np.random.randint(low=0, high=big_m)
            else:
                random_seed_values[0] = random_seed

    
            # random_seed_values[0] = 1965620
            
            stt_time = time.perf_counter() 

            inst = ihd.create_solve_inst_aith(options_list, random_seed_values[0])

            quality = inst.Cquality

            print('Finshed.\nQuality: ' + str(quality))

            
            inst.full_solution_report(report=0, doPlots=create_python_plots)

            end_time = time.perf_counter()
            elapsed_time = end_time - stt_time

            print('Those were results for ' + str(current_file) + ' running for ' + str(np.round(elapsed_time, 1)) + ' seconds.')

            f = open(results_file_name, 'a')
            # f.write('------------------------------------------------------------\n')
            f.write('Instance: ' + current_file + '\n')
            f.write('Quality: ' + str(inst.Cquality) + '\n')
            f.write('Measure: ' + str(inst.quality_measure) + '\n')
            f.write('Nurses: ' + str(inst.nNurses) + '\n')
            f.write('Jobs: ' + str(inst.nJobs) + '\n')
            f.write('Total Time: ' + str(inst.totalTime) + '\n')
            f.write('Total Service Time: ' + str(inst.totalServiceTime) + '\n')
            f.write('Total Travel Time: ' + str(inst.totalTravelTime) + '\n')
            f.write('Total Waiting Time: ' + str(inst.totalWaitingTime) + '\n')
            f.write('Total Tardiness: ' + str(inst.totalTardiness) + '\n')
            f.write('Total Overtime: ' + str(inst.totalOvertime) + '\n')
            f.write('Total Preference: ' + str(inst.totalPref) + '\n')
            f.write('Max Travel Time: ' + str(inst.maxTravelTime) + '\n')
            f.write('Max Waiting Time: ' + str(inst.maxWaitingTime) + '\n')
            f.write('Max Diff Workload: ' + str(inst.maxDiffWorkload) + '\n')
            f.write('Elapsed Time (s): ' + str(elapsed_time) + '\n')
            f.write('------------------------------------------------------------\n')
            f.close()
            print('Instance: ', current_file)
            print('Quality: ', inst.Cquality)
            print('Measure: ', inst.quality_measure)
            print('Nurses: ', inst.nNurses)
            print('Jobs: ', inst.nJobs)
            print('Total Time: ', inst.totalTime)
            print('Total Service Time: ', inst.totalServiceTime)
            print('Total Travel Time: ', inst.totalTravelTime)
            print('Total Waiting Time: ', inst.totalWaitingTime)
            print('Total Tardiness: ', inst.totalTardiness)
            print('Total Overtime: ', inst.totalOvertime)
            print('Total Preference: ', inst.totalPref)
            print('Max Travel Time: ', inst.maxTravelTime)
            print('Max Waiting Time: ', inst.maxWaitingTime)
            print('Max Diff Workload: ', inst.maxDiffWorkload)
            print('Elapsed Time (s): ', elapsed_time)
            print('Max travel coeff: ', inst.algorithmOptions[58])
            print('Max waiting coeff: ', inst.algorithmOptions[59])
            print('WB Coeff: ', inst.algorithmOptions[60])
            
            print('\n\n=======================================================')
            print('=======================================================\n\n')
        # End for fileidx loop

        f = open(results_file_name, 'a')
        f.write('\nRun finished at: ' + str(datetime.now()))
        f.write('\n------------------------------------------------------------\n')
        f.close()
    # End if-elif
    print('\nEnd of program.')
### --- End def main --- ###

if __name__ == '__main__':
    repetitions = 1
    for i in range(repetitions):
        main()
### --- End __name__ --- ###