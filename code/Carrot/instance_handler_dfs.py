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

# Display all rows and columns of dataframes in command prompt:
pd.set_option("display.max_rows", None, "display.max_columns", None)

def create_solve_inst(client_df, carershift_df, carerday_df, planning_date, options_vector, wb_balance, quality_measure, max_time_seconds, random_seed):
    
    # Create instance of the INSTANCE class
    inst = INSTANCE()
   
    # Build/fill inst object
    inst = convert_dfs_inst(inst, client_df, carershift_df, carerday_df, planning_date)
        
    inst.lambda_1 = 1
    inst.lambda_2 = 1
    inst.lambda_3 = 1
    inst.lambda_4 = 1
    inst.lambda_5 = 1
    inst.lambda_6 = 10
    inst.quality_measure = quality_measure
    inst.MAX_TIME_SECONDS = max_time_seconds
    options_vector[55] = wb_balance # alpha_5 Workload balance (from user input)

    # Create instances of JOB and CARER classes for inst
    inst.init_job_and_carer_objects(client_df, carerday_df)

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
    inst.solve(randomSeed=random_seed, printAllCallData=False)
    inst.solve = []
    inst.lib = []
    inst.fun = []
    inst.Cquality = -1*inst.od[0][0] # Cquality = solution quality from C program, it's stored in od[0][0] (shortcut, should probably change this!)
    inst.od[0][0] = saved_od_data

    return inst
### --- End def create_solve_inst --- ###

def convert_dfs_inst(inst, client_df, carershift_df, carerday_df, planning_date):
  
    inst.name = str(client_df.loc[0]['area']) + '_' + str(pd.Timestamp.date(planning_date)) # name is 'inst_area_date'
    inst.fname = inst.name # Note: Need to change
    inst.area = client_df.loc[0]['area']
    inst.date = pd.Timestamp.date(planning_date)
    inst.nCarers = len(carerday_df)
    inst.nJobs = len(client_df)
    inst.nShifts = len(carershift_df)
    inst.nSkills = 5 # NOTE: this is a random number just for testing.
    cwd = os.getcwd()

    # Create folder called 'output', if such a folder does not already exist.
    outputfiles_path = os.path.join(cwd, 'output')
    if not os.path.exists(outputfiles_path):
        os.mkdir(outputfiles_path)
    
    # Create and fill arrays with data from the instance
    inst.carerWorkingTimes = np.zeros((inst.nCarers, 3), dtype=np.int32) # carerWorkingTimes is nCarers x 3, col[0] = start time, col[1] = finish time, col[2] = max working time.
    for i in range(inst.nCarers):
        inst.carerWorkingTimes[i][0] = carerday_df.iloc[i]['start']
        inst.carerWorkingTimes[i][1] = carerday_df.iloc[i]['end']
        inst.carerWorkingTimes[i][2] = carerday_df.iloc[i]['duration']
    

    inst.jobTimeInfo = np.zeros((inst.nJobs, 3), dtype=np.int32) # jobTimeInfo is nJobs x 3, col[0] = startTW, col[1] = endTW, col[2] = length of job (time)
    for i in range(inst.nJobs):
        inst.jobTimeInfo[i][0] = client_df.iloc[i]['tw_start']
        inst.jobTimeInfo[i][1] = client_df.iloc[i]['tw_end']
        inst.jobTimeInfo[i][2] = client_df.iloc[i]['duration']

    
    inst.doubleService = np.zeros(inst.nJobs, dtype=np.int32)
    for i in range(inst.nJobs):
        if client_df.iloc[i]['num_carers'] > 1:
            inst.doubleService[i] = 1
    nDS = np.sum(inst.doubleService) # Number of jobs that are double services
    

    inst.od = np.zeros((inst.nJobs+1, inst.nJobs+1), dtype=np.float64) # TIME IN MINUTES, THIS WILL BE USED IN C
    osrm_table_request(inst, client_df, carerday_df, 'od')

    inst.travelCostMatrix = np.zeros((inst.nJobs, inst.nJobs), dtype=np.float64) # NEW, OD_COST, 11/06/2021, take time in minutes and convert to cost.
    calculate_travel_cost_matrix(inst)
    
    inst.carer_travel_from_depot = np.zeros((inst.nCarers, inst.nJobs), dtype=np.float64) # From carer's home to job - TIME IN MINS, THIS WILL BE USED IN C
    osrm_table_request(inst, client_df, carerday_df, 'carerfrom')

    inst.carer_travel_to_depot = np.zeros((inst.nCarers, inst.nJobs), dtype=np.float64) # From job to carer's home - TIME IN MINS, THIS WILL BE USED IN C
    osrm_table_request(inst, client_df, carerday_df, 'carerto')
       
    inst.carerSkills = np.ones((inst.nCarers, inst.nSkills), dtype=np.int32) # Note: this will only work if nSkills > 0.
    inst.jobSkillsRequired = np.ones((inst.nJobs, inst.nSkills), dtype=np.int32) # Note: this will only work if nSkills > 0.
    inst.prefScore = np.zeros((inst.nJobs, inst.nCarers), dtype=np.float64)
    inst.dependsOn = np.full(inst.nJobs, -1, dtype=np.int32) # Set all jobs to -1, means no jobs are dependent on one another.
    inst.algorithmOptions = np.zeros(100, dtype=np.float64)
    # NOTE: what happens if nDS == 0, i.e. there are no double services? Cannot have third dimension of capabilityofDS matrix be zero, need to make it one.
    inst.capabilityOfDoubleServices = np.ones((inst.nCarers, inst.nCarers, nDS), dtype=np.int32) # NOTE: just for testing, setting it to all ones so that every pair of nurses is capable of doing the double service.
    inst.mk_mind = np.zeros(inst.nJobs+1, dtype=np.int32) #nJobs+1 because that's what is taken in by C, mk_mind[i] = mk_mind_data[i+1]
    inst.mk_maxd = np.zeros(inst.nJobs+1, dtype=np.int32)
    inst.solMatrix = np.zeros((inst.nCarers, inst.nJobs), dtype=np.int32)
    inst.timeMatrix = np.full((inst.nCarers, inst.nJobs), -1, dtype=np.float64) # NEW 03/06/2021
    inst.carerWaitingTime = np.zeros(inst.nCarers, dtype=np.float64) # NEW 03/06/2021
    inst.carerTravelTime = np.zeros(inst.nCarers, dtype=np.float64) # NEW 03/06/2021
    inst.violatedTW = np.zeros(inst.nJobs, dtype=np.float64) # NEW 03/06/2021
    inst.carerWaitingMatrix = np.zeros((inst.nCarers, inst.nJobs), dtype=np.float64) # NEW 03/06/2021
    inst.carerTravelMatrix = np.zeros((inst.nCarers, inst.nJobs), dtype=np.float64) # NEW 03/06/2021
    inst.totalsArray = np.zeros(19, dtype=np.float64) # NEW 04/06/2021

    # Matrix of size 50 by 4 by nCarers, where number of rows = number of shifts, col[0] = shift number, col[1] = start time, col[2]= end time, col[3] = duration of shift, and third dimension is number of nurses.
    # i.e each 2d matrix is 50x4, and there are nCarers lots of 2d matrices to form a 3d matrix.
    numCarers = 0
    numShifts = 0
    shift_count = 0
    inst.shift_matrix = np.full((50, 4, inst.nCarers), -1, dtype=np.int32)

    # Start with first carer 0
    prevCarer = carershift_df.iloc[0]['carer_id']
    inst.shift_matrix[shift_count][0][numCarers] = numShifts
    inst.shift_matrix[shift_count][1][numCarers] = carershift_df.iloc[0]['start']
    inst.shift_matrix[shift_count][2][numCarers] = carershift_df.iloc[0]['end']
    inst.shift_matrix[shift_count][3][numCarers] = carershift_df.iloc[0]['duration']

    # Fill shift_matrix for the rest of the nurses
    for i in range(1, len(carershift_df)):
        currentCarer = carershift_df.iloc[i]['carer_id']
        if prevCarer == currentCarer: # If currentCarer is the same as the previousNurse, then the carer has another shift.
            shift_count += 1
            numShifts += 1
            inst.shift_matrix[shift_count][0][numCarers] = numShifts
            inst.shift_matrix[shift_count][1][numCarers] = carershift_df.iloc[i]['start']
            inst.shift_matrix[shift_count][2][numCarers] = carershift_df.iloc[i]['end']
            inst.shift_matrix[shift_count][3][numCarers] = carershift_df.iloc[i]['duration']
        else: # Otherwise currentCarer is a new carer, so add this shift detail to the next matrix in the 3d matrix
            shift_count = 0
            numCarers += 1
            numShifts = 0
            inst.shift_matrix[shift_count][0][numCarers] = numShifts
            inst.shift_matrix[shift_count][1][numCarers] = carershift_df.iloc[i]['start']
            inst.shift_matrix[shift_count][2][numCarers] = carershift_df.iloc[i]['end']
            inst.shift_matrix[shift_count][3][numCarers] = carershift_df.iloc[i]['duration']
            prevCarer = currentCarer

    # Create unavailability matrix, same format as shift_matrix
    inst.unavail_matrix = np.full((50,4,inst.nCarers), -1, dtype=np.int32)
    inst.carer_unavail = np.zeros(inst.nCarers, dtype=np.float64) # Array that hold the number of unavailble shifts for each carer.

    for k in range(inst.nCarers): # For each carer
        num_unavail = 0 # Set the number of unavailable shifts to 0 for that carer
        if inst.shift_matrix[1][0][k] < 0: # If the carer k does not have a second shift, then the carer only have one shift (which is the whole day) and so is not unavailable at all
            inst.carer_unavail[k] = 0 # The number of unavailable shifts for this carer is 0
            continue
        elif inst.shift_matrix[1][0][k] > 0: # If the carer k does have a second shift, then the carer has at least one unavailble shift throughout the day
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
                inst.carer_unavail[k] += 1 # update the number of unavailable shifts for this carer k.
    # End for loop
    
    return inst
