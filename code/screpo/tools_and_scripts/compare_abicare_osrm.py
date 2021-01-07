#--------------------#
# compare_abicare_osrm.py
# 17/12/2020
# This file compares estimated times and mileage from abicare's csv file, exmileage.csv, with osrm data, and plots the data.
# Options include peak times, off peak times, all times, and can collect and plot the data from multiple days in one area.
# Other functions: compare/plot the number of jobs wrt the length of shift, and plot the number of jobs at given times of day (morning, afternoon, evening).
# Note: Aldershot area, instance [4] (06Nov2020), client 10's information is from a client in Hampshire, not a client in Aldershot.
# Therefore, in compare_all_inst_time_dist, an if statement has been added for aldershot, instance[4], which sets the mileage, metres, and estimated time for that job to 0.0.
# It is set to 0.0 rather than np.nan as the previous client in Aldershot, before the Hampshire client, is actually the same client, 114! 
# That is, the carer travelled from client 114 in Aldershot, to another client in Hampshire, and then back to client 114 in Aldershot again.
#--------------------#

# Import modules
import os
import sys
import glob
import math
import folium
import pickle
import requests # For website
import geopandas
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

# Import python file in project
import class_cpo_df1 as ccd

# Display all rows and columns of dataframes in command prompt:
pd.set_option("display.max_rows", None, "display.max_columns", None)

def compare_all_inst_time_dist(filename, timeofday='all'):
    all_instances = pickle.load(open(filename, 'rb'))
    area_name = all_instances[0]['area']
    # print(a_name)
    # exit(-1)
    cpo_inst = ccd.CPO_DF1()

    abitime_all = []
    abidist_all = []
    osrmtime_all = []
    osrmdist_all = []
    for i in range(len(all_instances)):
        idict = all_instances[i]
        # print('peak initial: ', peak)
        if i==5 and (timeofday == 'peak' or timeofday == 'offpeak'): # Weekdays only!
            break
        # if filename == 'all_inst_Aldershot.p' and i==4: # NOTE: Aldershot 06Nov (all_instances[4]), client 10's information is from a client in Hampshire, not a client in Aldershot. The previous client is actually the same client, 114!
        #     # Replaces times and distances with 0 instead.
        #     # print('here: ', i)
        #     idict['tasks'].loc[10, 'miles'] = 0.0
        #     idict['tasks'].loc[10, 'metres'] = 0.0
        #     idict['tasks'].loc[10, 'esttime'] = 0.0
        abitime_tidy, abidist_tidy, osrmtime_tidy, osrmdist_tidy = calc_timedist_abi_osrm(idict, cpo_inst, timeofday)
    
        # Add lists from this instance i (_tidy) to the list of all instances (_all)
        abitime_all = abitime_all + abitime_tidy
        abidist_all = abidist_all + abidist_tidy
        osrmtime_all = osrmtime_all + osrmtime_tidy
        osrmdist_all = osrmdist_all + osrmdist_tidy
    # End for loop
        
    if (len(abitime_all) != len(osrmtime_all)) and (len(abidist_all) != len(osrmdist_all)):
        print('[ERROR]: len(abitime_all) != len(osrmtime_all) *AND* len(abidist_all) != len(osrmdist_all).')
        print('len(abitime_all): ', len(abitime_all), ' len(osrmtime_all): ', len(osrmtime_all))
        print('len(abidist_all): ', len(abidist_all), ' len(osrmdist_all): ', len(osrmdist_all))
        print('Terminating program.')
        exit(-1)
    elif len(abitime_all) != len(osrmtime_all):
        print('[ERROR]: len(abitime_all) != len(osrmtime_all).')
        print('len(abitime_all): ', len(abitime_all), ' len(osrmtime_all): ', len(osrmtime_all))
        print('Terminating program.')
        exit(-1)
    elif len(abidist_all) != len(osrmdist_all):
        print('[ERROR]: len(abidist_all) != len(osrmdist_all).')
        print('len(abidist_all): ', len(abidist_all), ' len(osrmdist_all): ', len(osrmdist_all))
        print('Terminating program.')
        exit(-1)

    # print('abitime_all len: ', len(abitime_all))
    # print('abidist_all len: ', len(abidist_all))
    # print('osrmtime_all len: ', len(osrmtime_all))
    # print('osrmdist_all len: ', len(osrmdist_all))

    plot_time_comparison(abitime_all, osrmtime_all, len(abitime_all), area_name, timeofday)
    # plot_time_comparison(osrmtime_all, abitime_all, len(abitime_all), area_name, timeofday)
    if timeofday == 'all': # Only plot distance comparison for all times, no need to do it for peak/offpeak times because the distances won't change.
        plot_dist_comparison(abidist_all, osrmdist_all, len(abidist_all), area_name)
        # plot_dist_comparison(osrmdist_all, abidist_all, len(abidist_all), area_name)
### --- End of def compare_all_inst_time_dist --- ###

def calc_timedist_abi_osrm(idict, cpo_inst, timeofday='all'):
    # Create time/dist matrices for abi and osrm data, and then convert into lists containing only data that abicare has - this allows us to compare the data fairly.

    # Create time and distance matrices from abicare data and from osrm.
    abitime_matrix, abidist_matrix = od_abi(idict, timeofday)
    osrmtime_matrix, osrmdist_matrix = od_osrm(idict, cpo_inst)
    # print(abitime_matrix[:5, :5])
    # print(osrmtime_matrix[:5, :5])
    # exit(-1)

    # Convert the matrices into 1d arrays.
    abitime_array = np.asarray(abitime_matrix).reshape(-1)
    abidist_array = np.asarray(abidist_matrix).reshape(-1)
    osrmtime_array = np.asarray(osrmtime_matrix).reshape(-1)
    osrmdist_array = np.asarray(osrmdist_matrix).reshape(-1)
    # print('len abitime array: ', len(abitime_array))
    # print('len abidist array: ', len(abidist_array))
    # print('len osrmtime array: ', len(osrmtime_array))
    # print('len osrmdist array: ', len(osrmdist_array))

    abitime_tidy = []
    abidist_tidy = []
    osrmtime_tidy = []
    osrmdist_tidy = []
    # Abicare doesn't have all times/distances between all jobs, so as we want to compare the times/distances they have against osrm times/distances, we need to 'tidy' the data and only
    # store the times/distances that abicare has (in exmileage.csv). Therefore, the abi and osrm lists should still be the same size afterwards.
    for i in range(len(abitime_array)):
        if abitime_array[i] == -1: # If abicare does not have an estimated travel time for this index, then continue to the next index.
            continue
        else:
            abitime_tidy.append(abitime_array[i])
            abidist_tidy.append(abidist_array[i])
            osrmtime_tidy.append(osrmtime_array[i])
            osrmdist_tidy.append(osrmdist_array[i])

    return abitime_tidy, abidist_tidy, osrmtime_tidy, osrmdist_tidy
### --- End of def calc_timedist_abi_osrm --- ###

