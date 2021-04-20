# New instance_handler.py file for convert_dict_inst.py, joined together
#28/12/2020
import os
import osrm
import math
import time
import ctypes
import folium
import pickle
import datetime
import requests # For osrm API
import subprocess
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from math import sqrt
from datetime import timedelta
from numpy.ctypeslib import ndpointer

import mankowska_data as Mk
import convert_dict_inst as cdi
import tools_and_scripts.class_cpo_df as ccd

# Display all rows and columns of dataframes in command prompt:
pd.set_option("display.max_rows", None, "display.max_columns", None)

def create_solve_inst(idict, options_vector, random_seed):
    #New function to call test instance
    # inst = cdi.convert_dict_inst(idict, options_vector)
    mkVNS = False
    file_to_run = 'something.mat'
    
    inst = INSTANCE()
    cpo_inst = ccd.CPO_DF()

    if mkVNS: # For some reason an error occurs (cannot call inst.solve()) without this if statement, so just left it in.
        inst = Mk.generate_Mk(inst, matfile=file_to_run, filetype='large_vns')
    else:
        inst = convert_dict_inst(inst, idict, cpo_inst)
    inst.init_job_and_nurse_objects(idict, cpo_inst)

    inst.algorithmOptions = options_vector
    # inst.algorithmOptions[0] = 0.0 # ait_h
    # inst.algorithmOptions[0] = 1.0 # mankowska
    inst.algorithmOptions[0] = 5.0 # workload_balance
    # inst.algorithmOptions[0] = 6.0 # paper
    saved_od_data = inst.od[0][0]
    inst.solve(randomSeed=random_seed, printAllCallData=False)
    inst.solve = []
    inst.lib = []
    inst.fun = []
    inst.Cquality = -1*inst.od[0][0] # Cquality = solution quality from C program, it's stored in od[0][0] (shortcut, should probably change this!)
    inst.od[0][0] = saved_od_data

    return inst
### --- End def create_solve_inst --- ###

def convert_dict_inst(inst, idict, cpo_inst):
    # This function creates an instance 'inst' of the class 'INSTANCE', and initialises inst with data from our idict dictionary instance created by analyse_mileage.py.
    # This function takes a dictionary instance 'idict' as input, and returns the object 'inst'.

    # cpo_inst = ccd.CPO_DF()
    inst.name = idict['name']
    inst.fname = idict['fname']
    inst.area = idict['area']
    inst.date = idict['date']
    inst.nNurses = idict['stats']['ncarers']
    inst.nJobs = idict['stats']['ntasks']
    inst.nSkills = 5 # NOTE: this is a random number just for testing.

    inst.nurseWorkingTimes = np.zeros((inst.nNurses, 3), dtype=np.int32) # nurseWorkingTimes is nNurses x 3, col[0] = start time, col[1] = finish time, col[2] = max working time.
    for i in range(inst.nNurses): #range(len(idict['rota']['start']))
        inst.nurseWorkingTimes[i][0] = idict['rota']['start'][i]
        inst.nurseWorkingTimes[i][1] = idict['rota']['finish'][i]
        # inst.nurseWorkingTimes[i][0] = idict['rota']['home_start'][i]
        # inst.nurseWorkingTimes[i][1] = idict['rota']['home_finish'][i]
        # We dont have max working time, so we need to calculate the duration of each carer's shift (total minutes that each carer works during the day)
        maxWorkingTime = idict['rota']['finish'][i] - idict['rota']['start'][i]
        inst.nurseWorkingTimes[i][2] = maxWorkingTime
    
    inst.nurseSkills = np.ones((inst.nNurses, inst.nSkills), dtype=np.int32) # Note: this will only work if nSkills > 0.

    inst.jobTimeInfo = np.zeros((inst.nJobs, 3), dtype=np.int32) # jobTimeInfo is nJobs x 3, col[0] = startTW, col[1] = endTW, col[2] = length of job (time)
    for i in range(inst.nJobs): #range(len(idict['tasks']['duration']))
        inst.jobTimeInfo[i][0] = idict['tasks']['tw_start'][i]
        inst.jobTimeInfo[i][1] = idict['tasks']['tw_end'][i]
        inst.jobTimeInfo[i][2] = idict['tasks']['duration'][i]

    inst.jobSkillsRequired = np.ones((inst.nJobs, inst.nSkills), dtype=np.int32) # Note: this will only work if nSkills > 0.
    inst.prefScore = np.zeros((inst.nJobs, inst.nNurses), dtype=np.float64)
    inst.doubleService = np.zeros(inst.nJobs, dtype=np.int32)
    # inst.doubleService[2] = 1 # NOTE: just for testing, make one of the jobs a double service.
    inst.dependsOn = np.full(inst.nJobs, -1, dtype=np.int32) # Set all jobs to -1, means no jobs are dependent on one another.
    inst.algorithmOptions = np.zeros(100, dtype=np.float64)

    inst.od = np.zeros((inst.nJobs+1, inst.nJobs+1), dtype=np.float64) # TIME IN MINUTES, THIS WILL BE USED IN C
    osrm_table_request(inst, idict, cpo_inst, 'od')
    # print(inst.od[:5,:5])
    
    inst.nurse_travel_from_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From carer's home to job - TIME IN MINS, THIS WILL BE USED IN C
    # osrm_table_request(inst, idict, cpo_inst, 'nursefrom')

    inst.nurse_travel_to_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From job to carer's home - TIME IN MINS, THIS WILL BE USED IN C
    # osrm_table_request(inst, idict, cpo_inst, 'nurseto')
    
    #Note: this will not work as currently inst.doubleService is empty, and so nDS = 0 - third dimension of capabilityOfDS cannot be zero.
    nDS = np.sum(inst.doubleService) # Number of jobs that are double services, NOTE: in testing, this is equal to one as we've set a job (job [2]) to be a double service.
    inst.capabilityOfDoubleServices = np.ones((inst.nNurses, inst.nNurses, nDS), dtype=np.int32) # NOTE: just for testing, setting it to all ones so that every pair of nurses is capable of doing the double service.

    inst.mk_mind = np.zeros(inst.nJobs+1, dtype=np.int32) #nJobs+1 because that's what is taken in by C, mk_mind[i] = mk_mind_data[i+1]
    inst.mk_maxd = np.zeros(inst.nJobs+1, dtype=np.int32)

    inst.lambda_1 = 1
    inst.lambda_2 = 1
    inst.lambda_3 = 1
    inst.lambda_4 = 1
    inst.lambda_5 = 1
    inst.lambda_6 = 10

    inst.qualityMeasure = 'paper'
    # inst.qualityMeasure = 'mankowska'

    # For these variables - do we need to change them?
    inst.solMatrix = np.zeros((inst.nNurses, inst.nJobs), dtype=np.int32)
    inst.MAX_TIME_SECONDS = 600
    inst.verbose = 1
    # inst.secondsPerTU = 1 # What is this variable?
    inst.DSSkillType = 'strictly-shared'
    # inst.name = 'solution0'
    # inst.full_file_name = 'solution0'
    # inst.loadFromDisk = False
    # inst.mankowskaQuality = -1
    # inst.Cquality = -1
    # inst.xy = [] # Unsure

    return inst
