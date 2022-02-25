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
    # area, max_time_seconds, tw_interval, wb_coeff, maxtravel_coeff, maxwaiting_coeff, quality_measure, create_html_website, create_python_plots, codepoint_directory, input_filename, date_selected = tui.get_user_input_variables()
    area = 'Hampshire'
    tw_interval = 15
    wb_balance = 1
    wb_coeff = -1
    maxtravel_coeff = -1
    maxwaiting_coeff = -1
    quality_measure = 'default' # default = paper
    max_time_seconds = 60
    create_html_website = False
    create_python_plots = False
    input_filename = r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/Carrot/data/abicare/2021.07.15 - UOS - Clients - Carers - Rota - Anonymised.xlsx'
    codepoint_directory = r'C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/Carrot/data/codepo_gb'
    date_selected = pd.Timestamp("2021-07-27")

    # Convert date_selected from datetime_date to Timestamp
    planning_date = pd.Timestamp(date_selected)
    # planning_date = date_selected
    
    # Obtain day of week index of planning_date for both 2-week and 8-week schedules
    week_period2 = 2
    week_period8 = 8
    dayindex_2weeks = wc.calculate_cycle_day(planning_date, week_period2)
    dayindex_8weeks = wc.calculate_cycle_day(planning_date, week_period8)

    big_m = 10000000
    random_seed = np.random.randint(low=0, high=big_m) # Should random seed be added to tkinter box?

    # Create options vector
    options_vector = ihd.default_options_vector() 


    root = Tk()
    root.title('Carrot DST')
    inputBox = Frame(root, width=400, height=230)
    inputBox.pack()

    message_1 = Label(inputBox, text='Collecting data from input file')
    message_1.configure(font=('Helvetica', 10))
    message_1.place(x=25, y=15)
    loop_active = True
    while loop_active:
        root.update()
        client_df, nurseshift_df, nurseday_df = gdd.get_info_create_dfs(area, tw_interval, planning_date, dayindex_2weeks, dayindex_8weeks, print_statements=False, filename=input_filename, foldername=codepoint_directory)
        loop_active = False


    # Create dataframes for clients and carers using information from input excel file:
    # client_df, nurseshift_df, nurseday_df = gdd.get_info_create_dfs(area, tw_interval, planning_date, dayindex_2weeks, dayindex_8weeks, print_statements=False, filename=input_filename, foldername=codepoint_directory)
   
    inst = None
    # Start time of program
    start_time_program = time.perf_counter()

    loop_active = True
    while loop_active:
        root.update()
        message_2 = Label(inputBox, text='Running algorithm to solve instance using Carrot DST')
        message_2.configure(font=('Helvetica', 10))
        message_2.place(x=25, y=40)
        root.update()
        inst = ihd.create_solve_inst(client_df, nurseshift_df, nurseday_df, planning_date, options_vector, wb_coeff, maxtravel_coeff, maxwaiting_coeff, quality_measure, max_time_seconds, random_seed)
        message_3 = Label(inputBox, text='Algorithm complete')
        message_3.configure(font=('Helvetica', 10))
        message_3.place(x=25, y=65)
        root.update()
        # Main function of program: create the instance from the client and carer dataframes, and call the C program to run GRASP-VNS
        # inst = ihd.create_solve_inst(client_df, nurseshift_df, nurseday_df, planning_date, options_vector, wb_coeff, maxtravel_coeff, maxwaiting_coeff, quality_measure, max_time_seconds, random_seed)

        # End time of program
        end_time_program = time.perf_counter()
        elapsed_time = end_time_program - start_time_program
        message_4 = Label(inputBox, text='Post-processing solution')
        message_4.configure(font=('Helvetica', 10))
        message_4.place(x=25, y=90)
        root.update()
        # Fill jobObjs and carerObjs lists with solution data
        inst.full_solution_report(report=0, doPlots=create_python_plots)

        # Create map of solution in website
        if create_html_website:
            message_5 = Label(inputBox, text='Generating HTML map for solution')
            message_5.configure(font=('Helvetica', 10))
            message_5.place(x=25, y=115)
            root.update()
            inst.solution_to_website_dst(add_plots=create_python_plots)
        else:
            message_5 = Label(inputBox, text='Skipping HTML map')
            message_5.configure(font=('Helvetica', 10))
            message_5.place(x=25, y=115)
            root.update()

        # Put results in results_area_date.txt file
        message_6 = Label(inputBox, text='Writing results to output file')
        message_6.configure(font=('Helvetica', 10))
        message_6.place(x=25, y=140)
        root.update()
        inst.output_results_file(elapsed_time)
        
        # Output solution into client_df and save as csv
        message_7 = Label(inputBox, text='Writing solution to output file')
        message_7.configure(font=('Helvetica', 10))
        message_7.place(x=25, y=165)
        root.update()
        inst.solution_df_csv(client_df)

        # End of program
        message_8 = Label(inputBox, text='End of program')
        message_8.configure(font=('Helvetica', 10))
        message_8.place(x=25, y=190)
        root.update()
        print('\nEnd of program.')
        time.sleep(4)
        loop_active = False
    
    

### --- End def main --- ###

if __name__ == '__main__':
    repetitions = 1
    for i in range(repetitions):
        main()
### --- End __name__ --- ###