def od_abi(idict, timeofday='all'):
    # Create abitime_matrix and abidist_matrix, each of size nJobsxnJobs, that contain the times (in minutes) and distance (in metres) between jobs respectively.
    # First, create lists containing list [job index, esttime] and [job index, distance] respectively, then turn these list of lists into matrices of size len(list)x2,
    # where column[0] = job index, and column[1] = time/distance respectively. Then, use these matrices to fill abitime_matrix and abidist_matrix.

    nJobs = idict['stats']['ntasks']
    nNurses = idict['stats']['ncarers']

    # Create list of lists:
    i = 0
    list_abitime = [] # List of lists containing [job index, esttime to travel to job]
    list_abidist = [] # List of lists containing [job index, metres to travel to job]
    for k in range(nNurses):
        num_tasks = idict['rota'].loc[k, 'num_tasks'] # Number of tasks that nurse k is assigned 
        for j in range(num_tasks):
            iabitime = idict['tasks'].loc[i, 'esttime']
            iabidist = idict['tasks'].loc[i, 'metres']
            iabitwend = idict['tasks'].loc[i, 'tw_end']
            if j == 0: # If first task of nurse k, time and dist are nan.
                list_abitime.append([i, np.nan])
                list_abidist.append([i, np.nan])
                i+=1
            elif timeofday == 'peak': # If peak time only, then only append time and dist if the end of the task's time window is within the peak time, else append nan.
                if iabitwend < 420 or (iabitwend > 615 and iabitwend < 960) or iabitwend > 1155: # latest start time of job is between 7am and 10:15am or 4pm and 7:15pm
                    list_abitime.append([i, np.nan])
                    list_abidist.append([i, np.nan])
                    i+=1
                else:
                    list_abitime.append([i, iabitime])
                    list_abidist.append([i, iabidist])
                    i+=1
            elif timeofday == 'offpeak': # If off peak time only, then only append time and dist is the end of the task's time window is within off peak times, else append nan
                if iabitwend < 420 or (iabitwend > 615 and iabitwend < 960) or iabitwend > 1155: # latest start time of job is between 7am and 10:15am or 4pm and 7:15pm
                    list_abitime.append([i, iabitime])
                    list_abidist.append([i, iabidist])
                    i+=1
                else:
                    list_abitime.append([i, np.nan])
                    list_abidist.append([i, np.nan])
                    i+=1
            else: # If timeofday = all, then append time and dist to lists.
                list_abitime.append([i, iabitime])
                list_abidist.append([i, iabidist])
                i+=1
   
    # Convert lists into np arrays (2d arrays): these will be 'matrices with #rows = len(list_abitime), and #columns = 2 (column[0] = job index, column[1] = esttime or dist)
    array2d_abitime = np.asarray(list_abitime, dtype=np.float64)
    array2d_abidist = np.asarray(list_abidist, dtype=np.float64)

    # Create empty matrices, initialise with -1 to be able to distinguish between times/dists that are actually 0 (i.e. same location) and times/dists that don't exist (aren't in the exmileage.csv file).
    abitime_matrix = np.full((nJobs, nJobs), -1, dtype=np.float64) # NOTE: dimension are nJobs x nJobs, not nJobs+1 x nJobs + 1.  
    abidist_matrix = np.full((nJobs, nJobs), -1, dtype=np.float64) # NOTE: dimension are nJobs x nJobs, not nJobs+1 x nJobs + 1.  
    
    # Fill matrices
    for i in range(len(array2d_abitime)): # For i = 0,...,array2d_abitime.size() (number of jobs)
        if np.isnan(array2d_abitime[i][1]): # If job index i has no time (nan)
            continue
        else:
            abitime_matrix[i-1][i] = array2d_abitime[i][1] # Time to travel from previous job i-1 to this current job i
            abidist_matrix[i-1][i] = array2d_abidist[i][1] # Distance from previous job i-1 to current job i
    
    # Return matrices
    return abitime_matrix, abidist_matrix
### --- End of def od_abi --- ###

def od_osrm(idict, cpo_inst):
    # For osrm, the coordinates need to be in string form, and in the order 'longitude,latitude'.
    # Creates osrmtime_matrix and osrmdist_matrix, both of size nJobs x nJobs, containing times (mins) and distances (metres) respectively between jobs, data obtained from OSRM. 

    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']
    
    lonlat_jobs = []
    # lonlat_jobs1 = []
    for i in range(nJobs):
        pcidict = idict['tasks'].loc[i, 'postcode']
        lonlat = cpo_inst.find_postcode_lonlat(pcidict) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        # if i < 4:
        #     lonlat_jobs1.append(lonlat)
        lonlat_jobs.append(lonlat)
    lljSize = len(lonlat_jobs)
            
    lonlat_nurses = []
    for i in range(nNurses):
        pcidict = idict['rota'].loc[i, 'postcode']
        lonlat = cpo_inst.find_postcode_lonlat(pcidict) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        lonlat_nurses.append(lonlat)
    llnSize = len(lonlat_nurses)

    # coord_strings_list = []
    # for i in range(lljSize):
    #     coordstr = create_lonlat_string(lonlat_jobs[i])
    #     coord_strings_list.append(coordstr)
    # coord_strings = ';'.join(coord_strings_list)
    # print('coord_strings: ', coord_strings)
    # print(type(coord_strings))

    coord_strings = ';'.join([','.join([str(i), str(j)]) for i, j in lonlat_jobs])
    # print('stringllj: ', stringllj)
    # print(type(stringllj))
        
    # Get results from osrm:
    server = r'localhost:5000'
    str_call = r'http://' + server + '/table/v1/driving/' + coord_strings + '?annotations=distance,duration'
    # str_call = str_call + coord_strings + '?annotations=distance,duration'
    r = requests.get(str_call)
    osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.
    # print(osrmdict)

    # Fill od matrix:
    durations = osrmdict['durations'] # Given in seconds, /60 to get minutes.
    distances = osrmdict['distances']
    # print('durations shape: ', len(durations), ' distances shape: ', len(distances))

    # Create empty matrices: initialise with -1 to be able to distinguish between travel times/distances that are actually 0
    osrmtime_matrix = np.full((nJobs, nJobs), -1, dtype=np.float64) # NOTE: nJobs x nJobs, not nJobs+1 x nJobs+1 here, we're not using this for the main program. Matches dimensions of abitime_matrix.
    osrmdist_matrix = np.full((nJobs, nJobs), -1, dtype=np.float64) # NOTE: nJobs x nJobs, not nJobs+1 x nJobs+1 here, we're not using this for the main program. Matches dimensions of abidist_matrix.

    # Fill matrices
    for i in range(len(durations)):
        for j in range(len(durations[i])):
            time_seconds = durations[i][j]
            time_mins = time_seconds/60
            dist_metres = distances[i][j]
            osrmtime_matrix[i][j] = time_mins
            osrmdist_matrix[i][j] = dist_metres
            
    # Return matrices
    return osrmtime_matrix, osrmdist_matrix
### --- End of def od_osrm --- ### 

