#-----------------------#
# constructive_wrapper.py
# 01/11/2021
#-----------------------#

import os
import ctypes
import copy
import time
import pickle
import numpy as np
import pandas as pd
from datetime import datetime
import matplotlib.pyplot as plt
from numpy.ctypeslib import ndpointer
from joblib import Parallel, delayed
from tkinter import *

# Own modules:
# import python_resources as pr
import instance_handler_dfs as ihd
import tools_and_scripts.retrieve_info_dfs as rdi
import tools_and_scripts.tkinter_userinput as tui
import tools_and_scripts.get_data_dfs as gdd

def main():
    
    # Get user input variables using tkinter entry box:
    area, tw_interval, wb_balance, quality_measure, max_time_seconds, create_html_website, create_python_plots, codepoint_directory, input_filename = tui.get_user_input_variables()

    # exit(-1)

    random_seed = 13027 # NOTE: used for testing only. #35807 600seconds 13027

    # Create options vector:
    options_vector = ihd.default_options_vector() 

    # Create dataframes for clients and carers using information from excel file:
    client_df, carershift_df, carerday_df = rdi.retrieve_dfs(area, tw_interval, print_statements=False, filename=input_filename, foldername=codepoint_directory)
    # client_df, carershift_df, carerday_df = rdi.retrieve_dfs(area, tw_interval, print_statements=False, filename=input_filename, foldername=r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\codepo_gb')
    # client_df, carershift_df, carerday_df = gdd.get_info_create_dfs(area, tw_interval, print_statements=False)
    # print(client_df)
    # print(carershift_df)
    # print(carerday_df)
    # print('Len client_df:', len(client_df))
    # print('Len carershift_df:', len(carershift_df))
    # print('Len carerday_df:', len(carerday_df))
    # exit(-1)

    print('\n-------------------------------------------------------')
    start_time_program = time.perf_counter()

    inst = ihd.create_solve_inst(client_df, carershift_df, carerday_df, options_vector, wb_balance, quality_measure, max_time_seconds, random_seed) # new, for dfs
    print('Finshed.\nQuality: ' + str(inst.Cquality))

    end_time_program = time.perf_counter()
    elapsed_time = end_time_program - start_time_program
    print('Those were results for instance ' + str(inst.fname) + ' in ' + str(inst.area) + ', ' + str(inst.date))
    print('Total running time: ' + str(np.round(elapsed_time, 1)) + ' seconds.')
    
    # Create map of solution in website:
    if create_html_website:
        print('Generating website...')
        inst.full_solution_report(report=0, doPlots=create_python_plots)
        inst.solution_to_website_dst(add_plots=create_python_plots)
        inst.totalTravelCost = sum(inst.nurseTravelCost)
        inst.totalMileage = sum(inst.nurseMileage)
        inst.totalMileageCost = sum(inst.nurseMileageCost)
        inst.totalCost = inst.totalTravelCost + inst.totalMileageCost
        # print('totalTravelCost: ', inst.totalTravelCost)
        # print('totalMileageCost: ', inst.totalMileageCost)
        # print('totalCost: ', inst.totalCost)
        print('Done.') 

    # Put results in results_area_date.txt file:
    cwd = os.getcwd()
    results_filename = inst.fname + '_results.txt'
    outputfiles_path = os.path.join(cwd, 'output')
    resultsfile_path = os.path.join(outputfiles_path, results_filename)

    f = open(resultsfile_path, 'a')
    f.write('------------------------------------------------------------\n')
    f.write('Date: ' + str(datetime.now()) + '\n')
    f.write('Quality: ' + str(inst.Cquality) + '\n')
    f.write('Measure: ' + str(inst.quality_measure) + '\n')
    f.write('Carers: ' + str(inst.nNurses) + '\n')
    f.write('Jobs: ' + str(inst.nJobs) + '\n')
    f.write('Total Time: ' + str(inst.timemins_to_string(inst.totalTime)) + '\n')
    f.write('Total Service Time: ' + str(inst.timemins_to_string(inst.totalServiceTime)) + '\n')
    f.write('Total Travel Time: ' + str(inst.timemins_to_string(inst.totalTravelTime)) + '\n')
    f.write('Total Waiting Time: ' + str(inst.timemins_to_string(inst.totalWaitingTime)) + '\n')
    f.write('Total Tardiness: ' + str(inst.timemins_to_string(inst.totalTardiness)) + '\n')
    f.write('Total Overtime: ' + str(inst.timemins_to_string(inst.totalOvertime)) + '\n')
    f.write('Total Mileage: ' + str(inst.totalMileage) + '\n')
    f.write('Total Cost (£): ' + str(inst.totalCost) + '\n')
    f.write('Elapsed Time (s): ' + str(elapsed_time) + '\n')
    f.write('------------------------------------------------------------\n')
    f.close()
    print('Stored output values to', results_filename)

    # Output solution into client_df and save as csv:
    inst.solution_df_csv(client_df)

    print('End of program.')
### --- End def main --- ###

if __name__ == '__main__':
    repetitions = 1
    for i in range(repetitions):
        main()
### --- End __name__ ---###