#--------------------#
# RSPEA
# Retrieve information about carers and clients from abicare's new dataset format (client_carers_details.xlsx)
# 26/01/2022
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

    print('Collecting data from input file.\n')

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
                'start_time' : [], 'end_time' : [], 'weekly_cycle' : [], 'num_nurses' : []}

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
        client_numnurses = np.nan

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
            client_numnurses = client_details_row['Number of Carers'].values[0]

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
            client_dict['num_nurses'].append(client_numnurses)
        # End if add_to_dict == True
    # --- End for i in range(len(client_id)) loop --- #

    # Create dataframe from client_dict dictionary
    client_df = pd.DataFrame(client_dict)
   
    ######################## -------------------------------- CARER -------------------------------- ########################

    nursedetails_sheetname = 'Carer Availability Data'
    # Carer details sheet
    nursedf_xl = pd.read_excel(data_filename, sheet_name=nursedetails_sheetname, header=15)

    nursedf_xl.rename(columns={'CARERSOnlineId': 'Carer ID',
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

    nursedf_xl = nursedf_xl[nursedf_xl['Area'] == area] # filter by area
    nursedf_xl.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area

    # nursedf_xl['Carer ID'] = nursedf_xl['Carer ID'].str.replace(' ', '')
    # nursedf_xl['Carer ID'] = nursedf_xl['Carer ID'].str.lower()
    nursedf_xl['Postcode'] = nursedf_xl['Postcode'].str.replace(' ', '')
    nursedf_xl['Postcode'] = nursedf_xl['Postcode'].str.lower()

    # print(nursedf_xl[:10])
    # exit(-1)

    nursedf_xl['Date'] = nursedf_xl['Start Date'].dt.date
    nursedf_xl['Date Start Time'] = pd.to_datetime(nursedf_xl['Date'].astype(str) + ' ' + nursedf_xl['Start Time'].astype(str)) # yyyy-mm-dd hh:mmm:ss
    nursedf_xl['Date End Time'] = pd.to_datetime(nursedf_xl['Date'].astype(str) + ' ' + nursedf_xl['End Time'].astype(str)) # yyyy-mm-dd hh:mmm:ss

    # print('nursedf_xl:')
    # print(nursedf_xl)
    # exit(-1)

    # Dictionaries to hold carer details: one for the individual shifts of the day for each carer, one for the entire day of the carer
    # 'start' and 'end' are minutes from midnight, 'duration' is minutes, 'start_time' and 'end_time' are hh:mm:ss.
    nurse_shift_dict = {'unique_id' : [], 'shift' : [], 
                    'nurse_id' : [], 'postcode' : [], 'area' : [],
                    'start' : [], 'duration' : [], 'end' : [], 
                    'longitude' : [], 'latitude' : [],
                    'start_time' : [], 'end_time' : [], 'avail_day' : [], 'week_cycle' : []}

    nurse_day_dict = {'nurse_id' : [], 'postcode' : [], 'area' : [], 
                    'start' : [], 'duration' : [], 'end' : [], 
                    'longitude' : [], 'latitude' : [],
                    'start_time' : [], 'end_time' : [], 'avail_day' : [], 'week_cycle' : []}
    
    
    # Get information from excel data
    nurse_id = nursedf_xl['Carer ID']
    for i in range(len(nurse_id)):
        if pd.isnull(nurse_id[i]): # if nurse_id[i] == None
            print('[WARNING]: Entry number ', i, ' for ', area, 'has no Carer ID.')
            print('Skipping entry.')
            continue
        if(nurse_id[i] in nurse_day_dict['nurse_id']): # Skip over carers that have already been assessed.
            continue
        nurse_area = np.nan
        nurse_postcode = np.nan
        nurse_starttime = np.nan # hh:mm:ss 
        nurse_endtime = np.nan # hh:mm:ss 
        nurse_start = np.nan # minutes from midnight 
        nurse_end = np.nan # minutes from midnight
        nurse_duration = np.nan # minutes
        nurse_datestarttime = np.nan # yyyy-mm-dd hh:mmm:ss
        nurse_dateendtime = np.nan # yyyy-mm-dd hh:mmm:ss
        nurse_longitude = np.nan
        nurse_latitude = np.nan
        nurse_availday = np.nan
        nurse_weekcycle = np.nan

        nurse_id_rows = nursedf_xl[nursedf_xl['Carer ID'] == nurse_id[i]] # nurse_id_rows is a df that only contains information for the current nurse_id[i]
        # Check there is no missing information
        if len(nurse_id_rows) < 1:
            print('[ERROR]: No shifts available for Carer ', nurse_id[i])
            print('Terminating program.')
            exit(-1)
        # Now in this truncated df (nurse_id_rows), we need to go down the 'Available Day' column and find the highest number (0-55)
        # If the highest number is <=13, then the current carer (nurse_id[i]) is on a 2-week cycle, else if the highest number is >13 and <=55, then the current carer is on an 8-week cycle.
        highest_availday = 0
        for h in range(len(nurse_id_rows)):
            if nurse_id_rows.iloc[h]['Available Day'] > highest_availday:
                highest_availday = nurse_id_rows.iloc[h]['Available Day']
        if highest_availday <= 13:
            nurse_weekcycle = 2
        elif highest_availday > 13 and highest_availday <= 55:
            nurse_weekcycle = 8
        else:
            print('[ERROR]: invalid highest_availday value for Carer ', nurse_id[i], ', highest_availday is ', highest_availday)
            print('Terminating program.')
            exit(-1)

        # for j in range(56): # from 0 to 55

        nurse_shift_rows = nurse_id_rows[nurse_id_rows['Available Day'] == 0]
        if nurse_weekcycle == 2:
            nurse_shift_rows = nurse_id_rows[nurse_id_rows['Available Day'] == dayindex_2weeks]
        elif nurse_weekcycle == 8:
            nurse_shift_rows = nurse_id_rows[nurse_id_rows['Available Day'] == dayindex_8weeks]
        # print('i: ', i, 'nurse_id[i]: ', nurse_id[i], 'j: ', j)
        # print(nurse_shift_rows)
        # exit(-1)
        #NOTE: NEW - remove duplicate rows 06/09/2021
        nurse_shift_rows.drop_duplicates(inplace=True)
        if len(nurse_shift_rows) < 1: # No shifts for dayindex_weeks for this carer
            continue
        elif len(nurse_shift_rows) == 1: # this carer has only one shift on dayindex_weeks
            start_of_day = nurse_shift_rows.iloc[0]['Date Start Time'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first shift.
            shift = 1 # Only one shift
            nurse_uniqueid = str(nurse_id[i]) + '__' + str(shift)
            nurse_shift = shift
            nurse_area = nurse_shift_rows['Area'].values[0]
            nurse_postcode = nurse_shift_rows['Postcode'].values[0]
            nurse_starttime = nurse_shift_rows['Start Time'].values[0] # hh:mm:ss
            nurse_endtime = nurse_shift_rows['End Time'].values[0] # hh:mm:ss
            nurse_datestarttime = nurse_shift_rows.iloc[0]['Date Start Time'] # yyyy-mm-dd hh:mmm:ss
            nurse_dateendtime = nurse_shift_rows.iloc[0]['Date End Time'] # yyyy-mm-dd hh:mmm:ss
            nurse_start = time_dif_to_minutes(nurse_datestarttime, start_of_day) # minutes from midnight
            nurse_end = time_dif_to_minutes(nurse_dateendtime, start_of_day) # minutes from midnight 
            nurse_duration = nurse_end - nurse_start # minutes
            nurse_availday = nurse_shift_rows['Available Day'].values[0]
            nurse_longitude, nurse_latitude = cpo_inst.find_postcode_lonlat(nurse_postcode)
            if (nurse_longitude == None):
                # print('[ERROR]: No latitude and longitude found for Carer ', nurse_id[i], ', postcode: ', nurse_postcode)
                print('[WARNING]: No latitude and longitude found for Carer ', nurse_id[i], ', postcode: ', nurse_postcode)
                # print('Terminating program.')
                print('Skipping carer.')
                # exit(-1)
                continue
            
            # Add info to nurse_shift_dict:
            nurse_shift_dict['unique_id'].append(nurse_uniqueid)
            nurse_shift_dict['shift'].append(nurse_shift)
            nurse_shift_dict['nurse_id'].append(nurse_id[i])
            nurse_shift_dict['postcode'].append(nurse_postcode)
            nurse_shift_dict['area'].append(nurse_area)
            nurse_shift_dict['start'].append(nurse_start) # minutes from midnight
            nurse_shift_dict['duration'].append(nurse_duration) # minutes
            nurse_shift_dict['end'].append(nurse_end) # minutes from midnight
            nurse_shift_dict['longitude'].append(nurse_longitude)
            nurse_shift_dict['latitude'].append(nurse_latitude)
            nurse_shift_dict['start_time'].append(nurse_starttime) # hh:mm:ss
            nurse_shift_dict['end_time'].append(nurse_endtime) # hh:mm:ss
            nurse_shift_dict['avail_day'].append(nurse_availday)
            nurse_shift_dict['week_cycle'].append(nurse_weekcycle)

            # Add info to nurse_day_dict:
            nurse_day_dict['nurse_id'].append(nurse_id[i])
            nurse_day_dict['postcode'].append(nurse_postcode)
            nurse_day_dict['area'].append(nurse_area)
            nurse_day_dict['start'].append(nurse_start) # minutes from midnight
            nurse_day_dict['duration'].append(nurse_duration) # minutes
            nurse_day_dict['end'].append(nurse_end) # minutes from midnight
            nurse_day_dict['longitude'].append(nurse_longitude)
            nurse_day_dict['latitude'].append(nurse_latitude)
            nurse_day_dict['start_time'].append(nurse_starttime) # hh:mm:ss
            nurse_day_dict['end_time'].append(nurse_endtime) # hh:mm:ss
            nurse_day_dict['avail_day'].append(nurse_availday)
            nurse_day_dict['week_cycle'].append(nurse_weekcycle)
        elif len(nurse_shift_rows) > 1: # this carer has multiple shifts on dayindex_weeks
            start_of_day = nurse_shift_rows.iloc[0]['Date Start Time'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first shift.
            # First just get details of carer area, postcode, day, and lat/lon
            nurse_area = nurse_shift_rows['Area'].values[0]
            nurse_postcode = nurse_shift_rows['Postcode'].values[0]
            nurse_availday = nurse_shift_rows['Available Day'].values[0]
            nurse_longitude, nurse_latitude = cpo_inst.find_postcode_lonlat(nurse_postcode)
            if (nurse_longitude == None):
                # print('[ERROR]: No latitude and longitude found for Carer ', nurse_id[i], ', postcode: ', nurse_postcode)
                print('[WARNING]: No latitude and longitude found for Carer ', nurse_id[i], ', postcode: ', nurse_postcode)
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
            for k in range(len(nurse_shift_rows)-1, -1, -1): #for k in range (start from len(nurse_shift_rows)-1, end at -1 (so go up to 0), and step decrement by -1 each time)
                shift_count += 1
                nurse_uniqueid = str(nurse_id[i]) + '__' + str(shift_count)
                nurse_shift = shift_count
                nurse_starttime = nurse_shift_rows['Start Time'].values[k] # hh:mm:ss
                nurse_endtime = nurse_shift_rows['End Time'].values[k] # hh:mm:ss
                nurse_datestarttime = nurse_shift_rows.iloc[k]['Date Start Time'] # yyyy-mm-dd hh:mmm:ss
                nurse_dateendtime = nurse_shift_rows.iloc[k]['Date End Time'] # yyyy-mm-dd hh:mmm:ss
                nurse_start = time_dif_to_minutes(nurse_datestarttime, start_of_day) # minutes from midnight
                nurse_end = time_dif_to_minutes(nurse_dateendtime, start_of_day) # minutes from midnight
                nurse_duration = nurse_end - nurse_start # minutes
                if k == len(nurse_shift_rows)-1:
                    day_starttime = nurse_starttime # hh:mm:ss
                    day_start = nurse_start # minutes from midnight
                if k == 0:
                    day_endtime = nurse_endtime # hh:mm:ss
                    day_end = nurse_end # minutes from midnight

                # Add info to nurse_shift_dict
                nurse_shift_dict['unique_id'].append(nurse_uniqueid)
                nurse_shift_dict['shift'].append(nurse_shift)
                nurse_shift_dict['nurse_id'].append(nurse_id[i])
                nurse_shift_dict['postcode'].append(nurse_postcode)
                nurse_shift_dict['area'].append(nurse_area)
                nurse_shift_dict['start'].append(nurse_start) # minutes from midnight
                nurse_shift_dict['duration'].append(nurse_duration) # minutes
                nurse_shift_dict['end'].append(nurse_end) # minutes from midnight
                nurse_shift_dict['longitude'].append(nurse_longitude)
                nurse_shift_dict['latitude'].append(nurse_latitude)
                nurse_shift_dict['start_time'].append(nurse_starttime) # hh:mm:ss
                nurse_shift_dict['end_time'].append(nurse_endtime) # hh:mm:ss
                nurse_shift_dict['avail_day'].append(nurse_availday)
                nurse_shift_dict['week_cycle'].append(nurse_weekcycle)

                # Add info to nurse_day_dict
                if k == 0:
                    day_duration = day_end - day_start # minutes
                    nurse_day_dict['nurse_id'].append(nurse_id[i])
                    nurse_day_dict['postcode'].append(nurse_postcode)
                    nurse_day_dict['area'].append(nurse_area)
                    nurse_day_dict['start'].append(day_start) # minutes from midnight
                    nurse_day_dict['duration'].append(day_duration) # minutes
                    nurse_day_dict['end'].append(day_end) # minutes from midnight
                    nurse_day_dict['longitude'].append(nurse_longitude)
                    nurse_day_dict['latitude'].append(nurse_latitude)
                    nurse_day_dict['start_time'].append(day_starttime) # hh:mm:ss
                    nurse_day_dict['end_time'].append(day_endtime) # hh:mm:ss
                    nurse_day_dict['avail_day'].append(nurse_availday)
                    nurse_day_dict['week_cycle'].append(nurse_weekcycle)
            # End for k loop
        # End elif multiple shifts       
    # End main for i loop

    nurseshift_df = pd.DataFrame(nurse_shift_dict) # Convert dictionary into pd dataframe
    nurseday_df = pd.DataFrame(nurse_day_dict) # Convert dictionary into pd dataframe

    # nurseshift_df = nurseshift_df[nurseshift_df['avail_day'] == day_index] # filter by day_index
    # nurseshift_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by day_index

    # nurseday_df = nurseday_df[nurseday_df['avail_day'] == day_index] # filter by day_index
    # nurseday_df.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by day_index

    print('length of nurseshift_df: ', len(nurseshift_df))
    print('length of nurseday_df: ', len(nurseday_df))

    # print('nurse_day_dict:')
    # print(nurseday_df)

    # exit(-1)
    print('Finished collecting data.\n')

    return client_df, nurseshift_df, nurseday_df

    # if area == 'None':
    #     return client_df, nurseshift_df, nurseday_df
    # else:
    #     clientdf_area = client_df[client_df['area'] == area]
    #     carershiftdf_area = nurseshift_df[nurseshift_df['area'] == area]
    #     carerdaydf_area = nurseday_df[nurseday_df['area'] == area]
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



def nurse_works_this_slot(slot): # Function finds out whether there is 'Unavailable' in the given cell 'slot' in Carer Availability
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