def plot_time_comparison(xlist, ylist, numpoints, area_name, timeofday='all'):

    slope, intercept, r_value, p_value, std_err = stats.linregress(xlist, ylist)
    print('OSRM Time = ', slope, '* Abicare Time +', intercept)
    # print('Abicare Time = ', slope, '* OSRM Time +', intercept)
    print('r_value:', r_value)
    print('p_value:', p_value)
    print('std_err:', std_err)

    # Create figure:
    fig = plt.figure()
    if timeofday == 'peak':
        fig.suptitle(area_name + ' Peak Time Comparison', fontsize=13, fontweight='bold')
    elif timeofday == 'offpeak':
        fig.suptitle(area_name + ' Off-Peak Time Comparison', fontsize=13, fontweight='bold')
    else:
        fig.suptitle(area_name + ' Time Comparison', fontsize=13, fontweight='bold')
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.82)
    ax.set_xlabel('Abicare Time (mins)')
    ax.set_ylabel('OSRM Time (mins)')
    # ax.set_xlabel('OSRM Time (mins)')
    # ax.set_ylabel('Abicare Time (mins)')

    # Create subtitle:
    slope = round(slope, 3)
    intercept = round(intercept, 3)
    r_value = round(r_value, 3)
    p_value = round(p_value, 3)
    std_err = round(std_err, 3)
    strslope = str(slope)
    strintercept = str(intercept)
    strr_value = str(r_value)
    strp_value = str(p_value)
    strstd_err = str(std_err)
    strnumpoints = str(numpoints)
    subtitle = 'gradient: ' + strslope + '  intercept: ' + strintercept + '\nr_val: ' + strr_value + '  p_val: ' + strp_value + '  err: ' + strstd_err + '\npoints: ' + strnumpoints 
    if timeofday == 'peak':
        subtitle = subtitle + '\n7am-10am, 4pm-7pm'
    elif timeofday == 'offpeak':
        subtitle = subtitle + '\n12am-7am, 10am-4pm, 7pm-12am'
    ax.set_title(subtitle, fontsize=8)

    # Polynomial fit:
    pfit = np.polyfit(xlist, ylist, 2)
    polyfit = np.poly1d(pfit)

    # Regression line:
    xmin, xmax = np.min(xlist), np.max(xlist)
    x_reg_line = np.array([xmin, xmax])
    y_reg_line = intercept + slope*x_reg_line
    # plt.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Polynomial:
    x_poly = np.linspace(xmin, xmax, 100)
    # plt.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')

    # Plot the points (scatter) and polynomial and regression lines:
    ax.scatter(xlist, ylist, marker='.', color='black')
    ax.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')
    ax.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Create the legend, save and show the plot:
    ax.legend()
    if timeofday == 'peak':
        plt.savefig(area_name +'_peaktime_comparison_abi.png')
        # plt.savefig(area_name +'_peaktime_comparison_osrm.png')
    elif timeofday == 'offpeak':
        plt.savefig(area_name +'_offpeaktime_comparison_abi.png')
        # plt.savefig(area_name +'_offpeaktime_comparison_osrm.png')
    else:
        plt.savefig(area_name + '_time_comparison_abi.png')
        # plt.savefig(area_name + '_time_comparison_osrm.png')

    plt.show()
### --- End of def plot_time_comparison --- ###

def plot_dist_comparison(xlist, ylist, numpoints, area_name):
    slope, intercept, r_value, p_value, std_err = stats.linregress(xlist, ylist)
    print('OSRM Dist = ', slope, '* Abicare Dist +', intercept)
    # print('Abicare Dist = ', slope, '* OSRM Dist +', intercept)
    print('r_value:', r_value)
    print('p_value:', p_value)
    print('std_err:', std_err)

    # Create figure:
    fig = plt.figure()
    fig.suptitle(area_name + ' Distance Comparison', fontsize=13, fontweight='bold')
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.82)
    ax.set_xlabel('Abicare Distance (metres)')
    ax.set_ylabel('OSRM Distance (metres)')
    # ax.set_xlabel('OSRM Distance (metres)')
    # ax.set_ylabel('Abicare Distance (metres)')

    # Create subtitle:
    slope = round(slope, 3)
    intercept = round(intercept, 3)
    r_value = round(r_value, 3)
    p_value = round(p_value, 3)
    std_err = round(std_err, 3)
    strslope = str(slope)
    strintercept = str(intercept)
    strr_value = str(r_value)
    strp_value = str(p_value)
    strstd_err = str(std_err)
    strnumpoints = str(numpoints)
    subtitle = 'gradient: ' + strslope + '  intercept: ' + strintercept + '\nr_val: ' + strr_value + '  p_val: ' + strp_value + '  err: ' + strstd_err + '\npoints: ' + strnumpoints
    ax.set_title(subtitle, fontsize=9)

    # Polynomial fit:
    pfit = np.polyfit(xlist, ylist, 2)
    polyfit = np.poly1d(pfit)

    # Regression line:
    xmin, xmax = np.min(xlist), np.max(xlist)
    x_reg_line = np.array([xmin, xmax])
    y_reg_line = intercept + slope*x_reg_line
    # plt.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Polynomial:
    x_poly = np.linspace(xmin, xmax, 100)
    # plt.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')

    # Plot the points (scatter) and polynomial and regression lines:
    ax.scatter(xlist, ylist, marker='.', color='black')
    ax.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')
    ax.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Create the legend, save and show the plot:
    ax.legend()
    plt.savefig(area_name + '_dist_comparison_abi.png')
    # plt.savefig(area_name + '_dist_comparison_osrm.png')

    plt.show()
### --- End of def plot_dist_comparison --- ###

def plot_shift_jobs_abi(filename):
    # Abicare only. All instances (days) for an area together.
    # Scatter plot: xaxis = Shift duration (mins), y-axis = Number of jobs
    all_instances = pickle.load(open(filename, 'rb'))
    area_name = all_instances[0]['area']
    
    xlist = [] # shift_durations_all = []
    ylist = [] # num_tasks_all = []
    for i in range(len(all_instances)):
        idict = all_instances[i]
        shift_durations_list = idict['rota']['shift'].to_list()
        # print(shift_durations_list)
        num_tasks_list = idict['rota']['num_tasks'].to_list()
        xlist = xlist + shift_durations_list
        # print(xlist)
        ylist = ylist + num_tasks_list
    # print('\n-----\n')
    # print(xlist)
    # exit(-1)

    print('len(shift_durations_all(xlist)): ', len(xlist), ' len(num_tasks_all(ylist)): ', len(ylist))
    numpoints = len(xlist)

    if len(xlist) != len(ylist):
        print('[ERROR]: len(shift_durations_all(xlist)) != len(num_tasks_all(ylist)).')
        print('len(shift_durations_all(xlist)): ', len(xlist), ' len(num_tasks_all(ylist)): ', len(ylist))
        print('Terminating program.')
        exit(-1)

    slope, intercept, r_value, p_value, std_err = stats.linregress(xlist, ylist)
    print('Jobs = ', slope, '* Shift Duration +', intercept)
    print('r_value:', r_value)
    print('p_value:', p_value)
    print('std_err:', std_err)

    # Create figure:
    fig = plt.figure()
    fig.suptitle(area_name + ': Shift Duration v Number of Jobs', fontsize=13, fontweight='bold')
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.82)
    ax.set_xlabel('Shift Duration (mins)')
    ax.set_ylabel('Number of Jobs')

    # Create subtitle:
    slope = round(slope, 3)
    intercept = round(intercept, 3)
    r_value = round(r_value, 3)
    p_value = round(p_value, 3)
    std_err = round(std_err, 3)
    strslope = str(slope)
    strintercept = str(intercept)
    strr_value = str(r_value)
    strp_value = str(p_value)
    strstd_err = str(std_err)
    strnumpoints = str(numpoints)
    subtitle = 'gradient: ' + strslope + '  intercept: ' + strintercept + '\nr_val: ' + strr_value + '  p_val: ' + strp_value + '  err: ' + strstd_err + '\npoints: ' + strnumpoints 
    ax.set_title(subtitle, fontsize=9)

    # Polynomial fit:
    pfit = np.polyfit(xlist, ylist, 2)
    polyfit = np.poly1d(pfit)

    # Regression line:
    xmin, xmax = np.min(xlist), np.max(xlist)
    x_reg_line = np.array([xmin, xmax])
    y_reg_line = intercept + slope*x_reg_line
    # plt.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Polynomial:
    x_poly = np.linspace(xmin, xmax, 100)
    # plt.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')

    # Plot the points (scatter) and polynomial and regression lines:
    ax.scatter(xlist, ylist, marker='.', color='black')
    ax.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')
    ax.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Create the legend, save and show the plot:
    ax.legend()
    plt.savefig(area_name + '_shift_jobs_abi.png')

    plt.show()
### --- End of def plot_compare_shift_jobs --- ###

