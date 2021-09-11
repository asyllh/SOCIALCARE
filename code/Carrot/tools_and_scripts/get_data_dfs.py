#--------------------#
# CARROT - CARe ROuting Tool
# get_data_dfs.py
# Retrieve information about carers and clients from abicare's new dataset format (client_carers_details.xlsx)
# 25/06/2021
#--------------------#

import math
import pickle
import datetime
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats
import openpyxl
import tools_and_scripts.class_cpo_df as ccd
import tools_and_scripts.week_counter as wc

# filename = 'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\abicare\client_carer_details.xlsx' #File from Abicare

def time_dif_to_minutes(endtime, starttime, round=True):
    mins = (endtime - starttime).seconds/60
    if round:
        return np.round(mins, 0)
    else:
        return mins

def get_info_create_dfs(area = 'None', tw_interval = 15, planning_date=None, dayindex_2weeks=None, dayindex_8weeks=None, print_statements=False, filename='None', foldername='None'):

    if planning_date == None or dayindex_2weeks == None or dayindex_8weeks == None:
        print('[ERROR]: invalid timestamp for dayindex_2weeks and dayindex_8weeks.')
        print('Terminating program.')
        exit(-1)

    planning_day = planning_date.dayofweek

    # NOTE: NEED TO ADD NUMBER OF CARERS (DOUBLE SERVICE) JOBS 
    cpo_inst = ccd.CPO_DF(foldername)

    # data_filename = r'data\abicare\clientcarerdetails.xlsx'
    # data_filename = r'data\abicare\client_carer_details.xlsx'
    data_filename = filename

    ######################## -------------------------------- CLIENT -------------------------------- ########################

    clientdetails_sheetname = 'Client Contracts Data'

    # First sheet: Clients Details
    clientdf_xl = pd.read_excel(data_filename, sheet_name=clientdetails_sheetname, header=10)

    

    clientdf_xl.rename(columns={'CAREROOMSROOM': 'Client ID',
                                    'CAREROOMSSENSOR': 'Area', 
                                    'CAREROOMSContact1Postcode': 'Postcode', 
                                    'HomeVisitsStartDate': 'Start Date', 
                                    'HomeVisitsStartTime': 'Start Time',
                                    'HomeVisitsEndDate': 'End Date',
                                    'Visit EndTime (Calculated)': 'End Time',
                                    'HomeVisitsCyclePeriod': 'Weekly Cycle',
                                    'HomeVisitsRequiredNumber': 'Number of Carers'}, inplace=True)

    clientdf_xl = clientdf_xl[clientdf_xl['Area'] == area] # filter by area
    clientdf_xl.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area

    clientdf_xl['Client ID'] = clientdf_xl['Client ID'].str.replace(' ', '')
    clientdf_xl['Client ID'] = clientdf_xl['Client ID'].str.lower()
    clientdf_xl['Postcode'] = clientdf_xl['Postcode'].str.replace(' ', '')
    clientdf_xl['Postcode'] = clientdf_xl['Postcode'].str.lower()

    clientdf_xl['Date'] = clientdf_xl['Start Date'].dt.date
    clientdf_xl['Date Start Time'] = pd.to_datetime(clientdf_xl['Date'].astype(str) + ' ' + clientdf_xl['Start Time'].astype(str)) # yyyy-mm-dd hh:mmm:ss
    clientdf_xl['Date End Time'] = pd.to_datetime(clientdf_xl['Date'].astype(str) + ' ' + clientdf_xl['End Time'].astype(str)) # yyyy-mm-dd hh:mmm:ss
   
    # print(clientdf_xl)

    # exit(-1)
    # Dictionary to hold client details. 
    # 'start', 'end', 'tw_start', and 'tw_end' are minutes from midnight, 'duration' is minutes, 'start_time' and 'end_time' are hh:mm:ss.
    client_dict = {'client_id' : [], 'postcode' : [], 'area' : [], 'date' : [], 
                'start' : [],  'duration' : [], 'end' : [],  'tw_start' : [], 'tw_end' : [], 
                'longitude' : [], 'latitude' : [], 
                'start_time' : [], 'end_time' : [], 'weekly_cycle' : [], 'num_carers' : []}

    # start_of_day = clientdf_contracts.iloc[0]['From'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.
    
    # Get information from excel data
    no_details = 0
    multiple_details = 0
    # client_id = clientdf_xl['Client ID']
    # for i in range(len(client_id)):
    for i in range(len(clientdf_xl)):
        client_id = np.nan
        client_area = np.nan
        client_postcode = np.nan 
        client_date = np.nan
        client_starttime = np.nan # hh:mm:ss
        client_duration = np.nan # minutes
        client_endttime = np.nan # hh:mm:ss
        client_longitude = np.nan
        client_latitude = np.nan
        client_start = np.nan # minutes from midnight
        client_end = np.nan # minutes from midnight
        client_weeklycycle = np.nan
        client_numcarers = np.nan

        # client_details_row = clientdf_xl.loc[clientdf_xl['Client ID'] == client_id[i]]
        client_details_row = clientdf_xl.loc[[i]]
        # NOTE: need to check here that the size is exactly 1 (only one detail) and that there is a client id and information for that client.

        contract_start = pd.Timestamp(client_details_row.iloc[0]['Start Date'])
        contract_cycle = client_details_row['Weekly Cycle'].values[0]
        add_to_dict = False
        due_this_week = False
        due_this_week = wc.date_in_visit_cycle(planning_date, contract_start, contract_cycle)
        if due_this_week == False:
            continue
        elif due_this_week == True:
            if planning_day == 0 and client_details_row.iloc[0]['HomeVisitsMon'] == 1:
                add_to_dict = True
            elif planning_day == 1 and client_details_row.iloc[0]['HomeVisitsTue'] == 1:
                add_to_dict = True
            elif planning_day == 2 and client_details_row.iloc[0]['HomeVisitsWed'] == 1:
                add_to_dict = True
            elif planning_day == 3 and client_details_row.iloc[0]['HomeVisitsThu'] == 1:
                add_to_dict = True
            elif planning_day == 4 and client_details_row.iloc[0]['HomeVisitsFri'] == 1:
                add_to_dict = True
            elif planning_day == 5 and client_details_row.iloc[0]['HomeVisitsSat'] == 1:
                add_to_dict = True
            elif planning_day == 6 and client_details_row.iloc[0]['HomeVisitsSun'] == 1:
                add_to_dict = True
            
        if add_to_dict == True:
            start_of_day = client_details_row.iloc[0]['Date Start Time'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.
            client_id = client_details_row.iloc[0]['Client ID']
            client_area = client_details_row['Area'].values[0]
            client_postcode = client_details_row['Postcode'].values[0] 
            client_date = client_details_row.iloc[0]['Date']
            client_starttime = client_details_row['Start Time'].values[0] # hh:mm:ss 
            client_endttime = client_details_row['End Time'].values[0] # hh:mm:ss 
            client_work_datestarttime = client_details_row.iloc[0]['Date Start Time'] # yyyy-mm-dd hh:mmm:ss
            client_work_dateendtime = client_details_row.iloc[0]['Date End Time'] # yyyy-mm-dd hh:mmm:ss
            client_start = time_dif_to_minutes(client_work_datestarttime, start_of_day) # minutes from midnight 
            client_end = time_dif_to_minutes(client_work_dateendtime, start_of_day) # minutes from midnight
            client_duration = client_end - client_start # minutes
            client_weeklycycle = client_details_row['Weekly Cycle'].values[0]
            client_numcarers = client_details_row['Number of Carers'].values[0]

            client_longitude, client_latitude = cpo_inst.find_postcode_lonlat(client_postcode)
            if (client_longitude == None):
                # print('[ERROR]: No latitude and longitude found for Client ', client_id, ', postcode: ', client_postcode)
                print('[WARNING]: No latitude and longitude found for Client ', client_id, ', postcode: ', client_postcode)
                # print('Terminating program.')
                print('Skipping client.')
                # exit(-1)
                continue

            # Add details to client_dict dictionary
            client_dict['client_id'].append(str(client_id))
            client_dict['area'].append(client_area)
            client_dict['postcode'].append(client_postcode)
            client_dict['date'].append(client_date)
            client_dict['start_time'].append(client_starttime)
            client_dict['duration'].append(client_duration)
            client_dict['end_time'].append(client_endttime)
            client_dict['start'].append(client_start)
            client_dict['end'].append(client_end)
            client_dict['tw_start'].append(client_start - tw_interval)
            client_dict['tw_end'].append(client_start + tw_interval)
            client_dict['longitude'].append(client_longitude)
            client_dict['latitude'].append(client_latitude)
            client_dict['weekly_cycle'].append(client_weeklycycle)
            client_dict['num_carers'].append(client_numcarers)
        # End if add_to_dict == True
    # --- End for i in range(len(client_id)) loop --- #

    # Create dataframe from client_dict dictionary
    client_df = pd.DataFrame(client_dict)

    # if day_index == 0 or day_index == 7 or day_index == 14 or day_index == 21 or day_index == 28 or day_index == 35 or day_index == 42 or day_index == 49:
    #     client_df = client_df[client_df['mon'] == 1] # filter by monday
    #     client_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by monday
    # elif day_index == 1 or day_index == 8 or day_index == 15 or day_index == 22 or day_index == 29 or day_index == 36 or day_index == 43 or day_index == 50:
    #     client_df = client_df[client_df['tue'] == 1] # filter by tuesday
    #     client_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by tuesday
    # elif day_index == 2 or day_index == 9 or day_index == 16 or day_index == 23 or day_index == 30 or day_index == 37 or day_index == 44 or day_index == 51:
    #     client_df = client_df[client_df['wed'] == 1] # filter by wednesday
    #     client_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by wednesday
    # elif day_index == 3 or day_index == 10 or day_index == 17 or day_index == 24 or day_index == 31 or day_index == 38 or day_index == 45 or day_index == 52:
    #     client_df = client_df[client_df['thu'] == 1] # filter by thursday
    #     client_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by thursday
    # elif day_index == 4 or day_index == 11 or day_index == 18 or day_index == 25 or day_index == 32 or day_index == 39 or day_index == 46 or day_index == 53:
    #     client_df = client_df[client_df['fri'] == 1] # filter by friday
    #     client_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by friday
    # elif day_index == 5 or day_index == 12 or day_index == 19 or day_index == 26 or day_index == 33 or day_index == 40 or day_index == 47 or day_index == 54:
    #     client_df = client_df[client_df['sat'] == 1] # filter by saturday
    #     client_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by saturday
    # elif day_index == 6 or day_index == 13 or day_index == 20 or day_index == 27 or day_index == 34 or day_index == 41 or day_index == 48 or day_index == 55:
    #     client_df = client_df[client_df['sun'] == 1] # filter by sunday
    #     client_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by sunday



    print('len client_df: ', len(client_df))
    # exit(-1)
   
    ######################## -------------------------------- CARER -------------------------------- ########################

    carerdetails_sheetname = 'Carer Availability Data'
    # Carer details sheet
    carerdf_xl = pd.read_excel(data_filename, sheet_name=carerdetails_sheetname, header=15)

    carerdf_xl.rename(columns={'CARERSOnlineId': 'Carer ID',
                                    'CarerAreasDescription': 'Area', 
                                    'Primary Area Flag': 'Primary', 
                                    'CARERSPostcode': 'Postcode', 
                                    'EmployeeAvailableStart': 'Start Date',
                                    'EmployeeAvailableStartTime': 'Start Time',
                                    'EmployeeAvailableEnd': 'End Date',
                                    'EmployeeAvailableEndTime': 'End Time',
                                    'EmployeeAvailableAvailable': 'Carer Available',
                                    'EmployeeAvailableDay': 'Available Day',
                                    'Week Number & Day': 'Week Number and Day'}, inplace=True)

    carerdf_xl = carerdf_xl[carerdf_xl['Area'] == area] # filter by area
    carerdf_xl.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area

    # carerdf_xl['Carer ID'] = carerdf_xl['Carer ID'].str.replace(' ', '')
    # carerdf_xl['Carer ID'] = carerdf_xl['Carer ID'].str.lower()
    carerdf_xl['Postcode'] = carerdf_xl['Postcode'].str.replace(' ', '')
    carerdf_xl['Postcode'] = carerdf_xl['Postcode'].str.lower()

    # print(carerdf_xl[:10])
    # exit(-1)

    carerdf_xl['Date'] = carerdf_xl['Start Date'].dt.date
    carerdf_xl['Date Start Time'] = pd.to_datetime(carerdf_xl['Date'].astype(str) + ' ' + carerdf_xl['Start Time'].astype(str)) # yyyy-mm-dd hh:mmm:ss
    carerdf_xl['Date End Time'] = pd.to_datetime(carerdf_xl['Date'].astype(str) + ' ' + carerdf_xl['End Time'].astype(str)) # yyyy-mm-dd hh:mmm:ss

    # print('carerdf_xl:')
    # print(carerdf_xl)
    # exit(-1)

    # Dictionaries to hold carer details: one for the individual shifts of the day for each carer, one for the entire day of the carer
    # 'start' and 'end' are minutes from midnight, 'duration' is minutes, 'start_time' and 'end_time' are hh:mm:ss.
    carer_shift_dict = {'unique_id' : [], 'shift' : [], 
                    'carer_id' : [], 'postcode' : [], 'area' : [],
                    'start' : [], 'duration' : [], 'end' : [], 
                    'longitude' : [], 'latitude' : [],
                    'start_time' : [], 'end_time' : [], 'avail_day' : [], 'week_cycle' : []}

    carer_day_dict = {'carer_id' : [], 'postcode' : [], 'area' : [], 
                    'start' : [], 'duration' : [], 'end' : [], 
                    'longitude' : [], 'latitude' : [],
                    'start_time' : [], 'end_time' : [], 'avail_day' : [], 'week_cycle' : []}
    
    
    # Get information from excel data
    carer_id = carerdf_xl['Carer ID']
    for i in range(len(carer_id)):
        if pd.isnull(carer_id[i]): # if carer_id[i] == None
            print('[WARNING]: Entry number ', i, ' for ', area, 'has no Carer ID.')
            print('Skipping entry.')
            continue
        if(carer_id[i] in carer_day_dict['carer_id']): # Skip over carers that have already been assessed.
            continue
        carer_area = np.nan
        carer_postcode = np.nan
        carer_starttime = np.nan # hh:mm:ss 
        carer_endtime = np.nan # hh:mm:ss 
        carer_start = np.nan # minutes from midnight 
        carer_end = np.nan # minutes from midnight
        carer_duration = np.nan # minutes
        carer_datestarttime = np.nan # yyyy-mm-dd hh:mmm:ss
        carer_dateendtime = np.nan # yyyy-mm-dd hh:mmm:ss
        carer_longitude = np.nan
        carer_latitude = np.nan
        carer_availday = np.nan
        carer_weekcycle = np.nan

        carer_id_rows = carerdf_xl[carerdf_xl['Carer ID'] == carer_id[i]] # carer_id_rows is a df that only contains information for the current carer_id[i]
        # Check there is no missing information
        if len(carer_id_rows) < 1:
            print('[ERROR]: No shifts available for Carer ', carer_id[i])
            print('Terminating program.')
            exit(-1)
        # Now in this truncated df (carer_id_rows), we need to go down the 'Available Day' column and find the highest number (0-55)
        # If the highest number is <=13, then the current carer (carer_id[i]) is on a 2-week cycle, else if the highest number is >13 and <=55, then the current carer is on an 8-week cycle.
        highest_availday = 0
        for h in range(len(carer_id_rows)):
            if carer_id_rows.iloc[h]['Available Day'] > highest_availday:
                highest_availday = carer_id_rows.iloc[h]['Available Day']
        if highest_availday <= 13:
            carer_weekcycle = 2
        elif highest_availday > 13 and highest_availday <= 55:
            carer_weekcycle = 8
        else:
            print('[ERROR]: invalid highest_availday value for Carer ', carer_id[i], ', highest_availday is ', highest_availday)
            print('Terminating program.')
            exit(-1)

        # for j in range(56): # from 0 to 55

        carer_shift_rows = carer_id_rows[carer_id_rows['Available Day'] == 0]
        if carer_weekcycle == 2:
            carer_shift_rows = carer_id_rows[carer_id_rows['Available Day'] == dayindex_2weeks]
        elif carer_weekcycle == 8:
            carer_shift_rows = carer_id_rows[carer_id_rows['Available Day'] == dayindex_8weeks]
        # print('i: ', i, 'carer_id[i]: ', carer_id[i], 'j: ', j)
        # print(carer_shift_rows)
        # exit(-1)
        #NOTE: NEW - remove duplicate rows 06/09/2021
        carer_shift_rows.drop_duplicates(inplace=True)
        if len(carer_shift_rows) < 1: # No shifts for dayindex_weeks for this carer
            continue
        elif len(carer_shift_rows) == 1: # this carer has only one shift on dayindex_weeks
            start_of_day = carer_shift_rows.iloc[0]['Date Start Time'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first shift.
            shift = 1 # Only one shift
            carer_uniqueid = str(carer_id[i]) + '__' + str(shift)
            carer_shift = shift
            carer_area = carer_shift_rows['Area'].values[0]
            carer_postcode = carer_shift_rows['Postcode'].values[0]
            carer_starttime = carer_shift_rows['Start Time'].values[0] # hh:mm:ss
            carer_endtime = carer_shift_rows['End Time'].values[0] # hh:mm:ss
            carer_datestarttime = carer_shift_rows.iloc[0]['Date Start Time'] # yyyy-mm-dd hh:mmm:ss
            carer_dateendtime = carer_shift_rows.iloc[0]['Date End Time'] # yyyy-mm-dd hh:mmm:ss
            carer_start = time_dif_to_minutes(carer_datestarttime, start_of_day) # minutes from midnight
            carer_end = time_dif_to_minutes(carer_dateendtime, start_of_day) # minutes from midnight 
            carer_duration = carer_end - carer_start # minutes
            carer_availday = carer_shift_rows['Available Day'].values[0]
            carer_longitude, carer_latitude = cpo_inst.find_postcode_lonlat(carer_postcode)
            if (carer_longitude == None):
                # print('[ERROR]: No latitude and longitude found for Carer ', carer_id[i], ', postcode: ', carer_postcode)
                print('[WARNING]: No latitude and longitude found for Carer ', carer_id[i], ', postcode: ', carer_postcode)
                # print('Terminating program.')
                print('Skipping carer.')
                # exit(-1)
                continue
            
            # Add info to carer_shift_dict:
            carer_shift_dict['unique_id'].append(carer_uniqueid)
            carer_shift_dict['shift'].append(carer_shift)
            carer_shift_dict['carer_id'].append(carer_id[i])
            carer_shift_dict['postcode'].append(carer_postcode)
            carer_shift_dict['area'].append(carer_area)
            carer_shift_dict['start'].append(carer_start) # minutes from midnight
            carer_shift_dict['duration'].append(carer_duration) # minutes
            carer_shift_dict['end'].append(carer_end) # minutes from midnight
            carer_shift_dict['longitude'].append(carer_longitude)
            carer_shift_dict['latitude'].append(carer_latitude)
            carer_shift_dict['start_time'].append(carer_starttime) # hh:mm:ss
            carer_shift_dict['end_time'].append(carer_endtime) # hh:mm:ss
            carer_shift_dict['avail_day'].append(carer_availday)
            carer_shift_dict['week_cycle'].append(carer_weekcycle)

            # Add info to carer_day_dict:
            carer_day_dict['carer_id'].append(carer_id[i])
            carer_day_dict['postcode'].append(carer_postcode)
            carer_day_dict['area'].append(carer_area)
            carer_day_dict['start'].append(carer_start) # minutes from midnight
            carer_day_dict['duration'].append(carer_duration) # minutes
            carer_day_dict['end'].append(carer_end) # minutes from midnight
            carer_day_dict['longitude'].append(carer_longitude)
            carer_day_dict['latitude'].append(carer_latitude)
            carer_day_dict['start_time'].append(carer_starttime) # hh:mm:ss
            carer_day_dict['end_time'].append(carer_endtime) # hh:mm:ss
            carer_day_dict['avail_day'].append(carer_availday)
            carer_day_dict['week_cycle'].append(carer_weekcycle)
        elif len(carer_shift_rows) > 1: # this carer has multiple shifts on dayindex_weeks
            start_of_day = carer_shift_rows.iloc[0]['Date Start Time'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first shift.
            # First just get details of carer area, postcode, day, and lat/lon
            carer_area = carer_shift_rows['Area'].values[0]
            carer_postcode = carer_shift_rows['Postcode'].values[0]
            carer_availday = carer_shift_rows['Available Day'].values[0]
            carer_longitude, carer_latitude = cpo_inst.find_postcode_lonlat(carer_postcode)
            if (carer_longitude == None):
                # print('[ERROR]: No latitude and longitude found for Carer ', carer_id[i], ', postcode: ', carer_postcode)
                print('[WARNING]: No latitude and longitude found for Carer ', carer_id[i], ', postcode: ', carer_postcode)
                # print('Terminating program.')
                print('Skipping carer.')
                # exit(-1)
                continue
            # Now get the start and end times of each shift, starting from the bottom (in excel, the shifts are in reverse order, so the first shift is at the bottom)
            shift_count = 0
            day_starttime = 0
            day_start = 0
            day_endtime = 0
            day_end = 0
            for k in range(len(carer_shift_rows)-1, -1, -1): #for k in range (start from len(carer_shift_rows)-1, end at -1 (so go up to 0), and step decrement by -1 each time)
                shift_count += 1
                carer_uniqueid = str(carer_id[i]) + '__' + str(shift_count)
                carer_shift = shift_count
                carer_starttime = carer_shift_rows['Start Time'].values[k] # hh:mm:ss
                carer_endtime = carer_shift_rows['End Time'].values[k] # hh:mm:ss
                carer_datestarttime = carer_shift_rows.iloc[k]['Date Start Time'] # yyyy-mm-dd hh:mmm:ss
                carer_dateendtime = carer_shift_rows.iloc[k]['Date End Time'] # yyyy-mm-dd hh:mmm:ss
                carer_start = time_dif_to_minutes(carer_datestarttime, start_of_day) # minutes from midnight
                carer_end = time_dif_to_minutes(carer_dateendtime, start_of_day) # minutes from midnight
                carer_duration = carer_end - carer_start # minutes
                if k == len(carer_shift_rows)-1:
                    day_starttime = carer_starttime # hh:mm:ss
                    day_start = carer_start # minutes from midnight
                if k == 0:
                    day_endtime = carer_endtime # hh:mm:ss
                    day_end = carer_end # minutes from midnight

                # Add info to carer_shift_dict
                carer_shift_dict['unique_id'].append(carer_uniqueid)
                carer_shift_dict['shift'].append(carer_shift)
                carer_shift_dict['carer_id'].append(carer_id[i])
                carer_shift_dict['postcode'].append(carer_postcode)
                carer_shift_dict['area'].append(carer_area)
                carer_shift_dict['start'].append(carer_start) # minutes from midnight
                carer_shift_dict['duration'].append(carer_duration) # minutes
                carer_shift_dict['end'].append(carer_end) # minutes from midnight
                carer_shift_dict['longitude'].append(carer_longitude)
                carer_shift_dict['latitude'].append(carer_latitude)
                carer_shift_dict['start_time'].append(carer_starttime) # hh:mm:ss
                carer_shift_dict['end_time'].append(carer_endtime) # hh:mm:ss
                carer_shift_dict['avail_day'].append(carer_availday)
                carer_shift_dict['week_cycle'].append(carer_weekcycle)

                # Add info to carer_day_dict
                if k == 0:
                    day_duration = day_end - day_start # minutes
                    carer_day_dict['carer_id'].append(carer_id[i])
                    carer_day_dict['postcode'].append(carer_postcode)
                    carer_day_dict['area'].append(carer_area)
                    carer_day_dict['start'].append(day_start) # minutes from midnight
                    carer_day_dict['duration'].append(day_duration) # minutes
                    carer_day_dict['end'].append(day_end) # minutes from midnight
                    carer_day_dict['longitude'].append(carer_longitude)
                    carer_day_dict['latitude'].append(carer_latitude)
                    carer_day_dict['start_time'].append(day_starttime) # hh:mm:ss
                    carer_day_dict['end_time'].append(day_endtime) # hh:mm:ss
                    carer_day_dict['avail_day'].append(carer_availday)
                    carer_day_dict['week_cycle'].append(carer_weekcycle)
            # End for k loop
        # End elif multiple shifts       
    # End main for i loop

    carershift_df = pd.DataFrame(carer_shift_dict) # Convert dictionary into pd dataframe
    carerday_df = pd.DataFrame(carer_day_dict) # Convert dictionary into pd dataframe

    # carershift_df = carershift_df[carershift_df['avail_day'] == day_index] # filter by day_index
    # carershift_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by day_index

    # carerday_df = carerday_df[carerday_df['avail_day'] == day_index] # filter by day_index
    # carerday_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by day_index

    print('length of carershift_df: ', len(carershift_df))
    print('length of carerday_df: ', len(carerday_df))

    # print('carer_day_dict:')
    # print(carerday_df)

    # exit(-1)

    return client_df, carershift_df, carerday_df

    # if area == 'None':
    #     return client_df, carershift_df, carerday_df
    # else:
    #     clientdf_area = client_df[client_df['area'] == area]
    #     carershiftdf_area = carershift_df[carershift_df['area'] == area]
    #     carerdaydf_area = carerday_df[carerday_df['area'] == area]
    #     clientdf_area.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area
    #     carershiftdf_area.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area
    #     carerdaydf_area.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area
    #     carer_count = 0
    #     for i in range(len(carershiftdf_area)):
    #         shift_number = carershiftdf_area.iloc[i]['shift']
    #         if shift_number == 1:
    #             carer_count += 1
    #     return clientdf_area, carershiftdf_area, carerdaydf_area
### --- End of def get_info_create_dfs --- ###



def carer_works_this_slot(slot): # Function finds out whether there is 'Unavailable' in the given cell 'slot' in Carer Availability
    works = True
    if type(slot) == str: # If there is the string 'Unavailable' in the given 'slot', then the carer does not work that time
        works = False
    else: # Else, the 'slot' is empty, and so the carer is available to work that time.
        works = np.isnan(slot)
    return works



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


# if len(client_details_row) > 1: # client_id[i] has multiple rows in the client details list
#     multiple_details += 1
#     print('[WARNING]: Client ', client_id[i], ' has duplicate details.')
#     print(client_details_row)
#     print('Using only first entry.')
# if len(client_details_row) < 1: # client_id[i] is not in the client details list
#     no_details += 1
#     print('[WARNING]: Client ', client_id[i], ' has no details.')
# else:
#     start_of_day = client_details_row.iloc[0]['Date Start Time'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.
#     client_area = client_details_row['Area'].values[0]
#     client_postcode = client_details_row['Postcode'].values[0] 
#     client_date = client_details_row.iloc[0]['Start Date']
#     client_starttime = client_details_row['Start Time'].values[0] # hh:mm:ss 
#     client_endttime = client_details_row['End Time'].values[0] # hh:mm:ss 
#     client_work_datestarttime = client_details_row.iloc[0]['Date Start Time'] # yyyy-mm-dd hh:mmm:ss
#     client_work_dateendtime = client_details_row.iloc[0]['Date End Time'] # yyyy-mm-dd hh:mmm:ss
#     client_start = time_dif_to_minutes(client_work_datestarttime, start_of_day) # minutes from midnight 
#     client_end = time_dif_to_minutes(client_work_dateendtime, start_of_day) # minutes from midnight
#     client_duration = client_end - client_start # minutes

#     client_longitude, client_latitude = cpo_inst.find_postcode_lonlat(client_postcode)
#     if (client_longitude == None):
#         print('[ERROR]: No latitude and longitude found for Client ', client_id[i], ', postcode: ', client_postcode)
#         print('Terminating program.')
#         exit(-1)

#     # Add details to client_dict dictionary
#     client_dict['client_id'].append(str(client_id[i]))
#     client_dict['area'].append(client_area)
#     client_dict['postcode'].append(client_postcode)
#     client_dict['date'].append(client_date)
#     client_dict['start_time'].append(client_starttime)
#     client_dict['duration'].append(client_duration)
#     client_dict['end_time'].append(client_endttime)
#     client_dict['start'].append(client_start)
#     client_dict['end'].append(client_end)
#     client_dict['tw_start'].append(client_start - tw_interval)
#     client_dict['tw_end'].append(client_start + tw_interval)
#     client_dict['longitude'].append(client_longitude)
#     client_dict['latitude'].append(client_latitude)