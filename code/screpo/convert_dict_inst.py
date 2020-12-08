#-----------------------#
# convert_dict_inst.py
# Convert instance 'idict' in dictionary format (stored in all_inst_salisbury.p file), into an instance 'inst' of the INSTANCE class in instance_handler.py
# 04/12/2020
#-----------------------#
# Using np.int32, corresponds to C/C++ integer, -2147483648 to 2147483647

# Import modules
import os
import pickle
import math
import numpy as np
import pandas as pd

# Import python file in project
import instance_handler as hhc

# Display all rows and columns of dataframes in command prompt
# pd.set_option("display.max_rows", None, "display.max_columns", None)

def convert_dict_inst(idict):
    inst = hhc.INSTANCE()
    inst.nJobs = idict['stats']['ntasks']
    inst.nNurses = idict['stats']['ncarers']
    inst.nSkills = -1

    # nurseWorkingTimes is nNurses x 3, col[0] = start time, col[1] = finish time, col[2] = max working time.
    inst.nurseWorkingTimes = np.zeros((inst.nNurses, 3), dtype=np.int32)
    for i in range(inst.nNurses): #range(len(idict['rota']['start']))
        inst.nurseWorkingTimes[i][0] = idict['rota']['start'][i]
        inst.nurseWorkingTimes[i][1] = idict['rota']['finish'][i]
        # We dont have max working time, so we need to calculate the duration of each carer's shift (total minutes that each carer works during the day)
        maxWorkingTime = idict['rota']['finish'][i] - idict['rota']['start'][i]
        inst.nurseWorkingTimes[i][2] = maxWorkingTime
    
    inst.nurseSkills = np.ones((inst.nNurses, inst.nJobs), dtype=np.int32) #change to ones

    # jobTimeInfo is nJobs x 3, col[0] = startTW, col[1] = endTW, col[2] = length of job (time)
    inst.jobTimeInfo = np.zeros((inst.nJobs, 3), dtype=np.int32)
    for i in range(inst.nJobs): #range(len(idict['tasks']['duration']))
        inst.jobTimeInfo[i][0] = idict['tasks']['tw_start'][i]
        inst.jobTimeInfo[i][1] = idict['tasks']['tw_end'][i]
        inst.jobTimeInfo[i][2] = idict['tasks']['duration'][i]

        
    inst.jobSkillsRequired = np.ones((inst.nJobs, inst.nSkills), dtype=np.int32) #Note: this will not work as currently nSkills = -1.
    inst.prefScore = np.zeros((inst.nJobs, inst.nNurses), dtype=np.float64)
    inst.algorithmOptions = np.zeros(100, dtype=np.float64)
    inst.doubleService = np.zeros(inst.nJobs, dtype=np.int32)
    inst.dependsOn = np.zeros(inst.nJobs, dtype=np.int32)
    inst.od = np.zeros((inst.nJobs+1, inst.nJobs+1), dtype=np.float64)
    # inst.xy = [] # Unsure

    # For these variables - do we need to change them?
    inst.solMatrix = np.zeros((inst.nNurses, inst.nJobs), dtype=np.int32)
    inst.MAX_TIME_SECONDS = 30
    inst.verbose = 5
    inst.secondsPerTU = 1 # What is this variable?
    inst.name = 'solution0'
    inst.full_file_name = 'solution0'
    inst.loadFromDisk = False
    inst.mankowskaQuality = -1
    inst.Cquality = -1
    inst.DSSkillType = 'shared-duplicated'

    #Note: these will not work as self.nurse_travel_to/from_depot is not yet in def __init__ in class INSTANCE in instance_handler.py
    inst.nurse_travel_from_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64)
    inst.nurse_travel_to_depot = np.zeros((inst.nNurses, inst.nJobs), dtype=np.float64)

    #Note: this will not work as currently inst.doubleService is empty, and so nDS = 0 - third dimension of capabilityOfDS cannot be zero.
    nDS = np.sum(inst.doubleService) # Number of jobs that are double services
    inst.capabilityOfDoubleServices = np.zeros((inst.nNurses, inst.nNurses, nDS), dtype=np.int32)

    inst.mk_mind = np.zeros(inst.nJobs, dtype=np.int32)
    inst.mk_maxd = np.zeros(inst.nJobs, dtype=np.int32)

    # self.nurseRoute = []
    # self.nurseWaitingTime = []
    # self.nurseServiceTime = []
    # self.nurseTravelTime = []
    # self.nurseTime = []
    # self.totalWaitingTime = -1
    # self.totalServiceTime = -1
    # self.totalTravelTime = -1
    # self.totalTardiness = -1
    # self.nLateJobs = 0
    # self.totalTime = -1
    # self.qualityMeasure = 'standard'
    # self.maxTardiness = -1
    # self.c_quality = 0
    