def plot_bar_timeofday_jobs_abi(filename):
    # Abicare only. All instances (days) for an area together.
    # Bar chart: x-axis = three times of day (Morning, Afternoon, Evening), y-axis = Number of jobs
    all_instances = pickle.load(open(filename, 'rb'))
    area_name = all_instances[0]['area']
    all_morning = 0
    all_afternoon = 0
    all_evening = 0
    
    for i in range(len(all_instances)):
        idict = all_instances[i]
        nJobs = idict['stats']['ntasks']
        morning = 0
        afternoon = 0
        evening = 0
        for i in range(nJobs):
            job_starttime = idict['tasks'].loc[i, 'start']
            if job_starttime < 660: #If the job's start time is scheduled before 11am
                morning +=1
            elif job_starttime >= 660 and job_starttime < 960: # If the job's start time is scheduled at or after 11am and before 4pm
                afternoon += 1
            elif job_starttime >= 960: # If the job's start time is scheduled at or after 4pm
                evening += 1
            else:
                print ('ERROR: i = ', i, ', job_starttime = ', job_starttime)
                exit(-1)

        sum = morning + afternoon + evening
        if sum != nJobs:
            print ('ERROR: sum = ', sum, ', nJobs = ', nJobs)
            exit(-1)
        all_morning += morning
        all_afternoon += afternoon
        all_evening += evening

    timeofday = ['Morning\n6am-11am', 'Afternoon\n11am-4pm', 'Evening\n4pm-11pm']
    num_jobs = [all_morning, all_afternoon, all_evening]
    sum_all = all_morning + all_afternoon + all_evening

    df = pd.DataFrame({'Time of Day':['Morning\n6am-11am', 'Afternoon\n11am-4pm', 'Evening\n4pm-11pm'],'Number of Jobs':[all_morning, all_afternoon, all_evening]})
    colors = ['#f97ab5', '#64bff7', '#f1b05b'] # F1B05B #f3ad52

    # fig = plt.figure(figsize=(4.5,6))
    fig = plt.figure()
    fig.suptitle(area_name + ': Number of Jobs During Day', fontsize=13, fontweight='bold')
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.85)
    str_morning = str(all_morning)
    str_afternoon = str(all_afternoon)
    str_evening = str(all_evening)
    str_sumall = str(sum_all)
    subtitle = 'Morning jobs: ' + str_morning + '  Afternoon jobs: ' + str_afternoon + '  Evening jobs: ' + str_evening
    subtitle = subtitle + '\nTotal: ' + str_sumall
    # ax.set_title(area_name + ': Number of Jobs During Day', fontsize=13, fontweight='bold')
    ax.set_title(subtitle, fontsize=9)
    # fig.subplots_adjust(top=1)
    # ax.set_xlabel('Time of Day')
    ax.set_ylabel('Number of Jobs')
    # ax.bar(timeofday, num_jobs, width=0.45, color='dodgerblue')
    df.plot.bar(x='Time of Day',y='Number of Jobs',color=colors, ax=ax, legend=False, rot=0)
    plt.xlabel('')
    plt.savefig(area_name + '_timeofday_jobs_abi.png')
    plt.show()
### --- End of def plot_compare_timeofday_jobs --- ###

def calc_abicare_quality(idict):

    # print(idict['fname'])
    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']
    # print('nNurses: ', nNurses, 'nJobs: ', nJobs)
    # print(idict['tasks'])
    # print(idict['rota'])
    # exit(-1)

    travel_time = idict['stats']['ttraveltime']
    waiting_time = idict['stats']['twaitingtime']
    # print('travel_time ', idict['stats']['ttraveltime'])
    
    # Determine the shortest and longest shifts of all the nurses (in minutes)
    shift_list = np.array(idict['rota']['shift'])
    shortest_shift = np.min(shift_list)
    longest_shift = np.max(shift_list)
    
    day_diff = longest_shift - shortest_shift
    # print('shortest_shift: ', shortest_shift)
    # print('longest_shift: ', longest_shift)
    # print('day_diff: ', day_diff)

    # Ait H. quality:
    # ait_quality = -1*(0.3*totalTravel + ip->totPref);
    ait_quality = -1*((0.3*travel_time) + 0)
    # print('ait_quality: ', ait_quality)

    # Mankowska quality:
    # mk_quality = -1*(totalTravel + mk_allowed_tardiness + mk_max_tardiness)/3;
    mk_quality = -1*(travel_time + 0 + 0)/3
    # print('mk_quality: ', mk_quality)

    # Workload Balance quality:
    # wb_quality = -1*(0.3*totalTravel + ip->totPref) - 0.1*dayDiff + 0.1*minSpareTime;
    wb_quality = -1*((0.3*travel_time) + 0) - (0.1*day_diff) + (0.1*0)
    # print('wb_quality: ', wb_quality)

    # Paper quality:
    #paper_quality = ip->algorithmOptions[51]*totalTravel + ip->algorithmOptions[52]*totalWaitingTime + ip->algorithmOptions[53]*mk_allowed_tardiness + ip->algorithmOptions[54]*totalOvertime
    # + ip->algorithmOptions[55]*minSpareTime + ip->algorithmOptions[56]*ip->totPref + ip->algorithmOptions[57]*mk_max_tardiness;
    paper_quality = -1*travel_time + -1*waiting_time + -5*0 + -5*0 + 0.5*0 + 1*0 + 0*0
    # print('paper_quality: ', paper_quality)

    quality_filename = idict['area'] + '_quality_abi.txt'
    f = open(quality_filename, 'a')
    f.write('------------------------------------------------------------\n')
    f.write('Area: ' + str(idict['area']) + '\tDay: ' + str(idict['date']) + '\tnNurses: ' + str(nNurses) + '\tnJobs: ' + str(nJobs)) 
    f.write('\ntravel_time: ' + str(travel_time) + '\twaiting_time: ' + str(waiting_time))
    f.write('\nshortest_shift: ' + str(shortest_shift) + '\tlongest_shift: ' + str(longest_shift) + '\tday_diff: ' + str(day_diff))
    # f.write('\nsum_esttime: ' + str(sum_esttime))
    f.write('\nAit H Quality: ' + str(ait_quality))
    f.write('\nMankowska Quality: ' + str(mk_quality))
    f.write('\nWorkload Balance Quality: ' + str(wb_quality))
    f.write('\nPaper Quality: ' + str(paper_quality))
    f.write('\n')
    # f.write('\n------------------------------------------------------------\n')
    f.close()
### --- End def calc_abicare_quality --- ###

