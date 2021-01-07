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


def inst_dicts_to_dfs(inst):
    inst['stats'] = pd.DataFrame(inst['stats'])
    inst['abi'] = pd.DataFrame(inst['abi'])
    inst['dst'] = pd.DataFrame(inst['dst'])
    return inst
### --- End inst_dicts_to_dfs --- ###

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

def create_df(filename, a_name=''):
    # filename = '../outputs/results/HC.csv' #File from Abicare
    # a_name = 'Hampshire'
    df = pd.read_csv(filename)
    # df['Date'] = pd.to_datetime(df['Date'], format='%d-%b-%y %I:%M %p')

    inst = {
            'name' : a_name,
            'all_dates' : [],
            'nNurses' : 0,
            'nJobs' : 0,
            'nClients' : 0,
            'stats' : {'nurses' : [], 'jobs' : [], 'clients' : [], 'servicetime' : []},
            'abi' : {'ttime' : [], 'ttravel' : [], 'ttravelosrm' : [], 'twaiting' : [], 'twaitingosrm' : [], 'dist' : [], 'distjobs' : [], 'distjobsosrm' : [], 'aith': [], 'mk' : [], 'wb' : [], 'paper' : []},
            'dst' : {'ttime' : [], 'ttravel' : [], 'twaiting' : [], 'dist' : [], 'distjobs' : [], 'aith': [], 'mk' : [], 'wb' : [], 'paper' : [], 'algtime' : [], 'iterations' : []},
            'best' : {'ttime' : [], 'ttravel' : [], 'ttravelosrm' : [], 'twaiting' : [], 'twaitingosrm' : [], 'dist' : [], 'distjobs' : [], 'distjobsosrm' : [], 'aith' : [], 'mk' : [], 'wb' : [], 'paper' : []}
            }

    inst['nNurses'] = df.iloc[0]['nNurses']
    inst['nJobs'] = df.iloc[0]['nJobs']
    inst['nClients'] = df.iloc[0]['nClients']

    days_in_df = df['Date'].unique() # List of Dates in df
    # print(days_in_df)

    # exit(-1)

    for day in days_in_df:
        day_df = df[df['Date'] == day] # day_df = df but only containing infor for current area (area_df) and current day
        inst['all_dates'].append(day)
        inst['stats']['nurses'].append(day_df.iloc[0]['Nurses'])
        inst['stats']['jobs'].append(day_df.iloc[0]['Jobs'])
        inst['stats']['clients'].append(day_df.iloc[0]['Clients'])
        inst['stats']['servicetime'].append(day_df.iloc[0]['Service Time'])

        inst['abi']['ttime'].append(day_df.iloc[0]['Total Abi'])
        inst['abi']['ttravel'].append(day_df.iloc[0]['Travel Abi'])
        inst['abi']['ttravelosrm'].append(day_df.iloc[0]['Travel AbiOsrm'])
        inst['abi']['twaiting'].append(day_df.iloc[0]['Waiting Abi'])
        inst['abi']['twaitingosrm'].append(day_df.iloc[0]['Waiting AbiOsrm'])
        inst['abi']['dist'].append(day_df.iloc[0]['Dist Abi'])
        inst['abi']['distjobs'].append(day_df.iloc[0]['DistJobs Abi'])
        inst['abi']['distjobsosrm'].append(day_df.iloc[0]['DistJobs AbiOsrm'])

        inst['abi']['aith'].append(day_df.iloc[0]['AitH Abi'])
        inst['abi']['mk'].append(day_df.iloc[0]['MK Abi'])
        inst['abi']['wb'].append(day_df.iloc[0]['WB Abi'])
        inst['abi']['paper'].append(day_df.iloc[0]['Paper Abi'])

        inst['dst']['ttime'].append(day_df.iloc[0]['Total DST'])
        inst['dst']['ttravel'].append(day_df.iloc[0]['Travel DST'])
        inst['dst']['twaiting'].append(day_df.iloc[0]['Waiting DST'])
        inst['dst']['dist'].append(day_df.iloc[0]['Dist DST'])
        inst['dst']['distjobs'].append(day_df.iloc[0]['DistJobs DST'])

        inst['dst']['aith'].append(day_df.iloc[0]['AitH DST'])
        inst['dst']['mk'].append(day_df.iloc[0]['MK DST'])
        inst['dst']['wb'].append(day_df.iloc[0]['WB DST'])
        inst['dst']['paper'].append(day_df.iloc[0]['Paper DST'])
        inst['dst']['algtime'].append(day_df.iloc[0]['Time DST'])
        inst['dst']['iterations'].append(day_df.iloc[0]['Iterations DST'])

        inst['best']['ttime'].append(day_df.iloc[0]['Total Best'])
        inst['best']['ttravel'].append(day_df.iloc[0]['Travel Best'])
        inst['best']['ttravelosrm'].append(day_df.iloc[0]['Travel Osrm Best'])
        inst['best']['twaiting'].append(day_df.iloc[0]['Waiting Best'])
        inst['best']['twaitingosrm'].append(day_df.iloc[0]['Waiting Osrm Best'])
        inst['best']['dist'].append(day_df.iloc[0]['Dist Best'])
        inst['best']['distjobs'].append(day_df.iloc[0]['DistJobs Best'])
        inst['best']['distjobsosrm'].append(day_df.iloc[0]['DistJobs Osrm Best'])

        inst['best']['aith'].append(day_df.iloc[0]['AitH Best'])
        inst['best']['mk'].append(day_df.iloc[0]['MK Best'])
        inst['best']['wb'].append(day_df.iloc[0]['WB Best'])
        inst['best']['paper'].append(day_df.iloc[0]['Paper Best'])
        # print('nurses: ', inst['stats']['nurses'])
        # exit(-1)
    # inst_dicts_to_dfs(inst)

    return inst
    # pickle.dump(inst, open(str(inst['name']) + '_compare_abi_dst.p', 'wb'))
    # print('End.')
