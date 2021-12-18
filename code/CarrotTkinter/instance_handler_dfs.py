#-----------------------#
# CARROT - CARe ROuting Tool
# instance_handler_dfs.py
# main file containing functions to build and solve the instance of the problem
# 06/05/2021
#-----------------------#

import os
import osrm
import math
import time
import ctypes
import folium
import datetime
import requests # For osrm API
import subprocess
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from math import sqrt
from datetime import timedelta
from numpy.ctypeslib import ndpointer
from tkinter import *

# Display all rows and columns of dataframes in command prompt:
pd.set_option("display.max_rows", None, "display.max_columns", None)

def create_solve_inst(client_df, nurseshift_df, nurseday_df, planning_date, options_vector, wb_coeff, maxtravel_coeff, maxwaiting_coeff, quality_measure, max_time_seconds, random_seed):
    # root = Tk()
    # # root.title('User Input Parameters')
    # # inputBox = Frame(root, width=510, height=420)
    # # inputBox.pack()
    # inst = INSTANCE()

    # # ROOT = Tk()
    # LABEL = Label(root, text="Convert")
    # LABEL.pack()
    # LOOP_ACTIVE = True
    # while LOOP_ACTIVE:
    #     root.update()
    #     
    #     LOOP_ACTIVE = False

    # root.destroy()
    # Create instance of the INSTANCE class
    inst = INSTANCE()

    print('HERE')
   
    # Build/fill inst object
    inst = convert_dfs_inst(inst, client_df, nurseshift_df, nurseday_df, planning_date)
        
    inst.lambda_1 = 1
    inst.lambda_2 = 1
    inst.lambda_3 = 1
    inst.lambda_4 = 1
    inst.lambda_5 = 1
    inst.lambda_6 = 10
    inst.quality_measure = quality_measure
    inst.MAX_TIME_SECONDS = max_time_seconds
    options_vector[55] = wb_coeff # alpha_5 Workload balance (from user input)
    options_vector[60] = wb_coeff # alpha_5 Workload balance (from user input)
    options_vector[58] = maxtravel_coeff # alpha_8 max travel (from user input)
    options_vector[59] = maxwaiting_coeff # alpha_9 max waiting (from user input)

    # Create instances of JOB and NURSE classes for inst
    inst.instantiate_job_nurse_objects(client_df, nurseday_df)

    inst.algorithmOptions = options_vector
    if quality_measure == 'default':
        inst.algorithmOptions[0] = 6.0 # paper (default)
    elif quality_measure == 'ait h':
        inst.algorithmOptions[0] = 0.0 # ait_h
    elif quality_measure == 'mk':
        inst.algorithmOptions[0] = 1.0 # mankowska
    elif quality_measure == 'wb':
        inst.algorithmOptions[0] = 5.0 # workload_balance
    else:
        print('[ERROR]: (instance_handler.py) incorrect quality measure provided.')
        print('Quality measure must be one of the following: default, ait h, mk, or wb.')
        exit(-1)
    
    saved_od_data = inst.od[0][0]
    LOOP_ACTIVE = True
    while LOOP_ACTIVE:
        LABEL.configure(text='solve')
        root.update()
        inst.solve(randomSeed=random_seed, printAllCallData=False)
        LOOP_ACTIVE = False
    # root.destroy()

    exit(-1)

    inst.solve(randomSeed=random_seed, printAllCallData=False)
    inst.solve = []
    inst.lib = []
    inst.fun = []
    inst.Cquality = -1*inst.od[0][0] # Cquality = solution quality from C program, it's stored in od[0][0] (shortcut, should probably change this!)
    inst.od[0][0] = saved_od_data

    print('\n------------------------\n\nC program finished - back in Python.\nQuality: ' + str(inst.Cquality))

    return inst
### --- End def create_solve_inst --- ###

def convert_dfs_inst(inst, client_df, nurseshift_df, nurseday_df, planning_date):
  
    inst.name = str(client_df.loc[0]['area']) + '_' + str(pd.Timestamp.date(planning_date)) # name is 'inst_area_date'
    inst.fname = inst.name # Note: Need to change
    inst.area = client_df.loc[0]['area']
    inst.date = pd.Timestamp.date(planning_date)
    inst.nNurses = len(nurseday_df)
    inst.nJobs = len(client_df)
    inst.nShifts = len(nurseshift_df)
    inst.nSkills = 5 # NOTE: this is a random number just for testing.
    cwd = os.getcwd()

    # Create folder called 'output', if such a folder does not already exist.
    outputfiles_path = os.path.join(cwd, 'output')
    if not os.path.exists(outputfiles_path):
        os.mkdir(outputfiles_path)
    
    # Create and fill arrays with data from the instance

    # nurseWorkingTimes is nNurses x 5, col[0] = start time, col[1] = finish time, col[2] = total day length, col[3] = number of shifts, col[4] = duration of shifts (i.e. col[4] = col[2] - unavailable time).
    inst.nurseWorkingTimes = np.zeros((inst.nNurses, 5), dtype=np.int32) # Changed from 3 to 5 on 04/11/2021
    for i in range(inst.nNurses):
        inst.nurseWorkingTimes[i][0] = nurseday_df.iloc[i]['start']
        inst.nurseWorkingTimes[i][1] = nurseday_df.iloc[i]['end']
        inst.nurseWorkingTimes[i][2] = nurseday_df.iloc[i]['duration']
        inst.nurseWorkingTimes[i][3] = 0 # NEW: 04/11/2021
        inst.nurseWorkingTimes[i][4] = 0 # NEW: 04/11/2021
    
    # jobTimeInfo is nJobs x 3, col[0] = startTW, col[1] = endTW, col[2] = length of job (time)
    inst.jobTimeInfo = np.zeros((inst.nJobs, 3), dtype=np.int32) 
    for i in range(inst.nJobs):
        inst.jobTimeInfo[i][0] = client_df.iloc[i]['tw_start']
        inst.jobTimeInfo[i][1] = client_df.iloc[i]['tw_end']
        inst.jobTimeInfo[i][2] = client_df.iloc[i]['duration']

    
    inst.doubleService = np.zeros(inst.nJobs, dtype=np.int32)
    for i in range(inst.nJobs):
        if client_df.iloc[i]['num_nurses'] > 1:
            inst.doubleService[i] = 1
    nDS = np.sum(inst.doubleService) # Number of jobs that are double services

    # print('Double Service:\n')
    # for i in range(inst.nJobs):
    #     print(inst.doubleService[i], end=' ')
    # print('\n')

    # sumDurations = 0
    # for i in range(len(client_df)):
    #     sumDurations += client_df.iloc[i]['duration']
    # print('sumDurations: ', sumDurations)

    # exit(-1)
    

    inst.od = np.zeros((inst.nJobs+1, inst.nJobs+1), dtype=np.float64) # Time in minutes
    osrm_table_request(inst, client_df, nurseday_df, 'od')

    # inst.travelCostMatrix = np.zeros((inst.nJobs, inst.nJobs), dtype=np.float64) # NEW, OD_COST, 11/06/2021, take time in minutes and convert to cost.
    # calculate_travel_cost_matrix(inst)
    
    inst.nurse_travel_from_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From nurse's home to job - TIME IN MINS, THIS WILL BE USED IN C
    osrm_table_request(inst, client_df, nurseday_df, 'nursefrom')

    inst.nurse_travel_to_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From job to nurse's home - TIME IN MINS, THIS WILL BE USED IN C
    osrm_table_request(inst, client_df, nurseday_df, 'nurseto')
       
    inst.nurseSkills = np.ones((inst.nNurses, inst.nSkills), dtype=np.int32) # Note: this will only work if nSkills > 0.
    inst.jobSkillsRequired = np.ones((inst.nJobs, inst.nSkills), dtype=np.int32) # Note: this will only work if nSkills > 0.
    inst.prefScore = np.zeros((inst.nJobs, inst.nNurses), dtype=np.float64)
    inst.dependsOn = np.full(inst.nJobs, -1, dtype=np.int32) # Set all jobs to -1, means no jobs are dependent on one another.
    inst.algorithmOptions = np.zeros(100, dtype=np.float64)
    # NOTE: what happens if nDS == 0, i.e. there are no double services? Cannot have third dimension of capabilityofDS matrix be zero, need to make it one.
    inst.capabilityOfDoubleServices = np.ones((inst.nNurses, inst.nNurses, nDS), dtype=np.int32) # NOTE: just for testing, setting it to all ones so that every pair of nurses is capable of doing the double service.
    inst.mk_mind = np.zeros(inst.nJobs+1, dtype=np.int32) #nJobs+1 because that's what is taken in by C, mk_mind[i] = mk_mind_data[i+1]
    inst.mk_maxd = np.zeros(inst.nJobs+1, dtype=np.int32)
    inst.solMatrix = np.zeros((inst.nNurses, inst.nJobs), dtype=np.int32)
    inst.timeMatrix = np.full((inst.nNurses, inst.nJobs), -1, dtype=np.float64) # NEW 03/06/2021
    inst.nurseWaitingTime = np.zeros(inst.nNurses, dtype=np.float64) # NEW 03/06/2021
    inst.nurseTravelTime = np.zeros(inst.nNurses, dtype=np.float64) # NEW 03/06/2021
    inst.violatedTW = np.zeros(inst.nJobs, dtype=np.float64) # NEW 03/06/2021
    inst.nurseWaitingMatrix = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # NEW 03/06/2021
    inst.nurseTravelMatrix = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # NEW 03/06/2021
    inst.totalsArray = np.zeros(23, dtype=np.float64) # NEW 04/06/2021 - changed from 19 to 23 19/11/2021

    # Matrix of size 50 by 4 by nNurses, where number of rows = number of shifts, col[0] = shift number, col[1] = start time, col[2]= end time, col[3] = duration of shift, and third dimension is number of nurses.
    # i.e each 2d matrix is 50x4, and there are nNurses lots of 2d matrices to form a 3d matrix.
    numNurses = 0
    numShifts = 0
    shift_count = 0
    inst.shift_matrix = np.full((50, 4, inst.nNurses), -1, dtype=np.int32)

    # Start with first nurse 0
    prevNurse = nurseshift_df.iloc[0]['nurse_id']
    inst.shift_matrix[shift_count][0][numNurses] = numShifts
    inst.shift_matrix[shift_count][1][numNurses] = nurseshift_df.iloc[0]['start']
    inst.shift_matrix[shift_count][2][numNurses] = nurseshift_df.iloc[0]['end']
    inst.shift_matrix[shift_count][3][numNurses] = nurseshift_df.iloc[0]['duration']

    # Fill shift_matrix for the rest of the nurses
    for i in range(1, len(nurseshift_df)):
        currentNurse = nurseshift_df.iloc[i]['nurse_id']
        if prevNurse == currentNurse: # If currentNurse is the same as the previousNurse, then the nurse has another shift.
            shift_count += 1
            numShifts += 1
            inst.shift_matrix[shift_count][0][numNurses] = numShifts
            inst.shift_matrix[shift_count][1][numNurses] = nurseshift_df.iloc[i]['start']
            inst.shift_matrix[shift_count][2][numNurses] = nurseshift_df.iloc[i]['end']
            inst.shift_matrix[shift_count][3][numNurses] = nurseshift_df.iloc[i]['duration']
        else: # Otherwise currentNurse is a new nurse, so add this shift detail to the next matrix in the 3d matrix
            shift_count = 0
            numNurses += 1
            numShifts = 0
            inst.shift_matrix[shift_count][0][numNurses] = numShifts
            inst.shift_matrix[shift_count][1][numNurses] = nurseshift_df.iloc[i]['start']
            inst.shift_matrix[shift_count][2][numNurses] = nurseshift_df.iloc[i]['end']
            inst.shift_matrix[shift_count][3][numNurses] = nurseshift_df.iloc[i]['duration']
            prevNurse = currentNurse

    # Create unavailability matrix, same format as shift_matrix
    inst.unavail_matrix = np.full((50,4,inst.nNurses), -1, dtype=np.int32)
    inst.nurse_unavail = np.zeros(inst.nNurses, dtype=np.int32) # Array that hold the number of unavailble shifts for each nurse.

    for k in range(inst.nNurses): # For each nurse
        num_unavail = 0 # Set the number of unavailable shifts to 0 for that nurse
        if inst.shift_matrix[1][0][k] < 0: # If the nurse k does not have a second shift, then the nurse only have one shift (which is the whole day) and so is not unavailable at all
            inst.nurse_unavail[k] = 0 # The number of unavailable shifts for this nurse is 0
            continue
        elif inst.shift_matrix[1][0][k] > 0: # If the nurse k does have a second shift, then the nurse has at least one unavailble shift throughout the day
            i = 1
            while inst.shift_matrix[i][0][k] > i-1: 
                start_unavail = inst.shift_matrix[i-1][2][k] # start_unavail is the end of the previous shift
                end_unavail = inst.shift_matrix[i][1][k] # end unavail is the start of the next shift
                duration_unavail = end_unavail - start_unavail
                inst.unavail_matrix[i-1][0][k] = num_unavail
                inst.unavail_matrix[i-1][1][k] = start_unavail
                inst.unavail_matrix[i-1][2][k] = end_unavail
                inst.unavail_matrix[i-1][3][k] = duration_unavail
                i += 1
                num_unavail +=1
                inst.nurse_unavail[k] += 1 # update the number of unavailable shifts for this nurse k.
    # End for loop

    # NEW: 04/11/2021 - Update nurseWorkingTimes matrix for the extra columns: col[3] = number of shifts during the day for that nurse, and col[4] = duration of actual working time on shift for the nurse
    # That is, col[4] = col[2] (total shift length) - unavailable time (if the nurse has any). (04/11/2021)
    for i in range(inst.nNurses):
        num_avail_shifts = inst.nurse_unavail[i] + 1 # number of available shifts for nurse i is number of unavailable shifts + 1
        inst.nurseWorkingTimes[i][3] = num_avail_shifts # set number of available shifts for nurse i in cWT
        if num_avail_shifts == 1: # if there is only one shift, then nurse works only a single block, there are no separate shifts/unavailable times
            inst.nurseWorkingTimes[i][4] = inst.nurseWorkingTimes[i][2] # Set duration of shift lengths for nurse i as duration of whole day, as there are no unavailable times.
        elif num_avail_shifts > 1: # If nurse i has more than one shift, there is unavailable time between, so we need to calculate the duration of ONLY the working shift times
            sum_durations = 0
            for j in range(num_avail_shifts-1): # go through shift_matrix 3d matrix and sum up duration times for the shifts of nurse i
                sum_durations += inst.shift_matrix[j][3][i] # j = shift number, from 0 all the way up to num_unavail_shifts-1, 3=column that contains duration of each shift, i = nurse i
            inst.nurseWorkingTimes[i][4] = sum_durations
        # End if
    # End for

    #PRINT OUT NURSE WORKING TIMES:
    # print("Carer working times:\n")
    # for i in range(inst.nNurses):
    #     for j in range(5):
    #         print(inst.nurseWorkingTimes[i][j])
    #     print("\n")
    # print("end")
    # exit(-1)

    
    return inst