def solution_to_website_abi(idict, cpo_inst, doPlots=False, filename='unknown'):

    if doPlots:
        plot_pie_time_spent_abi(idict)
        plot_bar_time_per_nurse_abi(idict)
        
    # cpo_inst = ccd.CPO_DF()
    webFilename = ''
    if filename == 'unknown':
        webFilename = idict['fname'] + '_abi.html'
    else:
        webFilename = filename + '_abi.html'
    webFilename = os.path.join(os.getcwd(), webFilename)
    print('Website filename: ', str(webFilename))

    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']

    latlon_nurses = []
    lonlat_nurses = []
    for i in range(nNurses):
        pcidict = idict['rota'].loc[i, 'postcode']
        latlon = cpo_inst.find_postcode_latlon(pcidict)
        # lonlat = cpo_inst.find_postcode_lonlat(pcidict) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        lonlat = reverse_coord(latlon)
        latlon_nurses.append(latlon)
        lonlat_nurses.append(lonlat)
    latlonSize = len(latlon_nurses)
    lonlatSize = len(lonlat_nurses)
    
    xy = [] # List of lists (coords) [LON, LAT]
    pcnurse = idict['rota'].loc[0, 'postcode'] # First, add [lon, lat] of first nurse to xy
    lonlatnurse = cpo_inst.find_postcode_lonlat(pcnurse)
    xy.append(lonlatnurse)
    for i in range(nJobs): # Then add [lon, lat] of all jobs to xy
        pcidict = idict['tasks'].loc[i, 'postcode']
        lonlat = cpo_inst.find_postcode_lonlat(pcidict) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        xy.append(lonlat)
    xySize = len(xy)

    xyRev = [] # xy but with [LAT, LON] coords.
    for coord in xy:
        xyRev.append(reverse_coord(coord)) # Add coord (which is [lon, lat]) to xyRev but in [lat,lon] order.
    m = folium.Map(location=xyRev[0], zoom_start=14, tiles='cartodbpositron')

    foliumRouteLayers = []
    for ni in range(nNurses):
        route_colour = clusterColour(ni)
        foliumRouteLayers.append(folium.map.FeatureGroup(name='Nurse ' + str(ni)))
        nRoute = []
        nRouteRev = []

        niRoute = get_nurse_route_abi(ni, idict)
        for pos in range(nJobs):
            job = int(niRoute[pos])
            if niRoute[pos] < -0.1:
                break
            nRoute.append(tuple(xyRev[job+1]))
            nRouteRev.append(reverse_coord(xyRev[job+1])) # NOTE: Could just use xy instead??
            # Add a Marker:
            popupVal = '<b>Service ID:</b> ' + idict['tasks'].loc[job, 'client']
            popupVal = popupVal + '<br><b>Service no.:</b> ' + str(job)
            popupVal += '<br><b>Postcode:</b> ' + idict['tasks'].loc[job, 'postcode']
            # if self.jobObjs[job].doubleService:
                # popupVal = popupVal + ' (Double service)'
            popupVal = popupVal + '<br><b>Assigned nurse:</b> ' + str(ni)
            popupVal = popupVal + '<br><b>Arrival time:</b> ' + timemins_to_string(idict['tasks'].loc[job, 'start'])
            popupVal = popupVal + '<br><b>Departure Time:</b> ' + timemins_to_string(idict['tasks'].loc[job, 'end']) 
            popupVal = popupVal + '<br><b>Time Window:</b> ' + timemins_to_string(idict['tasks'].loc[job, 'tw_start']) + ' - ' + timemins_to_string(idict['tasks'].loc[job, 'tw_end'])
            border_colour = route_colour
            # if self.jobObjs[job].tardiness > 0:
                # popupVal = popupVal + '<br><b>*** Missed by:</b> ' + self.time_to_string(self.jobObjs[job].tardiness)
                # border_colour = '#ff0000'

            # if self.jobObjs[job].waitingToStart	> 0:
                # popupVal = popupVal + '<br><b>Waiting before start:</b> ' + self.time_to_string(self.jobObjs[job].waitingToStart)
            popupVal = popupVal + '<br><b>serviceTime:</b> ' + timemins_to_string(idict['tasks'].loc[job, 'duration'])
            # popupVal = popupVal + '<br><b>assignedNurse:</b> ' + str(self.jobObjs[idxNRpt].assignedNurse)
            popupVal = popupVal + '<br><b>positionInSchedule:</b> ' + str(pos)
            # popupVal = popupVal + '<br><b>skillsRequired:</b> ' + str(self.jobObjs[job].skillsRequired)
            # Add a circle around the area with popup:
            foliumRouteLayers[-1].add_child(folium.Circle(xyRev[job + 1], radius=30, popup=popupVal, color=border_colour, fill_color=route_colour, fill_opacity=0.5, fill=True))
        # End for pos in range(nJobs) loop

        # Obtain the routeList, which is a list of coordinates of the route ([lat,lon]), and the duration (SECONDS) and distance (METRES) of the route.
        routeList, dur, dist = route_points_osrm(lonlat_nurses[ni], lonlat_nurses[ni], goingThrough=nRouteRev)
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
    lht = lht + '''&nbsp; <b><u>Solution summary</u></b><br>'''
    lht = lht + '''&nbsp; <b>Total time: </b>''' + timemins_to_string(idict['stats']['totaltime']) + ''', of which:<br>'''
    lht = lht + '''&nbsp; <i> - Service time: </i>''' + timemins_to_string(idict['stats']['tservicetime']) + '''<br>'''
    lht = lht + '''&nbsp; <i> - Travel time: </i>''' + timemins_to_string(idict['stats']['ttraveltime']) + '''<br>'''
    lht = lht + '''&nbsp; <i> - Waiting time: </i>''' + timemins_to_string(idict['stats']['twaitingtime']) + '''<br>'''
    # lht = lht + '''&nbsp; <i> - Waiting time: </i>''' + time_to_string(self.totalWaitingTime) + '''<br><br>'''

    nursePart = '''&nbsp; <b><u>Nurse breakdown:</u> </b><br>'''
    for i in range(nNurses):
        nursePart = nursePart + '''<br>&nbsp; <b>Nurse ''' + str(i) + ' (' + str(idict['rota'].loc[i, 'carer']) + '''):</u> </b><br>'''
        # nursePart = nursePart + '''&nbsp; <i>Skills: </i>''' + str(nn.skills) + '''<br>'''
        nursePart = nursePart + '''&nbsp; <i>Shift start time: </i>''' + timemins_to_string(idict['rota'].loc[i, 'start']) + '''<br>'''
        nursePart = nursePart + '''&nbsp; <i>Shift end time: </i>''' + timemins_to_string(idict['rota'].loc[i, 'finish']) + '''<br>'''
        nursePart = nursePart + '''&nbsp; <i>Duration of shift: </i>''' + timemins_to_string(idict['rota'].loc[i, 'shift']) + '''<br>'''
        # nn.route = list(self.nurseRoute[i][:])
        nursePart = nursePart + '''&nbsp; <i>Number of services: </i>''' + str(idict['rota'].loc[i, 'num_tasks']) + '''<br>'''
        nursePart = nursePart + '''&nbsp; <i>Total service time: </i>''' + timemins_to_string(idict['rota'].loc[i, 'service_time']) + '''<br>'''
        nursePart = nursePart + '''&nbsp; <i>Total travel time: </i>''' + timemins_to_string(idict['rota'].loc[i, 'travel_time']) + '''<br>'''
        nursePart = nursePart + '''&nbsp; <i>Total waiting time: </i>''' + timemins_to_string(idict['rota'].loc[i, 'wait_time']) + '''<br>'''

        nurseRoute = get_nurse_route_abi(i, idict)
        if nurseRoute[0] > -1:
            nursePart = nursePart + '''&nbsp; <i>Service route: </i>[''' + idict['tasks'].loc[nurseRoute[0], 'client']
            j = 1
            while nurseRoute[j] > -1:
                client = idict['tasks'].loc[nurseRoute[j], 'client']
                nursePart = nursePart + ', ' + client
                j+=1
            nursePart = nursePart + ''']<br>'''
        # if len(nn.route) > 0:
        #     nursePart = nursePart + '''&nbsp; <i>Service route: </i>[''' + str(self.jobObjs[int(nn.route[0])].ID)
        #     for kkk in range(1,len(nn.route)):
        #         jobbb = self.jobObjs[int(nn.route[kkk])]
        #         if jobbb.doubleService:
        #             nursePart = nursePart + ', (' + str(jobbb.ID) + ')'
        #         else:
        #             nursePart = nursePart + ', ' + str(jobbb.ID)

        #     nursePart = nursePart + ''']<br>'''

    lht = lht + nursePart

    modalImages = [idict['fname'] + '_workload_abi.png', idict['fname'] + '_time_info_abi.png']
    modalCaptions = ['Workload distribution', 'Time distribution']

    for i,imn in enumerate(modalImages):
        lht = lht + hovering_image(imn, modalCaptions[i], i)

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
    for ni in range(nNurses):
        nurse_popup = 'Start location for nurse ' + str(ni)
        folium.Circle(latlon_nurses[ni], radius=50, popup=nurse_popup, color='black', fill_color='black', fill_opacity=0.5, fill=True).add_to(m)
    
    m.add_child(folium.map.LayerControl())
    m.save(webFilename)   
