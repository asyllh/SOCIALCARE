#--------------------#
# fc_analyse_mileage.py
# 17/12/2020
# 
#--------------------#

import math
import pickle
import datetime
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

# fc_analyse_mileage.py: full codepoint analyse mileage.

class POSTCODE_FINDER():
    """
    docstring
    """
    df = []
    filename = []
    def __init__(self, filename=r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\postcode\full_codepoint.csv'):
        self.filename = filename
        # self.df = pd.read_csv(self.filename)
        # self.df['post'] = self.df['Postcode 1'].str.replace(' ', '')
        # self.df['post'] = self.df['post'].str.lower()
        # self.n_adr_filename = r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\postcode\full_codepoint.csv'
        self.df = pd.read_csv(self.filename)
        self.df['PC'] = self.df['PC'].str.replace('"', '')
        self.df['PC'] = self.df['PC'].str.replace(' ', '') # Changed from self.adr_df['PC'].str.replace(' ', '') to self.adr_df['post'].str.replace(' ', '')
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

def time_dif_to_minutes(endtime, starttime, round=True):
    mins = (endtime - starttime).seconds/60
    if round:
        return np.round(mins, 0)
    else:
        return mins

def inst_dicts_to_dfs(inst):
    inst['rota'] = pd.DataFrame(inst['rota'])
    inst['tasks'] = pd.DataFrame(inst['tasks'])
    return inst

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

def print_inst(inst):
    for k in inst.keys():
        print('\n--  ', k, ' --')
        print(inst[k])

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
print('There are', len(u_areas), 'areas:', u_areas)
days_in_df = df['Date'].unique() # List of Dates in df
days_in_df.sort() # Sort the dates in chronological order
all_instances = []
largeprint = True
timewindow_interval = datetime.timedelta(minutes = 15) # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
only_areas = ['None']
min_addresses = 10
largeprintstr = ''

for a_name in u_areas: # For each area in the list of Areas
    if not (a_name in only_areas): # If current area is not Salisbury, go to next area in list of areas
        continue
    if largeprint:
        largeprintstr += '\n' + '###' + a_name + '###\n'

    area_df = df[df['Area'] == a_name] # area_df = df but only containing info for current area
    for day in days_in_df: # For each day in list of Dates
        day_df = area_df[area_df['Date'] == day] # day_df = df but only containing infor for current area (area_df) and current day
        carers_working = day_df['Employee'].unique() # List of carers working on current day
        if largeprint:
            largeprintstr += '\n' + '' + day + '-' + str(len(carers_working)) + 'carers'

        inst = {
                'name' : a_name + '_' + day.replace('-', '_'),
                'fname' : day.replace('-', '_') + '_' + a_name, # filename
                'area' : a_name,
                'date' : day,
                'stats' : {'ncarers' : 0, 'ntasks' : 0, 'ttraveltime' : 0, 'ttravelmiles' : 0, 'tservicetime' : 0, 'tgaptime' : 0, 'twaitingtime' : 0, 'totaltime' : 0},
                'txtsummary' : '',
                'rota' : {'carer' : [], 'postcode' : [], 'num_addr' : [], 'eastings': [], 'northings' : [], 'start' : [], 'finish' : [], 'shift' : [], 'home_start' : [], 'home_finish' : [], 'num_tasks' : [], 'travel_time' : [], 'wait_time' : [], 'service_time' : []},
                'tasks' : {'client' : [], 'postcode': [], 'num_addr': [], 'eastings': [], 'northings' : [], 'duration' : [], 'miles' : [], 'metres' : [], 'esttime' : [], 'start' : [], 'end' : [], 'tw_start' : [], 'tw_end' : []},
                'routes' : []
                }
                #['rota']['start'] is the start time of the carer IN MINUTES FROM MIDNIGHT (I.E. 12:00AM)
                #['rota']['end'] is the end time of the carer IN MINUTES FROM MIDNIGHT (I.E. 12:00AM)
        # print(day)        
        start_of_day = day_df.iloc[0]['start_dt'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.

        # set_info_08hants = 0
        # set_info_06aldershot = 0
        for carer in carers_working: # For each carer working on the current day (in the list of carers)
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

            if largeprint:
                largeprintstr += '\n' + '  ' + carer + '- visits (' + str(len(carer_df['Employee'])) + '):'

            prev_finish = -1
            travel_until_here = 0
            gap_until_here = 0
            waiting_until_here = 0 # New
            carer_waiting = 0 # New
            carer_travelling = 0 # New
            miles_until_here = 0
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
                # if inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114':
                #     print('here1')
                #     set_info_06aldershot += 1
                
                metres = 0
                travel_time_mins = 0
                # if inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114' and set_info_06aldershot == 3:
                #     print('here2')
                #     metres = 0.0
                #     travel_time_mins = 0.0
                #     miles = 0.0
                #     inst['tasks']['miles'].append(miles)
                #     inst['tasks']['metres'].append(metres)
                #     inst['tasks']['esttime'].append(travel_time_mins) # New
                # if inst['date'] == '8-Nov-20' and carer == 'Carer 79' and inst['tasks']['client'][-1] == 'Client 86' and set_info_08hants == 0:
                #     print('here')
                #     metres = 10323.2
                #     travel_time_mins = 10.593
                #     miles = 6.413
                #     inst['tasks']['miles'].append(miles)
                #     inst['tasks']['metres'].append(metres)
                #     inst['tasks']['esttime'].append(travel_time_mins) # New
                #     set_info_08hants = 1
                # else:
                #     if np.isnan(row['Miles']):
                #         inst['tasks']['miles'].append(0.0)
                #     else:
                #         inst['tasks']['miles'].append(row['Miles'])
                #     metres = miles_to_metres(row['Miles'])
                #     inst['tasks']['metres'].append(metres)
                #     travel_time_mins = minsecs_to_mins(row['Estimated Time (Mins)']) # New
                #     inst['tasks']['esttime'].append(travel_time_mins) # New
                if np.isnan(row['Miles']):
                    inst['tasks']['miles'].append(0.0)
                else:
                    inst['tasks']['miles'].append(row['Miles'])
                metres = miles_to_metres(row['Miles'])
                inst['tasks']['metres'].append(metres)
                travel_time_mins = minsecs_to_mins(row['Estimated Time (Mins)']) # New
                inst['tasks']['esttime'].append(travel_time_mins) # New
                

                # Solution info:
                if carer_row > 0:
                    # travel_until_here = row['Estimated Time (Mins)']
                    travel_until_here = travel_time_mins # New
                    # if inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114' and set_info_06aldershot == 3:
                    #     print('here3')
                    #     miles_until_here = 0.0
                    #     set_info_06aldershot = 10
                    # else:
                    #     if np.isnan(row['Miles']):
                    #         miles_until_here = 0.0
                    #     else:    
                    #         miles_until_here = row['Miles']
                    if np.isnan(row['Miles']):
                        miles_until_here = 0.0
                    else:    
                        miles_until_here = row['Miles']
                    # print(row['start_dt'])
                    # print(row['end_dt'])
                    gap_until_here = time_dif_to_minutes(row['start_dt'], prev_finish) # This is the time in minutes from the end time of the previous job to the start time of the current job
                    waiting_until_here = gap_until_here - travel_time_mins # New: waiting time between when carer arrives at job (end time of prev job + travel time) and the start time of the job.

                    # exit(-1)
                if largeprint:
                    largeprintstr += '\n' + '    - ' + row['Client'] + '(' + row['Postcode'] + ')'
                    largeprintstr += ' from ' + row['Start Time'] + 'to' + row['End Time']
                    largeprintstr += '(' + str(duration) + ' m) TimeToArrive: ' + str(travel_until_here) + 'm VisitGap' + str(gap_until_here) + 'm)'

                inst['stats']['tservicetime'] += duration
                inst['stats']['ttraveltime'] += travel_until_here
                inst['stats']['ttravelmiles'] += miles_until_here
                inst['stats']['tgaptime'] += gap_until_here
                inst['stats']['twaitingtime'] += waiting_until_here # New
                carer_travelling += travel_until_here # New
                carer_waiting += waiting_until_here # New
                carer_row += 1
                prev_finish = row['end_dt']
            # End of for index, row in carer_df.iterrows() loop
            inst['rota']['travel_time'].append(carer_travelling) # New
            inst['rota']['wait_time'].append(carer_waiting) # New
            inst['rota']['service_time'].append(carer_service) # New

            shift_duration = inst['rota']['finish'][-1] - inst['rota']['start'][-1] # New
            inst['rota']['shift'].append(shift_duration) # New
            home_start = inst['rota']['start'][-1] - 30 # New
            home_finish = inst['rota']['finish'][-1] + 30 # New
            inst['rota']['home_start'].append(home_start) # New
            inst['rota']['home_finish'].append(home_finish) # New

            # Close carer route:
            if eastings != None:
                carer_route.append([eastings, northings])
            inst['routes'].append(carer_route.copy())
        #End of for carer in carers_working loop
        inst['stats']['totaltime'] = inst['stats']['ttraveltime'] + inst['stats']['tservicetime'] + inst['stats']['twaitingtime']

        inst['txtsummary'] = largeprintstr
        all_instances.append(inst_dicts_to_dfs(inst).copy())
        pickle.dump(all_instances, open('all_inst_' + str(inst['area']) + '.p', 'wb'))
    # End of for day in days_in_df loop
# End of for a_name in u_areas loop

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