### --- End of def convert_dict_inst --- ###

def osrm_table_request(inst, client_df, nurseday_df, matrix='none'):
    # For osrm, the coordinates need to be in string form, and in the order 'longitude,latitude'. Our data is in 'lat,lon', so create_coord_string function swaps it around.

    nNurses = inst.nNurses
    nJobs = inst.nJobs
    area = inst.area

    lonlat_jobs = []
    for i in range(nJobs):
        lonlat = [client_df.iloc[i]['longitude'], client_df.iloc[i]['latitude']] #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        lonlat_jobs.append(lonlat)
    lljSize = len(lonlat_jobs)
        
    lonlat_nurses = []
    for i in range(nNurses):
        lonlat = [nurseday_df.iloc[i]['longitude'], nurseday_df.iloc[i]['latitude']] #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        lonlat_nurses.append(lonlat)
    llnSize = len(lonlat_nurses)

    server = r'localhost:5000'
    str_call = r'http://' + server + '/table/v1/driving/'

    # --- MATRIX TYPES --- #

    if matrix == 'od': # Only use osrm for od matrix        
        coord_strings_list = []
        for i in range(lljSize):
            coordstr = create_lonlat_string(lonlat_jobs[i])
            coord_strings_list.append(coordstr)
        coord_strings = ';'.join(coord_strings_list)
        
        # Get results from osrm:
        str_call = str_call + coord_strings
        r = requests.get(str_call)
        osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.
        
        # Fill od matrix:
        durations = osrmdict['durations']
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.od[i+1][j+1] = time_mins
    # --- End od matrix --- #

    elif matrix == 'nursefrom':
        # Going FROM depot, so time from each NURSE home to each JOB (source = nurse, destination = job)

        # Nurses first (sources):
        count = 0 # Used to count the number of sources and destinations, as the index of each [lon,lat] in the address needs to be added to the end of the address.
        source_list = [] # Indices for source long/lats
        coord_strings_list_nurses = [] # List of longs/lats as strings
        for i in range(llnSize):
            coordstrnurse = create_lonlat_string(lonlat_nurses[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_nurses.append(coordstrnurse) # Add the long/lat string to list
            source_list.append(str(i)) # Add index of long/lat pair to list
            count +=1
        coord_strings_nurses = ';'.join(coord_strings_list_nurses) # coord_strings_nurses = single string comprising all strings in coord_strings_list_nurses joined with semicolons in between
        source_string = ';'.join(source_list) # Join the list of strings together into a single string with semicolon between each string

        # Jobs second (destinations):
        destination_list = [] # Indices for destination long/lats
        coord_strings_list_jobs = [] # List of longs/lats as strings
        for i in range(lljSize):
            coordstrjob = create_lonlat_string(lonlat_jobs[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_jobs.append(coordstrjob) # Add the long/lat string to list
            destination_list.append(str(count)) # Add index of long/lat pair to list
            count += 1
        coord_strings_jobs = ';'.join(coord_strings_list_jobs) # coord_strings_jobs = single string comprising all strings in coord_strings_list_jobs joined with semicolons in between
        destination_string = ';'.join(destination_list) # Join the list of strings together into a single string with semicolon between each string

        # Get results from osrm:
        sources = '?sources=' + source_string # Create sources string
        destinations = '&destinations=' + destination_string # Create destinations string
        str_call = str_call + coord_strings_nurses + ';' + coord_strings_jobs + sources + destinations # Create full address
        r = requests.get(str_call) # Get results from osrm
        osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.

        # Fill nurse_travel_from_depot matrix:
        durations = osrmdict['durations']
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.nurse_travel_from_depot[i][j] = time_mins
    # --- End nurse from matrix --- #

    elif matrix == 'nurseto':
        # Going TO depot, so time from each JOB to each NURSE home (source = job, destination = nurse)

        # Jobs first (sources):
        count = 0 # Used to count the number of sources and destinations, as the index of each [lon,lat] in the address needs to be added to the end of the address.
        source_list = [] # Indices for source long/lats
        coord_strings_list_jobs = [] # List of longs/lats as strings
        for i in range(lljSize):
            coordstrjob = create_lonlat_string(lonlat_jobs[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_jobs.append(coordstrjob) # Add the long/lat string to list
            source_list.append(str(i)) # Add index of long/lat pair to list
            count +=1
        coord_strings_jobs = ';'.join(coord_strings_list_jobs) # coord_strings_jobs = single string comprising all strings in coord_strings_list_jobs joined with semicolons in between
        source_string = ';'.join(source_list) # Join the list of strings together into a single string with semicolon between each string

        # Nurses second (destinations):
        destination_list = [] # Indices for destination long/lats
        coord_strings_list_nurses = [] # List of longs/lats as strings
        for i in range(llnSize):
            coordstrnurse = create_lonlat_string(lonlat_nurses[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_nurses.append(coordstrnurse) # Add the long/lat string to list
            destination_list.append(str(count)) # Add index of long/lat pair to list
            count += 1
        coord_strings_nurses = ';'.join(coord_strings_list_nurses) # coord_strings_nurses = single string comprising all strings in coord_strings_list_nurses joined with semicolons in between
        destination_string = ';'.join(destination_list) # Join the list of strings together into a single string with semicolon between each string

        # Get results from osrm:
        sources = '?sources=' + source_string
        destinations = '&destinations=' + destination_string
        str_call = str_call + coord_strings_jobs + ';' + coord_strings_nurses + sources + destinations
        r = requests.get(str_call)
        osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.

        # Fill nurse_travel_to_depot matrix:
        durations = osrmdict['durations']
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.nurse_travel_to_depot[j][i] = time_mins
    # --- End nurse to matrix --- #
    else:
        print('[ERROR]: no matrix type specified for osrm_table_request.')
        print('Matrix type should be either \'od\', \'nursefrom\', or \'nurseto\'.')
        print('Terminating program.')
        exit(-1)
    # --- End else --- #
### --- End of def osrm__table_request --- ### 

def calculate_travel_cost_matrix(inst):
    # Take od matrix and use to convert time in minutes to time in hours, then multiply by travel cost and put into travelCostMatrix
    # Note that od is nJobs+1 x nJobs+1, but travelCostMatrix is only nJobs x nJobs
    # If trip is less than 25 minutes, then cost is £8.36/hour, else if trip is >= 25 minutes, then cost is £8.91/hour

    cost = 0
    travelTime = 0
    travelTimeHours = 0

    for i in range(inst.nJobs):
        for j in range(inst.nJobs):
            travelTime = inst.od[i+1][j+1]
            travelTimeHours = travelTime / 60
            if travelTime < 25 and travelTime >= 0:
                cost = travelTimeHours * 8.36
                inst.travelCostMatrix[i][j] = cost
            elif travelTime >= 25:
                cost = travelTimeHours * 8.91
                inst.travelCostMatrix[i][j] = cost
            else:
                print('[ERROR]: Incorrect value for travelTime when attempting to calculate travel cost.')
                print('travelTime = ', travelTime)
                print('Terminating program.')
                exit(-1)
### --- End of def calculate travel_cost_matrix --- ###

class GUI(object):
    def __init__(self):
        self.root = Tk()

        self.inputBox = Frame(self.root, width=510, height=420)
        self.inputBox.pack()

        title_label = Label(self.inputBox, text='User Input Parameters')
        title_label.configure(font=('Helvetica', 12))
        title_label.place(x=25, y=15)

    def start_thread(self):
        self.b_start['state'] = 'disable'
        # self.work_thread = threading.Thread(target=work)
        # self.work_thread.start()
        # self.root.after(50, self.check_thread)
        # self.root.after(50, self.update)

    def check_thread(self):
        print('do nothing')
        # self.root.destroy()        
### --- End Class GUI --- ###

class GEOPOINT(object):
    def __init__(self, latitude, longitude):
        self.lat = latitude
        self.long = longitude
    def latlong(self):
        return([self.lat, self.long])
    def longlat(self):
        return([self.long, self.lat])
### --- End class GEOPOINT  --- ###

class JOB(object):
    def __init__(self):
        # Characteristics
        self.ID = ''
        self.serviceTime = 0
        self.doubleService = 0
        self.dependsOn = []
        self.postcode = 'Unknown'
        # self.latlon = []
        # self.priority = 0
        self.hasTimewindow = False
        self.timewindow = [0,86400]
        self.hasPreferredTimewindow = False
        self.preferredTimewindow = [0, 86400]
        self.skillsRequired = []
        self.features = []
        self.preferences = []
        self.preferredNurses = []
        self.dependsOn = -1
        self.minimumGap = 0
        self.maximumGap = 0
        # self.location = [] # NOTE: NEW, 27/12/2020 ALH
        # Calculated (solution)
        self.assignedNurse = 0
        self.nurseID = 'Unknown'
        self.arrivalTime = 0 # time nurse arrives at job location
        self.startTime = 0 # NEW: 07/06/2021, actual time job starts
        self.departureTime = 0
        self.waitingToStart = 0
        self.tardiness = 0
        self.positionInSchedule = 0
        self.travelToJob = 0 # NEW: 09/06/2021, the travel time from the previous job/depot to this job
        self.markedReport = 0 # use this to determine for DS jobs if they have already been assessed in full_solution_report, if they haven't (=0) then need to create list, else if they have been assessed (=1) then need to append to the list.
        self.markedWebsite = 0 # use this to determine for DS jobs if they have already been assessed in solution_to_website, if they haven't (=0) then need to create list, else if they have been assessed (=1) then need to append to the list.
### --- End class JOB --- ###        

class NURSE(object):
    def __init__(self):
        # Characteristics
        self.ID = ''
        self.startLocation = []
        self.transportMode = 'car'
        self.shiftTimes = [0, 86400]
        self.maxWorking = 86400
        self.skills = []
        self.features = []
        self.preferences = []
        self.preferredJobs = []
        self.jobsToAvoid = []
        self.postcode = 'Unknown' # NOTE: NEW, 27/12/2020 ALH
        # Calculated (solution)
        self.jobsServed = 0
        self.startTime = 0
        self.finishTime = 0
        self.waitingTime = 0
        self.serviceTime = 0
        self.overtimeWork = 0
        self.travelTime = 0
        self.route = []
### --- End class NURSE --- ###                       

class INSTANCE(object):
    def __init__(self):
        self.nJobs = -1
        self.nNurses = -1
        self.nSkills = -1
        self.nShifts = -1
        self.nurseWorkingTimes = []
        self.nurseSkills = []
        self.jobTimeInfo = []
        self.jobSkillsRequired = []
        self.prefScore = []
        self.algorithmOptions = np.zeros(100, dtype=np.float64)
        self.doubleService = []
        self.dependsOn = []
        self.od = [] # Main od matrix, contains times in minutes - this will be used in C
        self.od_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.nurse_travel_from_depot = [] # Main nurse_travel_from_depot matrix, contains times in minutes - this will be used in C
        self.nurse_travel_to_depot = [] # Main nurse_travel_to_depot matrix, contains times in minutes - this will be used in C
        self.shift_matrix = [] # NOTE: NEW 22/05/2021, used to hold shift information for nurses 3D matrix
        self.unavail_matrix = [] # NOTE: NEW 22/05/2021, used to hold unavailable shift information for nurses 3D matrix
        self.nurse_unavail = [] # NOTE: NEW 22/05/2021, used to hold number of unavailable shifts for each nurse (1d array)
        self.nurse_travel_from_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.nurse_travel_to_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.xy = [] # x y coordinates for plotting routes on a map LAT-LONG
        self.solMatrix = []
        self.MAX_TIME_SECONDS = 30
        self.verbose = 1 # was 5, changed to 1 on 11/06/2021
        self.secondsPerTU = 1
        self.tw_interval = 15
        self.exclude_nurse_travel = True
        self.name = ''
        self.fname = ''
        self.area = ''
        self.date = ''
        self.full_file_name = 'solution0'
        self.loadFromDisk = False
        self.mankowskaQuality = -1
        self.Cquality = -1
        self.DSSkillType = 'strictly-shared' # See overleaf document for details, was shared-duplicate default, now changed to strictly-shared on 11/06/2021
        self.capabilityOfDoubleServices = []
        self.timeMatrix = [] # NOTE: NEW (03/06/2021)
        self.nurseWaitingTime = [] # MOVED FROM POST PROCESSED 03/06/2021
        self.nurseTravelTime = [] # MOVED FROM POST PROCESSED 03/06/2021
        self.violatedTW = [] # NEW 03/06/2021
        self.nurseWaitingMatrix = [] # NEW 03/06/2021
        self.nurseTravelMatrix = [] # NEW 03/06/2021
        # self.travelCostMatrix = [] # NEW 11/06/2021, cost of each trip for each pair of jobs, nJobs x nJobs, use od matrix, cost in pounds. (could also be called od_cost!)
        # self.odMileage = [] # NEW 11/06/2021, nJobs x nJobs, od matrix but with mileage instead od time in minutes
        # self.odMileageCost = [] # NEW 11/06/2021, nJobs x nJobs, cost of mileage for each trip, so x 0.25p per mile
        self.totalsArray = [] # NEW: 04/06/2021, keeps (in order): totalTime, totalWaitingTime, totalTravelTime, totalServiceTime, totalTardiness, maxTardiness, totalMKTardiness, mk_allowed_tardiness,
        # totalOvertime, maxOvertime, minSpareTime, maxSpareTime, shortestDay, longestDay, ait_quality, mk_quality, wb_quality, paper_quality, quality.

        # Post-processed solution:
        self.nurseRoute = [] # In post_process_solution, this will become equivalent to allNurseRoutes, with dimensions nNurses x nPositions(jobs), and so nurseRoute[nurse][position] = job.
        self.nurseServiceTime = []
        self.nurseTravelCost = [] # NEW 11/06/2021, total cost of all trips for that nurse, 1 x nNurses
        # self.nurseMileageMatrix = [] # NEW 11/06/2021, mileage of each trip for each nurse to each job (make sure to exclude to/from depot), nNurses x nJobs
        self.nurseMileage = [] # NEW 11/06/2021, total mileage travelled by each nurse, 1 x nNurses
        self.nurseMileageCost = [] # NEW 11/06/2021, total cost of mileage by each nurse, 1 x nNurses, multiply nurseMileage by 0.25p per mile to get cost in pounds.
        self.nurseTime = []
        self.totalWaitingTime = -1
        self.totalServiceTime = -1
        self.totalTravelTime = -1
        self.totalTardiness = -1
        self.totalOvertime = -1
        self.maxOvertime = -1
        self.minSpareTime = -1
        self.maxSpareTime = -1
        self.totalTime = -1
        self.maxTardiness = -1
        self.shortestDay = -1
        self.longestDay = -1
        self.aitHQuality = -1
        self.MKQuality = -1
        self.WBQuality = -1
        self.paperQuality = -1
        self.totalPref = -1
        self.maxTravelTime = -1
        self.maxWaitingTime = -1
        self.maxDiffWorkload = -1
        self.nLateJobs = 0
        self.quality_measure = 'standard'
        self.c_quality = 0
        self.mk_mind = []
        self.mk_maxd = []
        self.totalDistance = 0
        self.totalDistanceJobsOnly = 0
        self.distNurses = []
        self.distNursesJobsOnly = []
        self.totalTravelCost = 0 # NEW 11/06/2021
        self.totalMileage = 0 # NEW 11/06/2021
        self.totalMileageCost = 0 # NEW 11/06/2021
        self.totalCost = 0 # NEW 11/06/2021

        # Preferences weights and parameters:
        self.M = 1000000000 # Large value for unskilled jobs
        self.lambda_1 = 1
        self.lambda_2 = 1
        self.lambda_3 = 1
        self.lambda_4 = 1
        self.lambda_5 = 1
        self.lambda_6 = 1

        ### Define call to C ###
        if os.name == "nt":
            library_name = "./bin/constructive.dll"
            # library_name = "./bin/bnc_solver.dll"
            self.lib = ctypes.cdll.LoadLibrary(library_name)
        else:
            library_name = "./constructivelib.so"
            self.lib = ctypes.CDLL(library_name, mode=ctypes.RTLD_GLOBAL)

        self.fun = self.lib.python_entry
        self.fun.restype = None
        # self.fun.argtypes = [ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), ctypes.c_size_t]
        self.fun.argtypes = [ctypes.c_int, # nJobs_data,
                        ctypes.c_int, # nNurses_data,
                        ctypes.c_int, # nSkills_data,
                        ctypes.c_int, # verbose_data,
                        ctypes.c_float, # MAX_TIME_SECONDS
                        ctypes.c_int, # tw_interval_data
                        ctypes.c_bool, #exclude_nurse_travel_data
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * od_data
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * nurse_travel_from_depot
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * nurse_travel_to_depot
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # unavail_matrix
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # nurse_unavail
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # nurseWorkingTimes_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # jobTimeInfo_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # jobRequirements_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # nurseSkills_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# solMatrixPointer
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# doubleService
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# dependsOn
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# mk_mind
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# mk_maxd
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# capabilityOfDoubleServices
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),# prefScore
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),# algorithmOptions
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # timeMatrix, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # nurseWaitingTime, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # nurseTravelTime, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # violatedTW, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # nurseWaitingMatrix, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # nurseTravelMatrix, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # totalsArray, NEW 04/06/2021
                        ctypes.c_int] # Random seed
    ### --- End def  __init__ --- ### 

    def instantiate_job_nurse_objects(self, client_df, nurseday_df):
        # Create two lists containing objects of the classes JOB and NURSE respectively.
        self.jobObjs = []
        for ii in range(self.nJobs):
            self.jobObjs.append(JOB())
            self.jobObjs[-1].ID = ii # Set the ID for the object just added to jobObjs to be the index of the object (its position in the jobObjs list).

        self.nurseObjs = []
        for ii in range(self.nNurses):
            self.nurseObjs.append(NURSE())
            self.nurseObjs[-1].ID = ii # Set the ID for the object just added to nurseObjs to be the index of the object (its position in the nurseObjs list).
        
        # 1. Jobs:
        for j in range(self.nJobs):
            self.jobObjs[j].ID = client_df.iloc[j]['client_id']
            self.jobObjs[j].postcode = client_df.iloc[j]['postcode']
            latlon = [client_df.iloc[j]['latitude'], client_df.iloc[j]['longitude']]
            self.jobObjs[j].location = GEOPOINT(float(latlon[0]), float(latlon[1]))
            self.jobObjs[j].serviceTime = client_df.iloc[j]['duration']
            self.jobObjs[j].hasTimewindow = True
            jtw_start = client_df.iloc[j]['tw_start']
            jtw_end = client_df.iloc[j]['tw_end']
            self.jobObjs[j].timewindow = [jtw_start, jtw_end]
            self.jobObjs[j].hasPreferredTimewindow = False
            self.jobObjs[j].preferredTimewindow = [0, 24*3600]
            self.jobObjs[j].skillsRequired = []
            self.jobObjs[j].doubleService = self.doubleService[j]
            self.jobObjs[j].features = []
            self.jobObjs[j].preferences = []
            self.jobObjs[j].preferredNurses = []
            self.jobObjs[j].dependsOn = -1
            self.jobObjs[j].minimumGap = []
            self.jobObjs[j].maximumGap = []
            self.jobObjs[j].markedReport = 0
            self.jobObjs[j].markedWebsite = 0
        # End jobs

        # 2. Carers:
        for i in range(self.nNurses):
            self.nurseObjs[i].ID = nurseday_df.iloc[i]['nurse_id'] # NOTE: this could also be 'nurse' instead of unique_id, but unique_id has the shift number
            self.nurseObjs[i].postcode = nurseday_df.iloc[i]['postcode']
            latlon = [nurseday_df.iloc[i]['latitude'], nurseday_df.iloc[i]['longitude']]
            self.nurseObjs[i].startLocation = GEOPOINT(float(latlon[0]), float(latlon[1]))
            self.nurseObjs[i].transportMode = 'car'
            istart = nurseday_df.iloc[i]['start']
            ifinish = nurseday_df.iloc[i]['end']
            self.nurseObjs[i].shiftTimes = [float(istart), float(ifinish)]
            self.nurseObjs[i].maxWorking = float(nurseday_df.iloc[i]['duration'])
            self.nurseObjs[i].skills = []
            self.nurseObjs[i].features = []
            self.nurseObjs[i].preferences = []
            self.nurseObjs[i].preferredJobs = []
            self.nurseObjs[i].jobsToAvoid = []
        # End carers

        # Locations:
        self.xy = []
        self.xy.append(self.nurseObjs[0].startLocation.longlat()) # why only the first nurse [0] location? Note that coords appended are lon/lat, not lat/lon!
        for job in self.jobObjs:
            self.xy.append(job.location.longlat()) # Append all job coordinates - lon/lat, not lat/lon!
    ### --- End def instantiate_job_nurse_objects --- ###

    def fill_preferences(self):
        self.prefScore = np.zeros((self.nJobs, self.nNurses), dtype=np.float64) # Matrix, nJobs x nNurses (note that this is the only variable with jobxnurse, not nursexjob dimensions).
        for i,job in enumerate(self.jobObjs): # For each JOB object in the jobObjs list
            for j,nurse in enumerate(self.nurseObjs): # For each NURSE object in the nurseObjs list
                self.prefScore[i][j] = self.preference_score(job, nurse) # Set the preference score of job i and nurse j
    ### --- End def fill_preferences --- ###

    def preference_score(self, job, nurse):
        # Function determines preference score of given job and given nurse, where 'job' is a JOB object and 'nurse' is a NURSE object.
        nurseOK = 0
        if len(job.preferredNurses) > 0: # If the job does have preferred carers
            nurseOK = -1 # Penalise if there is someone on the list, but it's not met
            for pc in job.preferredNurses: # For each 'value' (string?) pc in preferredNurses list
                if pc.lower() == nurse.ID.lower(): # If the 'pc' index matched the ID of the nurse (nurse called in as parameter), then it means that this given job would prefer to have this nurse.
                    nurseOK = 1 # nurse is okay for this job!
                    break

        
        matchedPrefsJob = 0
        unmatchedPrefsJob = 0 # NOTE: Always zero??
        for pref in job.preferences: # For each value (job preference characteristic?) in job.preferences list
            if pref in nurse.features: # If the preference characteristic is one of the nurse's features, i.e. nurse has the desired characteristic
                matchedPrefsJob = matchedPrefsJob + 1 # Increase preference score for job
            else:
                matchedPrefsJob = matchedPrefsJob - 1 # Decrease preference score for job

        matchedPrefsNurse = 0
        unmatchedPrefsNurse = 0 # NOTE: Always zero??
        for pref in nurse.preferences: # For each value (nurse preference characteristic?) in nurse.preferences list
            if pref in job.features: # If the preference characteristic is one of the job's features, i.e. job has the desired characteristic
                matchedPrefsNurse = matchedPrefsNurse + 1 # Increase preference score for nurse
            else:
                matchedPrefsNurse = matchedPrefsNurse - 1 # Decrease preference score for nurse

        jobToAvoid = 0
        for jta in nurse.jobsToAvoid: # For each value (job ID?) in nurse.jobsToAvoid list
            if jta.lower() == job.ID.lower(): # If the ID of the current job is in the jobsToAvoid list, then jobToAvoid = -1
                jobToAvoid = -1
                break

        # Calculate preference score
        score = (
                self.lambda_1*nurseOK +
                self.lambda_2*matchedPrefsJob + self.lambda_3*unmatchedPrefsJob + 
                self.lambda_4*matchedPrefsNurse + self.lambda_5*unmatchedPrefsNurse +
                self.lambda_6*jobToAvoid
                )

        return(score)
    ### --- End def preference_score --- ###    
               
    def solve(self, randomSeed=0, printAllCallData=False):
        # Prepare some data: 
        self.mk_mind = np.asarray(self.mk_mind, dtype=np.int32)
        self.mk_maxd = np.asarray(self.mk_maxd, dtype=np.int32)

        self.nJobs = int(self.nJobs)
        self.nNurses = int(self.nNurses)
        self.nSkills = int(self.nSkills)
        self.od = np.ascontiguousarray(self.od)
        self.nurse_travel_from_depot = np.ascontiguousarray(self.nurse_travel_from_depot)
        self.nurse_travel_to_depot = np.ascontiguousarray(self.nurse_travel_to_depot)
        self.unavail_matrix = np.ascontiguousarray(self.unavail_matrix.reshape(-1), dtype=np.int32)
        self.nurse_unavail = np.ascontiguousarray(self.nurse_unavail, dtype=np.int32)	
        self.nurseWorkingTimes = np.ascontiguousarray(self.nurseWorkingTimes, dtype=np.int32)
        self.jobTimeInfo = np.ascontiguousarray(self.jobTimeInfo, dtype=np.int32)
        self.jobSkillsRequired = np.ascontiguousarray(self.jobSkillsRequired, dtype=np.int32)
        self.nurseSkills = np.ascontiguousarray(self.nurseSkills, dtype=np.int32)
        self.solMatrix = np.ascontiguousarray(self.solMatrix, dtype=np.int32)
        self.doubleService = np.ascontiguousarray(self.doubleService, dtype=np.int32)		
        self.dependsOn = np.ascontiguousarray(self.dependsOn, dtype=np.int32)
        self.mk_mind = np.ascontiguousarray(self.mk_mind, dtype=np.int32)
        self.mk_maxd = np.ascontiguousarray(self.mk_maxd, dtype=np.int32)
        self.capabilityOfDoubleServices = np.ascontiguousarray(self.capabilityOfDoubleServices.reshape(-1))
        self.prefScore = np.ascontiguousarray(self.prefScore)
        self.algorithmOptions = np.ascontiguousarray(self.algorithmOptions)
        self.timeMatrix = np.ascontiguousarray(self.timeMatrix) # NEW 03/06/2021
        self.nurseWaitingTime = np.ascontiguousarray(self.nurseWaitingTime) # NEW 03/06/2021
        self.nurseTravelTime = np.ascontiguousarray(self.nurseTravelTime) # NEW 03/06/2021
        self.violatedTW = np.ascontiguousarray(self.violatedTW) # NEW 03/06/2021
        self.nurseWaitingMatrix = np.ascontiguousarray(self.nurseWaitingMatrix) # NEW 03/06/2021
        self.nurseTravelMatrix = np.ascontiguousarray(self.nurseTravelMatrix) # NEW 03/06/2021
        self.totalsArray = np.ascontiguousarray(self.totalsArray) # NEW 04/06/2021

        if	(self.verbose > 5 or printAllCallData):
            print('nJobs (type ' + str(type(self.nJobs)) + ')')
            print(self.nJobs)
            print('nNurses (type ' + str(type(self.nNurses)) + ')')
            print(self.nNurses)
            print('nSkills (type ' + str(type(self.nSkills)) + ')')
            print(self.nSkills)
            print('verbose (type ' + str(type(self.verbose)) + ')')
            print(self.verbose)
            print('MAX_TIME_SECONDS (type ' + str(type(self.MAX_TIME_SECONDS)) + ')')
            print(self.MAX_TIME_SECONDS)
            
            print('od (type ' + str(type(self.od)) + ')')
            print('dtype = ' + str(self.od.dtype))
            print('Shape = ' + str(self.od.shape))
            print(self.od)

            print('nurse_travel_from_depot (type ' + str(type(self.nurse_travel_from_depot)) + ')')
            print('dtype = ' + str(self.nurse_travel_from_depot.dtype))
            print('Shape = ' + str(self.nurse_travel_from_depot.shape))
            print(self.nurse_travel_from_depot)

            print('nurse_travel_to_depot (type ' + str(type(self.nurse_travel_to_depot)) + ')')
            print('dtype = ' + str(self.nurse_travel_to_depot.dtype))
            print('Shape = ' + str(self.nurse_travel_to_depot.shape))
            print(self.nurse_travel_to_depot)
        
            print('nurseWorkingTimes (type ' + str(type(self.nurseWorkingTimes)) + ')')
            print('dtype = ' + str(self.nurseWorkingTimes.dtype))
            print('Shape = ' + str(self.nurseWorkingTimes.shape))
            print(self.nurseWorkingTimes)
            
            print('jobTimeInfo (type ' + str(type(self.jobTimeInfo)) + ')')
            print('dtype = ' + str(self.jobTimeInfo.dtype))
            print('Shape = ' + str(self.jobTimeInfo.shape))
            print(self.jobTimeInfo)
            
            print('jobSkillsRequired (type ' + str(type(self.jobSkillsRequired)) + ')')
            print('dtype = ' + str(self.jobSkillsRequired.dtype))
            print('Shape = ' + str(self.jobSkillsRequired.shape))
            print(self.jobSkillsRequired)
            
            print('nurseSkills (type ' + str(type(self.nurseSkills)) + ')')
            print('dtype = ' + str(self.nurseSkills.dtype))
            print('Shape = ' + str(self.nurseSkills.shape))
            print(self.nurseSkills)
            
            print('solMatrix (type ' + str(type(self.solMatrix)) + ')')
            print('dtype = ' + str(self.solMatrix.dtype))
            print('Shape = ' + str(self.solMatrix.shape))
            print(self.solMatrix)
            
            print('doubleService (type ' + str(type(self.doubleService)) + ')')
            print('dtype = ' + str(self.doubleService.dtype))
            print('Shape = ' + str(self.doubleService.shape))			
            print(self.doubleService)

            print('dependsOn (type ' + str(type(self.dependsOn)) + ')')
            print('dtype = ' + str(self.dependsOn.dtype))
            print('Shape = ' + str(self.dependsOn.shape))
            print(self.dependsOn)
            
            print('mk_mind (type ' + str(type(self.mk_mind)) + ')')
            print('dtype = ' + str(self.mk_mind.dtype))
            print('Shape = ' + str(self.mk_mind.shape))
            print(self.mk_mind)
            
            print('mk_maxd (type ' + str(type(self.mk_maxd)) + ')')
            print('dtype = ' + str(self.mk_maxd.dtype))
            print('Shape = ' + str(self.mk_maxd.shape))
            print(self.mk_maxd)

            print('capabilityOfDoubleServices (type ' + str(type(self.capabilityOfDoubleServices)) + ')')
            print('dtype = ' + str(self.capabilityOfDoubleServices.dtype))
            print('Shape = ' + str(self.capabilityOfDoubleServices.shape))
            print(self.capabilityOfDoubleServices)
            
            print('prefScore (type ' + str(type(self.prefScore)) + ')')
            print('dtype = ' + str(self.prefScore.dtype))
            print('Shape = ' + str(self.prefScore.shape))
            print(self.prefScore)

            print('algorithmOptions (type ' + str(type(self.algorithmOptions)) + ')')
            print('dtype = ' + str(self.algorithmOptions.dtype))
            print('Shape = ' + str(self.algorithmOptions.shape))
            print(self.algorithmOptions)
            
        print('\nEnd of Python program - calling C program (GRASP-VNS)')
        print('Max time: ' + str(self.MAX_TIME_SECONDS) + ' seconds, Random seed: ' + str(randomSeed))
        print('-------------------------------------------------\n')

        self.fun(self.nJobs, self.nNurses, self.nSkills, self.verbose, self.MAX_TIME_SECONDS, self.tw_interval, self.exclude_nurse_travel, self.od, self.nurse_travel_from_depot, self.nurse_travel_to_depot,
            self.unavail_matrix, self.nurse_unavail, self.nurseWorkingTimes, self.jobTimeInfo, self.jobSkillsRequired, self.nurseSkills, self.solMatrix, self.doubleService, self.dependsOn,
            self.mk_mind, self.mk_maxd, self.capabilityOfDoubleServices, self.prefScore, self.algorithmOptions, self.timeMatrix, self.nurseWaitingTime, self.nurseTravelTime, self.violatedTW, 
            self.nurseWaitingMatrix, self.nurseTravelMatrix, self.totalsArray, randomSeed)

        if self.verbose > 10:
            print('Returned this matrix: ')
            print(self.solMatrix)
            print('Postprocessing...')
        # time.sleep(2)
        # print('Done.')
        self.post_process_solution()
    ### --- End def solve --- ###  

    def post_process_solution(self):
        # This function creates self.nurseRoute, which is the same as allNurseRoutes[nurse][position] = job.
        # Generate nurse routes:
        # self.nurseWaitingTime = np.zeros(self.nNurses)
        self.nurseServiceTime = np.zeros(self.nNurses)
        # self.nurseTravelTime = np.zeros(self.nNurses)
        self.nurseTime = np.zeros(self.nNurses)
        self.totalWaitingTime = 0
        self.totalServiceTime = 0
        self.totalTravelTime = 0
        self.totalTime = 0
        self.nurseRoute = []
        self.nurseTravelCost = np.zeros(self.nNurses, dtype=np.float64) # NEW 11/06/2021, initialise the nurseTravelCost array to 1 x nNurses
        self.nurseMileage = np.zeros(self.nNurses, dtype=np.float64) # NEW 11/06/2021, initialise the nurseMileage array to 1 x nNurses
        self.nurseMileageCost = np.zeros(self.nNurses, dtype=np.float64) # NEW 11/06/2021, initialise the nurseMileageCost array to 1 x nNurses

        for nurse in range(self.nNurses):
            try:
                howMany = max(self.solMatrix[nurse]) + 1 # howMany = largest value in self.solMatrix[nurse], which is equivalent to the number of jobs in the nurse's route. Add one because job positions start at 0.
            except Exception as e:
                print('[ERROR]: in post_process_solution()')
                print(e)
                print('Nurse: ' + str(nurse))
                print('solMatrix: \n' + str(self.solMatrix))
            
            self.nurseRoute.append(np.full(howMany, -1)) # add array of size 'number of jobs' containing all -1 to the nurseRoute list.
            for i,sp in enumerate(self.solMatrix[nurse,:]): # For each index, value in solMatrix[nurse], all jobs
                if sp > -1: # If > -1 then job i is in position sp of nurse's route
                    self.nurseRoute[nurse][sp] = i # set nurseRoute[nurse][position] = job (like allNurseRoutes)
    ### --- End def post_process_solution --- ###

    def convert_minutes_to_hhmmss(self, mins):
        # Convert time in minutes (decimal) to time in hh:mm:ss
        minsRound = math.floor(mins)
        hours = mins // 60
        minutes = (minsRound % 60)
        seconds = (mins - minsRound) * 60
        seconds = round(seconds)
        if seconds == 60:
            seconds = 0
            minutes += 1

        return('{0:0>2}:{1:0>2}:{2:0>2}'.format(int(hours), int(minutes), int(seconds)))
    ### --- End def convert_minutes_to_hhmmss --- ###  

    def solution_to_website_dst(self, filename='unknown', add_plots=True):
        print('Generating website...')
        # Check if website generation is available:
        if (self.nurseObjs[0].startLocation == []):
            print('[WARNING]: Website generation is not available for this instance.')
            print('Skipping.')
            return
        webFilename = 'unknown.html'
        if filename == 'unknown':
            webFilename = self.fname + '.html'
        cwd = os.getcwd()
        outputfiles_path = os.path.join(cwd, 'output')
        webFilename = os.path.join(outputfiles_path, webFilename)
        print('Website filename: '  + str(webFilename))

        xyRev = [] # Reverse xy list, which has coords in [lat,lon] form, whereas xy has coords in [lon, lat] form.
        for coord in self.xy:
            xyRev.append(reverse_latlong(coord)) # Add coord (which is [lon, lat]) to xyRev but in [lat,lon] order.
        m = folium.Map(location=xyRev[0], zoom_start=14, tiles='cartodbpositron')
        
        foliumRouteLayers = []
        for ni in range(self.nNurses):
            route_colour = clusterColour(ni) #Returns a hex colour.
            # Create a FeatureGroup layer; you can put things in it and handle them as a single layer.
            foliumRouteLayers.append(folium.map.FeatureGroup(name='Carer ' + str(ni))) # Create FeatureGroup for the current nurse ni, add it to the list foliumRouteLayers.
            nRoute = []
            nRouteRev = []
            # nRoute.append(tuple(rxy[0]))

            niRoute = self.get_nurse_route_dst(ni) # nurse route for nurse ni, niRoute[position] = job.
            # print(niRoute)
            for pos in range(self.nJobs):
                job = int(niRoute[pos]) #job = the job at position pos in nurse ni's route.
                if niRoute[pos] < -0.1: # If no job at position pos in nurse ni's route, then nurse ni has no route, no jobs assigned to nurse ni, so break.
                    break
                nRoute.append(tuple(xyRev[job + 1])) #append as a tuple the lat/lon of job
                nRouteRev.append(reverse_latlong(xyRev[job + 1])) # append as a list the LON/LAT of the job (reversed, so lon/lat, not lat/lon!) NOTE: Why not just use xy??
                # Add a Marker:
                border_colour = route_colour
                popupVal = '<b>Client ID:</b> ' + str(self.jobObjs[job].ID)
                popupVal = popupVal + '<br><b>Job #:</b> ' + str(job) 
                popupVal += '<br><b>Postcode:</b> ' + str(self.jobObjs[job].postcode) 
                popupVal = popupVal + '<br><b>Time Window:</b> ' + self.convert_minutes_to_hhmmss(self.jobTimeInfo[int(job)][0]) + ' - ' + self.convert_minutes_to_hhmmss(self.jobTimeInfo[int(job)][1])
                popupVal = popupVal + '<br><b>Job Duration:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].serviceTime)
                if self.jobObjs[job].doubleService == 1:
                    if self.jobObjs[job].markedWebsite == 0:
                        popupVal = popupVal + '<br><b>Double Service</b>'
                        # Carer 1 information
                        popupVal = popupVal + '<br><b>Carer 1 ID:</b> ' + str(self.jobObjs[job].nurseID[0])
                        popupVal = popupVal + '<br><b>Carer 1 #:</b> ' + str(self.jobObjs[job].assignedNurse[0])
                        popupVal = popupVal + '<br><b>Carer 1 Arrive:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].arrivalTime[0])
                        popupVal = popupVal + '<br><b>Carer 1 Start:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].startTime[0])
                        popupVal = popupVal + '<br><b>Carer 1 Depart:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].departureTime[0])
                        popupVal = popupVal + '<br><b>Carer 1 Waiting:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].waitingToStart[0])
                        if self.jobObjs[job].tardiness[0] > 0:
                            popupVal = popupVal + '<br><b>Carer 1 Tardiness:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].tardiness[0])
                            border_colour = '#ff0000'
                        popupVal = popupVal + '<br><b>Carer 1 Position #:</b> ' + str(self.jobObjs[job].positionInSchedule[0])
                        popupVal = popupVal + '<br><b>Carer 1 Travel to Job:</b>' + self.convert_minutes_to_hhmmss(self.jobObjs[job].travelToJob[0])
                        
                        # Carer 2 information
                        popupVal = popupVal + '<br><b>Carer 2 ID:</b> ' + str(self.jobObjs[job].nurseID[1])
                        popupVal = popupVal + '<br><b>Carer 2 #:</b> ' + str(self.jobObjs[job].assignedNurse[1])
                        popupVal = popupVal + '<br><b>Carer 2 Arrive:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].arrivalTime[1])
                        popupVal = popupVal + '<br><b>Carer 2 Start:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].startTime[1])
                        popupVal = popupVal + '<br><b>Carer 2 Depart:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].departureTime[1])
                        popupVal = popupVal + '<br><b>Carer 2 Waiting:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].waitingToStart[1])
                        if self.jobObjs[job].tardiness[1] > 0:
                            popupVal = popupVal + '<br><b>Carer 2 Tardiness:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].tardiness[1])
                            border_colour = '#ff0000'
                        popupVal = popupVal + '<br><b>Carer 2 Position #:</b> ' + str(self.jobObjs[job].positionInSchedule[1])
                        popupVal = popupVal + '<br><b>Carer 2 Travel to Job:</b>' + self.convert_minutes_to_hhmmss(self.jobObjs[job].travelToJob[1])
                        self.jobObjs[job].markedWebsite += 1
                else:
                    popupVal = popupVal + '<br><b>Carer ID:</b> ' + str(self.jobObjs[job].nurseID)
                    popupVal = popupVal + '<br><b>Carer #:</b> ' + str(self.jobObjs[job].assignedNurse)
                    popupVal = popupVal + '<br><b>Arrive:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].arrivalTime)
                    popupVal = popupVal + '<br><b>Start:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].startTime)
                    popupVal = popupVal + '<br><b>Depart:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].departureTime)
                    popupVal = popupVal + '<br><b>Waiting:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].waitingToStart)
                    popupVal = popupVal + '<br><b>Tardiness:</b> ' + self.convert_minutes_to_hhmmss(self.jobObjs[job].tardiness)
                    popupVal = popupVal + '<br><b>Position #:</b> ' + str(self.jobObjs[job].positionInSchedule)
                    popupVal = popupVal + '<br><b>Travel to Job:</b>' + self.convert_minutes_to_hhmmss(self.jobObjs[job].travelToJob)

                # Add a circle around the area with popup:
                foliumRouteLayers[-1].add_child(folium.Circle(xyRev[job + 1], radius=30, popup=popupVal, color=border_colour, fill_color=route_colour, fill_opacity=0.5, fill=True))
            # End for pos in range(nJobs) loop
            if len(nRouteRev) < 1:
                continue 

            # Obtain the routeList, which is a list of coordinates of the route ([lat,lon]), and the duration (SECONDS) and distance (METRES) of the route.
            routeList, dist, dur, distjobs, durjobs = route_points_osrm(self.nurseObjs[ni].startLocation.longlat(), self.nurseObjs[ni].startLocation.longlat(), goingThrough=nRouteRev)
            distJobsMiles = metres_to_miles(distjobs) # get distance for this nurse in miles, not metres
            self.nurseMileage[ni] = distJobsMiles
            self.nurseMileageCost[ni] = distJobsMiles * 0.25
            self.totalDistance = self.totalDistance + dist
            self.totalDistanceJobsOnly = self.totalDistanceJobsOnly + distjobs
            self.distNurses.append(dist)
            self.distNursesJobsOnly.append(distjobs)
            foliumRouteLayers[-1].add_child(folium.PolyLine(routeList, color=route_colour, weight=2.5, opacity=1)) # Add a coloured line along the route onto the layer.
            m.add_child(foliumRouteLayers[-1]) # Add the layer onto the map m.
        # End for ni in range(nNurses) loop
        
        # Add some info bottom-left:
        lht =   '''
                <div style="position: fixed;
                            background-color: rgba(255, 255, 255, 0.6);
                            bottom: 50px; left: 50px; width: 350px; height: 600px; 
                            z-index:9999; font-size:14px;
                            border:1px solid grey;
                            overflow: auto;
                            ">              
                '''
        lht = lht + '''&nbsp; <b><u>Solution Summary: DST</u></b><br>'''
        lht = lht + '''&nbsp; <b>Total time: </b>''' + self.convert_minutes_to_hhmmss(self.totalTime) + ''', of which:<br>'''
        lht = lht + '''&nbsp; <i> - Service time: </i>''' + self.convert_minutes_to_hhmmss(self.totalServiceTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Travel time: </i>''' + self.convert_minutes_to_hhmmss(self.totalTravelTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Waiting time: </i>''' + self.convert_minutes_to_hhmmss(self.totalWaitingTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Total distance: </i>''' + str(self.totalDistance/1000) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Total distance jobs: </i>''' + str(self.totalDistanceJobsOnly/1000) + '''<br><br>'''

        nursePart = '''&nbsp; <b><u>Carer breakdown:</u> </b><br>'''
        for i, cc in enumerate(self.nurseObjs):
            nursePart = nursePart + '''<br>&nbsp; <b>Carer ''' + str(i) + ' (' + str(cc.ID) + '''):</u> </b><br>'''
            nursePart = nursePart + '''&nbsp; <i>Skills: </i>''' + str(cc.skills) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Shift start time: </i>''' + self.convert_minutes_to_hhmmss(self.nurseWorkingTimes[i][0]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Shift end time: </i>''' + self.convert_minutes_to_hhmmss(self.nurseWorkingTimes[i][1]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Duration of shift: </i>''' + self.convert_minutes_to_hhmmss(self.nurseWorkingTimes[i][2]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Actual start time: </i>''' + self.convert_minutes_to_hhmmss(cc.startTime) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Actual end time: </i>''' + self.convert_minutes_to_hhmmss(cc.finishTime) + '''<br>'''
            cc.route = list(self.nurseRoute[i][:])
            nursePart = nursePart + '''&nbsp; <i>Number of services: </i>''' + str(len(cc.route)) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Total service time: </i>''' + self.convert_minutes_to_hhmmss(self.nurseServiceTime[i]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Total travel time: </i>''' + self.convert_minutes_to_hhmmss(self.nurseTravelTime[i]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Total waiting time: </i>''' + self.convert_minutes_to_hhmmss(self.nurseWaitingTime[i]) + '''<br>'''
            if len(cc.route) > 0:
                nursePart = nursePart + '''&nbsp; <i>Service route: </i>[''' + str(self.jobObjs[int(cc.route[0])].ID)
                for kkk in range(1,len(cc.route)):
                    jobbb = self.jobObjs[int(cc.route[kkk])]
                    if jobbb.doubleService:
                        nursePart = nursePart + ', (' + str(jobbb.ID) + ')'
                    else:
                        nursePart = nursePart + ', ' + str(jobbb.ID)

                nursePart = nursePart + ''']<br>'''
    
        lht = lht + nursePart

        if add_plots == True:
            modalImages = [self.fname + '_workload_dst.png', self.fname + '_time_info_dst.png']
            modalCaptions = ['Workload distribution', 'Time distribution']

            for i,imn in enumerate(modalImages):
                lht = lht + self.hovering_image(imn, modalCaptions[i], i)

            # Add modal chart:
            lht = lht + '''
            <div id="myModal" class="modal">
            <span class="close">&times;</span>
            <img class="modal-content" id="img_of_modal">
            <div id="caption"></div>
            </div>

            <script>
            '''

            for i,imn in enumerate(modalImages):
                lht = lht + '''
                var img''' + str(i) + ''' = document.getElementById('myImg''' + str(i) + '''');
                    img''' + str(i) + '''.onclick = function(){
                        just_display(this);
                    }'''

            # Finish script:
            lht = lht + '''
            var modalImg = document.getElementById("img_of_modal");
            var modal = document.getElementById('myModal');
            var captionText = document.getElementById("caption");

            // Get the <span> element that closes the modal
            var span = document.getElementsByClassName("close")[0];

            // When the user clicks on <span> (x), close the modal
            span.onclick = function() { 
                modal.style.display = "none";
            }
            just_display = function(imag)
            {
                modal.style.display = "block";
                modalImg.src = imag.src;
                captionText.innerHTML = imag.alt;   
            }
            </script>'''

        lht = lht + '''</div>'''
        
        m.get_root().html.add_child(folium.Element(lht))

        # Depot, change to one per nurse!
        for nursej in range(self.nNurses):
            nurse_popup = 'Start location for nurse ' + str(nursej)
            # print(self.nurseObjs[nursej].startLocation)
            folium.Circle(self.nurseObjs[nursej].startLocation.latlong(), radius=50, popup=nurse_popup, color='black', fill_color='black', fill_opacity=0.5, fill=True).add_to(m)
        
        m.add_child(folium.map.LayerControl())
        m.save(webFilename)

        # Calculate travel variables (moved here from c_w.py 27/09/2021)
        self.totalTravelCost = sum(self.nurseTravelCost)
        self.totalMileage = sum(self.nurseMileage)
        self.totalMileageCost = sum(self.nurseMileageCost)
        self.totalCost = self.totalTravelCost + self.totalMileageCost

        print('Website generated successfully.')
    ### --- End def solution_to_website_dst --- ###

    def hovering_image(self, imName, altText, idd):
        return('''<img id="myImg''' + str(idd) + '''" src="''' + imName + '''"
                 alt="''' + altText + '''" width="300" height="200">
                <style>
                #myImg''' + str(idd) + ''' {
                    border-radius: 5px;
                    cursor: pointer;
                    transition: 0.3s;
                }
                #myImg''' + str(idd) + ''':hover {opacity: 0.7;}
                </style>
                ''')
    ### --- End def hovering_image --- ###

    def plot_pie_time_spent_dst(self):
        # labels = 'Travel', 'Waiting', 'Service'
        # times = [self.totalTravelTime, self.totalWaitingTime, self.totalServiceTime]
        # colours = ['#98d4f9', '#a6e781', '#f998c5'] ##a6e781 ##eeee76
        # plt.pie(times, labels=labels, colors=colours, autopct='%1.1f%%')
        # plt.title('Total time for ' + str(self.nNurses) + ' nurses: ' + self.convert_minutes_to_hhmmss(self.totalTime))
        # plt.axis('equal')
        # plt.draw()

        fig = plt.figure()
        fig.suptitle(self.area + ' ' + str(self.date) + ': DST', fontsize=13, fontweight='bold')
        ax = fig.add_subplot(111)
        # fig.subplots_adjust(top=0.72)
        subtitle = 'Total time for ' + str(self.nNurses) + ' carers: ' + self.convert_minutes_to_hhmmss(self.totalTime)
        subtitle = '\nTravel Time: ' + self.convert_minutes_to_hhmmss(self.totalTravelTime) + '  Service Time: ' + self.convert_minutes_to_hhmmss(self.totalServiceTime) + '  Waiting Time: ' + self.convert_minutes_to_hhmmss(self.totalWaitingTime)
        ax.set_title(subtitle, fontsize=8)

        labels = 'Travel', 'Waiting', 'Service'
        times = [self.totalTravelTime, self.totalWaitingTime, self.totalServiceTime]
        colours = ['#98d4f9', '#a6e781', '#f998c5'] ##a6e781 ##eeee76

        def func(pct, allvals):
            absolute = int(pct/100.*np.sum(allvals))
            return '{:.1f}%\n({:d} mins)'.format(pct, absolute)

        # wedges, texts, autotexts = ax.pie(times, autopct=lambda pct: func(pct, times), colors=colours)
        wedges, texts, autotexts = ax.pie(times, autopct='%1.1f%%', colors=colours)

        ax.legend(wedges, labels, title='Times', loc='center left', bbox_to_anchor=(1, 0, 0.5, 1))

        plt.setp(autotexts, size=8)

        plt.draw()
        cwd = os.getcwd()
        pie_plot_name = self.fname + '_time_info.png'
        outputfiles_path = os.path.join(cwd, 'output')
        # plt.savefig(self.fname + '_time_info' + '.png', bbox_inches='tight')
        plt.savefig(outputfiles_path + '/' + pie_plot_name, bbox_inches='tight')
        # plt.show()
        # exit(-1)
    ### --- End def plot_pie_time_spent_dst --- ###

    def plot_bar_time_per_nurse_dst(self):
        xpos = np.arange(self.nNurses)
        width = 0.35

        p1 = plt.bar(xpos, self.nurseServiceTime, width, color='#f998c5')
        p2 = plt.bar(xpos, self.nurseTravelTime, width, bottom=self.nurseServiceTime, color='#98d4f9')
        p3 = plt.bar(xpos, self.nurseWaitingTime, width, bottom=(self.nurseServiceTime + self.nurseTravelTime), color='#a6e781')

        plt.xlabel('Carer')
        plt.ylabel('Time')
        plt.title(self.area + ' ' + str(self.date) + ': DST', fontsize=13, fontweight='bold')
        ticksNames = []
        for i in xpos:
            ticksNames.append(str(self.nurseObjs[i].ID))
            # ticksNames.append(i)
            # ticksNames.append('Nurse ' + str(i))
        # maxYtick = 8*3600/self.secondsPerTU
        maxYtick = 15*60/self.secondsPerTU
        # tmarks = np.arange(0, maxYtick, 1800/self.secondsPerTU)
        tmarks = np.arange(0, maxYtick, 30)
        tt = 0
        tticks = []
        for x in tmarks:
            tticks.append(str(np.round(tt, 1)) + ' h')
            tt = tt + 0.5
        plt.yticks(tmarks, tuple(tticks), fontsize=6)
        plt.xticks(xpos, tuple(ticksNames), fontsize=6)
        plt.legend((p1[0], p2[0], p3[0]), ('Service Time', 'Travel Time', 'Waiting Time'))

        plt.draw()
        cwd = os.getcwd()
        bar_plot_name = self.fname + '_workload.png'
        outputfiles_path = os.path.join(cwd, 'output')
        # plt.savefig(self.fname + '_workload' + '.png', bbox_inches='tight')
        plt.savefig(outputfiles_path + '/' + bar_plot_name, bbox_inches='tight')
        # plt.show()
    ### --- End def plot_bar_time_per_nurse_dst --- ###

    def get_nurse_route_dst(self, ni):
        # nurseRoute = array, all -1s, then if index p has value j, it means that job j is in position p in nurse's route.
        nurseRoute = np.zeros(self.nJobs)
        for ii in range(self.nJobs):
            nurseRoute[ii] = -1

        for ii in range(self.nJobs):
            if (self.solMatrix[ni][ii] >= 0):
                nurseRoute[self.solMatrix[ni][ii]] = ii

        return nurseRoute
    ### --- End def get_nurse_route_dst --- ###

    def get_travel_time(self, i, j):
        return(self.od[int(i + 1)][int(j + 1)])
    ### --- End def get_travel_time --- ###

    def get_nurse_from_travel_time(self, i, j):
        return(self.nurse_travel_from_depot[int(i)][int(j)])
    ### --- End def get_travel_time --- ###

    def get_nurse_to_travel_time(self, i, j):
        return(self.nurse_travel_to_depot[int(i)][int(j)])
    ### --- End def get_travel_time --- ###

    def full_solution_report(self, report=2, doPlots=True):
        arriveAt = 0
        startAt = 0
        readyToNext = 0
        leaveAt = 0
        finishShiftAt = 0
        currentTime = 0
        self.totalServiceTime = 0
        self.totalTime = 0
        self.totalWaitingTime = 0
        self.totalTravelTime = 0
        self.totalTardiness = 0
        self.maxTardiness = 0
        self.totalOvertime = 0
        self.maxOvertime = 0
        self.minSpareTime = 0
        self.maxSpareTime = 0
        self.shortestDay = 0
        self.longestDay = 0
        self.aitHQuality = 0
        self.MKQuality = 0
        self.WBQuality = 0
        self.paperQuality = 0
        self.totalPref = 0
        self.maxTravelTime = 0
        self.maxWaitingTime = 0
        self.maxDiffWorkload = 0

        nurseID = -1

        for i in range(self.nNurses):
            job = -1
            prevJob = -1
            self.nurseServiceTime[i] = 0
            # print('Nurse: ', i)
            nurseID = self.nurseObjs[i].ID
            for p in range(len(self.nurseRoute[i])): # For each POSITION
                costOfTravel = 0
                job = self.nurseRoute[i][p] # job = job at position p in nurse i's route
                # print('job: ', job)
                self.nurseServiceTime[i] += self.jobTimeInfo[job][2]
                if p == 0: # If this job is the first job for this nurse, then the nurse arrives at the start of the job, no waiting time.
                    arriveAt = self.timeMatrix[i][job]
                    startAt = arriveAt
                else:
                    arriveAt = readyToNext
                    startAt = self.timeMatrix[i][job]
                currentTime = startAt + self.jobTimeInfo[job][2] # currentTime = time that nurse i finishes job j (start time of job + duration of job)
                leaveAt = currentTime # Nurse i leaves job j at this time.
                if p < (len(self.nurseRoute[i])-1): # if this job is NOT the last job (i.e. in the last position) of nurse i's route
                    nextJob = self.nurseRoute[i][p+1]
                    readyToNext = leaveAt + self.nurseTravelMatrix[i][nextJob] #time at which nurse i arrives at the next job
                    # if p == (len(self.nurseRoute[i]) - 2):
                    # print('nextJob: ', nextJob, ' nurseTMnextJob: ', self.convert_minutes_to_hhmmss(self.nurseTravelMatrix[i][nextJob]), 'readyToNext: ', self.convert_minutes_to_hhmmss(readyToNext), ' leaveAt: ' , self.convert_minutes_to_hhmmss(leaveAt), ' todepotjob: ', self.convert_minutes_to_hhmmss(self.get_nurse_to_travel_time(i, job)), ' todepot next: ', self.convert_minutes_to_hhmmss(self.get_nurse_to_travel_time(i, nextJob)))
                prevJob = job
                if self.doubleService[job] == 1: # job is a double service
                    if self.jobObjs[job].markedReport == 0: # no information for either of the two nurses have been added to the job
                        self.jobObjs[job].nurseID = [nurseID]
                        self.jobObjs[job].assignedNurse = [i]
                        self.jobObjs[job].arrivalTime = [arriveAt]
                        self.jobObjs[job].startTime = [startAt]
                        self.jobObjs[job].departureTime = [leaveAt]
                        self.jobObjs[job].waitingToStart = [self.nurseWaitingMatrix[i][job]]
                        self.jobObjs[job].tardiness = [self.violatedTW[job]]
                        self.jobObjs[job].positionInSchedule = [self.solMatrix[i][job]]
                        self.jobObjs[job].travelToJob = [self.nurseTravelMatrix[i][job]]
                        self.jobObjs[job].markedReport += 1
                    elif self.jobObjs[job].markedReport == 1: # information for one of the two nurses has already been added to the job
                        self.jobObjs[job].nurseID.append(nurseID)
                        self.jobObjs[job].assignedNurse.append(i)
                        self.jobObjs[job].arrivalTime.append(arriveAt)
                        self.jobObjs[job].startTime.append(startAt)
                        self.jobObjs[job].departureTime.append(leaveAt)
                        self.jobObjs[job].waitingToStart.append(self.nurseWaitingMatrix[i][job])
                        self.jobObjs[job].tardiness.append(self.violatedTW[job])
                        self.jobObjs[job].positionInSchedule.append(self.solMatrix[i][job])
                        self.jobObjs[job].travelToJob.append(self.nurseTravelMatrix[i][job])
                        self.jobObjs[job].markedReport += 1
                    else:
                        print('[ERROR]: self.jobObjs[job].markedReport for job ', job, ' is ', self.jobObjs[job].markedReport)
                        print('Terminating program.')
                        exit(-1)
                else: # job is not a DS
                    self.jobObjs[job].nurseID = nurseID
                    self.jobObjs[job].assignedNurse = i
                    self.jobObjs[job].arrivalTime = arriveAt
                    self.jobObjs[job].startTime = startAt
                    self.jobObjs[job].departureTime = leaveAt
                    self.jobObjs[job].waitingToStart = self.nurseWaitingMatrix[i][job]
                    self.jobObjs[job].tardiness = self.violatedTW[job]
                    self.jobObjs[job].positionInSchedule = self.solMatrix[i][job]
                    self.jobObjs[job].travelToJob = self.nurseTravelMatrix[i][job]
                costOfTravel = get_travel_cost(self.nurseTravelMatrix[i][job])
                self.nurseTravelCost[i] += costOfTravel
            # End for loop p
            if job > -1:
                travelTime = self.get_nurse_to_travel_time(i, job)
                finishShiftAt = leaveAt + travelTime
            else:
                finishShiftAt = self.nurseWorkingTimes[i][0]
            self.nurseObjs[i].finishTime = finishShiftAt
        # End for loop i

        self.totalTime = self.totalsArray[0]
        self.totalWaitingTime = self.totalsArray[1]
        self.totalTravelTime = self.totalsArray[2]
        self.totalServiceTime = self.totalsArray[3]
        self.totalTardiness = self.totalsArray[4]
        self.maxTardiness = self.totalsArray[5]
        self.totalOvertime = self.totalsArray[8]
        self.maxOvertime = self.totalsArray[9]
        self.minSpareTime = self.totalsArray[10]
        self.maxSpareTime = self.totalsArray[11]
        self.shortestDay = self.totalsArray[12]
        self.longestDay = self.totalsArray[13]
        self.aitHQuality = self.totalsArray[14]
        self.MKQuality = self.totalsArray[15]
        self.WBQuality = self.totalsArray[16]
        self.paperQuality = self.totalsArray[17]
        self.totalPref = self.totalsArray[19]
        self.maxTravelTime = self.totalsArray[20]
        self.maxWaitingTime = self.totalsArray[21]
        self.maxDiffWorkload = self.totalsArray[22]

        if doPlots:
            self.plot_pie_time_spent_dst()
            self.plot_bar_time_per_nurse_dst()
            
        return self.Cquality  
    ### --- End def full_solution_report --- ###

    def solution_df_csv(self, client_df):
        client_df = client_df.assign(carer_id=np.nan, arrive_job=np.nan, start_job=np.nan, depart_job=np.nan, travel_time=np.nan, waiting_time=np.nan, tardiness=np.nan)
        # client_df['arrive'] = pd.Series()
        # client_df['startJob'] = pd.Series()
        # client_df['depart'] = pd.Series()
        # client_df['travel_time'] = pd.Series()
        # client_df['waiting_time'] = pd.Series()
        # client_df['tardiness'] = pd.Series()

        for i in range(len(client_df)):
            for j in range(self.nJobs):
                if self.jobObjs[j].ID == client_df.iloc[i]['client_id']:
                    timeWindow = [client_df.iloc[i]['tw_start'], client_df.iloc[i]['tw_end']]
                    if self.jobObjs[j].timewindow == timeWindow:
                        if self.doubleService[j] == 1:
                            client_df.loc[i, 'nurse_id'] = self.jobObjs[j].nurseID[0]
                            client_df.loc[i, 'arrive_job'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].arrivalTime[0])
                            client_df.loc[i, 'start_job'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].startTime[0])
                            client_df.loc[i, 'depart_job'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].departureTime[0])
                            client_df.loc[i, 'waiting_time'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].waitingToStart[0])
                            client_df.loc[i, 'tardiness'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].tardiness[0])
                            client_df.loc[i, 'travel_time'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].travelToJob[0])
                            client_df.loc[i, 'position'] = self.jobObjs[j].positionInSchedule[0]

                            client_df.loc[i, 'nurse_id2'] = self.jobObjs[j].nurseID[1]
                            client_df.loc[i, 'arrive_job2'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].arrivalTime[1])
                            client_df.loc[i, 'start_job2'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].startTime[1])
                            client_df.loc[i, 'depart_job2'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].departureTime[1])
                            client_df.loc[i, 'waiting_time2'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].waitingToStart[1])
                            client_df.loc[i, 'tardiness2'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].tardiness[1])
                            client_df.loc[i, 'travel_time2'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].travelToJob[1])
                            client_df.loc[i, 'position2'] = self.jobObjs[j].positionInSchedule[1]
                            break
                        else:
                            client_df.loc[i, 'nurse_id'] = self.jobObjs[j].nurseID
                            client_df.loc[i, 'arrive_job'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].arrivalTime)
                            client_df.loc[i, 'start_job'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].startTime)
                            client_df.loc[i, 'depart_job'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].departureTime)
                            client_df.loc[i, 'waiting_time'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].waitingToStart)
                            client_df.loc[i, 'tardiness'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].tardiness)
                            client_df.loc[i, 'travel_time'] = self.convert_minutes_to_hhmmss(self.jobObjs[j].travelToJob)
                            client_df.loc[i, 'position'] = self.jobObjs[j].positionInSchedule
                            break
        
        cwd = os.getcwd()
        solutiondfcsv_filename = self.fname + '_solution.csv'
        outputfiles_path = os.path.join(cwd, 'output')    
        client_df.to_csv(outputfiles_path + '/' + solutiondfcsv_filename)
        print('Saved solution to', solutiondfcsv_filename)
    ### --- End def solution_df_csv --- ###

    def output_results_file(self, elapsed_time):
        cwd = os.getcwd()
        results_filename = self.fname + '_results.txt'
        outputfiles_path = os.path.join(cwd, 'output')
        resultsfile_path = os.path.join(outputfiles_path, results_filename)

        f = open(resultsfile_path, 'a')
        f.write('------------------------------------------------------------\n')
        f.write('Date: ' + str(datetime.datetime.now()) + '\n')
        f.write('Quality: ' + str(self.Cquality) + '\n')
        f.write('Measure: ' + str(self.quality_measure) + '\n')
        f.write('Nurses: ' + str(self.nNurses) + '\n')
        f.write('Jobs: ' + str(self.nJobs) + '\n')
        f.write('Total Time: ' + str(self.convert_minutes_to_hhmmss(self.totalTime)) + '\n')
        f.write('Total Service Time: ' + str(self.convert_minutes_to_hhmmss(self.totalServiceTime)) + '\n')
        f.write('Total Travel Time: ' + str(self.convert_minutes_to_hhmmss(self.totalTravelTime)) + '\n')
        f.write('Total Waiting Time: ' + str(self.convert_minutes_to_hhmmss(self.totalWaitingTime)) + '\n')
        f.write('Total Tardiness: ' + str(self.convert_minutes_to_hhmmss(self.totalTardiness)) + '\n')
        f.write('Total Overtime: ' + str(self.convert_minutes_to_hhmmss(self.totalOvertime)) + '\n')
        f.write('Total Preference: ' + str(self.totalPref) + '\n')
        f.write('Max Travel Time: ' + str(self.convert_minutes_to_hhmmss(self.maxTravelTime)) + '\n')
        f.write('Max Waiting Time: ' + str(self.convert_minutes_to_hhmmss(self.maxWaitingTime)) + '\n')
        f.write('Max Diff Workload: ' + str(self.maxDiffWorkload) + '\n')
        f.write('Total Mileage: ' + str(self.totalMileage) + '\n')
        f.write('Total Cost (£): ' + str(self.totalCost) + '\n')
        f.write('Elapsed Time (s): ' + str(elapsed_time) + '\n')
        f.write('------------------------------------------------------------\n')
        f.close()
        print('Stored output values to', results_filename)
    ### --- End def output_results_file --- ###