### --- End of def solution_to_website_abi --- ###

def plot_pie_time_spent_abi(idict):
    # fig, ax = plt.subplots(subplot_kw=dict(aspect="equal"))
    fig = plt.figure()
    fig.suptitle(idict['area'] + ' ' + idict['date'] + ': Abicare', fontsize=13, fontweight='bold')
    ax = fig.add_subplot(111)
    # fig.subplots_adjust(top=0.72)
    subtitle = 'Total time for ' + str(idict['stats']['ncarers']) + ' nurses: ' + timemins_to_string(idict['stats']['totaltime'])
    subtitle = '\nTravel Time: ' + timemins_to_string(idict['stats']['ttraveltime']) + '  Service Time: ' + timemins_to_string(idict['stats']['tservicetime']) + '  Waiting Time: ' + timemins_to_string(idict['stats']['twaitingtime'])
    ax.set_title(subtitle, fontsize=8)

    labels = 'Travel', 'Waiting', 'Service'
    times = [idict['stats']['ttraveltime'], idict['stats']['twaitingtime'], idict['stats']['tservicetime']]
    colours = ['#98d4f9', '#a6e781', '#f998c5'] ##a6e781 ##eeee76

    def func(pct, allvals):
        absolute = int(pct/100.*np.sum(allvals))
        return '{:.1f}%\n({:d} mins)'.format(pct, absolute)

    # wedges, texts, autotexts = ax.pie(times, autopct=lambda pct: func(pct, times), colors=colours)
    wedges, texts, autotexts = ax.pie(times, autopct='%1.1f%%', colors=colours)

    ax.legend(wedges, labels, title='Times', loc='center left', bbox_to_anchor=(1, 0, 0.5, 1))

    plt.setp(autotexts, size=8)

    plt.draw()
    plt.savefig(idict['fname'] + '_time_info_abi' + '.png', bbox_inches='tight')
    plt.show()
### --- End def plot_pie_time_spent_abi --- ###

def plot_bar_time_per_nurse_abi(idict):
    nNurses = idict['stats']['ncarers']
    xpos = np.arange(nNurses)
    width = 0.35

    service_time = np.array(idict['rota']['service_time'])
    travel_time = np.array(idict['rota']['travel_time'])
    waiting_time = np.array(idict['rota']['wait_time'])

    p1 = plt.bar(xpos, service_time, width, color='#f998c5')
    p2 = plt.bar(xpos, travel_time, width, bottom=service_time, color='#98d4f9')
    p3 = plt.bar(xpos, waiting_time, width, bottom=(service_time + travel_time), color='#a6e781')

    plt.xlabel('Carer')
    plt.ylabel('Time')
    plt.title(idict['area'] + ' ' + idict['date'] + ': Abicare', fontsize=13, fontweight='bold')
    ticksNames = []
    for i in xpos:
        # ticksNames.append(idict['rota'].loc[i, 'carer'])
        ticksNames.append(i+1)
    maxYtick = 15*60
    tmarks = np.arange(0, maxYtick, 30)
    tt = 0
    tticks = []
    for x in tmarks:
        tticks.append(str(np.round(tt, 1)) + ' h')
        tt = tt + 0.5
    plt.yticks(tmarks, tuple(tticks), fontsize=6)
    plt.xticks(xpos, tuple(ticksNames), fontsize=6)
    plt.legend((p1[0], p2[0], p3[0]), ('Service time', 'Travel time', 'Waiting time'))

    plt.draw()
    plt.savefig(idict['fname'] + '_workload_abi' + '.png', bbox_inches='tight')
    plt.show()
### --- End def plot_bar_time_per_nurse_abi --- ###

def get_nurse_route_abi(ni, idict):
    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']
    nurseRoute = np.full(nJobs, -1, dtype=np.int32)

    num_tasks = idict['rota'].loc[ni, 'num_tasks']
    if ni == 0:
        for i in range(num_tasks):
            nurseRoute[i] = i
    else:
        num_tasks_prev = 0
        # num_tasks = idict['rota'].loc[ni, 'num_tasks']
        # print('num_tasks1: ', num_tasks)
        for i in range(ni):
            # print('num_tasks2: ', idict['rota'].loc[i, 'num_tasks'])
            num_tasks_prev += idict['rota'].loc[i, 'num_tasks']
            # print('num_tasksprev1: ', num_tasks_prev)
        # print('num_tasksprev2: ', num_tasks_prev)
        for i in range(num_tasks):
            nurseRoute[i] = i + num_tasks_prev
    
    return nurseRoute
### End def get_nurse_route_abi --- ###

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
    dur = osrm_result['routes'][0]['duration'] # NOTE: THIS IS IN SECONDS.

    return [routeList, dur, dist]
### --- End def route_these_points --- ###

def hovering_image(imName, altText, idd):
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

def get_time_dist_abi_osrm(filename):
    # Function to get the travel times and distances for the abicare routes from osrm, used to compare fairly.
    all_instances = pickle.load(open(filename, 'rb'))
    # area_name = all_instances[0]['area']
    timeofday = 'all'
    # print(a_name)
    # exit(-1)
    cpo_inst = ccd.CPO_DF1()

    # abitime_all = []
    # abidist_all = []
    # osrmtime_all = []
    # osrmdist_all = []
    for i in range(len(all_instances)):
        idict = all_instances[i]
        # print('peak initial: ', peak)
        # if filename == 'all_inst_Aldershot.p' and i==4: # NOTE: Aldershot 06Nov (all_instances[4]), client 10's information is from a client in Hampshire, not a client in Aldershot. The previous client is actually the same client, 114!
        #     # Replaces times and distances with 0 instead.
        #     # print('here: ', i)
        #     idict['tasks'].loc[10, 'miles'] = 0.0
        #     idict['tasks'].loc[10, 'metres'] = 0.0
        #     idict['tasks'].loc[10, 'esttime'] = 0.0
        abitime_tidy, abidist_tidy, osrmtime_tidy, osrmdist_tidy = calc_timedist_abi_osrm(idict, cpo_inst, timeofday)
        print(idict['fname'])
        print('abitime_tidy sum: ', sum(abitime_tidy))
        print('osrmtime_tidy sum: ', sum(osrmtime_tidy))
        print('abitime_dist sum: ', sum(abidist_tidy)/1000)
        print('osrmdist_dist sum: ', sum(osrmdist_tidy)/1000)
        print('')
        # exit(-1)
    # End for loop
### --- End of def compare_all_inst_time_dist --- ###

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

def reverse_coord(coord):
    return [coord[1], coord[0]]
### --- End def reverse_latlong --- ###

def timemins_to_string(mins):
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

def miles_to_m_km(distmiles, km=False):
    if np.isnan(distmiles):
        # return distmiles
        return 0.0
    elif distmiles == 0:
        return distmiles
    else:
        kilometres = distmiles / 0.62137119
        # print(kilometres)
        if km == True:
            return kilometres
        else:   
            metres = kilometres*1000
            return metres
### --- End def miles_to_metres --- ###

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

