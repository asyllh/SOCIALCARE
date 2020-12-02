import pandas as pd
from scipy import stats
import numpy as np
import matplotlib.pyplot as plt
import datetime
import pickle

class POSTCODE_FINDER():
    """
    docstring
    """
    df = []
    filename = []
    def __init__(self, filename=r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\data\postcode\salisbury_postcode_lookup.csv'):
        self.filename = filename
        self.df = pd.read_csv(self.filename)
        self.df['post'] = self.df['Postcode 1'].str.replace(' ', '')
        self.df['post'] = self.df['post'].str.lower()
        self.n_adr_filename = r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\data\postcode\salisbury_full_codepoint.csv'
        self.adr_df = pd.read_csv(self.n_adr_filename)
        self.adr_df['post'] = self.adr_df['PC'].str.replace('"', '')
        self.adr_df['post'] = self.adr_df['post'].str.replace(' ', '') # Changed from self.adr_df['PC'].str.replace(' ', '') to self.adr_df['post'].str.replace(' ', '')
        self.adr_df['post'] = self.adr_df['post'].str.lower()

    def find_postcode_latlon(self, postcode): # Returns Latitude and Longitude of given postcode.
        dfview = self.df[self.df['post'] == postcode]
        if dfview.empty:
            return [None, None]
        else:
            return [dfview.iloc[0]['Latitude'], dfview.iloc[0]['Longitude']]
    
    def find_postcode_n_addresses(self, postcode): # Returns number of registered properties at a given postcode.
        dfview = self.adr_df[self.adr_df['post'] == postcode]
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
only_areas = ['Salisbury']
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
                'area' : a_name,
                'date' : day,
                'stats' : {'ncarers' : 0, 'ntasks' : 0, 'ttraveltime' : 0, 'ttravelmiles' : 0, 'tservicetime' : 0, 'tgaptime' : 0},
                'txtsummary' : '',
                'rota' : {'carer' : [], 'postcode' : [], 'num_addresses' : [], 'lat': [], 'lon' : [], 'start' : [], 'finish' : []},
                'tasks' : {'client' : [], 'postcode': [], 'num_addresses': [], 'lat': [], 'lon' : [], 'duration' : [], 'tw_start' : [], 'tw_end' : []},
                'routes' : []
                }
        start_of_day = day_df.iloc[0]['start_dt'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.
        # print('start_of_day:')
        # print(start_of_day)
        # print('day_df.iloc[0][start_dt]:')
        # print(day_df.iloc[0]['start_dt'])
        # end_of_day = day_df.iloc[0]['end_dt'].replace(hour=23, minute=59, second=59)

        for carer in carers_working: # For each carer working on the current day (in the list of carers)
            carer_df = day_df[day_df['Employee'] == carer] # carer_df = df but only containing info for current area, current day, and current carer
            inst['stats']['ncarers'] += 1 # Increase number of carers by one
            inst['rota']['carer'].append(carer) # Add current carer to the rota
            inst['rota']['postcode'].append(str(carer_df.iloc[0]['Employee Postcode']).lower().replace(' ', '')) # Add postcode of current carer's home to the rota
            nad = pdfinder.find_postcode_n_addresses(inst['rota']['postcode'][-1]) # nad = the number of addresses at the carer's postcode ([-1] is the last postcode in the list)
            inst['rota']['num_addresses'].append(nad) # Add number of addresses at the postcode to the rota
            if nad >= min_addresses: # If there exist a sufficient number of properties at the postcode (>= 10), then we can obtain the latitude and longitude of the postcode
                lat, lon = pdfinder.find_postcode_latlon(inst['rota']['postcode'][-1])
            else: # Too few properties at the postcode, cannot obtain latitude and longitude for privacy reasons.
                lat, lon = [None, None]

            inst['rota']['lat'].append(lat) # Add latitude to the rota
            inst['rota']['lon'].append(lon) # Add longitude to the rota
            inst['rota']['start'].append([60*24]) # Add start and end times to the rota
            inst['rota']['finish'].append([0])

            carer_route = []
            if lat != None: # Add latitude and longitude to carer_route if they exist
                carer_route.append([lat, lon])

            if largeprint:
                largeprintstr += '\n' + '  ' + carer + '- visits (' + str(len(carer_df['Employee'])) + '):'

            prev_finish = -1
            travel_until_here = 0
            gap_until_here = 0
            miles_until_here = 0
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

                duration = time_dif_to_minutes(row['end_dt'], row['start_dt']) # Duration of task/job.

                # Task info:
                inst['tasks']['client'].append(row['Client']) # Add client 'Client #' to tasks list
                inst['tasks']['postcode'].append(row['Postcode'].lower().replace(' ', '')) # Add postcode of client to tasks list
                nad = pdfinder.find_postcode_n_addresses(inst['rota']['postcode'][-1]) # nad = number of addresses at client's postcode
                inst['tasks']['num_addresses'].append(nad) # Add nad to tasks list
                if nad >= min_addresses: # If sufficient # of addresses at postcode, get latitude and longitude, else cannot for privacy reasons
                    lat, lon = pdfinder.find_postcode_latlon(inst['tasks']['postcode'][-1])
                else:
                    lat, lon = [None, None]

                if lat != None: # Add latitude and longitude of client's address to carer's route if allowed.
                    carer_route.append([lat, lon])

                inst['tasks']['lat'].append(lat) # Add latitude, longitude, and duration of job to tasks list
                inst['tasks']['lon'].append(lon)
                inst['tasks']['duration'].append(duration)

                twstart_td = row['start_dt'] - timewindow_interval #TW is allowed around start time of job
                twend_td = row['start_dt'] + timewindow_interval
                inst['tasks']['tw_start'].append(time_dif_to_minutes(twstart_td, start_of_day)) # Add TW start and end to tasks list
                inst['tasks']['tw_end'].append(time_dif_to_minutes(twend_td, start_of_day))

                # Solution info:
                if carer_row > 0:
                    travel_until_here = row['Estimated Time (Mins)']
                    miles_until_here = row['Miles']
                    gap_until_here = time_dif_to_minutes(row['start_dt'], prev_finish)
                if largeprint:
                    largeprintstr += '\n' + '    - ' + row['Client'] + '(' + row['Postcode'] + ')'
                    largeprintstr += ' from ' + row['Start Time'] + 'to' + row['End Time']
                    largeprintstr += '(' + str(duration) + ' m) TimeToArrive: ' + str(travel_until_here) + 'm VisitGap' + str(gap_until_here) + 'm)'

                inst['stats']['tservicetime'] += duration
                inst['stats']['ttraveltime'] += travel_until_here
                inst['stats']['ttravelmiles'] += miles_until_here
                inst['stats']['tgaptime'] += gap_until_here

                carer_row += 1
                prev_finish = row['end_dt']

            # Close carer route:
            if lat != None:
                carer_route.append([lat, lon])

            inst['routes'].append(carer_route.copy())
    
        inst['txtsummary'] = largeprintstr
        all_instances.append(inst_dicts_to_dfs(inst).copy())
        pickle.dump(all_instances, open('all_inst_salisbury.p', 'wb'))

#print('Last instance:')
#print(print_inst(all_instances[-1]))

# Get the data without NaNs
x_full = df['Miles'].values
y_full = df['Estimated Time (Mins)'].values
mask = ~np.isnan(x_full) & ~np.isnan(y_full)

x, y = x_full[mask] ,y_full[mask]

# Linear regression
slope, intercept, r_value, p_value, std_err = stats.linregress(x, y)
#print('Time = ', slope, '*Miles +', intercept)
#print('r_value:', r_value)
#print('p_value:', p_value)
#print('std_err:', std_err)

# Polynomial fit:
pfit = np.polyfit(x, y, 2)
polyfit = np.poly1d(pfit)

# Plot the points:
plt.scatter(x, y, marker='x', color='black')
plt.xlabel('Distance (miles)')
plt.ylabel('Time (mins)')
# Plot the regression line:
xmin, xmax = np.min(x), np.max(x)
x_reg_line = np.array([xmin, xmax])
y_reg_line = intercept + slope*x_reg_line
plt.plot(x_reg_line, y_reg_line, color='r')

# And the polynomial:
x_poly = np.linspace(xmin, xmax, 100)
plt.plot(x_poly, polyfit(x_poly), color='g')


#plt.show()