### --- End create_df --- ###

def compare_abi_dst():
    all_instances = pickle.load(open('all_compare_abi_dst.p', 'rb'))
    h_dict = all_instances[0]
    m_dict = all_instances[1]
    a_dict = all_instances[2]
    area_name = 'All Areas'
    # print(ad_dict['abi'])
    # area_name = ad_dict['name']
    # print(h_dict['name'])
    # print(m_dict['name'])
    # print(a_dict['name'])
    # print(h_dict['abi']['ttime'].to_list())
    # print(m_dict['abi']['ttime'].to_list())
    # print(a_dict['abi']['ttime'].to_list())
    # print(type(h_dict['abi']['ttime']))
    # exit(-1)
    # h_abittime = h_dict['abi']['ttime'].to_numpy(copy=True)
    # h_abittime = h_dict['abi']['ttime'].to_numpy()
    # print('type')
    # print(type(h_abittime))
    # print(type(h_dict['abi']['ttime']))
    # print(h_abittime)
    # print(h_dict['abi']['ttime'])
    # exit(-1)
    ttime_abi = h_dict['abi']['ttime'].to_list() + m_dict['abi']['ttime'].to_list() + a_dict['abi']['ttime'].to_list()
    # print(ttime_abi)
    # exit(-1)
    ttime_dst = h_dict['dst']['ttime'].to_list() + m_dict['dst']['ttime'].to_list() + a_dict['dst']['ttime'].to_list()
    # ttime_dst = ad_dict['dst']['ttime'].to_list()
    # print(ttime_dst)
    numpoints = len(ttime_abi)
    # print('numpoints: ', numpoints)
    plot_time_comparison(ttime_abi, ttime_dst, numpoints, area_name, plot_type='ttime')
### --- End compare_abi_dst --- ###

