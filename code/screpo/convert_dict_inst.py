#-----------------------#
# convert_dict_inst.py
# Convert instance 'idict' in dictionary format (stored in all_inst_salisbury.p file), into an instance 'inst' of the INSTANCE class in instance_handler.py
# 04/12/2020
#-----------------------#
# Using np.int32, corresponds to C/C++ integer, -2147483648 to 2147483647

# Import modules
import os
import sys
import glob
import pickle
import math
import requests # For website
import numpy as np
import pandas as pd
import geopandas
import matplotlib.pyplot as plt
from scipy import stats

# Import python file in project
import tools_and_scripts.class_cpo_df as ccd

# Display all rows and columns of dataframes in command prompt:
# pd.set_option("display.max_rows", None, "display.max_columns", None)

def convert_dict_inst(inst, idict):
    # This function creates an instance 'inst' of the class 'INSTANCE', and initialises inst with data from our idict dictionary instance created by analyse_mileage.py.
    # This function takes a dictionary instance 'idict' as input, and returns the object 'inst'.

    cpo_inst = ccd.CPO_DF()

    inst.nNurses = idict['stats']['ncarers']
    inst.nJobs = idict['stats']['ntasks']
    inst.nSkills = 5 # NOTE: this is a random number just for testing.

    inst.nurseWorkingTimes = np.zeros((inst.nNurses, 3), dtype=np.int32) # nurseWorkingTimes is nNurses x 3, col[0] = start time, col[1] = finish time, col[2] = max working time.
    for i in range(inst.nNurses): #range(len(idict['rota']['start']))
        inst.nurseWorkingTimes[i][0] = idict['rota']['start'][i]
        inst.nurseWorkingTimes[i][1] = idict['rota']['finish'][i]
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
    inst.doubleService[2] = 1 # NOTE: just for testing, make one of the jobs a double service.
    inst.dependsOn = np.full(inst.nJobs, -1) # Set all jobs to -1, means no jobs are dependent on one another.
    inst.algorithmOptions = np.zeros(100, dtype=np.float64)

    inst.od = np.zeros((inst.nJobs+1, inst.nJobs+1), dtype=np.float64) # TIME IN MINUTES, THIS WILL BE USED IN C
    osrm_table_request(inst, idict, cpo_inst, 'od')
    # print(inst.od[:5,:5])
    
    inst.nurse_travel_from_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From carer's home to job - TIME IN MINS, THIS WILL BE USED IN C
    osrm_table_request(inst, idict, cpo_inst, 'nursefrom')

    inst.nurse_travel_to_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From job to carer's home - TIME IN MINS, THIS WILL BE USED IN C
    osrm_table_request(inst, idict, cpo_inst, 'nurseto')
    
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

    # For these variables - do we need to change them?
    inst.solMatrix = np.zeros((inst.nNurses, inst.nJobs), dtype=np.int32)
    inst.MAX_TIME_SECONDS = 360
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

def osrm_request(coord1, coord2):
    # For osrm, the coordinates need to be in string form, and in the order 'longitude,latitude'. Our data is in 'lat,lon', so create_coord_string function swaps it around.
    server = r'localhost:5000'
    coordstr1 = create_coord_string(coord1)
    coordstr2 = create_coord_string(coord2)
    str_call = 'http://' + server + '/route/v1/driving/' + coordstr1 + ';' + coordstr2 + '?overview=false'
    r = requests.get(str_call)
    osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.

    # print(osrmdict)
    # print(osrmdict['routes'][0]['duration'], 'seconds.')
    # print(osrmdict['routes'][0]['distance'], 'metres.')
    time = osrmdict['routes'][0]['duration'] / 60 # Convert into minutes
    dist = osrmdict['routes'][0]['distance']
    return time, dist
### --- End of def osrm_request --- ###