### --- End of def convert_dict_inst --- ###

def osrm_table_request(inst, idict, cpo_inst, matrix='none'):
    # For osrm, the coordinates need to be in string form, and in the order 'longitude,latitude'. Our data is in 'lat,lon', so create_coord_string function swaps it around.

    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']
    area = idict['area']

    lonlat_jobs = []
    for i in range(nJobs):
        pcidict = idict['tasks'].loc[i, 'postcode']
        lonlat = cpo_inst.find_postcode_lonlat(pcidict) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        lonlat_jobs.append(lonlat)
    lljSize = len(lonlat_jobs)
        
    lonlat_nurses = []
    for i in range(nNurses):
        pcidict = idict['rota'].loc[i, 'postcode']
        lonlat = cpo_inst.find_postcode_lonlat(pcidict) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
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
        # od = np.zeros((5, 5), dtype=np.float64)
        # for i in range(len(durations)):
        #     for j in range(len(durations[i])):
        #         time_seconds = durations[i][j]
        #         time_mins = time_seconds/60
        #         inst.od[i+1][j+1] = time_mins
        #         # od[i+1][j+1] = time_mins

        # NOTE: NEW, making adjustments for osrm to abicare for each area. Remove nested for loops above, replace with these things.
        if area == 'Hampshire':
            for i in range(len(durations)):
                for j in range(len(durations[i])):
                    time_seconds = durations[i][j]
                    time_mins = time_seconds/60
                    time_mins_adjust = 1.078*time_mins + 1.203 # HAMPSHIRE
                    inst.od[i+1][j+1] = time_mins_adjust
        elif area == 'Monmouth':
            for i in range(len(durations)):
                for j in range(len(durations[i])):
                    time_seconds = durations[i][j]
                    time_mins = time_seconds/60
                    time_mins_adjust = 0.948*time_mins + 0.875 # MONMOUTH
                    inst.od[i+1][j+1] = time_mins_adjust
        elif area == 'Aldershot':
            for i in range(len(durations)):
                for j in range(len(durations[i])):
                    time_seconds = durations[i][j]
                    time_mins = time_seconds/60
                    time_mins_adjust = 1.165*time_mins + 0.59 # ALDERSHOT
                    inst.od[i+1][j+1] = time_mins_adjust
        else:
            print('ERROR: no matching area name in osrm_table_request. Exit.')
            exit(-1)

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
        # nurse_travel_from_depot = np.zeros((nNurses, nJobs), dtype=np.float64)
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.nurse_travel_from_depot[i][j] = time_mins
                # nurse_travel_from_depot[i][j] = time_mins
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
        # nurse_travel_to_depot = np.zeros((nNurses, nJobs), dtype=np.float64)
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.nurse_travel_to_depot[j][i] = time_mins
                # nurse_travel_to_depot[j][i] = time_mins
    # --- End nurse to matrix --- #
    else:
        print('ERROR: no matrix type specified for osrm_table_request.')
        print('Matrix type should be either \'od\', \'nursefrom\', or \'nurseto\'.')
        print('Terminating program.')
        exit(-1)
    # --- End else --- #