def plot_time_comparison(xlist, ylist, numpoints, area_name, plot_type=''):

    slope, intercept, r_value, p_value, std_err = stats.linregress(xlist, ylist)
    print('DST Time = ', slope, '* Abicare Time +', intercept)
    # print('Abicare Time = ', slope, '* OSRM Time +', intercept)
    print('r_value:', r_value)
    print('p_value:', p_value)
    print('std_err:', std_err)

    # Create figure:
    fig = plt.figure()
    # plt.axis('square')
    if plot_type == 'ttime':
        fig.suptitle(area_name + ' Total Time Comparison', fontsize=13, fontweight='bold')
    elif plot_type == 'ttravel':
        fig.suptitle(area_name + ' Total Travel Time Comparison', fontsize=13, fontweight='bold')
    elif plot_type == 'twaiting':
        fig.suptitle(area_name + ' Total Waiting Time Comparison', fontsize=13, fontweight='bold')
    else:
        print('ERROR: No plot_type given.')
        exit(-1)
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.82)
    ax.set_xlabel('Abicare Time (mins)')
    ax.set_ylabel('DST Time (mins)')
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
    # if timeofday == 'peak':
    #     subtitle = subtitle + '\n7am-10am, 4pm-7pm'
    # elif timeofday == 'offpeak':
    #     subtitle = subtitle + '\n12am-7am, 10am-4pm, 7pm-12am'
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
    # plt.xticks(np.arange(xmin, xmax+1, 250.0))
    # plt.yticks(np.arange(xmin, xmax+1, 250.0))
    # plt.xticks(np.arange(0, 51, 5))
    # plt.yticks(np.arange(0, 11, 1))
    ax.scatter(xlist, ylist, marker='.', color='black')
    # ax.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')
    ax.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Create the legend, save and show the plot:
    # ax.legend()
    if plot_type == 'ttime':
        plt.savefig('all_ttime_comparison.png')
        # plt.savefig(area_name +'_peaktime_comparison_osrm.png')
    elif plot_type == 'ttravel':
        plt.savefig('all_ttravel_comparison.png')
        # plt.savefig(area_name +'_offpeaktime_comparison_osrm.png')
    elif plot_type == 'twaiting':
        plt.savefig('all_twaiting_comparison.png')
        # plt.savefig(area_name + '_time_comparison_osrm.png')

    plt.show()
### --- End of def plot_time_comparison --- ###   

# compare_abi_dst()

# all_instances = []

# filename1 = '../outputs/results/HampshireComparison.csv' #File from Abicare
# inst1 = create_df(filename1, 'Hampshire')
# all_instances.append(inst_dicts_to_dfs(inst1).copy())

# filename2 = '../outputs/results/MonmouthComparison.csv' #File from Abicare
# inst2 = create_df(filename2, 'Monmouth')
# all_instances.append(inst_dicts_to_dfs(inst2).copy())

# filename3 = '../outputs/results/AldershotComparison.csv' #File from Abicare
# inst3 = create_df(filename3, 'Aldershot')
# all_instances.append(inst_dicts_to_dfs(inst3).copy())


# # pickle.dump(inst, open('all_compare_abi_dst.p', 'wb'))

# pickle.dump(all_instances, open('all_compare_abi_dst.p', 'wb'))
# print('Done.')
# exit(-1)
# print('end\n')
# create_df()
# for a_name in u_areas: # For each area in the list of Areas
#     if not (a_name in only_areas): # If current area is not Salisbury, go to next area in list of areas
#         continue
#     if largeprint:
#         largeprintstr += '\n' + '###' + a_name + '###\n'

#     area_df = df[df['Area'] == a_name] # area_df = df but only containing info for current area
#     for day in days_in_df: # For each day in list of Dates
#         day_df = area_df[area_df['Date'] == day] # day_df = df but only containing infor for current area (area_df) and current day
#         carers_working = day_df['Employee'].unique() # List of carers working on current day
#         if largeprint:
#             largeprintstr += '\n' + '' + day + '-' + str(len(carers_working)) + 'carers'