### --- End class INSTANCE --- ###

def default_options_vector():
    ov = np.zeros(100, dtype=np.float64)
    ov[0] = 6.0 # Quality meausre (might be modified automatically for MK, default: paper.)
    ov[1] = 0.0 # Two-opt active
    ov[2] = 1.0 # 2 exchange active
    ov[3] = 1.0 # Nurse order change active (neighbourhood in local search)
    ov[4] = 0.5 # GRASP delta low, originally 0.05, in hhc paper 0.25
    ov[5] = 0.48 # GRASP delta range, originally 0.25, in hhc paper 0.81
    ov[6] = 1.0 # Nurse order change active (In GRASP, between calls)
    ov[7] = 1.0 # performPathRelinking
    ov[8] = 20.0 # Solutions in pool, originally 10.0, in hhc paper 19
    ov[9] = 3.0 # PR_STRATEGY Binary, perform path relinking for every solution with one random solution in the pool
    ov[10] = 2.0 # GRASP: RCL strategy (1 or 2), originally 1.0
    ov[11] = 2.0 # PR_DIRECTION
    ov[12] = 0 # Use gap (1) or precedence (0)

    # Weights of objective function (for "paper" measure)
    # $\alpha_1$      &   1   &   Travel time \\
    # $\alpha_2$      &   1   &   Waiting time \\
    # $\alpha_3$      &   5   &   Tardiness \\
    # $\alpha_4$      &   5   &   Overtime \\   
    # $\alpha_5$      &   $\frac{1}{4}$   &   Workload balance \\       
    # $\alpha_6$      &   1   &   Preference score \\  

    # Paper
    ov[50] = 0 # 1 if tardiness and overtime are infeasible, 0 if feasible
    ov[51] = -1.0 # alpha_1 Travel time
    ov[52] = -1.0 # alpha_2 Waiting time
    ov[53] = -5.0 # alpha_3 Tardiness
    ov[54] = -5.0 # alpha_4 Overtime
    ov[55] = 0.50 # alpha_5 Workload balance
    ov[56] = 1 # alpha_6 Preference score
    ov[57] = 0 # alpha_7 Max tardiness (not in paper)
    ov[58] = -1 # alpha_8 Max Travel Time NEW: 06/11/2021
    ov[59] = -1 # alpha_9 Max Waiting Time NEW: 06/11/2021
    ov[60] = -1 # alpha_5 NEW FOR NEW WORKLOAD BALANCE VARIABLE 06/11/2021

    ov[99] = 0.0 # print all input data
    return ov
