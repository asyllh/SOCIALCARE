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
    area, tw_interval, wb_balance, quality_measure, max_time_seconds, create_html_website, create_python_plots, codepoint_directory, input_filename, date_selected = tui.get_user_input_variables()

    # Convert date_selected from datetime_date to Timestamp
    planning_date = pd.Timestamp(date_selected)
    
    # Obtain day of week index of planning_date for both 2-week and 8-week schedules
    week_period2 = 2
    week_period8 = 8
    dayindex_2weeks = wc.calculate_cycle_day(planning_date, week_period2)
    dayindex_8weeks = wc.calculate_cycle_day(planning_date, week_period8)

    random_seed = 13027 # Should random seed be added to tkinter box?

    # Create options vector
    options_vector = ihd.default_options_vector() 

    # Create dataframes for clients and carers using information from input excel file:
    client_df, carershift_df, carerday_df = gdd.get_info_create_dfs(area, tw_interval, planning_date, dayindex_2weeks, dayindex_8weeks, print_statements=False, filename=input_filename, foldername=codepoint_directory)
   
    # Start time of program
    start_time_program = time.perf_counter()

    # Main function of program: create the instance from the client and carer dataframes, and call the C program to run GRASP-VNS
    inst = ihd.create_solve_inst(client_df, carershift_df, carerday_df, planning_date, options_vector, wb_balance, quality_measure, max_time_seconds, random_seed)

    print('Finshed.\nQuality: ' + str(inst.Cquality))

    # End time of program
    end_time_program = time.perf_counter()
    elapsed_time = end_time_program - start_time_program

    # Fill jobObjs and carerObjs lists with solution data
    inst.full_solution_report(report=0, doPlots=create_python_plots)

    # Create map of solution in website
    if create_html_website:
        print('Generating website...')
        inst.solution_to_website_dst(add_plots=create_python_plots)
        inst.totalTravelCost = sum(inst.carerTravelCost)
        inst.totalMileage = sum(inst.carerMileage)
        inst.totalMileageCost = sum(inst.carerMileageCost)
        inst.totalCost = inst.totalTravelCost + inst.totalMileageCost   
    print('Done.') 

    # Put results in results_area_date.txt file
    cwd = os.getcwd()
    results_filename = inst.fname + '_results.txt'
    outputfiles_path = os.path.join(cwd, 'output')
    resultsfile_path = os.path.join(outputfiles_path, results_filename)

    f = open(resultsfile_path, 'a')
    f.write('------------------------------------------------------------\n')
    f.write('Date: ' + str(datetime.now()) + '\n')
    f.write('Quality: ' + str(inst.Cquality) + '\n')
    f.write('Measure: ' + str(inst.quality_measure) + '\n')
    f.write('Carers: ' + str(inst.nCarers) + '\n')
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

    # Output solution into client_df and save as csv
    inst.solution_df_csv(client_df)

    print('End of program.')
### --- End def main --- ###

if __name__ == '__main__':
    repetitions = 1
    for i in range(repetitions):
        main()
### --- End __name__ --- ###