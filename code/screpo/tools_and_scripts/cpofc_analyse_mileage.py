#--------------------#
# cpofc_analyse_mileage.py
# 08/01/2021
# 
#--------------------#

# Import modules
import os
import sys
import glob
import math
import folium
import pickle
import datetime
import requests # For website
import geopandas
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

# Import python file in project
import class_cpo_df as ccd

# cpofc_analyse_mileage.py: codepoint open and full codepoint analyse mileage.

class POSTCODE_FINDER():
    """
    docstring
    """
    df = []
    filename = []
    def __init__(self, filename=r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\postcode\full_codepoint.csv'):
        self.filename = filename
        self.df = pd.read_csv(self.filename)
        self.df['PC'] = self.df['PC'].str.replace('"', '')
        self.df['PC'] = self.df['PC'].str.replace(' ', '') 
        self.df['PC'] = self.df['PC'].str.lower()

    def find_postcode_eastnorth(self, postcode): # Returns eastings and northings of given postcode
        dfview = self.df[self.df['PC'] == postcode]
        if dfview.empty:
            return [None, None]
        else:
            # Need to convert eastings and northings to lat and lon
            return [dfview.iloc[0]['EA'], dfview.iloc[0]['NO']]
    
    def find_postcode_n_addresses(self, postcode): # Returns number of registered properties at a given postcode.
        dfview = self.df[self.df['PC'] == postcode]
        if dfview.empty:
            return -1
        else:
            return dfview.iloc[0]['RP']
### --- End class POSTCODE_FINDER --- ###

def time_dif_to_minutes(endtime, starttime, round=True):
    mins = (endtime - starttime).seconds/60
    if round:
        return np.round(mins, 0)
    else:
        return mins
### --- End time_dif_to_minutes --- ###

def inst_dicts_to_dfs(inst):
    inst['rota'] = pd.DataFrame(inst['rota'])
    inst['tasks'] = pd.DataFrame(inst['tasks'])
    return inst
### --- End inst_dicts_to_dfs --- ###

def miles_to_metres(distmiles):
    if np.isnan(distmiles):
        # return distmiles
        return 0.0
    elif distmiles == 0:
        return distmiles
    else:
        km = distmiles / 0.62137119
        metres = km*1000
        return metres
### --- End miles_to_metres --- ###

def minsecs_to_mins(time):
    if np.isnan(time):
        # return time
        return 0.0
    elif time == 0:
        return time
    else:    
        timeRound = math.floor(time)
        timeSecDecimal = time - timeRound
        timeSecWhole = timeSecDecimal * 100
        timeSecMins = timeSecWhole / 60
        timeTotal = timeRound + timeSecMins
        return timeTotal
### --- End minsecs_to_mins --- ###

def check_miles(miles):
    if np.isnan(miles):
        return 0.0
    else:
        return miles
### --- End check_miles --- ###

def print_inst(inst):
    for k in inst.keys():
        print('\n--  ', k, ' --')
        print(inst[k])
### --- End print_inst --- ###

def get_osrm_time_dist(pcfrom, pcto, cpo_inst):
    coords = []
    print('pcfrom: ', pcfrom)
    print('pcto: ', pcto)
    # lonlatfrom = cpo_inst.find_postcode_lonlat(inst['tasks']['postcode'][-2])
    lonlatfrom = cpo_inst.find_postcode_lonlat(pcfrom)
    coords.append(lonlatfrom)
    lonlatto = cpo_inst.find_postcode_lonlat(inst['tasks']['postcode'][-1])
    coords.append(lonlatto)

    coord_strings = ';'.join([','.join([str(i), str(j)]) for i, j in coords])
    # print('stringllj: ', stringllj)
    # print(type(stringllj))
        
    # Get results from osrm:
    server = r'localhost:5000'
    str_call = r'http://' + server + '/route/v1/driving/' + coord_strings + '?overview=false'
    print('str_call: ', str_call)
    # str_call = str_call + coord_strings + '?annotations=distance,duration'
    r = requests.get(str_call)
    osrmdict = r.json() # .json file has dictionary containing information on the route, including distance and duration.
    print(osrmdict)
    dist = osrmdict['routes'][0]['distance']
    dur = osrmdict['routes'][0]['duration']
    dur = dur/60.0
    print('distance: ', dist)
    print('duration: ', dur)
    exit(-1)

    return dist, dur
### --- End get_osrm_time_dist --- ###

def get_osrm(inst, cpo_inst):

    nNurses = inst['stats']['ncarers']
    nJobs = inst['stats']['ntasks']
    
    lonlat_jobs = []
    lonlat_jobs1 = []
    for i in range(nJobs):
        pcinst = inst['tasks']['postcode'][i]
        lonlat = cpo_inst.find_postcode_lonlat(pcinst) #Note that we're getting coords in order lon/lat, not lat/lon, so we don't need to swap them over for osrm.
        if i < 6:
            lonlat_jobs1.append(lonlat)
        lonlat_jobs.append(lonlat)
    lljSize = len(lonlat_jobs)
            
    coord_strings = ';'.join([','.join([str(i), str(j)]) for i, j in lonlat_jobs])
        
    # Get results from osrm:
    server = r'localhost:5000'
    str_call = r'http://' + server + '/table/v1/driving/' + coord_strings + '?annotations=distance,duration'
    r = requests.get(str_call)
    osrminst = r.json() # .json file has dictionary containing information on the route, including distance and duration.
    # print(osrminst)

    # Fill od matrix:
    distances = osrminst['distances']
    durations = osrminst['durations'] # NOTE: Given in seconds, /60 to get minutes.
    # print('durations shape: ', len(durations), ' distances shape: ', len(distances))
    # print('distances: ')
    # print(distances[:10])

    # Filling inst['tasks']['metresosrm'] and inst['tasks']['esttimeosrm'] lists for individual travel times and distances for each task, excluding the first task of each nurse (i.e. travel from nurse home to first job.)
    i = 0
    for j in range(nJobs):
        # print('i: ', i, 'j: ', j)
        if i < nNurses: # To prevent error when inst['rota']['firstjob'][i] goes out of range, i.e. when i > len(inst['rota']['firstjob']).
            if j == inst['rota']['firstjob'][i]: # If index j is the same as the index of the first job for a nurse, then just append 0 as we don't want time/distance to/from nurse's home to first/last job.
                inst['tasks']['metresosrm'].append(0.0)
                inst['tasks']['esttimeosrm'].append(0.0)
                i+=1
            else:
                dist_metres = distances[j-1][j]
                time_seconds = durations[j-1][j]
                time_mins = time_seconds/60
                inst['tasks']['metresosrm'].append(dist_metres)
                inst['tasks']['esttimeosrm'].append(time_mins)
        elif i >= nNurses:
            dist_metres = distances[j-1][j]
            time_seconds = durations[j-1][j]
            time_mins = time_seconds/60
            inst['tasks']['metresosrm'].append(dist_metres)
            inst['tasks']['esttimeosrm'].append(time_mins)
            
    
    # Filling inst['rota']['metresosrm'] and inst['rota']['travel_timeosrm'] lists for total travel time and metres for each nurse.
    i = 0
    for k in range(nNurses):
        num_tasks = inst['rota']['num_tasks'][k]
        nurse_metresosrm = 0
        nurse_timeosrm = 0
        for j in range(num_tasks):
            nurse_metresosrm = nurse_metresosrm + inst['tasks']['metresosrm'][i]
            nurse_timeosrm = nurse_timeosrm + inst['tasks']['esttimeosrm'][i]
            i+=1
        inst['rota']['metresosrm'].append(nurse_metresosrm)
        inst['rota']['travel_timeosrm'].append(nurse_timeosrm)

    # Fill in inst['rota']['wait_timeosrm]
    for i in range(nNurses):
        wait_timeosrm = inst['rota']['shift'][i] - inst['rota']['travel_timeosrm'][i] - inst['rota']['service_time'][i]
        inst['rota']['wait_timeosrm'].append(wait_timeosrm)
    
    # Update inst['stats'] for osrm values: ttravelmetresosrm, ttraveltimeosrm, and twaitingtimeosrm
    inst['stats']['ttravelmetresosrm'] = sum(inst['rota']['metresosrm'])
    inst['stats']['ttraveltimeosrm'] = sum(inst['rota']['travel_timeosrm'])
    inst['stats']['twaitingtimeosrm'] = inst['stats']['totaltime'] - inst['stats']['ttraveltimeosrm'] - inst['stats']['tservicetime']

    return inst
### --- End of def get_osrm --- ### 

cpo_inst = ccd.CPO_DF()

filename = '../data/abicare/exmileage.csv' #File from Abicare

df = pd.read_csv(filename)

# Remove 1st and 9th of November, as they seem incomplete
df = df[df['Date'] != '1-Nov-20']
df = df[df['Date'] != '9-Nov-20']

# Create new dfs that contain the date and start time of each job, and the date and end time of each job
df['start_dt'] = df['Date'] + ' ' + df['Start Time']
df['end_dt'] = df['Date'] + ' ' + df['End Time']
# Convert date (day(2)-MON(letters3)-year(2)) and time (12hour:minutes AM/PM) to year(4)-mon(2)-day(2) 24hr:mins:sec
df['start_dt'] = pd.to_datetime(df['start_dt'], format='%d-%b-%y %I:%M %p')
df['end_dt'] = pd.to_datetime(df['end_dt'], format='%d-%b-%y %I:%M %p')

# print('DF headers:')
# print(df.keys())
# ['Employee', 'Employee No', 'Employee Postcode', 'Area Code', 'Client',
#        'Address', 'Address2', 'Town', 'County', 'Postcode', 'Area', 'Funder',
#        'Date', 'Start Time', 'End Time', 'Pay Rate', 'Hours of call', 'Miles',
#        'Estimated Time (Mins)']

pdfinder = POSTCODE_FINDER() # Instantiate object

u_areas = df['Area'].unique() # List of Areas in df (exmileage.csv)
# print('There are', len(u_areas), 'areas:', u_areas)
days_in_df = df['Date'].unique() # List of Dates in df
days_in_df.sort() # Sort the dates in chronological order
all_instances = []
largeprint = True
timewindow_interval = datetime.timedelta(minutes = 15) # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
only_areas = ['Aldershot']
min_addresses = 10
# largeprintstr = ''

for a_name in u_areas: # For each area in the list of Areas
    if not (a_name in only_areas): # If current area is not Salisbury, go to next area in list of areas
        continue

    area_df = df[df['Area'] == a_name] # area_df = df but only containing info for current area
    for day in days_in_df: # For each day in list of Dates
        day_df = area_df[area_df['Date'] == day] # day_df = df but only containing infor for current area (area_df) and current day
        carers_working = day_df['Employee'].unique() # List of carers working on current day

        inst = {
                'name' : a_name + '_' + day.replace('-', '_'),
                'fname' : day.replace('-', '_') + '_' + a_name, # filename
                'area' : a_name,
                'date' : day,
                'stats' : {'ncarers' : 0, 'ntasks' : 0, 'totaltime' : 0, 'tservicetime' : 0, 'ttraveltime' : 0, 'ttraveltimeosrm' : 0, 'twaitingtime' : 0, 'twaitingtimeosrm' : 0, 'tgaptime' : 0, 'ttravelmiles' : 0, 'ttravelmetres' : 0, 'ttravelmetresosrm' : 0}, #ttravelm = ttravel in metres.
                'txtsummary' : '',
                'rota' : {'carer' : [], 'postcode' : [], 'num_addr' : [], 'eastings': [], 'northings' : [], 'start' : [], 'finish' : [], 'shift' : [], 'num_tasks' : [], 'firstjob' : [], 'travel_time' : [], 'travel_timeosrm' : [], 'wait_time' : [], 'wait_timeosrm' : [], 'metres' : [], 'metresosrm' : [], 'service_time' : []},
                'tasks' : {'client' : [], 'postcode': [], 'num_addr': [], 'eastings': [], 'northings' : [], 'duration' : [], 'miles' : [], 'metres' : [], 'metresosrm' : [], 'esttime' : [], 'esttimeosrm' : [], 'start' : [], 'end' : [], 'tw_start' : [], 'tw_end' : []},
                'routes' : []
                }
                #['rota']['start'] is the start time of the carer IN MINUTES FROM MIDNIGHT (I.E. 12:00AM)
                #['rota']['end'] is the end time of the carer IN MINUTES FROM MIDNIGHT (I.E. 12:00AM)
        # print(day)        
        start_of_day = day_df.iloc[0]['start_dt'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.

        set_info_08hants = 0
        set_info_06aldershot = 0
        index_task = 0
        firstjobindex = 0
        for carer in carers_working: # For each carer working on the current day (in the list of carers)
            inst['rota']['firstjob'].append(firstjobindex)
            carer_df = day_df[day_df['Employee'] == carer] # carer_df = df but only containing info for current area, current day, and current carer
            inst['stats']['ncarers'] += 1 # Increase number of carers by one
            inst['rota']['carer'].append(carer) # Add current carer to the rota
            inst['rota']['postcode'].append(str(carer_df.iloc[0]['Employee Postcode']).lower().replace(' ', '')) # Add postcode of current carer's home to the rota
            nad = pdfinder.find_postcode_n_addresses(inst['rota']['postcode'][-1]) # nad = the number of addresses at the carer's postcode ([-1] is the last postcode in the list)
            inst['rota']['num_addr'].append(nad) # Add number of addresses at the postcode to the rota
            if nad >= min_addresses: # If there exist a sufficient number of properties at the postcode (>= 10), then we can obtain the latitude and longitude of the postcode
                eastings, northings = pdfinder.find_postcode_eastnorth(inst['rota']['postcode'][-1])
            else: # Too few properties at the postcode, cannot obtain latitude and longitude for privacy reasons.
                eastings, northings = [None, None]

            inst['rota']['eastings'].append(eastings) # Add latitude to the rota
            inst['rota']['northings'].append(northings) # Add longitude to the rota
            inst['rota']['start'].append([60*24]) # Add start and end times to the rota
            inst['rota']['finish'].append([0])
            inst['rota']['num_tasks'].append(len(carer_df)) # Add the number of jobs that each carer has that day.
            
            carer_route = []
            if eastings != None: # Add latitude and longitude to carer_route if they exist
                carer_route.append([eastings, northings])

            prev_finish = -1
            travel_until_here = 0
            gap_until_here = 0
            waiting_until_here = 0 # New
            carer_waiting = 0 # New
            carer_travelling = 0 # New
            carer_distance = 0 # NEW 08/01
            miles_until_here = 0
            metres_until_here = 0 # NEW 08/01
            carer_service = 0 # New
            carer_row = 0
            for index, row in carer_df.iterrows():
                inst['stats']['ntasks'] += 1 # Increase number of tasks (jobs) by one
                # Update carer working times
                sov_mins = time_dif_to_minutes(row['start_dt'], start_of_day) # Changed from row['end_dt'] to row['start_dt']
                eov_mins = time_dif_to_minutes(row['end_dt'], start_of_day)
                if inst['rota']['start'][-1] > sov_mins:
                    inst['rota']['start'][-1] = sov_mins

                if inst['rota']['finish'][-1] < eov_mins:
                    inst['rota']['finish'][-1] = eov_mins

                # Task info:
                inst['tasks']['client'].append(row['Client']) # Add client 'Client #' to tasks list
                inst['tasks']['postcode'].append(row['Postcode'].lower().replace(' ', '')) # Add postcode of client to tasks list
                nad = pdfinder.find_postcode_n_addresses(inst['tasks']['postcode'][-1]) # nad = number of addresses at client's postcode
                inst['tasks']['num_addr'].append(nad) # Add nad to tasks list
                if nad >= min_addresses: # If sufficient # of addresses at postcode, get latitude and longitude, else cannot for privacy reasons
                    eastings, northings = pdfinder.find_postcode_eastnorth(inst['tasks']['postcode'][-1])
                else:
                    eastings, northings = [None, None]

                if eastings != None: # Add latitude and longitude of client's address to carer's route if allowed.
                    carer_route.append([eastings, northings])

                inst['tasks']['eastings'].append(eastings) # Add latitude, longitude, and duration of job to tasks list
                inst['tasks']['northings'].append(northings)
                duration = time_dif_to_minutes(row['end_dt'], row['start_dt']) # Duration of task/job.
                inst['tasks']['duration'].append(duration)
                carer_service += duration # New
                inst['tasks']['start'].append(time_dif_to_minutes(row['start_dt'], start_of_day))
                inst['tasks']['end'].append(time_dif_to_minutes(row['end_dt'], start_of_day))

                twstart_td = row['start_dt'] - timewindow_interval #TW is allowed around start time of job
                twend_td = row['start_dt'] + timewindow_interval
                inst['tasks']['tw_start'].append(time_dif_to_minutes(twstart_td, start_of_day)) # Add TW start and end to tasks list
                inst['tasks']['tw_end'].append(time_dif_to_minutes(twend_td, start_of_day))
                if inst['area'] == 'Aldershot' and inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114':
                    print('here1')
                    set_info_06aldershot += 1
                
                miles = 0
                metres = 0
                travel_time_mins = 0
                if inst['area'] == 'Hampshire' and inst['date'] == '8-Nov-20' and carer == 'Carer 79' and inst['tasks']['client'][-1] == 'Client 86' and set_info_08hants == 0:
                    print('here')
                    metres = 10323.2
                    travel_time_mins = 10.593
                    miles = 6.413
                    inst['tasks']['miles'].append(miles)
                    inst['tasks']['metres'].append(metres)
                    inst['tasks']['esttime'].append(travel_time_mins) # New
                    set_info_08hants = 1
                elif inst['area'] == 'Aldershot' and inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114' and set_info_06aldershot == 3:
                    print('here2')
                    metres = 0.0
                    travel_time_mins = 0.0
                    miles = 0.0
                    inst['tasks']['miles'].append(miles)
                    inst['tasks']['metres'].append(metres)
                    inst['tasks']['esttime'].append(travel_time_mins) # New
                else:
                    miles = check_miles(row['Miles'])
                    inst['tasks']['miles'].append(miles) # NEW 09/01
                    metres = miles_to_metres(row['Miles'])
                    inst['tasks']['metres'].append(metres)
                    travel_time_mins = minsecs_to_mins(row['Estimated Time (Mins)']) # New
                    inst['tasks']['esttime'].append(travel_time_mins) # New

                # miles = check_miles(row['Miles'])
                # inst['tasks']['miles'].append(miles) # NEW 09/01
                # metres = miles_to_metres(row['Miles'])
                # inst['tasks']['metres'].append(metres)
                # travel_time_mins = minsecs_to_mins(row['Estimated Time (Mins)']) # New
                # inst['tasks']['esttime'].append(travel_time_mins) # New
                
                # Solution info:
                if carer_row > 0:
                    travel_until_here = travel_time_mins # New
                    miles_until_here = miles # NEW 09/01                  
                    metres_until_here = metres # NEW 08/01                    
                    gap_until_here = time_dif_to_minutes(row['start_dt'], prev_finish) # This is the time in minutes from the end time of the previous job to the start time of the current job
                    waiting_until_here = gap_until_here - travel_time_mins # New: waiting time between when carer arrives at job (end time of prev job + travel time) and the start time of the job.

                inst['stats']['tservicetime'] += duration
                inst['stats']['ttraveltime'] += travel_until_here
                inst['stats']['ttravelmiles'] += miles_until_here
                inst['stats']['ttravelmetres'] += metres_until_here # NEW 08/01
                inst['stats']['tgaptime'] += gap_until_here
                inst['stats']['twaitingtime'] += waiting_until_here # New
                carer_travelling += travel_until_here # New
                carer_waiting += waiting_until_here # New
                carer_distance += metres_until_here
                carer_row += 1
                prev_finish = row['end_dt']
                index_task +=1
                firstjobindex +=1
            # End of for index, row in carer_df.iterrows() loop
            inst['rota']['travel_time'].append(carer_travelling) # New
            inst['rota']['wait_time'].append(carer_waiting) # New
            inst['rota']['service_time'].append(carer_service) # New
            inst['rota']['metres'].append(carer_distance) # NEW 08/01
            shift_duration = inst['rota']['finish'][-1] - inst['rota']['start'][-1] # New
            inst['rota']['shift'].append(shift_duration) # New

            # Close carer route:
            if eastings != None:
                carer_route.append([eastings, northings])
            inst['routes'].append(carer_route.copy())
        #End of for carer in carers_working loop
        inst['stats']['totaltime'] = inst['stats']['ttraveltime'] + inst['stats']['tservicetime'] + inst['stats']['twaitingtime']
        inst = get_osrm(inst, cpo_inst)
        all_instances.append(inst_dicts_to_dfs(inst).copy())
        pickle.dump(all_instances, open('cpo_all_inst_' + str(inst['area']) + '.p', 'wb'))
    # End of for day in days_in_df loop
# End of for a_name in u_areas loop
print('END.')


#print('Last instance:')
#print(print_inst(all_instances[-1]))

# Get the data without NaNs
# x_full = df['Miles'].values
# y_full = df['Estimated Time (Mins)'].values
# mask = ~np.isnan(x_full) & ~np.isnan(y_full)

# x, y = x_full[mask] ,y_full[mask]

# Linear regression
# slope, intercept, r_value, p_value, std_err = stats.linregress(x, y)
#print('Time = ', slope, '*Miles +', intercept)
#print('r_value:', r_value)
#print('p_value:', p_value)
#print('std_err:', std_err)

# Polynomial fit:
# pfit = np.polyfit(x, y, 2)
# polyfit = np.poly1d(pfit)

# # Plot the points:
# plt.scatter(x, y, marker='x', color='black')
# plt.xlabel('Distance (miles)')
# plt.ylabel('Time (mins)')
# # Plot the regression line:
# xmin, xmax = np.min(x), np.max(x)
# x_reg_line = np.array([xmin, xmax])
# y_reg_line = intercept + slope*x_reg_line
# plt.plot(x_reg_line, y_reg_line, color='r')

# # And the polynomial:
# x_poly = np.linspace(xmin, xmax, 100)
# plt.plot(x_poly, polyfit(x_poly), color='g')


#plt.show()