def pie_chart_old(idict):
    labels = 'Travel', 'Waiting', 'Service'
    times = [idict['stats']['ttraveltime'], idict['stats']['twaitingtime'], idict['stats']['tservicetime']]
    colours = ['#98d4f9', '#a6e781', '#f998c5'] ##a6e781 ##eeee76
    plt.pie(times, labels=labels, colors=colours, autopct='%1.1f%%')
    plt.title('Total time for ' + str(idict['stats']['ncarers']) + ' nurses: ' + timemins_to_string(idict['stats']['totaltime']))
    # NOTE: need to add area name, date, and 'abicare' to title, as well as the actual times of the different segments onto the pie chart with the percentages.
    plt.axis('equal')
    plt.draw()
### --- End def pie_chart_old --- ###

# inst = {
#         'name' : a_name + '_' + day.replace('-', '_'),
#         'area' : a_name,
#         'date' : day,
#         'stats' : {'ncarers' : 0, 'ntasks' : 0, 'ttraveltime' : 0, 'ttravelmiles' : 0, 'tservicetime' : 0, 'tgaptime' : 0, 'twaitingtime' : 0, 'totaltime' : 0},
#         'txtsummary' : '',
#         'rota' : {'carer' : [], 'postcode' : [], 'num_addresses' : [], 'eastings': [], 'northings' : [], 'start' : [], 'finish' : [], 'shift' : [], 'home_start' : [], 'home_finish' : [], 'num_tasks' : [], 'travel_time' : [], 'wait_time' : [], 'service_time' : []},
#         'tasks' : {'client' : [], 'postcode': [], 'num_addresses': [], 'eastings': [], 'northings' : [], 'duration' : [], 'miles' : [], 'metres' : [], 'esttime' : [], 'start' : [], 'end' : [], 'tw_start' : [], 'tw_end' : []},
#         'routes' : []
#         }

# get_time_dist_abi_osrm('all_inst_Aldershot.p')


# exit(-1)
all_instances = pickle.load(open('all_inst_Aldershot.p', 'rb'))

for idict_index in range(len(all_instances)):
    idict = all_instances[idict_index]
    print(idict['fname'])
    print(idict['stats']['tservicetime'])
    print('')
#     print('ttravelmiles in metres: ', miles_to_m_km(idict['stats']['ttravelmiles'], km=False))
#     print('ttravelmiles in km: ', miles_to_m_km(idict['stats']['ttravelmiles'], km=True))
#     print('')
print('Done.')
exit(-1)

# idict_index = 1
# idict = all_instances[idict_index]
# print(idict['fname'])
# print('ttravelmiles in metres: ', miles_to_metres(idict['stats']['ttravelmiles']))

# nNurses = idict['stats']['ncarers']
# nJobs = idict['stats']['ntasks']
# total_metres = 0
# for i in range(nJobs):
#     total_metres = total_metres + idict['tasks'].loc[i, 'metres']
# print('total metres: ', total_metres)
# # print('Done.')
# exit(-1)
# # idict = 1st instance in all_instances, which is 02-Nov-2020 in Hampshire. ncarers = 10, ntasks = 69.
# cpo_inst = ccd.CPO_DF()
# for idict_index in range(len(all_instances)):
#     idict = all_instances[idict_index]
#     plot_bar_time_per_nurse_abi(idict)
#     print('Done: ', idict_index)
# plot_bar_time_per_nurse_abi(idict)

# for idict_index in range(len(all_instances)):
#     idict = all_instances[idict_index]
#     solution_to_website_abi(idict, cpo_inst, doPlots=True)
#     print('Done: ', idict_index)

# exit(-1)
# plot_shift_jobs_abi('all_inst_Aldershot.p')

# compare_all_inst_time_dist('all_inst_Hampshire.p')
# for idict_index in range(len(all_instances)):
    # idict = all_instances[idict_index]
    # calc_abicare_quality(idict)
    # print('\n')
#     print('quality: ', quality)

# quality = calc_abicare_quality(idict)

# exit(-1)
# area_name = idict['area']

# index_list = []
#     num_tasks_prev = 0
#     for i in range(nNurses):
#         num_tasks = idict['rota'].loc[i, 'num_tasks']
#         for j in range(num_tasks):
#             index_list.append(j + num_tasks_prev)
#         num_tasks_prev += num_tasks

# print(idict['name'])
# nNurses = idict['stats']['ncarers']
# nJobs = idict['stats']['ntasks']
# print('nNurses: ', nNurses, 'nJobs: ', nJobs)
# plot_compare_timeofday_jobs('all_inst_aldershot.p')
# exit(-1)

# morning = 0
# afternoon = 0
# evening = 0
# for i in range(nJobs):
#     job_starttime = idict['tasks'].loc[i, 'start']
#     if job_starttime < 660: #If the job's start time is scheduled before 11am
#         morning +=1
#     elif job_starttime >= 660 and job_starttime < 960: # If the job's start time is scheduled at or after 11am and before 4pm
#         afternoon += 1
#     elif job_starttime >= 960: # If the job's start time is scheduled at or after 4pm
#         evening += 1
#     else:
#         print ('ERROR: i = ', i, ', job_starttime = ', job_starttime)
#         exit(-1)

# sum = morning + afternoon + evening
# if sum != nJobs:
#     print ('ERROR: sum = ', sum, ', nJobs = ', nJobs)
#     exit(-1)
# timeofday = ['Morning', 'Afternoon', 'Evening']
# num_jobs = [morning, afternoon, evening]

# fig = plt.figure(figsize=(4.5,6))
# # plt.figure(figsize=(7,7), dpi=300)
# # ax = fig.add_axes([0,0,1,1])
# # ax.set_title('Scores by group and gender')
# # fig.suptitle(area_name + ': Number of Jobs During Day', fontsize=13, fontweight='bold')
# ax = fig.add_subplot(111)
# ax.set_title(area_name + ': Number of Jobs During Day', fontsize=13, fontweight='bold')
# # fig.subplots_adjust(top=1)
# ax.set_xlabel('Time of Day')
# ax.set_ylabel('Number of Jobs')
# ax.bar(timeofday, num_jobs, width=0.45, color='dodgerblue')
# # plt.bar(timeofday, num_jobs, width=0.45, color='dodgerblue')
# # plt.xlabel('Time of Day')
# # plt.ylabel('Number of Jobs')
# plt.show()

# print(idict['tasks'])

# print(idict['rota'])



# area_name = all_instances[0]['area']
# shift_durations_all = []
# num_tasks_all = []
# for i in range(len(all_instances)):
#     idict = all_instances[i]
#     shift_durations_list = idict['rota']['shift'].to_list()
#     num_tasks_list = idict['rota']['num_tasks'].to_list()
#     shift_durations_all = shift_durations_all + shift_durations_list
#     num_tasks_all = num_tasks_all + num_tasks_list

# print('len(shift_durations_all): ', len(shift_durations_all), ' len(num_tasks_all): ', len(num_tasks_all))

# if len(shift_durations_all) != len(num_tasks_all):
#     print('[ERROR]: len(shift_durations_all) != len(num_tasks_all).')
#     print('len(shift_durations_all): ', len(shift_durations_all), ' len(num_tasks_all): ', len(num_tasks_all))
#     print('Terminating program.')
#     exit(-1)

# print(num_tasks_list)



# plot_shift_jobs()


# idict['tasks'].loc[10, 'miles'] = 0.0
# idict['tasks'].loc[10, 'metres'] = 0.0
# idict['tasks'].loc[10, 'esttime'] = 0.0


# print(idict['name'])
# nNurses = idict['stats']['ncarers']
# nJobs = idict['stats']['ntasks']
# print('nNurses: ', nNurses, 'nJobs: ', nJobs)

# # print(idict['tasks'])
# print(idict['rota'])

# cpo_inst = ccd.CPO_DF()

# pcidict1 = idict['tasks'].loc[6, 'postcode']
# print(pcidict1)
# lonlat1 = cpo_inst.find_postcode_lonlat(pcidict1) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.