def osrm_table_request_old(inst, idict, matrix='none'):
    # For osrm, the coordinates need to be in string form, and in the order 'longitude,latitude'. Our data is in 'lat,lon', so create_coord_string function swaps it around.

    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']

    lat_lon_jobs = []
    for i in range(nJobs):
        lat = idict['tasks'].loc[i, 'lat']
        lon = idict['tasks'].loc[i, 'lon']
        coord = [lat, lon]
        lat_lon_jobs.append(coord)
    lljSize = len(lat_lon_jobs)
        
    lat_lon_nurses = []
    for i in range(nNurses):
        lat = idict['rota'].loc[i, 'lat']
        lon = idict['rota'].loc[i, 'lon']
        coord = [lat, lon]
        lat_lon_nurses.append(coord)
    llnSize = len(lat_lon_nurses)

    server = r'localhost:5000'
    str_call = r'http://' + server + '/table/v1/driving/'

    # --- MATRIX TYPES --- #

    if matrix == 'od': # Only use osrm for od matrix        
        # server = r'localhost:5000'
        # str_call = 'http://' + server + '/table/v1/driving/'
        coord_strings_list = []
        # for i in range(lljSize):
        for i in range(lljSize):
            coordstr = create_coord_string(lat_lon_jobs[i])
            coord_strings_list.append(coordstr)
        # print('coord_strings_list_old:')
        # print(coord_strings_list)
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
            coordstrnurse = create_coord_string(lat_lon_nurses[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_nurses.append(coordstrnurse) # Add the long/lat string to list
            source_list.append(str(i)) # Add index of long/lat pair to list
            count +=1
        coord_strings_nurses = ';'.join(coord_strings_list_nurses) # coord_strings_nurses = single string comprising all strings in coord_strings_list_nurses joined with semicolons in between
        source_string = ';'.join(source_list) # Join the list of strings together into a single string with semicolon between each string

        # Jobs second (destinations):
        destination_list = [] # Indices for destination long/lats
        coord_strings_list_jobs = [] # List of longs/lats as strings
        for i in range(lljSize):
            coordstrjob = create_coord_string(lat_lon_jobs[i]) # Convert each lat/long coord into a long/lat STRING
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
            coordstrjob = create_coord_string(lat_lon_jobs[i]) # Convert each lat/long coord into a long/lat STRING
            coord_strings_list_jobs.append(coordstrjob) # Add the long/lat string to list
            source_list.append(str(i)) # Add index of long/lat pair to list
            count +=1
        coord_strings_jobs = ';'.join(coord_strings_list_jobs) # coord_strings_jobs = single string comprising all strings in coord_strings_list_jobs joined with semicolons in between
        source_string = ';'.join(source_list) # Join the list of strings together into a single string with semicolon between each string

        # Nurses second (destinations):
        destination_list = [] # Indices for destination long/lats
        coord_strings_list_nurses = [] # List of longs/lats as strings
        for i in range(llnSize):
            coordstrnurse = create_coord_string(lat_lon_nurses[i]) # Convert each lat/long coord into a long/lat STRING
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
        print('ERROR: no matrix type specified for osrm_table_request.')
        print('Matrix type should be either \'od\', \'nursefrom\', or \'nurseto\'.')
        print('Terminating program.')
        exit(-1)
    # --- End else --- #
### --- End of def osrm_table_request_old --- ### 

def osrm_table_request(inst, idict, cpo_inst, matrix='none'):
    # For osrm, the coordinates need to be in string form, and in the order 'longitude,latitude'. Our data is in 'lat,lon', so create_coord_string function swaps it around.

    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']

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
        for i in range(len(durations)):
            for j in range(len(durations[i])):
                time_seconds = durations[i][j]
                time_mins = time_seconds/60
                inst.od[i+1][j+1] = time_mins
                # od[i+1][j+1] = time_mins
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

def create_coord_string(coord):
    # For osrm_request, we need the coordinates in string form to put into the web address. 
    # The coordinates also need to be in the order 'long,lat', however in lat_lon_list the data is stored as [lat, lon].
    # This function takes in a set of coordinates [lat,lon], reverses the order to [lon, lat], and then converts the floats into a string.
    # E.g.: given coord = [51.2, -1.7], coord_reverse = [-1.7, 51.2], temp_string_coord_reverse = ['-1.7', '51.2'], string_coord_reverse = '-1.7,51.2'
    lat = coord[0]
    lon = coord[1]
    coord_reverse = [lon, lat] #Note: THIS IS THE OTHER WAY AROUND FOR TEST_REQUEST.PY/OSRM_REQUEST FUNCTON -  [LON LAT], NOT [LAT LON]

    temp_string_coord_reverse = [str(float) for float in coord_reverse]
    # print('temp_string_coord_reverse:', temp_string_coord_reverse)
    string_coord_reverse = ','.join(temp_string_coord_reverse)
    # print('string_coord_reverse: ', string_coord_reverse)

    return string_coord_reverse
### --- End of def create_coord_string --- ###  

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

def assign_nan_lon_lat(idict):
    # Need to replace latitude and longitude of certain carers and clients whose original postcodes are private (and so in idict = nan), replacing them with other random locations in Salisbury.
    
    # Carer 16 - taking postcode SP2 7HD
    idict['rota'].loc[1, 'lat'] = 51.077959
    idict['rota'].loc[1, 'lon'] = -1.813821

    # Carer 81 - taking postcode SP4 7XH
    idict['rota'].loc[9, 'lat'] = 51.166563
    idict['rota'].loc[9, 'lon'] = -1.766612

    # CLient 14 - taking postcode SP2 8JJ
    idict['tasks'].loc[69, 'lat'] = 51.059365
    idict['tasks'].loc[69, 'lon'] = -1.798328
    idict['tasks'].loc[91, 'lat'] = 51.059365
    idict['tasks'].loc[91, 'lon'] = -1.798328

    # CLient 16 - taking postcode SP2 7TQ
    idict['tasks'].loc[7, 'lat'] = 51.067452
    idict['tasks'].loc[7, 'lon'] = -1.800636
    idict['tasks'].loc[15, 'lat'] = 51.067452
    idict['tasks'].loc[15, 'lon'] = -1.800636
    idict['tasks'].loc[47, 'lat'] = 51.067452
    idict['tasks'].loc[47, 'lon'] = -1.800636
    idict['tasks'].loc[71, 'lat'] = 51.067452
    idict['tasks'].loc[71, 'lon'] = -1.800636

    # CLient 78 - taking postcode SP2 7BS
    idict['tasks'].loc[38, 'lat'] = 51.074628
    idict['tasks'].loc[38, 'lon'] = -1.805556
    idict['tasks'].loc[87, 'lat'] = 51.074628
    idict['tasks'].loc[87, 'lon'] = -1.805556

    # CLient 89 - taking postcode SP4 9HT
    idict['tasks'].loc[6, 'lat'] = 51.190740
    idict['tasks'].loc[6, 'lon'] = -1.756244
    idict['tasks'].loc[20, 'lat'] = 51.190740
    idict['tasks'].loc[20, 'lon'] = -1.756244
    idict['tasks'].loc[27, 'lat'] = 51.190740
    idict['tasks'].loc[27, 'lon'] = -1.756244
    idict['tasks'].loc[33, 'lat'] = 51.190740
    idict['tasks'].loc[33, 'lon'] = -1.756244

    # CLient 103 - taking postcode SP4 7NA
    idict['tasks'].loc[4, 'lat'] = 51.171357
    idict['tasks'].loc[4, 'lon'] = -1.776100
    idict['tasks'].loc[30, 'lat'] = 51.171357
    idict['tasks'].loc[30, 'lon'] = -1.776100
    idict['tasks'].loc[37, 'lat'] = 51.171357
    idict['tasks'].loc[37, 'lon'] = -1.776100
    idict['tasks'].loc[41, 'lat'] = 51.171357
    idict['tasks'].loc[41, 'lon'] = -1.776100
    idict['tasks'].loc[65, 'lat'] = 51.171357
    idict['tasks'].loc[65, 'lon'] = -1.776100
    idict['tasks'].loc[86, 'lat'] = 51.171357
    idict['tasks'].loc[86, 'lon'] = -1.776100

    # CLient 104 - taking postcode SP9 7JX
    idict['tasks'].loc[3, 'lat'] = 51.246233
    idict['tasks'].loc[3, 'lon'] = -1.669790
    idict['tasks'].loc[29, 'lat'] = 51.246233
    idict['tasks'].loc[29, 'lon'] = -1.669790
    idict['tasks'].loc[36, 'lat'] = 51.246233
    idict['tasks'].loc[36, 'lon'] = -1.669790
    idict['tasks'].loc[40, 'lat'] = 51.246233
    idict['tasks'].loc[40, 'lon'] = -1.669790
    idict['tasks'].loc[64, 'lat'] = 51.246233
    idict['tasks'].loc[64, 'lon'] = -1.669790
    idict['tasks'].loc[85, 'lat'] = 51.246233
    idict['tasks'].loc[85, 'lon'] = -1.669790

    # CLient 133 - taking postcode SP1 2LF
    idict['tasks'].loc[2, 'lat'] = 51.063203
    idict['tasks'].loc[2, 'lon'] = -1.792218

    # CLient 152 - taking postcode SP2 0FA
    idict['tasks'].loc[24, 'lat'] = 51.085944
    idict['tasks'].loc[24, 'lon'] = -1.849359
    idict['tasks'].loc[31, 'lat'] = 51.085944
    idict['tasks'].loc[31, 'lon'] = -1.849359

    # CLient 237 - taking postcode SP1 3NT
    idict['tasks'].loc[49, 'lat'] = 51.083611
    idict['tasks'].loc[49, 'lon'] = -1.800222
    idict['tasks'].loc[52, 'lat'] = 51.083611
    idict['tasks'].loc[52, 'lon'] = -1.800222
    idict['tasks'].loc[53, 'lat'] = 51.083611
    idict['tasks'].loc[53, 'lon'] = -1.800222
    idict['tasks'].loc[57, 'lat'] = 51.083611
    idict['tasks'].loc[57, 'lon'] = -1.800222
    idict['tasks'].loc[62, 'lat'] = 51.083611
    idict['tasks'].loc[62, 'lon'] = -1.800222
    idict['tasks'].loc[63, 'lat'] = 51.083611
    idict['tasks'].loc[63, 'lon'] = -1.800222
    idict['tasks'].loc[79, 'lat'] = 51.083611
    idict['tasks'].loc[79, 'lon'] = -1.800222
    idict['tasks'].loc[93, 'lat'] = 51.083611
    idict['tasks'].loc[93, 'lon'] = -1.800222
    idict['tasks'].loc[98, 'lat'] = 51.083611
    idict['tasks'].loc[98, 'lon'] = -1.800222
    idict['tasks'].loc[99, 'lat'] = 51.083611
    idict['tasks'].loc[99, 'lon'] = -1.800222

    return idict
### --- End def assign_nan_lon_lat --- ###

def assign_missing_postcode(idict):
    # CLient 16 - taking postcode SP2 7TQ as its current postcode, sp27xx, is missing from codepoint open.
    idict['tasks'].loc[7, 'postcode'] = 'sp27tq'
    idict['tasks'].loc[15, 'postcode'] = 'sp27tq'
    idict['tasks'].loc[47, 'postcode'] = 'sp27tq'
    idict['tasks'].loc[71, 'postcode'] = 'sp27tq'
    return idict
### End def assign_missing_postcode --- ###

def crow_flies_distance(coord1, coord2):
    # From https://stackoverflow.com/questions/13845152/calculate-as-the-crow-flies-distance-php
    lat1 = coord1[0]
    lon1 = coord1[1]
    lat2 = coord2[0]
    lon2 = coord2[1]

    distance = math.acos(
        math.cos(lat1 * (math.pi/180)) *
        math.cos(lon1 * (math.pi/180)) *
        math.cos(lat2 * (math.pi/180)) *
        math.cos(lon2 * (math.pi/180)) 
        +
        math.cos(lat1 * (math.pi/180)) *
        math.sin(lon1 * (math.pi/180)) *
        math.cos(lat2 * (math.pi/180)) *
        math.sin(lon2 * (math.pi/180))
        +
        math.sin(lat1 * (math.pi/180)) *
        math.sin(lat2 * (math.pi/180))
    ) * 6371
    
    return distance*1000 # In metres.
## --- End of def crow_flies_distance --- #

def dist_metres_to_minutes(dist):
    # This function takes the distance in metres from crow_flies distance and converts it to time in minutes approximately
    # The value 11 (was 13.2806) is used as a VERY approximate value to convert the distances to time.
    time_secs = dist / 11
    time_mins = time_secs / 60

    return time_mins
### --- End of dist_metres_to_minutes --- ###

# all_instances = pickle.load(open('tools_and_scripts/all_inst_salisbury.p', 'rb'))

# idict = 7th instance in all_instances, which is 08-Nov-2020 in Salisbury. Chose this inst as it is the only one which has all postcodes for carers and clients. ncarers = 12, ntasks = 100.
# idict = all_instances[6]
# Assign different latitudes and longitudes to the carers and clients that do not have them in idict due to privacy reasons (so lat/lon = nan) - this is just so we can use this instance for a test.
# idict = assign_missing_postcode(idict)

# print(idict['tasks'])
# cpo_inst = ccd.CPO_DF()

# osrm_table_request(idict, cpo_inst, 'od')

# nNurses = idict['stats']['ncarers']
# nJobs = idict['stats']['ntasks']
# print('nNurses: ', nNurses, 'nJobs: ', nJobs)
# print(idict['tasks'])
# print(idict['rota'])






# coordmain = lat_lon_jobs[0]
# coord1 = lat_lon_jobs[1]
# coord2 = lat_lon_jobs[2]
# coord3 = lat_lon_jobs[3]


# # server = r'localhost:5000'
# coordstrmain = create_coord_string(coordmain)
# coordstr1 = create_coord_string(coord1)
# coordstr2 = create_coord_string(coord2)
# coordstr3 = create_coord_string(coord3)

# stringlist = []
# stringlist.append(coordstrmain)
# stringlist.append(coordstr1)
# stringlist.append(coordstr2)
# stringlist.append(coordstr3)
# print('stringslist: ', stringlist)
# print('\n')
# stringlistjoin = ';'.join(stringlist)
# print('stringlistjoin: ', stringlistjoin)

# str_call = 'http://' + server + '/table/v1/driving/' + coordstrmain + ';' + coordstr1 + ';' + coordstr2 + ';' + coordstr3 + '?sources=0;1'
# r = requests.get(str_call)
# osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.

# print(osrmdict)
# print(osrmdict['durations'])
# print(type(osrmdict['durations']))
# durations = osrmdict['durations']
# print(durations)
# durations_mins = []
# od = np.zeros((5, 5), dtype=np.float64)
# for i in range(len(durations)):
    # temp_durations_mins = []
    # for j in range(len(durations[i])):
        # time_seconds = durations[i][j]
        # print('time_seconds: ', time_seconds)
        # time_mins = time_seconds/60
        # od[i+1][j+1] = time_mins
        # temp_durations_mins.append(time_mins)
    # durations_mins.append(temp_durations_mins)
    # print('duration_mins: ', durations_mins)
# print('durations_mins:\n')
# print(durations_mins)
# print('odmatrix:\n')
# print(od)
# print(i, ': ', osrmdict['durations'][0][i], ' seconds, ', time_mins, ' mins')
# print(durations)
# print(osrmdict['routes'][0]['duration'], 'seconds.')
# print(osrmdict['routes'][0]['distance'], 'metres.')
# time = osrmdict['routes'][0]['duration'] / 60 # Convert into minutes
# dist = osrmdict['routes'][0]['distance']

# print('lat_lon_jobs[3]:', lat_lon_jobs[3])
# print('lat_lon_jobs[3]:', lat_lon_jobs[3])
# time, dist = osrm_request(lat_lon_jobs[3], lat_lon_jobs[3])
# print('time1: ', time, '\tdist1: ', dist)
# print('\n')
# dist_crow = crow_flies_distance(lat_lon_jobs[8], lat_lon_jobs[9])
# timeMins = dist_metres_to_minutes(dist_crow)
# print('time2:', timeMins, '\tdist2: ', dist_crow)
# print('timeMins: ', timeMins, 'mins.')



# distance1 = crow_flies_distance(lat_lon_jobs[0], lat_lon_jobs[1])
# print('distance1:', distance1, ' metres.')

# od = np.zeros((nJobs+1, nJobs+1), dtype=np.float64) # TIME IN MINUTES, THIS WILL BE USED IN C
# for i in range(nJobs):
#     coord1 = lat_lon_jobs[i]
#     for j in range(nJobs):
#         if j == i:
#             continue
#         coord2 = lat_lon_jobs[j]
#         dist = crow_flies_distance(coord1, coord2)
#         od[i+1][j+1] = dist

# NOTE: ERROR! using crow flies distance gives distance in metres BUT C program takes in od as MINUTES! So, a distance of e.g. 1821 metres is over 30 HOURS! 
# Instead, for now we shall will randomly assign travel minutes between 5 and 30, which should be appropriate for the Salisbury area.
# inst.od = np.zeros((inst.nJobs+1, inst.nJobs+1), dtype=np.float64) # TIME IN MINUTES, THIS WILL BE USED IN C
# for i in range(inst.nJobs):
#     coord1 = lat_lon_jobs[i]
#     for j in range(inst.nJobs):
#         if j == i:
#             continue
#         coord2 = lat_lon_jobs[j]
#         dist = crow_flies_distance(coord1, coord2)
#         inst.od[i+1][j+1] = dist

# NOTE: ERROR! using crow flies distance gives distance in metres BUT C program takes in nurse_travel_to/from_depot as MINUTES! So, a distance of e.g. 1821 metres is over 30 HOURS! 
# Instead, for now we shall will randomly assign travel minutes between 5 and 30, which should be appropriate for the Salisbury area.
# inst.nurse_travel_from_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From carer's home to job - TIME IN MINS, THIS WILL BE USED IN C
# inst.nurse_travel_to_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From job to carer's home - TIME IN MINS, THIS WILL BE USED IN C
# for i in range(inst.nNurses):
#     coordNurse = lat_lon_nurses[i]
#     for j in range(inst.nJobs):
#         coordJob = lat_lon_jobs[j]
#         dist = crow_flies_distance(coordNurse, coordJob)
#         inst.nurse_travel_from_depot[i][j] = dist
#         inst.nurse_travel_to_depot[i][j] = dist

###-----------------------------------------------------------------------------------------------------------------------------------------------###

# Instance format from analyse_mileage.py (in this file, this is referred to as 'idict')
# inst = {
#       'name' : a_name + '_' + day.replace('-', '_'),
#       'area' : a_name,
#       'date' : day,
#       'stats' : {'ncarers' : 0, 'ntasks' : 0, 'ttraveltime' : 0, 'ttravelmiles' : 0, 'tservicetime' : 0, 'tgaptime' : 0},
#       'txtsummary' : '',
#       'rota' : {'carer' : [], 'postcode' : [], 'num_addresses' : [], 'lat': [], 'lon' : [], 'start' : [], 'finish' : []},
#       'tasks' : {'client' : [], 'postcode': [], 'num_addresses': [], 'lat': [], 'lon' : [], 'duration' : [], 'tw_start' : [], 'tw_end' : []},
#       'routes' : []
# }

# def crow_flies_distance(coord1, coord2):
#     # From https://stackoverflow.com/questions/13845152/calculate-as-the-crow-flies-distance-php
#     lat1 = coord1[0]
#     lon1 = coord1[1]
#     lat2 = coord2[0]
#     lon2 = coord2[1]

#     distance = math.acos(
#         math.cos(lat1 * (math.pi/180)) *
#         math.cos(lon1 * (math.pi/180)) *
#         math.cos(lat2 * (math.pi/180)) *
#         math.cos(lon2 * (math.pi/180)) 
#         +
#         math.cos(lat1 * (math.pi/180)) *
#         math.sin(lon1 * (math.pi/180)) *
#         math.cos(lat2 * (math.pi/180)) *
#         math.sin(lon2 * (math.pi/180))
#         +
#         math.sin(lat1 * (math.pi/180)) *
#         math.sin(lat2 * (math.pi/180))
#     ) * 6371
    
#     return distance*1000 # In metres.
### --- End of def crow_flies_distance function --- #

# NOTE: ERROR! using crow flies distance gives distance in metres BUT C program takes in od as MINUTES! So, a distance of e.g. 1821 metres is over 30 HOURS! 
# Instead, for now we shall convert the distance in metres to time by dividing the metres by 13.28, then dividing by 60 to get the time in minutes.
# for i in range(inst.nJobs):
    #     coord1 = lat_lon_jobs[i]
    #     for j in range(inst.nJobs):
    #         if j == i:
    #             continue
    #         coord2 = lat_lon_jobs[j]
    #         dist = crow_flies_distance(coord1, coord2)
    #         time_mins = dist_metres_to_minutes(dist)
    #         inst.od[i+1][j+1] = time_mins
    
    # inst.od_dist = np.zeros((inst.nJobs+1, inst.nJobs+1), dtype=np.float64)
    # for i in range(inst.nJobs):
    #     for j in range(inst.nJobs):
    #         if j == i:
    #             continue
    #         time, dist = osrm_request(lat_lon_jobs[i], lat_lon_jobs[j])
    #         inst.od[i+1][j+1] = time
    #         inst.od_dist[i+1][j+1] = dist

# for i in range(inst.nNurses):
    #     coordNurse = lat_lon_nurses[i]
    #     for j in range(inst.nJobs):
    #         coordJob = lat_lon_jobs[j]
    #         dist = crow_flies_distance(coordNurse, coordJob)
    #         time_mins = dist_metres_to_minutes(dist)
    #         inst.nurse_travel_from_depot[i][j] = time_mins
    #         inst.nurse_travel_to_depot[i][j] = time_mins
    # inst.nurse_travel_from_depot_dist = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From carer's home to job
    # inst.nurse_travel_to_depot_dist = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64) # From job to carer's home
    # for i in range(inst.nNurses):
    #     for j in range(inst.nJobs):
    #         time, dist = osrm_request(lat_lon_nurses[i], lat_lon_jobs[j]) # Going FROM carer's home i TO job j.
    #         inst.nurse_travel_from_depot[i][j] = time
    #         inst.nurse_travel_from_depot_dist[i][j] = dist

    # for i in range(inst.nNurses):
    #     for j in range(inst.nJobs):
    #         dist, time = osrm_request(lat_lon_jobs[j], lat_lon_nurses[i]) # Going FROM job j TO carer's home i.
    #         inst.nurse_travel_to_depot[i][j] = time
    #         inst.nurse_travel_to_depot_dist[i][j] = dist
