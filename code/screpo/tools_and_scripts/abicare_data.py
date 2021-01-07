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

    def find_postcode_latlon(self, postcode): # Returns Latitude and Longitude of given postcode.
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
# print('There are', len(u_areas), 'areas:', u_areas)
days_in_df = df['Date'].unique() # List of Dates in df
days_in_df.sort() # Sort the dates in chronological order
all_instances = []
largeprint = True
timewindow_interval = datetime.timedelta(minutes = 15) # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
only_areas = ['Hampshire']
min_addresses = 10
largeprintstr = ''

for a_name in u_areas: # For each area in the list of Areas
    if not (a_name in only_areas): # If current area is not Salisbury, go to next area in list of areas
        continue
    # if largeprint:
    #     largeprintstr += '\n' + '###' + a_name + '###\n'
    print(a_name)

    area_df = df[df['Area'] == a_name] # area_df = df but only containing info for current area
    print('Number of tasks: ', len(area_df))
    carers_df = area_df['Employee'].unique()
    print('Number of carers: ', len(carers_df))
    clients_df = area_df['Client'].unique()
    print('Number of clients: ', len(clients_df))
    print('')

    clients_df.sort() 
    for client in clients_df: # For each day in list of Dates
        clientsnum_df = area_df[area_df['Client'] == client] # day_df = df but only containing infor for current area (area_df) and current day
        print(client, '\tnum_tasks: ', len(clientsnum_df))
    # print('\nEnd.\n')
    print('')

    for day in days_in_df: # For each day in list of Dates
        day_df = area_df[area_df['Date'] == day] # day_df = df but only containing infor for current area (area_df) and current day
        print(day)
        print('num_tasks: ', len(day_df))
        carer_day_df = day_df['Employee'].unique()
        print('num_carers: ', len(carer_day_df))
        client_day_df = day_df['Client'].unique()
        print('num_clients: ', len(client_day_df))
        print('')

        # client_day_df.sort()
        # for client in client_day_df: # For each day in list of Dates
        #     clientnumday_df = day_df[day_df['Client'] == client] # day_df = df but only containing infor for current area (area_df) and current day
        #     print('Client: ', client, '\tnum tasks: ', len(clientnumday_df))
        # break

        # carers_working = day_df['Employee'].unique() # List of carers working on current day
        # print('Number of carers: ', len(carers_working))
        
    # End of for day in days_in_df loop
# End of for a_name in u_areas loop
print('\nEnd.\n')
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