### --- End of def osrm__table_request --- ### 

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
        self.doubleService = False
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
        self.assignedNurse = -1
        self.positionInSchedule = 0
        self.tardiness = 0
        self.waitingToStart = 0
        self.arrivalTime = 0
        self.departureTime = 0
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
        self.nurse_travel_from_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.nurse_travel_to_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.xy = [] # x y coordinates for plotting routes on a map LAT-LONG
        self.solMatrix = []
        self.MAX_TIME_SECONDS = 30
        self.verbose = 5
        self.secondsPerTU = 1
        self.name = ''
        self.fname = ''
        self.area = ''
        self.date = ''
        self.full_file_name = 'solution0'
        self.loadFromDisk = False
        self.mankowskaQuality = -1
        self.Cquality = -1
        self.DSSkillType = 'shared-duplicated' # See overleaf document for details
        self.capabilityOfDoubleServices = []

        # Post-processed solution:
        self.nurseRoute = [] # In post_process_solution, this will become equivalent to allNurseRoutes, with dimensions nNurses x nPositions(jobs), and so nurseRoute[nurse][position] = job.
        self.nurseWaitingTime = []
        self.nurseServiceTime = []
        self.nurseTravelTime = []
        self.nurseTime = []
        self.totalWaitingTime = -1
        self.totalServiceTime = -1
        self.totalTravelTime = -1
        self.totalTardiness = -1
        self.totalTime = -1
        self.maxTardiness = -1
        self.nLateJobs = 0
        self.qualityMeasure = 'standard'
        self.c_quality = 0
        self.mk_mind = []
        self.mk_maxd = []
        self.totalDistance = 0
        self.totalDistanceJobsOnly = 0
        self.distNurses = []
        self.distNursesJobsOnly = []

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
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * od_data
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * nurse_travel_from_depot
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * nurse_travel_to_depot
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
                        ctypes.c_int] # Random seed
    ### --- End def  __init__ --- ### 

    def init_job_and_nurse_objects(self, idict, cpo_inst):
        # Create two lists containing objects of the classes JOBS and NURSES respectively.
        self.jobObjs = []
        for ii in range(self.nJobs):
            self.jobObjs.append(JOB())
            self.jobObjs[-1].ID = ii # Set the ID for the object just added to jobObjs to be the index of the object (its position in the jobObjs list).

        self.nurseObjs = []
        for ii in range(self.nNurses):
            self.nurseObjs.append(NURSE())
            self.nurseObjs[-1].ID = ii # Set the ID for the object just added to nurseObjs to be the index of the object (its position in the nurseObjs list).
        
        # Start with jobs:
        for j in range(self.nJobs):
            self.jobObjs[j].ID = idict['tasks'].loc[j, 'client']
            self.jobObjs[j].postcode = idict['tasks'].loc[j, 'postcode']
            pcidict = idict['tasks'].loc[j, 'postcode']
            latlon = cpo_inst.find_postcode_latlon(pcidict)
            self.jobObjs[j].location = GEOPOINT(float(latlon[0]), float(latlon[1]))
            self.jobObjs[j].serviceTime = idict['tasks'].loc[j, 'duration']
            self.jobObjs[j].hasTimewindow = True
            jtw_start = idict['tasks'].loc[j, 'tw_start']
            jtw_end = idict['tasks'].loc[j, 'tw_end']
            self.jobObjs[j].timewindow = [jtw_start, jtw_end]
            self.jobObjs[j].hasPreferredTimewindow = False
            self.jobObjs[j].preferredTimewindow = [0, 24*3600]
            self.jobObjs[j].skillsRequired = []
            self.jobObjs[j].doubleService = False
            self.jobObjs[j].features = []
            self.jobObjs[j].preferences = []
            self.jobObjs[j].preferredCarers = []
            self.jobObjs[j].dependsOn = -1
            self.jobObjs[j].minimumGap = []
            self.jobObjs[j].maximumGap = []
        # End jobs

        for i in range(self.nNurses):
            self.nurseObjs[i].ID = idict['rota'].loc[i, 'carer']
            self.nurseObjs[i].postcode = idict['rota'].loc[i, 'postcode']
            pcidict = idict['rota'].loc[i, 'postcode']
            # print(i, ': ', pcidict)
            latlon = cpo_inst.find_postcode_latlon(pcidict)
            # print(i, ': ', latlon)
            self.nurseObjs[i].startLocation = GEOPOINT(float(latlon[0]), float(latlon[1]))
            self.nurseObjs[i].transportMode = 'car'
            istart = idict['rota'].loc[i, 'start']
            ifinish = idict['rota'].loc[i, 'finish']
            self.nurseObjs[i].shiftTimes = [float(istart), float(ifinish)]
            self.nurseObjs[i].maxWorking = float(idict['rota'].loc[i, 'shift'])
            self.nurseObjs[i].skills = []
            self.nurseObjs[i].features = []
            self.nurseObjs[i].preferences = []
            self.nurseObjs[i].preferredJobs = []
            self.nurseObjs[i].jobsToAvoid = []

        self.xy = []
        self.xy.append(self.nurseObjs[0].startLocation.longlat()) # why only the first nurse [0] location? Note that coords appended are lon/lat, not lat/lon!
        for job in self.jobObjs:
            self.xy.append(job.location.longlat()) # Append all job coordinates - lon/lat, not lat/lon!
    ### --- End def init_job_and_nurse_objects --- ###

    def fill_preferences(self):
        self.prefScore = np.zeros((self.nJobs, self.nNurses), dtype=np.float64) # Matrix, nJobs x nNurses (note that this is the only variable with jobxnurse, not nursexjob dimensions).
        for i,job in enumerate(self.jobObjs): # For each JOB object in the jobObjs list
            for j,nurse in enumerate(self.nurseObjs): # For each NURSE object in the nurseObjs list
                self.prefScore[i][j] = self.preference_score(job, nurse) # Set the preference score of job i and nurse j
    ### --- End def fill_preferences --- ###

    def preference_score(self, job, nurse):
        # Function determines preference score of given job and given nurse, where 'job' is a JOB object and 'nurse' is a NURSE object.
        carerOK = 0
        if len(job.preferredCarers) > 0: # If the job does have preferred carers
            carerOK = -1 # Penalise if there is someone on the list, but it's not met
            for pc in job.preferredCarers: # For each 'value' (string?) pc in preferredCarers list
                if pc.lower() == nurse.ID.lower(): # If the 'pc' index matched the ID of the nurse (nurse called in as parameter), then it means that this given job would prefer to have this nurse.
                    carerOK = 1 # nurse is okay for this job!
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
                self.lambda_1*carerOK +
                self.lambda_2*matchedPrefsJob + self.lambda_3*unmatchedPrefsJob + 
                self.lambda_4*matchedPrefsNurse + self.lambda_5*unmatchedPrefsNurse +
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
        self.nNurses = int(self.nNurses)
        self.nSkills = int(self.nSkills)
        self.od = np.ascontiguousarray(self.od)
        self.nurse_travel_from_depot = np.ascontiguousarray(self.nurse_travel_from_depot)
        self.nurse_travel_to_depot = np.ascontiguousarray(self.nurse_travel_to_depot)
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
            
            print('\n ---------------- end of python data - start call to C ----------------\n\n\n ')

        # Call:
        self.fun(self.nJobs, self.nNurses, self.nSkills, self.verbose, self.MAX_TIME_SECONDS, self.od, self.nurse_travel_from_depot, self.nurse_travel_to_depot,
            self.nurseWorkingTimes, self.jobTimeInfo, self.jobSkillsRequired, self.nurseSkills, self.solMatrix, self.doubleService, self.dependsOn,
            self.mk_mind, self.mk_maxd, self.capabilityOfDoubleServices, self.prefScore, self.algorithmOptions, randomSeed)

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
        self.nurseWaitingTime = np.zeros(self.nNurses)
        self.nurseServiceTime = np.zeros(self.nNurses)
        self.nurseTravelTime = np.zeros(self.nNurses)
        self.nurseTime = np.zeros(self.nNurses)
        self.totalWaitingTime = 0
        self.totalServiceTime = 0
        self.totalTravelTime = 0
        self.totalTime = 0
        self.nurseRoute = []
        # self.c_quality = np.min(self.solMatrix)

        for nurse in range(self.nNurses):
            # howMany = 0
            try:
                howMany = max(self.solMatrix[nurse]) + 1 # howMany = largest value in self.solMatrix[nurse], which is equivalent to the number of jobs in the nurse's route. Add one because job positions start at 0.
            except Exception as e:
                print('ERROR in post_process_solution()')
                print(e)
                print('Nurse: ' + str(nurse))
                print('solMatrix: \n' + str(self.solMatrix))
            
            self.nurseRoute.append(np.zeros(howMany)) # add array of size 'number of jobs' containing all zeros to the nurseRoute list.
            for i,sp in enumerate(self.solMatrix[nurse,:]): # For each index, value in solMatrix[nurse], all jobs
                if sp > -1: # If > -1 then job i is in position sp of nurse's route
                    self.nurseRoute[nurse][sp] = i # set nurseRoute[nurse][position] = job (like allNurseRoutes)
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
        # print('seconds: ', seconds)
        return('{0:0>2}:{1:0>2}:{2:0>2}'.format(int(hours), int(minutes), int(seconds)))
    ### --- End def timemins_to_string --- ###  

    def simple_solution_plot(self, filename='none'):
        # Check if plots are available:
        if (self.nurseObjs[0].startLocation == []):
            for no in self.nurseObjs:
                no.startLocation = GEOPOINT(self.xy[0][1], self.xy[0][0])
            # print('*** WARNING: Plots not available for this instance. Skipping... ***')
            # return

        fig, ax = plt.subplots()
        ncount = -1
        nLegends = []
        for nursej in range(self.nNurses):
            ncount += 1
            nLegends.append('R' + str(ncount))
            nLegends.append('N' + str(ncount) + ', S:' + str(self.nurseSkills[ncount]))
            routeCol = clusterColour(nursej)
            # foliumRouteLayers.append(folium.map.FeatureGroup(name='Nurse ' + str(nursej)))
            nRoute = []
            nRouteRev = []

            nRoute.append(self.nurseObjs[nursej].startLocation.longlat())
            nr = self.get_nurse_route_dst(nursej)
            for pt in range(self.nJobs):
                if nr[pt] < -0.1:
                    break
                nRoute.append(self.xy[int(nr[pt] + 1)])
                # nRouteRev.append(reverse_latlong(rxy[int(nr[pt] + 1)]))
                # Add a Marker:
                # popupVal = 'Patient: ' + str(nr[pt]) + '<br>Service time: ' + str(self.jobTimeInfo[int(nr[pt])][2])
                # popupVal = popupVal + '<br>Nurse: ' + str(nursej)
                # foliumRouteLayers[-1].add_child(folium.CircleMarker(rxy[int(nr[pt] + 1)],
                # radius=10,
                # popup= popupVal,
                # color=routeCol,
                # fill_color=routeCol))

            nRoute.append(self.nurseObjs[nursej].startLocation.longlat())
            x,y = zip(*nRoute)
            ax.plot(x, y, 'ko', ms=10)
            ax.plot(x, y, routeCol)
            ax.plot(self.nurseObjs[nursej].startLocation.longlat()[0], self.nurseObjs[nursej].startLocation.longlat()[1], 'rs', ms=10)

        
        for	ptidx,pt in enumerate(self.xy):
            if ptidx > 0:
                textlabel = "P(%d) " % (ptidx - 1) #+ str(self.jobTimeInfo[ptidx - 1])
                if self.doubleService[ptidx - 1]:
                    textlabel += '\nDS ' + str(self.jobSkillsRequired[ptidx - 1])
            else:
                textlabel = 'DEPOT'
            plt.text(pt[0], pt[1], textlabel, rotation=0, verticalalignment='center',color='blue', fontsize=14)

        ax.legend(tuple(nLegends))
        ax.set_title("Routes for " + str(self.nNurses) + " nurses", fontsize=18)
        ax.grid(True, color='gray', alpha=0.5)
        ax.axis('equal')

        if filename == 'none':
            plt.show(block=False)
        else:
            plt.savefig(filename + '_route' + '.png', bbox_inches='tight')
    ### --- End def simple_solution_plot --- ###

    def solution_to_website_dst(self, filename='unknown'):
        # Check if website generation is available:
        if (self.nurseObjs[0].startLocation == []):
            print('*** WARNING: Website generation is not available for this instance. Skipping... ***')
            return
        webFilename = 'unknown.html'
        if filename == 'unknown':
            webFilename = self.fname + '_dst.html'
        webFilename = os.path.join(os.getcwd(), webFilename)
        print('Website filename: '  + str(webFilename))

        xyRev = [] # Reverse xy list, which has coords in [lat,lon] form, whereas xy has coords in [lon, lat] form.
        for coord in self.xy:
            xyRev.append(reverse_latlong(coord)) # Add coord (which is [lon, lat]) to xyRev but in [lat,lon] order.
        m = folium.Map(location=xyRev[0], zoom_start=14, tiles='cartodbpositron')
        
        foliumRouteLayers = []
        for ni in range(self.nNurses):
            route_colour = clusterColour(ni) #Returns a hex colour.
            # Create a FeatureGroup layer; you can put things in it and handle them as a single layer.
            foliumRouteLayers.append(folium.map.FeatureGroup(name='Nurse ' + str(ni))) # Create FeatureGroup for the current nurse ni, add it to the list foliumRouteLayers.
            nRoute = []
            nRouteRev = []
            # nRoute.append(tuple(rxy[0]))

            niRoute = self.get_nurse_route_dst(ni) # nurse route for nurse ni, niRoute[position] = job.
            for pos in range(self.nJobs):
                job = int(niRoute[pos]) #job = the job at position pos in nurse ni's route.
                if niRoute[pos] < -0.1: # If no job at position pos in nurse ni's route, then nurse ni has no route, no jobs assigned to nurse ni, so break.
                    break
                nRoute.append(tuple(xyRev[job + 1])) #append as a tuple the lat/lon of job
                nRouteRev.append(reverse_latlong(xyRev[job + 1])) # append as a list the LON/LAT of the job (reversed, so lon/lat, not lat/lon!) NOTE: Why not just use xy??
                # Add a Marker:
                popupVal = '<b>Service ID:</b> ' + str(self.jobObjs[job].ID)
                popupVal = popupVal + '<br><b>Service no.:</b> ' + str(job) 
                popupVal += '<br><b>Postcode:</b> ' + str(self.jobObjs[job].postcode) 
                if self.jobObjs[job].doubleService:
                    popupVal = popupVal + ' (Double service)'
                popupVal = popupVal + '<br><b>Assigned nurse:</b> ' + str(ni) 
                popupVal = popupVal + '<br><b>Arrival time:</b> ' + self.timemins_to_string(self.jobObjs[job].arrivalTime) 
                popupVal = popupVal + '<br><b>Departure Time:</b> ' + self.timemins_to_string(self.jobObjs[job].departureTime) 
                popupVal = popupVal + '<br><b>Time Window:</b> ' + self.timemins_to_string(self.jobTimeInfo[int(job)][0]) + ' - ' + self.timemins_to_string(self.jobTimeInfo[int(job)][1])
                border_colour = route_colour
                if 	self.jobObjs[job].tardiness > 0:
                    popupVal = popupVal + '<br><b>*** Missed by:</b> ' + self.timemins_to_string(self.jobObjs[job].tardiness) 
                    border_colour = '#ff0000'
                if self.jobObjs[job].waitingToStart	> 0:
                    popupVal = popupVal + '<br><b>Waiting before start:</b> ' + self.timemins_to_string(self.jobObjs[job].waitingToStart) 
                popupVal = popupVal + '<br><b>serviceTime:</b> ' + self.timemins_to_string(self.jobObjs[job].serviceTime) 
                # popupVal = popupVal + '<br><b>assignedNurse:</b> ' + str(self.jobObjs[idxNRpt].assignedNurse)
                popupVal = popupVal + '<br><b>positionInSchedule:</b> ' + str(self.jobObjs[job].positionInSchedule)
                popupVal = popupVal + '<br><b>skillsRequired:</b> ' + str(self.jobObjs[job].skillsRequired)
                # Add a circle around the area with popup:
                foliumRouteLayers[-1].add_child(folium.Circle(xyRev[job + 1], radius=30, popup=popupVal, color=border_colour, fill_color=route_colour, fill_opacity=0.5, fill=True))
            # End for pos in range(nJobs) loop

            # Obtain the routeList, which is a list of coordinates of the route ([lat,lon]), and the duration (SECONDS) and distance (METRES) of the route.
            routeList, dist, dur, distjobs, durjobs = route_points_osrm(self.nurseObjs[ni].startLocation.longlat(), self.nurseObjs[ni].startLocation.longlat(), goingThrough=nRouteRev)
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

        nursePart = '''&nbsp; <b><u>Nurse breakdown:</u> </b><br>'''
        for i, nn in enumerate(self.nurseObjs):
            nursePart = nursePart + '''<br>&nbsp; <b>Nurse ''' + str(i) + ' (' + str(nn.ID) + '''):</u> </b><br>'''
            nursePart = nursePart + '''&nbsp; <i>Skills: </i>''' + str(nn.skills) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Shift start time: </i>''' + self.timemins_to_string(self.nurseWorkingTimes[i][0]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Shift end time: </i>''' + self.timemins_to_string(self.nurseWorkingTimes[i][1]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Duration of shift: </i>''' + self.timemins_to_string(self.nurseWorkingTimes[i][2]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Actual start time: </i>''' + self.timemins_to_string(self.nurseWorkingTimes[i][0]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Actual end time: </i>''' + self.timemins_to_string(nn.finishTime) + '''<br>'''
            nn.route = list(self.nurseRoute[i][:])
            nursePart = nursePart + '''&nbsp; <i>Number of services: </i>''' + str(len(nn.route)) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Total service time: </i>''' + self.timemins_to_string(self.nurseServiceTime[i]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Total travel time: </i>''' + self.timemins_to_string(self.nurseTravelTime[i]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Total waiting time: </i>''' + self.timemins_to_string(self.nurseWaitingTime[i]) + '''<br>'''
            if len(nn.route) > 0:
                nursePart = nursePart + '''&nbsp; <i>Service route: </i>[''' + str(self.jobObjs[int(nn.route[0])].ID)
                for kkk in range(1,len(nn.route)):
                    jobbb = self.jobObjs[int(nn.route[kkk])]
                    if jobbb.doubleService:
                        nursePart = nursePart + ', (' + str(jobbb.ID) + ')'
                    else:
                        nursePart = nursePart + ', ' + str(jobbb.ID)

                nursePart = nursePart + ''']<br>'''
    
        lht = lht + nursePart

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
            folium.Circle(self.nurseObjs[nursej].startLocation.latlong(), radius=50, popup=nurse_popup, color='black', fill_color='black', fill_opacity=0.5, fill=True).add_to(m)
        
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
        # plt.title('Total time for ' + str(self.nNurses) + ' nurses: ' + self.timemins_to_string(self.totalTime))
        # plt.axis('equal')
        # plt.draw()

        fig = plt.figure()
        fig.suptitle(self.area + ' ' + self.date + ': DST', fontsize=13, fontweight='bold')
        ax = fig.add_subplot(111)
        # fig.subplots_adjust(top=0.72)
        subtitle = 'Total time for ' + str(self.nNurses) + ' nurses: ' + self.timemins_to_string(self.totalTime)
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
        plt.savefig(self.fname + '_time_info_dst' + '.png', bbox_inches='tight')
        plt.show()
    ### --- End def plot_pie_time_spent_dst --- ###

    def plot_bar_time_per_nurse_dst(self):
        xpos = np.arange(self.nNurses)
        width = 0.35

        p1 = plt.bar(xpos, self.nurseServiceTime, width, color='#f998c5')
        p2 = plt.bar(xpos, self.nurseTravelTime, width, bottom=self.nurseServiceTime, color='#98d4f9')
        p3 = plt.bar(xpos, self.nurseWaitingTime, width, bottom=(self.nurseServiceTime + self.nurseTravelTime), color='#a6e781')

        plt.xlabel('Carer')
        plt.ylabel('Time')
        plt.title(self.area + ' ' + self.date + ': DST', fontsize=13, fontweight='bold')
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
        plt.savefig(self.fname + '_workload_dst' + '.png', bbox_inches='tight')
        plt.show()
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

    def full_solution_report(self, report=2, doPlots=True):
        arriveAt = 0
        leaveAt = 0
        onlyTravelTime = 0
        self.totalWaitingTime = 0
        self.totalTardiness = 0
        self.totalTime = 0
        self.maxTardiness = 0
        self.nLateJobs = 0

        for i in range(self.nNurses): # For each nurse i = 0,...,nNurses
            job = -1
            prevJob = -1
            currentTime = self.nurseWorkingTimes[i][0] # Start time of current nurse i (start of shift)
            if (report > 1):
                print('Nurse ', i,  ' starts at (', currentTime, ') ', self.timemins_to_string(currentTime))
            self.nurseTravelTime[i] = 0
            self.nurseWaitingTime[i] = 0
            self.nurseServiceTime[i] = 0

            for j in self.nurseRoute[i]: # For each job j in nurseRoute[i] (allNurseRoutes[i])
                job = int(j) # job = the job number at position j in nurse i's route.
                # Trip from depot:
                # NOTE: Surely this should be the travel time from the depot if prevJob = -1? It'll just be 0, and the od matrix doesn't have values in the 0 column or row.
                travelTime = self.get_travel_time(prevJob, job) # self.od[int(prevJob + 1)][int(job + 1)]

                self.nurseTravelTime[i] = self.nurseTravelTime[i] + travelTime
                currentTime = currentTime + travelTime 
                self.nurseServiceTime[i] = self.nurseServiceTime[i] + self.jobTimeInfo[job][2] # Service time = duration of job (length of time job takes).
                arriveAt = currentTime # Time nurse arrives at the job.

                waitingTime = 0
                if (arriveAt < self.jobTimeInfo[job][0]): # If nurse arrives at the job before the start of the job time window
                    waitingTime = self.jobTimeInfo[job][0] - arriveAt # waiting time = start of job time window - arrival time
                self.nurseWaitingTime[i] = self.nurseWaitingTime[i] + waitingTime 

                tardiness = 0
                if (arriveAt > self.jobTimeInfo[job][1]): # If nurse arrives at the job after the end of the job time window
                    tardiness = arriveAt - self.jobTimeInfo[job][1] # tardiness = arrival time - end of job time window

                if tardiness > self.maxTardiness: # Update max tardiness.
                    self.maxTardiness = tardiness
                self.totalTardiness = self.totalTardiness  + tardiness # Update total tardiness.

                prevJob = job # Make prevJob be the current job.
                currentTime = currentTime + self.jobTimeInfo[job][2] + waitingTime # Update current time to be the time after the job has been completed.
                leaveAt = currentTime

                # SET JOB OBJECT INFO: self.jobObjs is a list of size 1 x nJobs of JOB objects.
                self.jobObjs[job].arrivalTime = arriveAt
                self.jobObjs[job].departureTime = leaveAt
                self.jobObjs[job].serviceTime = self.jobTimeInfo[job][2] # jobTimeInfo[job][2] = duration/length of job (mins)
                self.jobObjs[job].tardiness = tardiness
                self.jobObjs[job].waitingToStart = waitingTime
                if self.jobObjs[job].assignedNurse is list:
                    self.jobObjs[job].assignedNurse.append(i)
                else:
                    self.jobObjs[job].assignedNurse = [i]
                self.jobObjs[job].positionInSchedule = self.solMatrix[i][job]

                if (report > 1):                    
                    print('\tArrives at job ', job, ' at (', arriveAt, ') ', self.timemins_to_string(arriveAt), ' and leaves at (', leaveAt, ') ', self.timemins_to_string(leaveAt))
                    if (waitingTime > 0):
                        print('\t\tNeeds to wait for (', waitingTime, ') ', self.timemins_to_string(waitingTime), ' before starting the job.')
                    if (tardiness > 0):
                        print('\t\t*** Misses the time window by (', tardiness, ') ', self.timemins_to_string(tardiness), '! ***')
                        self.nLateJobs = self.nLateJobs + 1
            # End of for loop j in self.nurseRoute[i]    
             
            # Return to depot:
            if job > -1: # i.e. if nurseRoute[i] has jobs
                # NOTE: Surely this should be the travel time to the depot? 
                travelTime = self.get_travel_time(job, -1) # self.od[int(job + 1)][int(-1 + 1)] = # self.od[int(job + 1)][0]. Note that 0 columns/rows not used in od matrix, just zeros.
                self.nurseTravelTime[i] = self.nurseTravelTime[i] + travelTime
                finishShiftAt = leaveAt + travelTime
            else:
                finishShiftAt = self.nurseWorkingTimes[i][0] # if job = -1 it means that nurseRoute[i] has no jobs, and so the nurse finishes their shift at the start time of their shift, no work done.

            # Update totals:
            self.totalServiceTime = self.totalServiceTime + self.nurseServiceTime[i]
            onlyTravelTime = onlyTravelTime  + self.nurseTravelTime[i]
            self.totalWaitingTime = self.totalWaitingTime + self.nurseWaitingTime[i]

            self.nurseObjs[i].finishTime = finishShiftAt
            if (report > 1):
                print('\tFinishes at the depot at (', finishShiftAt, ') ', self.timemins_to_string(finishShiftAt))
                if (finishShiftAt > self.nurseWorkingTimes[i][1]):
                    print('\t\t*** This nurse is finishing late (by (', finishShiftAt-self.nurseWorkingTimes[i][1], ') ', self.timemins_to_string(finishShiftAt - self.nurseWorkingTimes[i][1]), ')\n')
        # End for loop i in range(self.nNurses)
        
        if (report > 0):
            print('\nTotal travel time: ', onlyTravelTime, ' (', self.timemins_to_string(onlyTravelTime), ')')
        self.totalTravelTime = onlyTravelTime

        self.totalTime = self.totalTravelTime + self.totalServiceTime + self.totalWaitingTime

        if self.qualityMeasure == 'mankowska':
            quality = (self.totalTravelTime + self.totalTardiness + self.maxTardiness)/3 # Mankowska
            print('Quality returned from C DLL: ', self.Cquality)
            print('Mankowska measure (python computed) = ', quality)
            print('\ttotalTravelTime = ', self.totalTravelTime)
            print('\ttotalTardiness = ', self.totalTardiness)
            print('\tmaxTardiness = ', self.maxTardiness)
        else:
            quality = -1000000 * self.totalTardiness - self.totalTime

        self.mankowskaQuality = quality
        # print('Solution quality: ' + self.time_to_string(quality))
        if report > 0:
            print('Computed quality: ', quality)
            print('From: totalTime = ', self.totalTime, '\ntotalTardiness = ', self.totalTardiness, '\nmaxTardiness = ', self.maxTardiness)
            print('Total time required to complete the solution: ', self.timemins_to_string(self.totalTime))
            if self.nLateJobs > 0:
                print('The solution is infeasible. Nurses arrive late to ', self.nLateJobs, ' services.')
                print('The total tardiness time is ', self.timemins_to_string(self.totalTardiness))
            else:
                print('All jobs are served on time. Nurses need to wait a total of ', self.timemins_to_string(self.totalWaitingTime), ' because of early arrivals.')
            
        if doPlots:
            self.plot_pie_time_spent_dst()
            self.plot_bar_time_per_nurse_dst()

        return quality
    ### --- End def full_solution_report --- ###
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
    ov[0] = 5.0 # Quality meausre (might be modified automatically for MK, default: workload balance.)
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

    geoTemp = osrm_result['routes'][0]['geometry']['coordinates']

    routeList = []
    for coordPair in geoTemp:
        routeList.append([float(coordPair[1]), float(coordPair[0])])
    dist = osrm_result['routes'][0]['distance']
    dur = osrm_result['routes'][0]['duration'] # NOTE: THIS IS IN SECONDS!! NEED TO CHANGE TO MINUTES.
    firstlegdist = osrm_result['routes'][0]['legs'][0]['distance']
    lastlegdist = osrm_result['routes'][0]['legs'][-1]['distance']
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

# def initialise_nurse_job(self, idict, cpo_inst):
    #     # cpo_inst = ccd.CPO_DF()
    #     # Start with jobs:
    #     for j in range(self.nJobs):
    #         self.jobObjs[j].ID = idict['tasks'].loc[j, 'client']
    #         self.jobObjs[j].postcode = idict['tasks'].loc[j, 'postcode']
    #         pcidict = idict['tasks'].loc[j, 'postcode']
    #         latlon = cpo_inst.find_postcode_latlon(pcidict)
    #         self.jobObjs[j].location = GEOPOINT(float(latlon[0]), float(latlon[1]))
    #         self.jobObjs[j].serviceTime = idict['tasks'].loc[j, 'duration']
    #         self.jobObjs[j].hasTimewindow = True
    #         jtw_start = idict['tasks'].loc[j, 'tw_start']
    #         jtw_end = idict['tasks'].loc[j, 'tw_end']
    #         self.jobObjs[j].timewindow = [jtw_start, jtw_end]
    #         self.jobObjs[j].hasPreferredTimewindow = False
    #         self.jobObjs[j].preferredTimewindow = [0, 24*3600]
    #         self.jobObjs[j].skillsRequired = []
    #         self.jobObjs[j].doubleService = False
    #         self.jobObjs[j].features = []
    #         self.jobObjs[j].preferences = []
    #         self.jobObjs[j].preferredCarers = []
    #         self.jobObjs[j].dependsOn = -1
    #         self.jobObjs[j].minimumGap = []
    #         self.jobObjs[j].maximumGap = []
    #     # End jobs

    #     for i in range(self.nNurses):
    #         self.nurseObjs[i].ID = idict['rota'].loc[i, 'carer']
    #         self.nurseObjs[i].postcode = idict['rota'].loc[i, 'postcode']
    #         pcidict = idict['rota'].loc[i, 'postcode']
    #         latlon = cpo_inst.find_postcode_latlon(pcidict)
    #         self.nurseObjs[i].startLocation = GEOPOINT(float(latlon[0]), float(latlon[1]))
    #         self.nurseObjs[i].transportMode = 'car'
    #         istart = idict['rota'].loc[i, 'start']
    #         ifinish = idict['rota'].loc[i, 'finish']
    #         self.nurseObjs[i].shiftTimes = [float(istart), float(ifinish)]
    #         self.nurseObjs[i].maxWorking = float(idict['rota'].loc[i, 'shift'])
    #         self.nurseObjs[i].skills = []
    #         self.nurseObjs[i].features = []
    #         self.nurseObjs[i].preferences = []
    #         self.nurseObjs[i].preferredJobs = []
    #         self.nurseObjs[i].jobsToAvoid = []

    #     self.xy = []
    #     self.xy.append(self.nurseObjs[0].startLocation.longlat()) # why only the first nurse [0] location? Note that coords appended are lon/lat, not lat/lon!
    #     for job in self.jobObjs:
    #         self.xy.append(job.location.longlat()) # Append all job coordinates - lon/lat, not lat/lon!
    # ### --- End initialise_nurse_job --- ###