#         inst = {
#                 'name' : a_name + '_' + day.replace('-', '_'),
#                 'fname' : day.replace('-', '_') + '_' + a_name, # filename
#                 'area' : a_name,
#                 'date' : day,
#                 'stats' : {'ncarers' : 0, 'ntasks' : 0, 'ttraveltime' : 0, 'ttravelmiles' : 0, 'tservicetime' : 0, 'tgaptime' : 0, 'twaitingtime' : 0, 'totaltime' : 0},
#                 'txtsummary' : '',
#                 'rota' : {'carer' : [], 'postcode' : [], 'num_addr' : [], 'eastings': [], 'northings' : [], 'start' : [], 'finish' : [], 'shift' : [], 'home_start' : [], 'home_finish' : [], 'num_tasks' : [], 'travel_time' : [], 'wait_time' : [], 'service_time' : []},
#                 'tasks' : {'client' : [], 'postcode': [], 'num_addr': [], 'eastings': [], 'northings' : [], 'duration' : [], 'miles' : [], 'metres' : [], 'esttime' : [], 'start' : [], 'end' : [], 'tw_start' : [], 'tw_end' : []},
#                 'routes' : []
#                 }
#                 #['rota']['start'] is the start time of the carer IN MINUTES FROM MIDNIGHT (I.E. 12:00AM)
#                 #['rota']['end'] is the end time of the carer IN MINUTES FROM MIDNIGHT (I.E. 12:00AM)
#         # print(day)        
#         start_of_day = day_df.iloc[0]['start_dt'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.

#         # set_info_08hants = 0
#         # set_info_06aldershot = 0
#         for carer in carers_working: # For each carer working on the current day (in the list of carers)
#             carer_df = day_df[day_df['Employee'] == carer] # carer_df = df but only containing info for current area, current day, and current carer
#             inst['stats']['ncarers'] += 1 # Increase number of carers by one
#             inst['rota']['carer'].append(carer) # Add current carer to the rota
#             inst['rota']['postcode'].append(str(carer_df.iloc[0]['Employee Postcode']).lower().replace(' ', '')) # Add postcode of current carer's home to the rota
#             nad = pdfinder.find_postcode_n_addresses(inst['rota']['postcode'][-1]) # nad = the number of addresses at the carer's postcode ([-1] is the last postcode in the list)
#             inst['rota']['num_addr'].append(nad) # Add number of addresses at the postcode to the rota
#             if nad >= min_addresses: # If there exist a sufficient number of properties at the postcode (>= 10), then we can obtain the latitude and longitude of the postcode
#                 eastings, northings = pdfinder.find_postcode_latlon(inst['rota']['postcode'][-1])
#             else: # Too few properties at the postcode, cannot obtain latitude and longitude for privacy reasons.
#                 eastings, northings = [None, None]

#             inst['rota']['eastings'].append(eastings) # Add latitude to the rota
#             inst['rota']['northings'].append(northings) # Add longitude to the rota
#             inst['rota']['start'].append([60*24]) # Add start and end times to the rota
#             inst['rota']['finish'].append([0])
#             inst['rota']['num_tasks'].append(len(carer_df)) # Add the number of jobs that each carer has that day.
            
#             carer_route = []
#             if eastings != None: # Add latitude and longitude to carer_route if they exist
#                 carer_route.append([eastings, northings])

#             if largeprint:
#                 largeprintstr += '\n' + '  ' + carer + '- visits (' + str(len(carer_df['Employee'])) + '):'

#             prev_finish = -1
#             travel_until_here = 0
#             gap_until_here = 0
#             waiting_until_here = 0 # New
#             carer_waiting = 0 # New
#             carer_travelling = 0 # New
#             miles_until_here = 0
#             carer_service = 0 # New
#             carer_row = 0
#             for index, row in carer_df.iterrows():
#                 inst['stats']['ntasks'] += 1 # Increase number of tasks (jobs) by one
#                 # Update carer working times
#                 sov_mins = time_dif_to_minutes(row['start_dt'], start_of_day) # Changed from row['end_dt'] to row['start_dt']
#                 eov_mins = time_dif_to_minutes(row['end_dt'], start_of_day)
#                 if inst['rota']['start'][-1] > sov_mins:
#                     inst['rota']['start'][-1] = sov_mins