### --- End of def convert_dict_inst --- ###

def osrm_table_request(inst, client_df, carerday_df, matrix='none'):
    # For osrm, the coordinates need to be in string form, and in the order 'longitude,latitude'. Our data is in 'lat,lon', so create_coord_string function swaps it around.

    nCarers = inst.nCarers
    nJobs = inst.nJobs
    area = inst.area

    lonlat_jobs = []
    for i in range(nJobs):
        lonlat = [client_df.iloc[i]['longitude'], client_df.iloc[i]['latitude']] #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        lonlat_jobs.append(lonlat)
    lljSize = len(lonlat_jobs)
        
    lonlat_carers = []
    for i in range(nCarers):
        lonlat = [carerday_df.iloc[i]['longitude'], carerday_df.iloc[i]['latitude']] #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        lonlat_carers.append(lonlat)
    llcSize = len(lonlat_carers)

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

    elif matrix == 'carerfrom':
        # Going FROM depot, so time from each CARER home to each JOB (source = carer, destination = job)

        # Nurses first (sources):
        count = 0 # Used to count the number of sources and destinations, as the index of each [lon,lat] in the address needs to be added to the end of the address.
        source_list = [] # Indices for source long/lats
        coord_strings_list_carers = [] # List of longs/lats as strings
        for i in range(llcSize):
            coordstrcarer = create_lonlat_string(lonlat_carers[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_carers.append(coordstrcarer) # Add the long/lat string to list
            source_list.append(str(i)) # Add index of long/lat pair to list
            count +=1
        coord_strings_carers = ';'.join(coord_strings_list_carers) # coord_strings_carers = single string comprising all strings in coord_strings_list_carers joined with semicolons in between
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
        str_call = str_call + coord_strings_carers + ';' + coord_strings_jobs + sources + destinations # Create full address
        r = requests.get(str_call) # Get results from osrm
        osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.

        # Fill carer_travel_from_depot matrix:
        durations = osrmdict['durations']
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.carer_travel_from_depot[i][j] = time_mins
    # --- End carer from matrix --- #

    elif matrix == 'carerto':
        # Going TO depot, so time from each JOB to each CARER home (source = job, destination = carer)

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
        coord_strings_list_carers = [] # List of longs/lats as strings
        for i in range(llcSize):
            coordstrcarer = create_lonlat_string(lonlat_carers[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_carers.append(coordstrcarer) # Add the long/lat string to list
            destination_list.append(str(count)) # Add index of long/lat pair to list
            count += 1
        coord_strings_carers = ';'.join(coord_strings_list_carers) # coord_strings_carers = single string comprising all strings in coord_strings_list_carers joined with semicolons in between
        destination_string = ';'.join(destination_list) # Join the list of strings together into a single string with semicolon between each string

        # Get results from osrm:
        sources = '?sources=' + source_string
        destinations = '&destinations=' + destination_string
        str_call = str_call + coord_strings_jobs + ';' + coord_strings_carers + sources + destinations
        r = requests.get(str_call)
        osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.

        # Fill carer_travel_to_depot matrix:
        durations = osrmdict['durations']
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.carer_travel_to_depot[j][i] = time_mins
    # --- End carer to matrix --- #
    else:
        print('[ERROR]: no matrix type specified for osrm_table_request.')
        print('Matrix type should be either \'od\', \'carerfrom\', or \'carerto\'.')
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
        # self.doubleService = False
        self.doubleService = 0
        self.dependsOn = []
        self.postcode = 'Unknown'
        self.latlong = []
        self.priority = 0
        self.hasTimewindow = False
        self.timewindow = [0,86400]
        self.hasPreferredTimewindow = False
        self.preferredTimewindow = [0, 86400]
        self.skillsRequired = []
        self.features = []
        self.preferences = []
        self.preferredCarers = []
        self.dependsOn = -1
        self.minimumGap = 0
        self.maximumGap = 0
        self.location = [] # NOTE: NEW, 27/12/2020 ALH
        # Calculated (solution)
        self.assignedCarer = 0
        self.carerID = 'Unknown'
        self.arrivalTime = 0 # time carer arrives at job location
        self.startTime = 0 # NEW: 07/06/2021, actual time job starts
        self.departureTime = 0
        self.waitingToStart = 0
        self.tardiness = 0
        self.positionInSchedule = 0
        self.travelToJob = 0 # NEW: 09/06/2021, the travel time from the previous job/depot to this job
        self.markedReport = 0 # use this to determine for DS jobs if they have already been assessed in full_solution_report, if they haven't (=0) then need to create list, else if they have been assessed (=1) then need to append to the list.
        self.markedWebsite = 0 # use this to determine for DS jobs if they have already been assessed in solution_to_website, if they haven't (=0) then need to create list, else if they have been assessed (=1) then need to append to the list.
### --- End class JOB --- ###        

class CARER(object):
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
### --- End class CARER --- ###                       

class INSTANCE(object):
    def __init__(self):
        self.nJobs = -1
        self.nCarers = -1
        self.nSkills = -1
        self.nShifts = -1
        self.carerWorkingTimes = []
        self.carerSkills = []
        self.jobTimeInfo = []
        self.jobSkillsRequired = []
        self.prefScore = []
        self.algorithmOptions = np.zeros(100, dtype=np.float64)
        self.doubleService = []
        self.dependsOn = []
        self.od = [] # Main od matrix, contains times in minutes - this will be used in C
        self.od_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.carer_travel_from_depot = [] # Main carer_travel_from_depot matrix, contains times in minutes - this will be used in C
        self.carer_travel_to_depot = [] # Main carer_travel_to_depot matrix, contains times in minutes - this will be used in C
        self.shift_matrix = [] # NOTE: NEW 22/05/2021, used to hold shift information for nurses 3D matrix
        self.unavail_matrix = [] # NOTE: NEW 22/05/2021, used to hold unavailable shift information for nurses 3D matrix
        self.carer_unavail = [] # NOTE: NEW 22/05/2021, used to hold number of unavailable shifts for each carer (1d array)
        self.carer_travel_from_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.carer_travel_to_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.xy = [] # x y coordinates for plotting routes on a map LAT-LONG
        self.solMatrix = []
        self.MAX_TIME_SECONDS = 30
        self.verbose = 1 # was 5, changed to 1 on 11/06/2021
        self.secondsPerTU = 1
        self.tw_interval = 15
        self.exclude_carer_travel = True
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
        self.carerWaitingTime = [] # MOVED FROM POST PROCESSED 03/06/2021
        self.carerTravelTime = [] # MOVED FROM POST PROCESSED 03/06/2021
        self.violatedTW = [] # NEW 03/06/2021
        self.carerWaitingMatrix = [] # NEW 03/06/2021
        self.carerTravelMatrix = [] # NEW 03/06/2021
        self.travelCostMatrix = [] # NEW 11/06/2021, cost of each trip for each pair of jobs, nJobs x nJobs, use od matrix, cost in pounds. (could also be called od_cost!)
        # self.odMileage = [] # NEW 11/06/2021, nJobs x nJobs, od matrix but with mileage instead od time in minutes
        # self.odMileageCost = [] # NEW 11/06/2021, nJobs x nJobs, cost of mileage for each trip, so x 0.25p per mile
        self.totalsArray = [] # NEW: 04/06/2021, keeps (in order): totalTime, totalWaitingTime, totalTravelTime, totalServiceTime, totalTardiness, maxTardiness, totalMKTardiness, mk_allowed_tardiness,
        # totalOvertime, maxOvertime, minSpareTime, maxSpareTime, shortestDay, longestDay, ait_quality, mk_quality, wb_quality, paper_quality, quality.

        # Post-processed solution:
        self.carerRoute = [] # In post_process_solution, this will become equivalent to allNurseRoutes, with dimensions nCarers x nPositions(jobs), and so carerRoute[carer][position] = job.
        self.carerServiceTime = []
        self.carerTravelCost = [] # NEW 11/06/2021, total cost of all trips for that carer, 1 x nCarers
        # self.nurseMileageMatrix = [] # NEW 11/06/2021, mileage of each trip for each carer to each job (make sure to exclude to/from depot), nCarers x nJobs
        self.carerMileage = [] # NEW 11/06/2021, total mileage travelled by each carer, 1 x nCarers
        self.carerMileageCost = [] # NEW 11/06/2021, total cost of mileage by each carer, 1 x nCarers, multiply carerMileage by 0.25p per mile to get cost in pounds.
        self.carerTime = []
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
        self.nLateJobs = 0
        self.quality_measure = 'standard'
        self.c_quality = 0
        self.mk_mind = []
        self.mk_maxd = []
        self.totalDistance = 0
        self.totalDistanceJobsOnly = 0
        self.distCarers = []
        self.distCarersJobsOnly = []
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
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * carer_travel_from_depot
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * carer_travel_to_depot
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # unavail_matrix
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # carer_unavail
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
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # carerWaitingTime, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # carerTravelTime, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # violatedTW, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # carerWaitingMatrix, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # carerTravelMatrix, NEW 03/06/2021
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # totalsArray, NEW 04/06/2021
                        ctypes.c_int] # Random seed
    ### --- End def  __init__ --- ### 

    def init_job_and_carer_objects(self, client_df, carerday_df):
        # Create two lists containing objects of the classes JOBS and NURSES respectively.
        self.jobObjs = []
        for ii in range(self.nJobs):
            self.jobObjs.append(JOB())
            self.jobObjs[-1].ID = ii # Set the ID for the object just added to jobObjs to be the index of the object (its position in the jobObjs list).

        self.carerObjs = []
        for ii in range(self.nCarers):
            self.carerObjs.append(CARER())
            self.carerObjs[-1].ID = ii # Set the ID for the object just added to carerObjs to be the index of the object (its position in the carerObjs list).
        
        # Start with jobs:
        for j in range(self.nJobs):
            # self.jobObjs[j].ID = idict['tasks'].loc[j, 'client']
            self.jobObjs[j].ID = client_df.iloc[j]['client_id']
            # self.jobObjs[j].postcode = idict['tasks'].loc[j, 'postcode']
            self.jobObjs[j].postcode = client_df.iloc[j]['postcode']
            # pcidict = idict['tasks'].loc[j, 'postcode']
            # latlon = cpo_inst.find_postcode_latlon(pcidict)
            latlon = [client_df.iloc[j]['latitude'], client_df.iloc[j]['longitude']]
            self.jobObjs[j].location = GEOPOINT(float(latlon[0]), float(latlon[1]))
            # self.jobObjs[j].serviceTime = idict['tasks'].loc[j, 'duration']
            self.jobObjs[j].serviceTime = client_df.iloc[j]['duration']
            self.jobObjs[j].hasTimewindow = True
            # jtw_start = idict['tasks'].loc[j, 'tw_start']
            # jtw_end = idict['tasks'].loc[j, 'tw_end']
            jtw_start = client_df.iloc[j]['tw_start']
            jtw_end = client_df.iloc[j]['tw_end']
            self.jobObjs[j].timewindow = [jtw_start, jtw_end]
            self.jobObjs[j].hasPreferredTimewindow = False
            self.jobObjs[j].preferredTimewindow = [0, 24*3600]
            self.jobObjs[j].skillsRequired = []
            # self.jobObjs[j].doubleService = False # commented out 01/09/2021
            self.jobObjs[j].doubleService = self.doubleService[j] # Added 01/09/2021
            self.jobObjs[j].features = []
            self.jobObjs[j].preferences = []
            self.jobObjs[j].preferredCarers = []
            self.jobObjs[j].dependsOn = -1
            self.jobObjs[j].minimumGap = []
            self.jobObjs[j].maximumGap = []
            self.jobObjs[j].markedReport = 0
            self.jobObjs[j].markedWebsite = 0
        # End jobs

        for i in range(self.nCarers):
            # self.carerObjs[i].ID = idict['rota'].loc[i, 'carer']
            self.carerObjs[i].ID = carerday_df.iloc[i]['carer_id'] # NOTE: this could also be 'carer' instead of unique_id, but unique_id has the shift number
            # self.carerObjs[i].postcode = idict['rota'].loc[i, 'postcode']
            self.carerObjs[i].postcode = carerday_df.iloc[i]['postcode']
            # pcidict = idict['rota'].loc[i, 'postcode']
            # print(i, ': ', pcidict)
            latlon = [carerday_df.iloc[i]['latitude'], carerday_df.iloc[i]['longitude']]
            # print('latlon', latlon)
            # print(i, ': ', latlon)
            self.carerObjs[i].startLocation = GEOPOINT(float(latlon[0]), float(latlon[1]))
            # print('self.carerObjs[i].startLocation', self.carerObjs[i].startLocation)
            self.carerObjs[i].transportMode = 'car'
            # istart = idict['rota'].loc[i, 'start']
            # ifinish = idict['rota'].loc[i, 'finish']
            istart = carerday_df.iloc[i]['start']
            ifinish = carerday_df.iloc[i]['end']
            self.carerObjs[i].shiftTimes = [float(istart), float(ifinish)]
            self.carerObjs[i].maxWorking = float(carerday_df.iloc[i]['duration'])
            self.carerObjs[i].skills = []
            self.carerObjs[i].features = []
            self.carerObjs[i].preferences = []
            self.carerObjs[i].preferredJobs = []
            self.carerObjs[i].jobsToAvoid = []
        # exit(-1)
        self.xy = []
        self.xy.append(self.carerObjs[0].startLocation.longlat()) # why only the first carer [0] location? Note that coords appended are lon/lat, not lat/lon!
        for job in self.jobObjs:
            self.xy.append(job.location.longlat()) # Append all job coordinates - lon/lat, not lat/lon!

        # for j in range(self.nJobs):
        #     print('j: ', j, ' client_id: ', self.jobObjs[j].ID, ' tw: ', self.jobObjs[j].timewindow)
        # exit(-1)
    ### --- End def init_job_and_nurse_objects --- ###

    def fill_preferences(self):
        self.prefScore = np.zeros((self.nJobs, self.nCarers), dtype=np.float64) # Matrix, nJobs x nCarers (note that this is the only variable with jobxnurse, not nursexjob dimensions).
        for i,job in enumerate(self.jobObjs): # For each JOB object in the jobObjs list
            for j,carer in enumerate(self.carerObjs): # For each CARER object in the carerObjs list
                self.prefScore[i][j] = self.preference_score(job, carer) # Set the preference score of job i and carer j
    ### --- End def fill_preferences --- ###

    def preference_score(self, job, carer):
        # Function determines preference score of given job and given carer, where 'job' is a JOB object and 'carer' is a CARER object.
        carerOK = 0
        if len(job.preferredCarers) > 0: # If the job does have preferred carers
            carerOK = -1 # Penalise if there is someone on the list, but it's not met
            for pc in job.preferredCarers: # For each 'value' (string?) pc in preferredCarers list
                if pc.lower() == carer.ID.lower(): # If the 'pc' index matched the ID of the carer (carer called in as parameter), then it means that this given job would prefer to have this carer.
                    carerOK = 1 # carer is okay for this job!
                    break

        
        matchedPrefsJob = 0
        unmatchedPrefsJob = 0 # NOTE: Always zero??
        for pref in job.preferences: # For each value (job preference characteristic?) in job.preferences list
            if pref in carer.features: # If the preference characteristic is one of the carer's features, i.e. carer has the desired characteristic
                matchedPrefsJob = matchedPrefsJob + 1 # Increase preference score for job
            else:
                matchedPrefsJob = matchedPrefsJob - 1 # Decrease preference score for job

        matchedPrefsCarer = 0
        unmatchedPrefsCarer = 0 # NOTE: Always zero??
        for pref in carer.preferences: # For each value (carer preference characteristic?) in carer.preferences list
            if pref in job.features: # If the preference characteristic is one of the job's features, i.e. job has the desired characteristic
                matchedPrefsCarer = matchedPrefsCarer + 1 # Increase preference score for carer
            else:
                matchedPrefsCarer = matchedPrefsCarer - 1 # Decrease preference score for carer

        jobToAvoid = 0
        for jta in carer.jobsToAvoid: # For each value (job ID?) in carer.jobsToAvoid list
            if jta.lower() == job.ID.lower(): # If the ID of the current job is in the jobsToAvoid list, then jobToAvoid = -1
                jobToAvoid = -1
                break

        # Calculate preference score
        score = (
                self.lambda_1*carerOK +
                self.lambda_2*matchedPrefsJob + self.lambda_3*unmatchedPrefsJob + 
                self.lambda_4*matchedPrefsCarer + self.lambda_5*unmatchedPrefsCarer +
                self.lambda_6*jobToAvoid
                )

        return(score)
    ### --- End def preference_score --- ###    
               
    def solve(self, randomSeed=0, printAllCallData=False):
        if self.verbose > 0:
            print('Calling C function for a max. of ' + str(self.MAX_TIME_SECONDS) + ' seconds, random seed is ' + str(randomSeed) + '.')

        # Prepare some data: 
        self.mk_mind = np.asarray(self.mk_mind, dtype=np.int32)
        self.mk_maxd = np.asarray(self.mk_maxd, dtype=np.int32)

        self.nJobs = int(self.nJobs)
        self.nCarers = int(self.nCarers)
        self.nSkills = int(self.nSkills)
        self.od = np.ascontiguousarray(self.od)
        self.carer_travel_from_depot = np.ascontiguousarray(self.carer_travel_from_depot)
        self.carer_travel_to_depot = np.ascontiguousarray(self.carer_travel_to_depot)
        self.unavail_matrix = np.ascontiguousarray(self.unavail_matrix.reshape(-1), dtype=np.int32)
        self.carer_unavail = np.ascontiguousarray(self.carer_unavail, dtype=np.int32)	
        self.carerWorkingTimes = np.ascontiguousarray(self.carerWorkingTimes, dtype=np.int32)
        self.jobTimeInfo = np.ascontiguousarray(self.jobTimeInfo, dtype=np.int32)
        self.jobSkillsRequired = np.ascontiguousarray(self.jobSkillsRequired, dtype=np.int32)
        self.carerSkills = np.ascontiguousarray(self.carerSkills, dtype=np.int32)
        self.solMatrix = np.ascontiguousarray(self.solMatrix, dtype=np.int32)
        self.doubleService = np.ascontiguousarray(self.doubleService, dtype=np.int32)		
        self.dependsOn = np.ascontiguousarray(self.dependsOn, dtype=np.int32)
        self.mk_mind = np.ascontiguousarray(self.mk_mind, dtype=np.int32)
        self.mk_maxd = np.ascontiguousarray(self.mk_maxd, dtype=np.int32)
        self.capabilityOfDoubleServices = np.ascontiguousarray(self.capabilityOfDoubleServices.reshape(-1))
        self.prefScore = np.ascontiguousarray(self.prefScore)
        self.algorithmOptions = np.ascontiguousarray(self.algorithmOptions)
        self.timeMatrix = np.ascontiguousarray(self.timeMatrix) # NEW 03/06/2021
        self.carerWaitingTime = np.ascontiguousarray(self.carerWaitingTime) # NEW 03/06/2021
        self.carerTravelTime = np.ascontiguousarray(self.carerTravelTime) # NEW 03/06/2021
        self.violatedTW = np.ascontiguousarray(self.violatedTW) # NEW 03/06/2021
        self.carerWaitingMatrix = np.ascontiguousarray(self.carerWaitingMatrix) # NEW 03/06/2021
        self.carerTravelMatrix = np.ascontiguousarray(self.carerTravelMatrix) # NEW 03/06/2021
        self.totalsArray = np.ascontiguousarray(self.totalsArray) # NEW 04/06/2021

        if	(self.verbose > 5 or printAllCallData):
            print('nJobs (type ' + str(type(self.nJobs)) + ')')
            print(self.nJobs)
            print('nCarers (type ' + str(type(self.nCarers)) + ')')
            print(self.nCarers)
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

            print('carer_travel_from_depot (type ' + str(type(self.carer_travel_from_depot)) + ')')
            print('dtype = ' + str(self.carer_travel_from_depot.dtype))
            print('Shape = ' + str(self.carer_travel_from_depot.shape))
            print(self.carer_travel_from_depot)

            print('carer_travel_to_depot (type ' + str(type(self.carer_travel_to_depot)) + ')')
            print('dtype = ' + str(self.carer_travel_to_depot.dtype))
            print('Shape = ' + str(self.carer_travel_to_depot.shape))
            print(self.carer_travel_to_depot)
        
            print('carerWorkingTimes (type ' + str(type(self.carerWorkingTimes)) + ')')
            print('dtype = ' + str(self.carerWorkingTimes.dtype))
            print('Shape = ' + str(self.carerWorkingTimes.shape))
            print(self.carerWorkingTimes)
            
            print('jobTimeInfo (type ' + str(type(self.jobTimeInfo)) + ')')
            print('dtype = ' + str(self.jobTimeInfo.dtype))
            print('Shape = ' + str(self.jobTimeInfo.shape))

            print(self.jobTimeInfo)
            
            print('jobSkillsRequired (type ' + str(type(self.jobSkillsRequired)) + ')')
            print('dtype = ' + str(self.jobSkillsRequired.dtype))
            print('Shape = ' + str(self.jobSkillsRequired.shape))

            print(self.jobSkillsRequired)
            
            print('carerSkills (type ' + str(type(self.carerSkills)) + ')')
            print('dtype = ' + str(self.carerSkills.dtype))
            print('Shape = ' + str(self.carerSkills.shape))

            print(self.carerSkills)
            
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
            
            print('\n ---------------- End of python data - start call to C ----------------\n\n\n ')

        self.fun(self.nJobs, self.nCarers, self.nSkills, self.verbose, self.MAX_TIME_SECONDS, self.tw_interval, self.exclude_carer_travel, self.od, self.carer_travel_from_depot, self.carer_travel_to_depot,
            self.unavail_matrix, self.carer_unavail, self.carerWorkingTimes, self.jobTimeInfo, self.jobSkillsRequired, self.carerSkills, self.solMatrix, self.doubleService, self.dependsOn,
            self.mk_mind, self.mk_maxd, self.capabilityOfDoubleServices, self.prefScore, self.algorithmOptions, self.timeMatrix, self.carerWaitingTime, self.carerTravelTime, self.violatedTW, 
            self.carerWaitingMatrix, self.carerTravelMatrix, self.totalsArray, randomSeed)

        if self.verbose > 10:
            print('Returned this matrix: ')
            print(self.solMatrix)
            print('Postprocessing...')
        # time.sleep(2)
        # print('Done.')
        self.post_process_solution()
    ### --- End def solve --- ###  

    def post_process_solution(self):
        # This function creates self.carerRoute, which is the same as allNurseRoutes[carer][position] = job.
        # Generate carer routes:
        # self.carerWaitingTime = np.zeros(self.nCarers)
        self.carerServiceTime = np.zeros(self.nCarers)
        # self.carerTravelTime = np.zeros(self.nCarers)
        self.carerTime = np.zeros(self.nCarers)
        self.totalWaitingTime = 0
        self.totalServiceTime = 0
        self.totalTravelTime = 0
        self.totalTime = 0
        self.carerRoute = []
        self.carerTravelCost = np.zeros(self.nCarers, dtype=np.float64) # NEW 11/06/2021, initialise the carerTravelCost array to 1 x nCarers
        self.carerMileage = np.zeros(self.nCarers, dtype=np.float64) # NEW 11/06/2021, initialise the carerMileage array to 1 x nCarers
        self.carerMileageCost = np.zeros(self.nCarers, dtype=np.float64) # NEW 11/06/2021, initialise the carerMileageCost array to 1 x nCarers
        # self.c_quality = np.min(self.solMatrix)

        for carer in range(self.nCarers):
            # howMany = 0
            try:
                howMany = max(self.solMatrix[carer]) + 1 # howMany = largest value in self.solMatrix[carer], which is equivalent to the number of jobs in the carer's route. Add one because job positions start at 0.
            except Exception as e:
                print('[ERROR]: in post_process_solution()')
                print(e)
                print('Nurse: ' + str(carer))
                print('solMatrix: \n' + str(self.solMatrix))
            
            # self.carerRoute.append(np.zeros(howMany)) # add array of size 'number of jobs' containing all zeros to the carerRoute list.
            self.carerRoute.append(np.full(howMany, -1)) # add array of size 'number of jobs' containing all -1 to the carerRoute list.
            for i,sp in enumerate(self.solMatrix[carer,:]): # For each index, value in solMatrix[carer], all jobs
                if sp > -1: # If > -1 then job i is in position sp of carer's route
                    self.carerRoute[carer][sp] = i # set carerRoute[carer][position] = job (like allNurseRoutes)
    ### --- End def post_process_solution --- ###

    def timemins_to_string(self, mins):
        minsRound = math.floor(mins)
        # print('minsRound: ', minsRound)
        hours = mins // 60
        # print('hours:', hours)
        minutes = (minsRound % 60)
        # print('minutes: ', minutes)
        seconds = (mins - minsRound) * 60
        seconds = round(seconds)

        if seconds == 60:
            seconds = 0
            minutes += 1
        # print('seconds: ', seconds)
        return('{0:0>2}:{1:0>2}:{2:0>2}'.format(int(hours), int(minutes), int(seconds)))
    ### --- End def timemins_to_string --- ###  

    def solution_to_website_dst(self, filename='unknown', add_plots=True):
        # Check if website generation is available:
        if (self.carerObjs[0].startLocation == []):
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
        for ni in range(self.nCarers):
            route_colour = clusterColour(ni) #Returns a hex colour.
            # Create a FeatureGroup layer; you can put things in it and handle them as a single layer.
            foliumRouteLayers.append(folium.map.FeatureGroup(name='Carer ' + str(ni))) # Create FeatureGroup for the current carer ni, add it to the list foliumRouteLayers.
            nRoute = []
            nRouteRev = []
            # nRoute.append(tuple(rxy[0]))

            niRoute = self.get_nurse_route_dst(ni) # carer route for carer ni, niRoute[position] = job.
            # print(niRoute)
            for pos in range(self.nJobs):
                job = int(niRoute[pos]) #job = the job at position pos in carer ni's route.
                if niRoute[pos] < -0.1: # If no job at position pos in carer ni's route, then carer ni has no route, no jobs assigned to carer ni, so break.
                    break
                nRoute.append(tuple(xyRev[job + 1])) #append as a tuple the lat/lon of job
                nRouteRev.append(reverse_latlong(xyRev[job + 1])) # append as a list the LON/LAT of the job (reversed, so lon/lat, not lat/lon!) NOTE: Why not just use xy??
                # Add a Marker:
                border_colour = route_colour
                popupVal = '<b>Client ID:</b> ' + str(self.jobObjs[job].ID)
                popupVal = popupVal + '<br><b>Job #:</b> ' + str(job) 
                popupVal += '<br><b>Postcode:</b> ' + str(self.jobObjs[job].postcode) 
                popupVal = popupVal + '<br><b>Time Window:</b> ' + self.timemins_to_string(self.jobTimeInfo[int(job)][0]) + ' - ' + self.timemins_to_string(self.jobTimeInfo[int(job)][1])
                popupVal = popupVal + '<br><b>Job Duration:</b> ' + self.timemins_to_string(self.jobObjs[job].serviceTime)
                if self.jobObjs[job].doubleService == 1:
                    if self.jobObjs[job].markedWebsite == 0:
                        popupVal = popupVal + '<br><b>Double Service</b>'
                        # Carer 1 information
                        popupVal = popupVal + '<br><b>Carer 1 ID:</b> ' + str(self.jobObjs[job].carerID[0])
                        popupVal = popupVal + '<br><b>Carer 1 #:</b> ' + str(self.jobObjs[job].assignedCarer[0])
                        popupVal = popupVal + '<br><b>Carer 1 Arrive:</b> ' + self.timemins_to_string(self.jobObjs[job].arrivalTime[0])
                        popupVal = popupVal + '<br><b>Carer 1 Start:</b> ' + self.timemins_to_string(self.jobObjs[job].startTime[0])
                        popupVal = popupVal + '<br><b>Carer 1 Depart:</b> ' + self.timemins_to_string(self.jobObjs[job].departureTime[0])
                        popupVal = popupVal + '<br><b>Carer 1 Waiting:</b> ' + self.timemins_to_string(self.jobObjs[job].waitingToStart[0])
                        if self.jobObjs[job].tardiness[0] > 0:
                            popupVal = popupVal + '<br><b>Carer 1 Tardiness:</b> ' + self.timemins_to_string(self.jobObjs[job].tardiness[0])
                            border_colour = '#ff0000'
                        popupVal = popupVal + '<br><b>Carer 1 Position #:</b> ' + str(self.jobObjs[job].positionInSchedule[0])
                        popupVal = popupVal + '<br><b>Carer 1 Travel to Job:</b>' + self.timemins_to_string(self.jobObjs[job].travelToJob[0])
                        
                        # Carer 2 information
                        popupVal = popupVal + '<br><b>Carer 2 ID:</b> ' + str(self.jobObjs[job].carerID[1])
                        popupVal = popupVal + '<br><b>Carer 2 #:</b> ' + str(self.jobObjs[job].assignedCarer[1])
                        popupVal = popupVal + '<br><b>Carer 2 Arrive:</b> ' + self.timemins_to_string(self.jobObjs[job].arrivalTime[1])
                        popupVal = popupVal + '<br><b>Carer 2 Start:</b> ' + self.timemins_to_string(self.jobObjs[job].startTime[1])
                        popupVal = popupVal + '<br><b>Carer 2 Depart:</b> ' + self.timemins_to_string(self.jobObjs[job].departureTime[1])
                        popupVal = popupVal + '<br><b>Carer 2 Waiting:</b> ' + self.timemins_to_string(self.jobObjs[job].waitingToStart[1])
                        if self.jobObjs[job].tardiness[1] > 0:
                            popupVal = popupVal + '<br><b>Carer 2 Tardiness:</b> ' + self.timemins_to_string(self.jobObjs[job].tardiness[1])
                            border_colour = '#ff0000'
                        popupVal = popupVal + '<br><b>Carer 2 Position #:</b> ' + str(self.jobObjs[job].positionInSchedule[1])
                        popupVal = popupVal + '<br><b>Carer 2 Travel to Job:</b>' + self.timemins_to_string(self.jobObjs[job].travelToJob[1])
                        self.jobObjs[job].markedWebsite += 1
                else:
                    popupVal = popupVal + '<br><b>Carer ID:</b> ' + str(self.jobObjs[job].carerID)
                    popupVal = popupVal + '<br><b>Carer #:</b> ' + str(self.jobObjs[job].assignedCarer)
                    popupVal = popupVal + '<br><b>Arrive:</b> ' + self.timemins_to_string(self.jobObjs[job].arrivalTime)
                    popupVal = popupVal + '<br><b>Start:</b> ' + self.timemins_to_string(self.jobObjs[job].startTime)
                    popupVal = popupVal + '<br><b>Depart:</b> ' + self.timemins_to_string(self.jobObjs[job].departureTime)
                    popupVal = popupVal + '<br><b>Waiting:</b> ' + self.timemins_to_string(self.jobObjs[job].waitingToStart)
                    popupVal = popupVal + '<br><b>Tardiness:</b> ' + self.timemins_to_string(self.jobObjs[job].tardiness)
                    popupVal = popupVal + '<br><b>Position #:</b> ' + str(self.jobObjs[job].positionInSchedule)
                    popupVal = popupVal + '<br><b>Travel to Job:</b>' + self.timemins_to_string(self.jobObjs[job].travelToJob)
                
                # border_colour = route_colour
                # if self.jobObjs[job].tardiness > 0:
                    # popupVal = popupVal + '<br><b>Tardiness:</b> ' + self.timemins_to_string(self.jobObjs[job].tardiness) 
                    # border_colour = '#ff0000' # red
                # if self.jobObjs[job].waitingToStart	> 0:
                    # popupVal = popupVal + '<br><b>Waiting:</b> ' + self.timemins_to_string(self.jobObjs[job].waitingToStart) 
                 
                # popupVal = popupVal + '<br><b>assignedCarer:</b> ' + str(self.jobObjs[idxNRpt].assignedCarer)
                # popupVal = popupVal + '<br><b>PositionInSchedule:</b> ' + str(self.jobObjs[job].positionInSchedule)
                # popupVal = popupVal + '<br><b>SkillsRequired:</b> ' + str(self.jobObjs[job].skillsRequired)
                # Add a circle around the area with popup:
                foliumRouteLayers[-1].add_child(folium.Circle(xyRev[job + 1], radius=30, popup=popupVal, color=border_colour, fill_color=route_colour, fill_opacity=0.5, fill=True))
            # End for pos in range(nJobs) loop
            if len(nRouteRev) < 1:
                continue 

            # Obtain the routeList, which is a list of coordinates of the route ([lat,lon]), and the duration (SECONDS) and distance (METRES) of the route.
            # print('self.carerObjs[ni].startLocation.longlat()', self.carerObjs[ni].startLocation.longlat())
            # print('self.carerObjs[ni].startLocation.longlat()', self.carerObjs[ni].startLocation.longlat())
            # print('nRouteRev', nRouteRev)
            routeList, dist, dur, distjobs, durjobs = route_points_osrm(self.carerObjs[ni].startLocation.longlat(), self.carerObjs[ni].startLocation.longlat(), goingThrough=nRouteRev)
            distJobsMiles = metres_to_miles(distjobs) # get distance for this carer in miles, not metres
            self.carerMileage[ni] = distJobsMiles
            self.carerMileageCost[ni] = distJobsMiles * 0.25
            self.totalDistance = self.totalDistance + dist
            self.totalDistanceJobsOnly = self.totalDistanceJobsOnly + distjobs
            self.distCarers.append(dist)
            self.distCarersJobsOnly.append(distjobs)
            foliumRouteLayers[-1].add_child(folium.PolyLine(routeList, color=route_colour, weight=2.5, opacity=1)) # Add a coloured line along the route onto the layer.
            m.add_child(foliumRouteLayers[-1]) # Add the layer onto the map m.
        # End for ni in range(nCarers) loop
        
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
                # &nbsp; Cool Legend <br>
                #               &nbsp; East &nbsp; <i class="fa fa-map-marker fa-2x" style="color:green"></i><br>
                #               &nbsp; West &nbsp; <i class="fa fa-map-marker fa-2x" style="color:red"></i>
        lht = lht + '''&nbsp; <b><u>Solution Summary: DST</u></b><br>'''
        lht = lht + '''&nbsp; <b>Total time: </b>''' + self.timemins_to_string(self.totalTime) + ''', of which:<br>'''
        lht = lht + '''&nbsp; <i> - Service time: </i>''' + self.timemins_to_string(self.totalServiceTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Travel time: </i>''' + self.timemins_to_string(self.totalTravelTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Waiting time: </i>''' + self.timemins_to_string(self.totalWaitingTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Total distance: </i>''' + str(self.totalDistance/1000) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Total distance jobs: </i>''' + str(self.totalDistanceJobsOnly/1000) + '''<br><br>'''

        carerPart = '''&nbsp; <b><u>Carer breakdown:</u> </b><br>'''
        for i, cc in enumerate(self.carerObjs):
            carerPart = carerPart + '''<br>&nbsp; <b>Carer ''' + str(i) + ' (' + str(cc.ID) + '''):</u> </b><br>'''
            carerPart = carerPart + '''&nbsp; <i>Skills: </i>''' + str(cc.skills) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Shift start time: </i>''' + self.timemins_to_string(self.carerWorkingTimes[i][0]) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Shift end time: </i>''' + self.timemins_to_string(self.carerWorkingTimes[i][1]) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Duration of shift: </i>''' + self.timemins_to_string(self.carerWorkingTimes[i][2]) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Actual start time: </i>''' + self.timemins_to_string(cc.startTime) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Actual end time: </i>''' + self.timemins_to_string(cc.finishTime) + '''<br>'''
            cc.route = list(self.carerRoute[i][:])
            carerPart = carerPart + '''&nbsp; <i>Number of services: </i>''' + str(len(cc.route)) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Total service time: </i>''' + self.timemins_to_string(self.carerServiceTime[i]) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Total travel time: </i>''' + self.timemins_to_string(self.carerTravelTime[i]) + '''<br>'''
            carerPart = carerPart + '''&nbsp; <i>Total waiting time: </i>''' + self.timemins_to_string(self.carerWaitingTime[i]) + '''<br>'''
            if len(cc.route) > 0:
                carerPart = carerPart + '''&nbsp; <i>Service route: </i>[''' + str(self.jobObjs[int(cc.route[0])].ID)
                for kkk in range(1,len(cc.route)):
                    jobbb = self.jobObjs[int(cc.route[kkk])]
                    if jobbb.doubleService:
                        carerPart = carerPart + ', (' + str(jobbb.ID) + ')'
                    else:
                        carerPart = carerPart + ', ' + str(jobbb.ID)

                carerPart = carerPart + ''']<br>'''
    
        lht = lht + carerPart

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

        # Depot, change to one per carer!
        for carerj in range(self.nCarers):
            carer_popup = 'Start location for carer ' + str(carerj)
            # print(self.carerObjs[nursej].startLocation)
            folium.Circle(self.carerObjs[carerj].startLocation.latlong(), radius=50, popup=carer_popup, color='black', fill_color='black', fill_opacity=0.5, fill=True).add_to(m)
        
        m.add_child(folium.map.LayerControl())
        m.save(webFilename)
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
        # plt.title('Total time for ' + str(self.nCarers) + ' nurses: ' + self.timemins_to_string(self.totalTime))
        # plt.axis('equal')
        # plt.draw()

        fig = plt.figure()
        fig.suptitle(self.area + ' ' + str(self.date) + ': DST', fontsize=13, fontweight='bold')
        ax = fig.add_subplot(111)
        # fig.subplots_adjust(top=0.72)
        subtitle = 'Total time for ' + str(self.nCarers) + ' carers: ' + self.timemins_to_string(self.totalTime)
        subtitle = '\nTravel Time: ' + self.timemins_to_string(self.totalTravelTime) + '  Service Time: ' + self.timemins_to_string(self.totalServiceTime) + '  Waiting Time: ' + self.timemins_to_string(self.totalWaitingTime)
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
        plt.show()
        # exit(-1)
    ### --- End def plot_pie_time_spent_dst --- ###

    def plot_bar_time_per_nurse_dst(self):
        xpos = np.arange(self.nCarers)
        width = 0.35

        p1 = plt.bar(xpos, self.carerServiceTime, width, color='#f998c5')
        p2 = plt.bar(xpos, self.carerTravelTime, width, bottom=self.carerServiceTime, color='#98d4f9')
        p3 = plt.bar(xpos, self.carerWaitingTime, width, bottom=(self.carerServiceTime + self.carerTravelTime), color='#a6e781')

        plt.xlabel('Carer')
        plt.ylabel('Time')
        plt.title(self.area + ' ' + str(self.date) + ': DST', fontsize=13, fontweight='bold')
        ticksNames = []
        for i in xpos:
            ticksNames.append(str(self.carerObjs[i].ID))
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
        plt.show()
    ### --- End def plot_bar_time_per_nurse_dst --- ###

    def get_nurse_route_dst(self, ni):
        # carerRoute = array, all -1s, then if index p has value j, it means that job j is in position p in carer's route.
        carerRoute = np.zeros(self.nJobs)
        for ii in range(self.nJobs):
            carerRoute[ii] = -1

        for ii in range(self.nJobs):
            if (self.solMatrix[ni][ii] >= 0):
                carerRoute[self.solMatrix[ni][ii]] = ii

        return carerRoute
    ### --- End def get_nurse_route_dst --- ###

    def get_travel_time(self, i, j):
        return(self.od[int(i + 1)][int(j + 1)])
    ### --- End def get_travel_time --- ###

    def get_nurse_from_travel_time(self, i, j):
        return(self.carer_travel_from_depot[int(i)][int(j)])
    ### --- End def get_travel_time --- ###

    def get_nurse_to_travel_time(self, i, j):
        return(self.carer_travel_to_depot[int(i)][int(j)])
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

        carerID = -1

        for i in range(self.nCarers):
            job = -1
            prevJob = -1
            self.carerServiceTime[i] = 0
            # print('Nurse: ', i)
            carerID = self.carerObjs[i].ID
            for p in range(len(self.carerRoute[i])): # For each POSITION
                costOfTravel = 0
                job = self.carerRoute[i][p] # job = job at position p in carer i's route
                # print('job: ', job)
                self.carerServiceTime[i] += self.jobTimeInfo[job][2]
                if p == 0: # If this job is the first job for this carer, then the carer arrives at the start of the job, no waiting time.
                    arriveAt = self.timeMatrix[i][job]
                    startAt = arriveAt
                else:
                    arriveAt = readyToNext
                    startAt = self.timeMatrix[i][job]
                currentTime = startAt + self.jobTimeInfo[job][2] # currentTime = time that carer i finishes job j (start time of job + duration of job)
                leaveAt = currentTime # Nurse i leaves job j at this time.
                if p < (len(self.carerRoute[i])-1): # if this job is NOT the last job (i.e. in the last position) of carer i's route
                    nextJob = self.carerRoute[i][p+1]
                    readyToNext = leaveAt + self.carerTravelMatrix[i][nextJob] #time at which carer i arrives at the next job
                    # if p == (len(self.carerRoute[i]) - 2):
                    # print('nextJob: ', nextJob, ' nurseTMnextJob: ', self.timemins_to_string(self.carerTravelMatrix[i][nextJob]), 'readyToNext: ', self.timemins_to_string(readyToNext), ' leaveAt: ' , self.timemins_to_string(leaveAt), ' todepotjob: ', self.timemins_to_string(self.get_nurse_to_travel_time(i, job)), ' todepot next: ', self.timemins_to_string(self.get_nurse_to_travel_time(i, nextJob)))
                prevJob = job
                if self.doubleService[job] == 1: # job is a double service
                    if self.jobObjs[job].markedReport == 0: # no information for either of the two nurses have been added to the job
                        self.jobObjs[job].carerID = [carerID]
                        self.jobObjs[job].assignedCarer = [i]
                        self.jobObjs[job].arrivalTime = [arriveAt]
                        self.jobObjs[job].startTime = [startAt]
                        self.jobObjs[job].departureTime = [leaveAt]
                        self.jobObjs[job].waitingToStart = [self.carerWaitingMatrix[i][job]]
                        self.jobObjs[job].tardiness = [self.violatedTW[job]]
                        self.jobObjs[job].positionInSchedule = [self.solMatrix[i][job]]
                        self.jobObjs[job].travelToJob = [self.carerTravelMatrix[i][job]]
                        self.jobObjs[job].markedReport += 1
                    elif self.jobObjs[job].markedReport == 1: # information for one of the two nurses has already been added to the job
                        self.jobObjs[job].carerID.append(carerID)
                        self.jobObjs[job].assignedCarer.append(i)
                        self.jobObjs[job].arrivalTime.append(arriveAt)
                        self.jobObjs[job].startTime.append(startAt)
                        self.jobObjs[job].departureTime.append(leaveAt)
                        self.jobObjs[job].waitingToStart.append(self.carerWaitingMatrix[i][job])
                        self.jobObjs[job].tardiness.append(self.violatedTW[job])
                        self.jobObjs[job].positionInSchedule.append(self.solMatrix[i][job])
                        self.jobObjs[job].travelToJob.append(self.carerTravelMatrix[i][job])
                        self.jobObjs[job].markedReport += 1
                    else:
                        print('[ERROR]: self.jobObjs[job].markedReport for job ', job, ' is ', self.jobObjs[job].markedReport)
                        print('Terminating program.')
                        exit(-1)
                else: # job is not a DS
                    self.jobObjs[job].carerID = carerID
                    self.jobObjs[job].assignedCarer = i
                    self.jobObjs[job].arrivalTime = arriveAt
                    self.jobObjs[job].startTime = startAt
                    self.jobObjs[job].departureTime = leaveAt
                    # self.jobObjs[job].serviceTime = self.jobTimeInfo[job][2] # Not needed here, this is already done in init_job_and_nurse_objects function.
                    self.jobObjs[job].waitingToStart = self.carerWaitingMatrix[i][job]
                    self.jobObjs[job].tardiness = self.violatedTW[job]
                    self.jobObjs[job].positionInSchedule = self.solMatrix[i][job]
                    self.jobObjs[job].travelToJob = self.carerTravelMatrix[i][job]
                    # if self.jobObjs[job].assignedCarer is list:
                        # self.jobObjs[job].assignedCarer.append(i)
                        # self.jobObjs[job].assignedCarer.append(carerID)
                    # else:
                        # self.jobObjs[job].assignedCarer = [i]
                        # self.jobObjs[job].assignedCarer = [carerID]
                costOfTravel = get_travel_cost(self.carerTravelMatrix[i][job])
                self.carerTravelCost[i] += costOfTravel
            # End for loop p
            if job > -1:
                travelTime = self.get_nurse_to_travel_time(i, job)
                finishShiftAt = leaveAt + travelTime
            else:
                finishShiftAt = self.carerWorkingTimes[i][0]
            self.carerObjs[i].finishTime = finishShiftAt
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
                            client_df.loc[i, 'carer_id'] = self.jobObjs[j].carerID[0]
                            client_df.loc[i, 'arrive_job'] = self.timemins_to_string(self.jobObjs[j].arrivalTime[0])
                            client_df.loc[i, 'start_job'] = self.timemins_to_string(self.jobObjs[j].startTime[0])
                            client_df.loc[i, 'depart_job'] = self.timemins_to_string(self.jobObjs[j].departureTime[0])
                            client_df.loc[i, 'waiting_time'] = self.timemins_to_string(self.jobObjs[j].waitingToStart[0])
                            client_df.loc[i, 'tardiness'] = self.timemins_to_string(self.jobObjs[j].tardiness[0])
                            client_df.loc[i, 'travel_time'] = self.timemins_to_string(self.jobObjs[j].travelToJob[0])
                            client_df.loc[i, 'position'] = self.jobObjs[j].positionInSchedule[0]

                            client_df.loc[i, 'carer_id2'] = self.jobObjs[j].carerID[1]
                            client_df.loc[i, 'arrive_job2'] = self.timemins_to_string(self.jobObjs[j].arrivalTime[1])
                            client_df.loc[i, 'start_job2'] = self.timemins_to_string(self.jobObjs[j].startTime[1])
                            client_df.loc[i, 'depart_job2'] = self.timemins_to_string(self.jobObjs[j].departureTime[1])
                            client_df.loc[i, 'waiting_time2'] = self.timemins_to_string(self.jobObjs[j].waitingToStart[1])
                            client_df.loc[i, 'tardiness2'] = self.timemins_to_string(self.jobObjs[j].tardiness[1])
                            client_df.loc[i, 'travel_time2'] = self.timemins_to_string(self.jobObjs[j].travelToJob[1])
                            client_df.loc[i, 'position2'] = self.jobObjs[j].positionInSchedule[1]
                            break
                        else:
                            client_df.loc[i, 'carer_id'] = self.jobObjs[j].carerID
                            client_df.loc[i, 'arrive_job'] = self.timemins_to_string(self.jobObjs[j].arrivalTime)
                            client_df.loc[i, 'start_job'] = self.timemins_to_string(self.jobObjs[j].startTime)
                            client_df.loc[i, 'depart_job'] = self.timemins_to_string(self.jobObjs[j].departureTime)
                            client_df.loc[i, 'waiting_time'] = self.timemins_to_string(self.jobObjs[j].waitingToStart)
                            client_df.loc[i, 'tardiness'] = self.timemins_to_string(self.jobObjs[j].tardiness)
                            client_df.loc[i, 'travel_time'] = self.timemins_to_string(self.jobObjs[j].travelToJob)
                            client_df.loc[i, 'position'] = self.jobObjs[j].positionInSchedule
                            break
        

        cwd = os.getcwd()
        solutiondfcsv_filename = self.fname + '_solution.csv'
        outputfiles_path = os.path.join(cwd, 'output')    
        client_df.to_csv(outputfiles_path + '/' + solutiondfcsv_filename)
        print('Saved solution to', solutiondfcsv_filename)
        # print(client_df)
        # exit(-1)
    ### --- End def solution_df_csv --- ###
### --- End class INSTANCE --- ###

def default_options_vector_type(measure=''):
    ov = np.zeros(100, dtype=np.float64)
    ov[1] = 0.0 # Quality meausre (might be modified automatically for MK, default: Ait H.)
    ov[1] = 1.0 # Two-opt active
    ov[2] = 1.0 # 2 exchange active
    ov[3] = 0.0 # Nurse order change active (neighbourhood in local search)
    ov[4] = 0.05 # GRASP delta low
    ov[5] = 0.25 # GRASP delta range
    ov[6] = 1.0 # Nurse order change active (In GRASP, between calls)
    ov[7] = 1.0 # performPathRelinking
    ov[8] = 10.0 # Solutions in pool
    ov[9] = 1.0 # Binary, perform path relinking for every solution with one random solution in the pool
    ov[10] = 1.0 # GRASP: RCL strategy (1 or 2)

    # Weights of objective function (for "paper" measure)  
    # $\alpha_1$: Travel Time:
    # ait_h: -0.3*60, mk: -1/3*60, paper: -1
    # $\alpha_2$: Waiting Time:
    # ait_h: 0, mk: 0, paper: -1
    # $\alpha_3$: Tardiness:
    # ait_h: INF, mk: -1/3*60, paper: -5
    # $\alpha_4$: Overtime:
    # ait_h: INF, mk: 0, paper: -5
    # $\alpha_5$: Workload Balance:
    # ait_h: 0, mk: 0, paper: 0.5
    # $\alpha_6$: Preference Score:
    # ait_h: -1, mk: 0, paper: 1
    # $\alpha_7$: Maximum Tardiness
    # ait_h: INF, mk: -1/3*60, paper: 0

    if measure == 'ait_h':
        ov[0] = 0.0
        ov[50] = 1 # 1 if tardiness and overtime are infeasible, 0 if feasible
        ov[51] = -0.3*60 # alpha_1 Travel time
        ov[52] = 0 # alpha_2 Waiting time
        ov[53] = 0 # alpha_3 Tardiness
        ov[54] = 0 # alpha_4 Overtime
        ov[55] = 0 # alpha_5 Workload balance
        ov[56] = -1 # alpha_6 Preference score
        ov[57] = 0 # alpha_7 Max tardiness (allowed))
    elif measure == 'mankowska':
        ov[0] = 1.0
        ov[50] = 0 # 1 if tardiness and overtime are infeasible, 0 if feasible
        ov[51] = -1/3*60 # alpha_1 Travel time
        ov[52] = 0 # alpha_2 Waiting time
        ov[53] = -1/3*60 # alpha_3 Tardiness
        ov[54] = 0 # alpha_4 Overtime
        ov[55] = 0 # alpha_5 Workload balance
        ov[56] = 0 # alpha_6 Preference score
        ov[57] = -1/3*60 # alpha_7 Max tardiness (not in paper)
        ov[12] = 0 # Use gap (1) or precedence (0)
    elif measure == 'workload_balance' or measure == 'balanced':
        ov[0] = 5.0
        ov[50] = 0 # 1 if tardiness and overtime are infeasible, 0 if feasible
        ov[51] = -1.0 # alpha_1 Travel time
        ov[52] = -1.0 # alpha_2 Waiting time
        ov[53] = -5.0 # alpha_3 Tardiness
        ov[54] = -5.0 # alpha_4 Overtime
        ov[55] = 0.50 # alpha_5 Workload balance
        ov[56] = 1 # alpha_6 Preference score
        ov[57] = 0 # alpha_7 Max tardiness (not in paper)
    elif measure == 'paper':
        ov[0] = 6.0
        ov[50] = 0 # 1 if tardiness and overtime are infeasible, 0 if feasible
        ov[51] = -1.0 # alpha_1 Travel time
        ov[52] = -1.0 # alpha_2 Waiting time
        ov[53] = -5.0 # alpha_3 Tardiness
        ov[54] = -5.0 # alpha_4 Overtime
        ov[55] = 0.50 # alpha_5 Workload balance
        ov[56] = 1 # alpha_6 Preference score
        ov[57] = 0 # alpha_7 Max tardiness (not in paper)
    else:
        print('No measure type given: cannot create defaults for options_vector.')
        print('Terminating program.')
        exit(-1)

    # ov[99] = 0.0 # print all input data
    return ov
### --- End def default_options_vector --- ###

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

    ov[99] = 0.0 # print all input data
    return ov
### --- End def default_options_vector --- ###

def metres_to_miles(metres):
    #miles = metres * 0.00062137
    if np.isnan(metres):
        # return distmiles
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
    # print('temp_string_coord:', temp_string_coord)
    string_coord = ','.join(temp_string_coord)
    # print('string_coord: ', string_coord)

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
    # print(str_call)
    r = requests.get(str_call)
    osrm_result = r.json()
    # print(osrm_result)
    # exit(-1)
    # print(osrm_result)
    geoTemp = osrm_result['routes'][0]['geometry']['coordinates']

    routeList = []
    for coordPair in geoTemp:
        routeList.append([float(coordPair[1]), float(coordPair[0])])
    dist = osrm_result['routes'][0]['distance']
    dur = osrm_result['routes'][0]['duration'] # NOTE: THIS IS IN SECONDS!! NEED TO CHANGE TO MINUTES.
    firstlegdist = osrm_result['routes'][0]['legs'][0]['distance'] # distance from carer's home to first job
    lastlegdist = osrm_result['routes'][0]['legs'][-1]['distance'] # distance from last job back to carer's home
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