### --- End of def convert_dict_inst function --- #

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
### --- End of def crow_flies_distance function --- #

all_instances = pickle.load(open('tools_and_scripts/all_inst_salisbury.p', 'rb'))

# idict = 7th instance in all_instances, which is 08-Nov-2020 in Salisbury. Chose this inst as it is the only one which has all postcodes for carers and clients. ncarers = 12, ntasks = 100.
idict = all_instances[6] 

# Need to replace latitude and longitude of certain carers and clients whose original postcodes are private, replacing them with other random locations in Salisbury:
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

ncarers = idict['stats']['ncarers']
ntasks = idict['stats']['ntasks']

odMatrixIdict = np.zeros((ntasks+1, ntasks+1), dtype=np.float64)
for i in range(ntasks):
    lat1 = idict['tasks'].loc[i, 'lat']
    lon1 = idict['tasks'].loc[i, 'lon']
    coord1 = [lat1, lon1]
    for j in range(ntasks):
        if j == i:
            continue
        lat2 = idict['tasks'].loc[j, 'lat']
        lon2 = idict['tasks'].loc[j, 'lon']
        coord2 = [lat2, lon2]
        dist = crow_flies_distance(coord1, coord2)
        odMatrixIdict[i+1][j+1] = dist
# print(odMatrixIdict)

# Because we're using the direct distance (crow flies) and not road distances, the distances to/from a carer's house to a particular job will be the same both ways. This will need to be changed.
nurseDistFromDepot = np.zeros((ncarers, ntasks), dtype=np.float64) # From carer's home to job
nurseDistToDepot = np.zeros((ncarers, ntasks), dtype=np.float64) # From job to carer's home
for i in range(ncarers):
    lat1 = idict['rota'].loc[i, 'lat']
    lon1 = idict['rota'].loc[i, 'lon']
    coordCarer = [lat1, lon1]
    for j in range(ntasks):
        lat2 = idict['tasks'].loc[j, 'lat']
        lon2 = idict['tasks'].loc[j, 'lon']
        coordTask = [lat2, lon2]
        dist = crow_flies_distance(coordCarer, coordTask)
        nurseDistFromDepot[i][j] = dist
        nurseDistToDepot[i][j] = dist


# lat1 = idict['tasks'].loc[0, 'lat']
# lon1 = idict['tasks'].loc[0, 'lon']
# coord1 = [lat1, lon1]
# print('coord1: ', coord1)
# lat2 = idict['tasks'].loc[1, 'lat']
# lon2 = idict['tasks'].loc[1, 'lon']
# coord2 = [lat2, lon2]
# print('coord2: ', coord2)

# dist = crow_flies_distance(coord1, coord2)
# print('distance:', dist)
# for i in range(ntasks):
    
    # print(i, '[', idict['rota']['lat'][i], ', ', idict['rota']['lon'][i],']')
# print(idict['rota'])
# print(idict['tasks'])


# print(type(idict['rota']))