#                 if inst['rota']['finish'][-1] < eov_mins:
#                     inst['rota']['finish'][-1] = eov_mins

#                 # Task info:
#                 inst['tasks']['client'].append(row['Client']) # Add client 'Client #' to tasks list
#                 inst['tasks']['postcode'].append(row['Postcode'].lower().replace(' ', '')) # Add postcode of client to tasks list
#                 nad = pdfinder.find_postcode_n_addresses(inst['tasks']['postcode'][-1]) # nad = number of addresses at client's postcode
#                 inst['tasks']['num_addr'].append(nad) # Add nad to tasks list
#                 if nad >= min_addresses: # If sufficient # of addresses at postcode, get latitude and longitude, else cannot for privacy reasons
#                     eastings, northings = pdfinder.find_postcode_latlon(inst['tasks']['postcode'][-1])
#                 else:
#                     eastings, northings = [None, None]

#                 if eastings != None: # Add latitude and longitude of client's address to carer's route if allowed.
#                     carer_route.append([eastings, northings])

#                 inst['tasks']['eastings'].append(eastings) # Add latitude, longitude, and duration of job to tasks list
#                 inst['tasks']['northings'].append(northings)
#                 duration = time_dif_to_minutes(row['end_dt'], row['start_dt']) # Duration of task/job.
#                 inst['tasks']['duration'].append(duration)
#                 carer_service += duration # New
#                 inst['tasks']['start'].append(time_dif_to_minutes(row['start_dt'], start_of_day))
#                 inst['tasks']['end'].append(time_dif_to_minutes(row['end_dt'], start_of_day))

#                 twstart_td = row['start_dt'] - timewindow_interval #TW is allowed around start time of job
#                 twend_td = row['start_dt'] + timewindow_interval
#                 inst['tasks']['tw_start'].append(time_dif_to_minutes(twstart_td, start_of_day)) # Add TW start and end to tasks list
#                 inst['tasks']['tw_end'].append(time_dif_to_minutes(twend_td, start_of_day))
#                 # if inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114':
#                 #     print('here1')
#                 #     set_info_06aldershot += 1
                
#                 metres = 0
#                 travel_time_mins = 0
#                 # if inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114' and set_info_06aldershot == 3:
#                 #     print('here2')
#                 #     metres = 0.0
#                 #     travel_time_mins = 0.0
#                 #     miles = 0.0
#                 #     inst['tasks']['miles'].append(miles)
#                 #     inst['tasks']['metres'].append(metres)
#                 #     inst['tasks']['esttime'].append(travel_time_mins) # New
#                 # if inst['date'] == '8-Nov-20' and carer == 'Carer 79' and inst['tasks']['client'][-1] == 'Client 86' and set_info_08hants == 0:
#                 #     print('here')
#                 #     metres = 10323.2
#                 #     travel_time_mins = 10.593
#                 #     miles = 6.413
#                 #     inst['tasks']['miles'].append(miles)
#                 #     inst['tasks']['metres'].append(metres)
#                 #     inst['tasks']['esttime'].append(travel_time_mins) # New
#                 #     set_info_08hants = 1
#                 # else:
#                 #     if np.isnan(row['Miles']):
#                 #         inst['tasks']['miles'].append(0.0)
#                 #     else:
#                 #         inst['tasks']['miles'].append(row['Miles'])
#                 #     metres = miles_to_metres(row['Miles'])
#                 #     inst['tasks']['metres'].append(metres)
#                 #     travel_time_mins = minsecs_to_mins(row['Estimated Time (Mins)']) # New
#                 #     inst['tasks']['esttime'].append(travel_time_mins) # New
#                 if np.isnan(row['Miles']):
#                     inst['tasks']['miles'].append(0.0)
#                 else:
#                     inst['tasks']['miles'].append(row['Miles'])
#                 metres = miles_to_metres(row['Miles'])
#                 inst['tasks']['metres'].append(metres)
#                 travel_time_mins = minsecs_to_mins(row['Estimated Time (Mins)']) # New
#                 inst['tasks']['esttime'].append(travel_time_mins) # New
                