### --- End def default_options_vector --- ###

def metres_to_miles(metres):
    #miles = metres * 0.00062137
    if np.isnan(metres):
        return 0.0
    elif metres == 0:
        return metres
    else:
        miles = metres * 0.00062137
        return miles
### --- End def metres_to_miles --- ###

def get_travel_cost(travelTime):
    # convert time in minutes to time in hours, then multiply by travel cost
    # If trip is less than 25 minutes, then cost is £8.36/hour, else if trip is >= 25 minutes, then cost is £8.91/hour

    cost = 0
    travelTimeHours = travelTime / 60
    if travelTime < 25 and travelTime >= 0:
        cost = travelTimeHours * 8.36
        return cost
    elif travelTime >= 25:
        cost = travelTimeHours * 8.91
        return cost
    else:
        print('[ERROR]: Incorrect value for travelTime when attempting to calculate travel cost.')
        print('travelTime = ', travelTime)
        print('Terminating program.')
        exit(-1)
### --- End of def calculate travel_cost_matrix --- ###

def create_lonlat_string(coord):
    # For osrm_request, we need the coordinates in string form to put into the web address. 
    # This function takes in a set of coordinates [lon,lat] and converts the two floats into a single string.
    # No need to swap coordinates as coord is in the format [lon,lat].
    # E.g.: coord = [-1.7, 51.2], temp_string_coord = ['-1.7', '51.2'], string_coord_reverse = '-1.7,51.2'
    
    temp_string_coord = [str(float) for float in coord]
    string_coord = ','.join(temp_string_coord)

    return string_coord
