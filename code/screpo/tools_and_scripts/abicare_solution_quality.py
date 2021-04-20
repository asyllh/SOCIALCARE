# Calculate quality of abicare solution.
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

# Display all rows and columns of dataframes in command prompt:
pd.set_option("display.max_rows", None, "display.max_columns", None)

# Instance format from fc_analyse_mileage.py (in this file, this is referred to as 'idict')
# inst = {
#         'name' : a_name + '_' + day.replace('-', '_'),
#         'fname' : day.replace('-', '_') + '_' + a_name, # filename
#         'area' : a_name,
#         'date' : day,
#         'stats' : {'ncarers' : 0, 'ntasks' : 0, 'ttraveltime' : 0, 'ttravelmiles' : 0, 'tservicetime' : 0, 'tgaptime' : 0, 'twaitingtime' : 0, 'totaltime' : 0},
#         'txtsummary' : '',
#         'rota' : {'carer' : [], 'postcode' : [], 'num_addr' : [], 'eastings': [], 'northings' : [], 'start' : [], 'finish' : [], 'shift' : [], 'home_start' : [], 'home_finish' : [], 'num_tasks' : [], 'travel_time' : [], 'wait_time' : [], 'service_time' : []},
#         'tasks' : {'client' : [], 'postcode': [], 'num_addr': [], 'eastings': [], 'northings' : [], 'duration' : [], 'miles' : [], 'metres' : [], 'esttime' : [], 'start' : [], 'end' : [], 'tw_start' : [], 'tw_end' : []},
#         'routes' : []
#         }

def calc_abicare_quality(idict):
    print(idict['name'])
    nNurses = idict['stats']['ncarers']
    nJobs = idict['stats']['ntasks']
    print('nNurses: ', nNurses, 'nJobs: ', nJobs)
    # print(idict['tasks'])
    # print(idict['rota'])
    # exit(-1)

    # Only obtain esttime for jobs that are being travelled to from another job, not from carer's home (or from another job not in our area!)
    i = 0
    list_esttime = []
    for k in range(nNurses):
        num_tasks = idict['rota'].loc[k, 'num_tasks']
        for j in range(num_tasks):
            iesttime = idict['tasks'].loc[i, 'esttime']
            if j == 0:
                list_esttime.append(np.nan) # If job is first for the carer, then set esttime to nan.
                i+=1
            else:
                list_esttime.append(iesttime)
                i+=1

    list_esttime_tidy = [x for x in list_esttime if ~np.isnan(x)] # Remove all nans from list

    sum_esttime = sum(list_esttime_tidy) # This sum will be used in the quality.
    # print('esttime sum: ', sum_esttime)
    # print('ttraveltime: ', idict['stats']['ttraveltime'])

    # Determine the shortest and longest shifts of all the nurses (in minutes)
    shift_list = np.array(idict['rota']['shift'])
    shortest_shift = np.min(shift_list)
    longest_shift = np.max(shift_list)
    shortestShift = 9999999.0
    longestShift = 0.0
    for i in range(nNurses):
        shift_duration = idict['rota'].loc[i, 'shift']
        if shift_duration < shortestShift:
            shortestShift = shift_duration
        if shift_duration > longestShift:
            longestShift = shift_duration

    # print('shortest: ', shortestShift)
    # print('longest: ', longestShift)

    day_diff = longestShift - shortestShift

    #quality = -1 * (0.3 * totaltraveltime(mins) + ip->totPref) - 0.1 * dayDiff + 0.1 * minSpareTime
    quality = -1 * (0.3 * sum_esttime) - (0.1 * day_diff)

    return quality
### --- End def calc_abicare_quality --- ###

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
### --- End def time_to_string --- ###  

exit(-1)
# all_instances = pickle.load(open('all_inst_hampshire.p', 'rb'))
all_instances = pickle.load(open('all_inst_Monmouth.p', 'rb'))
# all_instances = pickle.load(open('TEST_all_inst_Hampshire.p', 'rb'))
# idict_index = 1
for idict_index in range(len(all_instances)):

    idict = all_instances[idict_index]
    print(idict['fname'])
    print(idict['stats']['ttraveltime'])
    print(idict['stats']['ttravelmiles'])
    print(idict['stats']['tservicetime'])
    print(idict['stats']['tgaptime'])
    print(idict['stats']['twaitingtime'])
    print(idict['stats']['totaltime'])
    print('\n')
# print(idict['rota'])
# print(idict['tasks'])




# print(idict['rota'])
# print(idict['stats']['twaitingtime'])
# print(idict['stats']['ttraveltime'])
# print(idict['stats']['tservicetime'])
# totaltime = timemins_to_string(idict['stats']['totaltime'])
# print('total time: ', totaltime)

exit(-1)


quality = calc_abicare_quality(idict)
print('quality: ', quality)

# tgaptime = timemins_to_string(idict['stats']['tgaptime'])
# ttraveltime = timemins_to_string(idict['stats']['ttraveltime'])
# tservicetime = timemins_to_string(idict['stats']['tservicetime'])
# print('tgaptime: ', tgaptime)
# print('ttraveltime: ', ttraveltime)
# print('tservicetime: ', tservicetime)


# mins = 220.25 # = 3hours, 40 mins, 15 seconds
# mins = 128.883333 # 2 hours, 8 mins, 53 seconds
# mins = 780.0333333 # 13 hours, 0 mins, 2 seconds
# mins = 400 # 54 mins, 30 seconds

# minsRound = math.floor(mins)
# print('minsRound: ', minsRound)
# hours = mins // 60
# print('hours:', hours) # = 3
# # minutes = (mins % 60)
# minutes = (minsRound % 60)
# print('minutes: ', minutes)
# seconds = (mins - minsRound) * 60
# seconds = round(seconds)
# print('seconds: ', seconds)

# timeRound = math.floor(time)
# timeSecDecimal = time - timeRound
# timeSecWhole = timeSecDecimal * 100
# timeSecMins = timeSecWhole / 60
# timeTotal = timeRound + timeSecMins

# hours = seconds // 3600
# minutes = (seconds % 3600) // 60
# seconds = seconds % 60
# print('{0:0>2}:{1:0>2}:{2:0>2}'.format(int(hours), int(minutes), int(seconds)))

# minutes = 13215 % 3600
# minutes = 600 % 11
# print(minutes)