# pcidict2 = idict['tasks'].loc[7, 'postcode']
# print(pcidict2)
# lonlat2 = cpo_inst.find_postcode_lonlat(pcidict2) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.

# print('ll1: ', lonlat1)
# print('ll2: ', lonlat2)
# exit(-1)

# abitime_matrix, abidist_matrix = od_abi(idict)
# print('time shape: ', abitime_matrix.shape, ' dist shape: ', abidist_matrix.shape)
# exit(-1)



### --- END --- ###

# exit(-1)
# print(len(all_instances))
# exit(-1)
# print(idict['name'])
# nNurses = idict['stats']['ncarers']
# nJobs = idict['stats']['ntasks']
# print('nNurses: ', nNurses, 'nJobs: ', nJobs)
# print(idict['rota'])
# exit(-1)

# esttime_matrix = abicare_esttime(idict)
# print(esttime_matrix[:5, :5])
# print(esttime_array[:3])
# print(len(esttime_array))
# print(type(esttime_array))
# print(esttime_array.shape)
# print('esttimematrix shape: ', esttime_matrix.shape)



# osrmtime_matrix = od_osrmtime(idict, cpo_inst)
# print(osrmtime_matrix[:5, :5])
# print('osrmtimematrix shape: ', osrmtime_matrix.shape)

# esttime_array = np.asarray(esttime_matrix).reshape(-1)
# osrmtime_array = np.asarray(osrmtime_matrix).reshape(-1)
# print('len esttime array: ', len(esttime_array))
# print('len osrmtime array: ', len(osrmtime_array))

# esttime_tidy = []
# osrmtime_tidy = []
# for i in range(len(esttime_array)):
#     if esttime_array[i] == -1:
#         continue
#     else:
#         esttime_tidy.append(esttime_array[i])
#         osrmtime_tidy.append(osrmtime_array[i])


# print(len(esttime_tidy))
# print(len(osrmtime_tidy))

# print(type(esttime_tidy))
# print(type(osrmtime_tidy))

# plot_time_comparison(esttime_tidy, osrmtime_tidy)


# i = 0
# for k in range(nNurses):
#     n_tasks = idict['rota'].loc[k, 'num_tasks']
#     # print(n_tasks)
#     # print(type(n_tasks))
#     for j in range(n_tasks):
#         if j == 0:
#             i+=1
#             # print('here')
#             # continue
#         elif esttime_array[i] == -1:
#             i+=1
#             # continue
#         else:
#             esttime_tidy.append(esttime_array[i])
#             osrmtime_tidy.append(osrmtime_array[i])
#             i+=1


# list1 = [1,2,3]
# list3 = list1
# list2 = [4,5,6]
# list1= list1+list2
# print('list1: ', list1)
# print('list2: ', list2)
# print('list3: ', list3)
# exit(-1)



# idict_tasks = {
#     'tasks' : { 'client' : [], 'postcode' : [], 'num_addresses' : [], 'eastings' : [], 'northings' : [], 'metres' : [], 'esttime' : []}
# }
# idict_tasks['tasks']['client'] = idict['tasks']['client']
# idict_tasks['tasks']['postcode'] = idict['tasks']['postcode']
# idict_tasks['tasks']['num_addresses'] = idict['tasks']['num_addresses']
# idict_tasks['tasks']['eastings'] = idict['tasks']['eastings']
# idict_tasks['tasks']['northings'] = idict['tasks']['northings']
# # idict_tasks['tasks']['miles'] = idict['tasks']['miles']
# idict_tasks['tasks']['metres'] = idict['tasks']['metres']
# idict_tasks['tasks']['esttime'] = idict['tasks']['esttime']
# idict_tasks['tasks'] = pd.DataFrame(idict_tasks['tasks'])
# print(idict_tasks['tasks'])
# print(idict['rota'])
# print(idict['name'])
# nNurses = idict['stats']['ncarers']
# nJobs = idict['stats']['ntasks']
# print('nNurses: ', nNurses, 'nJobs: ', nJobs)



# print(osrmtime_array[:3])
# print(len(osrmtime_array))
# print(type(osrmtime_array))
# print(osrmtime_array.shape)

# list_of_lists = [[1.1, 5.5], [2.2, 6.6], [3.3, 7.7], [4.4, 8.8]]
# print(list_of_lists)
# array_np = np.asarray(list_of_lists, dtype=np.float64)
# print(array_np)
# print(type(array_np))
# print(array_np.shape)
# print('shapes:')
# print(array_np.shape[0])
# print(array_np.shape[1])
# print(len(array_np))
# print(array_np[0][1])

# idict_tasks = {
#     'tasks' : { 'client' : [], 'postcode' : [], 'eastings' : [], 'northings' : [], 'metres' : [], 'esttime' : []}
# }
# idict_tasks['tasks']['client'] = idict['tasks']['client']
# idict_tasks['tasks']['postcode'] = idict['tasks']['postcode']
# idict_tasks['tasks']['eastings'] = idict['tasks']['eastings']
# idict_tasks['tasks']['northings'] = idict['tasks']['northings']
# # idict_tasks['tasks']['miles'] = idict['tasks']['miles']
# idict_tasks['tasks']['metres'] = idict['tasks']['metres']
# idict_tasks['tasks']['esttime'] = idict['tasks']['esttime']
# idict_tasks['tasks'] = pd.DataFrame(idict_tasks['tasks'])
# print(idict_tasks['tasks'])

# cpo_inst = ccd.CPO_DF()

# od = osrm_table_request_noinst(idict, cpo_inst, matrix='od')
# print('od')
# print(od)
# nurse_travel_from_depot = osrm_table_request_noinst(idict, cpo_inst, matrix='nursefrom')
# print('nurse_travel_from_depot')
# print(nurse_travel_from_depot)
# nurse_travel_to_depot = osrm_table_request_noinst(idict, cpo_inst, matrix='nurseto')
# print('nurse_travel_to_depot')
# print(nurse_travel_to_depot)

# def plot_dist_comparison(xlist, ylist, numpoints):
#     slope, intercept, r_value, p_value, std_err = stats.linregress(xlist, ylist)
#     print('Abicare Dist = ', slope, '* OSRM Dist +', intercept)
#     print('r_value:', r_value)
#     print('p_value:', p_value)
#     print('std_err:', std_err)

#     # Polynomial fit:
#     pfit = np.polyfit(xlist, ylist, 2)
#     polyfit = np.poly1d(pfit)

#     # Plot the points:
#     plt.scatter(xlist, ylist, marker='.', color='black')
#     # plt.xticks(np.arange(min(xlist), max(xlist)+1, 2.0)) # x-axis is labelled with values 0,2,4,...
#     # plt.yticks(np.arange(min(ylist), max(ylist)+1, 2.0)) # y-axis is labelled with values 0,2,4,...
#     plt.xlabel('Abicare Dist (metres)')
#     plt.ylabel('OSRM Dist (metres)')
#     # Plot the regression line:
#     xmin, xmax = np.min(xlist), np.max(xlist)
#     x_reg_line = np.array([xmin, xmax])
#     y_reg_line = intercept + slope*x_reg_line
#     plt.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

#     # And the polynomial:
#     x_poly = np.linspace(xmin, xmax, 100)
#     plt.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')

#     # Add legend and title, then save and show the plot.
#     strnumpoints = str(numpoints)
#     numpointlabel = strnumpoints + ' points'
#     plt.plot([], [], ' ', label=numpointlabel)
#     plt.legend(loc='upper left')
#     plt.title('Monmouthshire Distance Comparison')
#     plt.savefig('Monmouthshire_dist_comparison.png')
#     plt.show()
### --- End of def plot_dist_comparison --- ###