### --- End of def create_lonlat_string --- ###  

def reverse_latlong(latlong):
    return [latlong[1], latlong[0]]
### --- End def reverse_latlong --- ###   

def route_points_osrm(p1, p2, goingThrough=None):
    # Returns a list of coordinates with the route between the points, as well as the duration and distance
    # NOTE: Duration is is SECONDS, distance is is metres.
    # [routeList, duration, distance]

    stringcoord1 = ','.join([str(p1[0]), str(p1[1])])
    stringcoord2 = ','.join([str(p2[0]), str(p2[1])])
    stringGoingThrough = ';'.join([','.join([str(i), str(j)]) for i, j in goingThrough])
    string_all = ';'.join([stringcoord1, stringGoingThrough, stringcoord2])

    server = r'localhost:5000'
    str_call = 'http://' + server + '/route/v1/driving/' + string_all + '?overview=full&geometries=geojson'
    r = requests.get(str_call)
    osrm_result = r.json()
    geoTemp = osrm_result['routes'][0]['geometry']['coordinates']

    routeList = []
    for coordPair in geoTemp:
        routeList.append([float(coordPair[1]), float(coordPair[0])])
    dist = osrm_result['routes'][0]['distance']
    dur = osrm_result['routes'][0]['duration'] # NOTE: THIS IS IN SECONDS!! NEED TO CHANGE TO MINUTES.
    firstlegdist = osrm_result['routes'][0]['legs'][0]['distance'] # distance from nurse's home to first job
    lastlegdist = osrm_result['routes'][0]['legs'][-1]['distance'] # distance from last job back to nurse's home
    distjobs = dist - (firstlegdist + lastlegdist)
    firstlegdur = osrm_result['routes'][0]['legs'][0]['duration'] # NOTE: IN SECONDS!! NEED TO CHANGE TO MINUTES
    lastlegdur = osrm_result['routes'][0]['legs'][-1]['duration'] # NOTE: IN SECONDS!! NEED TO CHANGE TO MINUTES.
    durjobs = dur - (firstlegdur + lastlegdur)

    return [routeList, dist, dur, distjobs, durjobs]