#                 # Solution info:
#                 if carer_row > 0:
#                     # travel_until_here = row['Estimated Time (Mins)']
#                     travel_until_here = travel_time_mins # New
#                     # if inst['date'] == '6-Nov-20' and carer == 'Carer 22' and inst['tasks']['client'][-1] == 'Client 114' and set_info_06aldershot == 3:
#                     #     print('here3')
#                     #     miles_until_here = 0.0
#                     #     set_info_06aldershot = 10
#                     # else:
#                     #     if np.isnan(row['Miles']):
#                     #         miles_until_here = 0.0
#                     #     else:    
#                     #         miles_until_here = row['Miles']
#                     if np.isnan(row['Miles']):
#                         miles_until_here = 0.0
#                     else:    
#                         miles_until_here = row['Miles']
#                     # print(row['start_dt'])
#                     # print(row['end_dt'])
#                     gap_until_here = time_dif_to_minutes(row['start_dt'], prev_finish) # This is the time in minutes from the end time of the previous job to the start time of the current job
#                     waiting_until_here = gap_until_here - travel_time_mins # New: waiting time between when carer arrives at job (end time of prev job + travel time) and the start time of the job.

#                     # exit(-1)
#                 if largeprint:
#                     largeprintstr += '\n' + '    - ' + row['Client'] + '(' + row['Postcode'] + ')'
#                     largeprintstr += ' from ' + row['Start Time'] + 'to' + row['End Time']
#                     largeprintstr += '(' + str(duration) + ' m) TimeToArrive: ' + str(travel_until_here) + 'm VisitGap' + str(gap_until_here) + 'm)'

#                 inst['stats']['tservicetime'] += duration
#                 inst['stats']['ttraveltime'] += travel_until_here
#                 inst['stats']['ttravelmiles'] += miles_until_here
#                 inst['stats']['tgaptime'] += gap_until_here
#                 inst['stats']['twaitingtime'] += waiting_until_here # New
#                 carer_travelling += travel_until_here # New
#                 carer_waiting += waiting_until_here # New
#                 carer_row += 1
#                 prev_finish = row['end_dt']
#             # End of for index, row in carer_df.iterrows() loop
#             inst['rota']['travel_time'].append(carer_travelling) # New
#             inst['rota']['wait_time'].append(carer_waiting) # New
#             inst['rota']['service_time'].append(carer_service) # New

#             shift_duration = inst['rota']['finish'][-1] - inst['rota']['start'][-1] # New
#             inst['rota']['shift'].append(shift_duration) # New
#             home_start = inst['rota']['start'][-1] - 30 # New
#             home_finish = inst['rota']['finish'][-1] + 30 # New
#             inst['rota']['home_start'].append(home_start) # New
#             inst['rota']['home_finish'].append(home_finish) # New

#             # Close carer route:
#             if eastings != None:
#                 carer_route.append([eastings, northings])
#             inst['routes'].append(carer_route.copy())
#         #End of for carer in carers_working loop
#         inst['stats']['totaltime'] = inst['stats']['ttraveltime'] + inst['stats']['tservicetime'] + inst['stats']['twaitingtime']

#         inst['txtsummary'] = largeprintstr
#         all_instances.append(inst_dicts_to_dfs(inst).copy())
#         pickle.dump(all_instances, open('all_inst_' + str(inst['area']) + '.p', 'wb'))
    # End of for day in days_in_df loop
# End of for a_name in u_areas loop