### --- End def route_these_points --- ###

def clusterColour(clusterNumber):
    # From: https://www.w3schools.com/tags/ref_colornames.asp
    clusterNumber = clusterNumber % 18
    if clusterNumber == 0:
        col = '#F03A2C' # Red
    elif clusterNumber == 1:
        col = '#F3AD52' # Light Orange
    elif clusterNumber == 2:
        col = '#FFD700' # Yellow
    elif clusterNumber == 3:
        col = '#99ED57' # Light Green
    elif clusterNumber == 4:
        col = '#46DFD4' # Turquoise
    elif clusterNumber == 5:
        col = '#5FCBF3' # Light Blue
    elif clusterNumber == 6:
        col = '#9C83EE' # Light Purple
    elif clusterNumber == 7:
        col = '#F285D4' # Light Pink
    elif clusterNumber == 8:
        col = '#F0419E' # Dark Pink
    elif clusterNumber == 9:
        col = '#F48C06' # Dark Orange
    elif clusterNumber == 10:
        col = '#40830A' # Dark Green
    elif clusterNumber == 11:
        col = '#136AE1' # Dark Blue
    elif clusterNumber == 12:
        col = '#6A3BCE' # Dark Purple
    elif clusterNumber == 13:
        col = '#A61206' # Dark Red
    elif clusterNumber == 14:
        col = '#8BA608' # Olive
    elif clusterNumber == 15:
        col = '#838AEE' # Light Purple Blue
    elif clusterNumber == 16:
        col = '#FA9286' # Peach
    elif clusterNumber == 17:
        col = '#14318C' # Navy

    return(col)
### --- End def clusterColour --- ###

