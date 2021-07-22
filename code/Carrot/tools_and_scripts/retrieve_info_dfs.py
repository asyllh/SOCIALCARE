#--------------------#
# retrieve_info_dfs.py
# 21/04/2021
# 
#--------------------#

import math
import pickle
import datetime
import openpyxl
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

# Own modules:
import tools_and_scripts.class_cpo_df as ccd

def time_dif_to_minutes(endtime, starttime, round=True):
    mins = (endtime - starttime).seconds/60
    if round:
        return np.round(mins, 0)
    else:
        return mins

def carer_works_this_slot(slot): # Function finds out whether there is 'Unavailable' in the given cell 'slot' in Carer Availability
    works = True
    if type(slot) == str: # If there is the string 'Unavailable' in the given 'slot', then the carer does not work that time
        works = False
    else: # Else, the 'slot' is empty, and so the carer is available to work that time.
        works = np.isnan(slot)
    return works

# filename = 'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\abicare\clientcarerdetails.xlsx' #File from Abicare

def retrieve_dfs(area = 'None', tw_interval = 15, print_statements=True, filename='None', foldername='None'):

    # cpo_inst = ccd.CPO_DF(r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\codepo_gb')
    cpo_inst = ccd.CPO_DF(foldername)

    # data_filename = r'data\abicare\clientcarerdetails.xlsx'
    data_filename = filename
    carerdetails_sheetname = 'Carer Details'
    careravail_sheetname = 'Carer Availability'

    # Read date of the rota
    book = openpyxl.load_workbook(data_filename)
    sheet = book.active
    sheet = book[careravail_sheetname]
    date_txt = sheet.cell(row=2, column=1).value
    date_txt = date_txt.split(' ')[1]
    date_pd = pd.to_datetime(date_txt, format='%d/%m/%Y').date()

   ### -------------------------------- Client -------------------------------- ###

    clientdetails_sheetname = 'Clients Details'
    clientcontracts_sheetname = 'Client Contracts'

    # First sheet: Clients Details
    clientdf_details = pd.read_excel(data_filename, sheet_name=clientdetails_sheetname, header=2)
    clientdf_details.drop(clientdf_details.tail(1).index,inplace=True) # Remove the last row from the dataframe as it is just a row of NaN values.
    clientdf_details['Client ID'] = clientdf_details['Client ID'].str.replace(' ', '')
    clientdf_details['Client ID'] = clientdf_details['Client ID'].str.lower()
    clientdf_details['Postcode'] = clientdf_details['Postcode'].str.replace(' ', '')
    clientdf_details['Postcode'] = clientdf_details['Postcode'].str.lower()
   
    # Second sheet: Client Contracts
    clientdf_contracts = pd.read_excel(data_filename, sheet_name=clientcontracts_sheetname, header=5)
    clientdf_contracts['From'] = pd.to_datetime(clientdf_contracts['From'], format='%d/%m/%Y %H:%M')
    clientdf_contracts['Date'] = clientdf_contracts['From'].dt.date
    clientdf_contracts = clientdf_contracts[clientdf_contracts['Date'] == date_pd] # filter by date_pd
    clientdf_contracts.reset_index(inplace=True) # reset the index of the dataframe now that we've filtered by data_pd
    clientdf_contracts['Client'] = clientdf_contracts['Client'].str.replace(' ', '')
    clientdf_contracts['Client'] = clientdf_contracts['Client'].str.lower()
    clientdf_contracts['Duration TD'] = pd.to_timedelta(clientdf_contracts['Duration'], unit='min')
    clientdf_contracts['Start Time'] = clientdf_contracts['From'].dt.time
    clientdf_contracts['To'] = clientdf_contracts['From'] + clientdf_contracts['Duration TD']
    clientdf_contracts['End Time'] = clientdf_contracts['To'].dt.time
    clientdf_contracts['Area'] = clientdf_contracts['Employee']
    clientdf_contracts['Postcode'] = clientdf_contracts['Employee']

    # Dictionary to hold client details
    client_work = {'client_id' : [], 'postcode' : [], 'area' : [], 'date' : [], 
                'start' : [],  'duration' : [], 'end' : [],  'tw_start' : [], 'tw_end' : [], 
                'longitude' : [], 'latitude' : [], 
                'start_time' : [], 'end_time' : [], 'exception' : [], 'county' : [], 'job_function' : []}

    start_of_day = clientdf_contracts.iloc[0]['From'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.
    
    no_details = 0
    multiple_details = 0
    client_id = clientdf_contracts['Client']

    for i in range(len(client_id)):
        client_details_row = clientdf_details.loc[clientdf_details['Client ID'] == client_id[i]]
        client_work_job_function = np.nan
        client_work_area = np.nan
        client_work_county = np.nan 
        client_work_postcode = np.nan 
        client_work_date = np.nan
        client_work_starttime = np.nan
        client_work_duration = np.nan
        client_work_endtime = np.nan 
        client_work_exception = np.nan
        client_work_longitude = np.nan
        client_work_latitude = np.nan
        client_work_start = np.nan 
        client_work_end = np.nan 

        # Check there is no missing information or too much information
        if len(client_details_row) > 1: # client_id[i] has multiple rows in the client details list
            multiple_details += 1
            print('[WARNING]: Client ', client_id[i], ' has duplicate details.')
            print(client_details_row)
            print('Using only first entry.')
        if len(client_details_row) < 1: # client_id[i] is not in the client details list
            no_details += 1
            print('[WARNING]: Client ', client_id[i], ' has no details.')
        else:
            client_work_job_function = client_details_row['Job Function'].values[0]
            client_work_area = client_details_row['Area'].values[0]
            client_work_county = client_details_row['County'].values[0] 
            client_work_postcode = client_details_row['Postcode'].values[0] 
            client_work_date = clientdf_contracts['Date'][i]
            client_work_from = clientdf_contracts['From'][i] # NOTE: this will not be in the final df
            client_work_to = clientdf_contracts['To'][i] # NOTE: this will not be in the final df
            client_work_starttime = clientdf_contracts['Start Time'][i]
            client_work_duration = clientdf_contracts['Duration'][i]
            client_work_endtime = clientdf_contracts['End Time'][i]
            client_work_exception = clientdf_contracts['Exception'][i]
            client_work_start = time_dif_to_minutes(client_work_from, start_of_day)
            client_work_end = time_dif_to_minutes(client_work_to, start_of_day)
            client_work_longitude, client_work_latitude = cpo_inst.find_postcode_lonlat(client_work_postcode)
            if (client_work_longitude == None):
                print('[ERROR]: No latitude and longitude found for Client ', client_id[i], ', postcode: ', client_work_postcode)
                print('Terminating program.')
                exit(-1)

            # Add details to client_work dictionary
            client_work['client_id'].append(str(client_id[i]))
            client_work['job_function'].append(client_work_job_function)
            client_work['area'].append(client_work_area)
            client_work['county'].append(client_work_county)
            client_work['postcode'].append(client_work_postcode)
            client_work['date'].append(client_work_date)
            client_work['start_time'].append(client_work_starttime)
            client_work['duration'].append(client_work_duration)
            client_work['end_time'].append(client_work_endtime)
            client_work['start'].append(client_work_start)
            client_work['end'].append(client_work_end)
            client_work['tw_start'].append(client_work_start - tw_interval)
            client_work['tw_end'].append(client_work_start + tw_interval)
            client_work['exception'].append(client_work_exception)
            client_work['longitude'].append(client_work_longitude)
            client_work['latitude'].append(client_work_latitude)
    # --- End for i in range(len(client_id)) loop --- #
        
    if print_statements:
        print('Number of clients with no details: ', no_details)
        print('Number of clients with multiple details: ', multiple_details)

    # Create dataframe from client_work dictionary
    client_df = pd.DataFrame(client_work)
   
    ### -------------------------------- Carer -------------------------------- ###

    # Carer details sheet
    carerdf_details = pd.read_excel(data_filename, sheet_name=carerdetails_sheetname, header=2)

    key_carer_id = 'On-line Identity'
    carerdf_details['Postcode'] = carerdf_details['Postcode'].str.replace(' ', '')
    carerdf_details['Postcode'] = carerdf_details['Postcode'].str.lower()

    # Carer availability sheet
    carerdf_avail = pd.read_excel(data_filename, sheet_name=careravail_sheetname, header=2)
    h_keys = carerdf_avail.keys()

    #ID's are located under the "Nights" column
    carer_id = carerdf_avail['Nights']

    # Split by even/odd rows
    carer_id = carer_id[::2].values # start:stop:step, so from the beginning to the end of all rows, but every other step
    carer_hours = carerdf_avail[1::2].values # start:stop:step, so from row 1 to the end but every other step
    slot_duration = 15
    # Nights is from 23:00 to 6:00am, 7 hours
    first_slot_duration = 60*7 # Double check this, how long is "nights"?

    # NOTE: These versions of the dictionaries don't have eastings and northings, means we don't have to use POSTCODE_FINDER which uses full codepoint
    carer_shift_work = {'unique_id' : [], 'carer' : [], 'postcode' : [], 'area' : [],
                'shift' : [], 'start' : [], 'duration' : [], 'end' : [], 
                'longitude' : [], 'latitude' : [],
                'start_time' : [], 'end_time' : [],
                'grade' : [], 'job_function' : [],
                'not_on_rota' : [], 'driver' : []}
    
    carer_day_work = {'carer' : [], 'postcode' : [], 'area' : [], 'start' : [], 'duration' : [], 'end' : [], 
                'longitude' : [], 'latitude' : [],
                'start_time' : [], 'end_time' : [],
                'grade' : [], 'job_function' : [],
                'not_on_rota' : [], 'driver' : []}

    for i in range(len(carer_id)): # For each carer id number in the Carer Availability sheet (first row, under 'Nights')
        # Extract and save carer info from the other DF:
        if(carer_id[i] in carer_day_work['carer']):
            print('[WARNING]: duplicate carer availability details for Carer ', carer_id[i])
            print('Skipping duplicate details.')
            continue
        carer_details_row = carerdf_details[carerdf_details[key_carer_id] == carer_id[i]] # carer_details_row is the dataframe containing details for only the current carer_id in the Carer Details sheet
        carer_work_grade = np.nan
        carer_work_job_function = np.nan
        carer_work_postcode = np.nan
        carer_work_not_on_rota = np.nan
        carer_work_driver = np.nan
        carer_work_area = np.nan
        carer_work_longitude = np.nan
        carer_work_latitude = np.nan

        # Check there is no missing/too much info
        if len(carer_details_row) > 1: # If the carer_id[i] has more than one row in Carer Details, then there are duplicate entries
            print('[WARNING]: Carer ', carer_id[i], ' has duplicate details.')
            print(carer_details_row)
            print('Using only first entry.')
            
        if len(carer_details_row) < 1: # If the carer_id[i] has no rows in Carer Details, then there is no information for that carer.
            print('[WARNING]: Carer ', carer_id[i], ' has no details.')
            continue
        else:
            # This is the expected case, only 1 entry:
            carer_work_grade = carer_details_row['Grade'].values[0]
            carer_work_job_function = carer_details_row['Primary Job Function'].values[0]
            carer_work_postcode = carer_details_row['Postcode'].values[0]
            carer_work_not_on_rota = carer_details_row['Not on Rota'].values[0]
            carer_work_driver = carer_details_row['Driver'].values[0]
            carer_work_area = carer_details_row['Primary Area'].values[0]
            carer_work_longitude, carer_work_latitude = cpo_inst.find_postcode_lonlat(carer_work_postcode)
            if (carer_work_longitude == None):
                print('[ERROR]: No latitude and longitude found for Carer ', carer_id[i], ', postcode: ', carer_work_postcode)
                print('Program terminating.')
                exit(-1)

        wh = []
        in_shift = carer_works_this_slot(carer_hours[i,0]) # in_shift = true if carer i is available for column 0 (Nights), else = false if cell contains 'Unavailable'
        if in_shift: # If carer i is available for Night shifts, then append 'Nights' and duration to wh list.
            wh.append({'start' : h_keys[0], 'duration' : 15})
            # print('h_keys[0]:', h_keys[0])
            # exit(-1)
        for j in range(len(carer_hours[i])): # For j = 0 to the number of time shifts (22:45, 22:30, ...,  06:00) for the current carer i
            if in_shift and carer_works_this_slot(carer_hours[i,j]): # If the current carer i was available for the previous time slot and is also available for the current time slot j
                wh[-1]['duration'] += slot_duration # increase shift duration for carer
                wh[-1]['start'] = h_keys[j] # Update start time of shift
            elif not in_shift and carer_works_this_slot(carer_hours[i,j]): # If the current carer i was NOT available for the previous time slot but IS available for the current time slot j
                wh.append({'start' : h_keys[j], 'duration' : 15}) # add this as the start of the carer's shift or a new shift for the same carer
            in_shift = carer_works_this_slot(carer_hours[i,j]) # update in_shift from the previous shift to the current shift j

        # Update carer_shift_work dictionary for current carer i
        shift_count = 0
        day_start = 0
        day_start_time = 0
        day_end = 0
        day_end_time = 0
        for k in range(len(wh) - 1, -1, -1): #for k in range (start from len(wh)-1, end at -1 (so go up to 0), and step decrement by -1 each time)
            shift_count += 1 
            carer_shift_work['unique_id'].append(str(carer_id[i]) + '___' + str(shift_count))
            carer_shift_work['carer'].append(str(carer_id[i]))
            carer_shift_work['shift'].append(shift_count)
            carer_shift_work['start_time'].append(wh[k]['start']) # Start time in hh:mm:ss
            start_dt = datetime.datetime.combine(date_pd, wh[k]['start']) # wh[k]['start'] is datetime.time, need to combine with date_pd to get datetime.datetime, , i.e. yyyy-mm-dd hh:mm:ss
            carer_work_start = time_dif_to_minutes(start_dt, start_of_day) # Get carer start time in minutes from midnight
            carer_shift_work['start'].append(carer_work_start)
            if k == len(wh)-1:
                day_start_time = wh[k]['start']
                day_start = carer_work_start

            carer_shift_work['duration'].append(wh[k]['duration']) # type is int
            end_time = (datetime.datetime.combine(datetime.date(1,1,1), carer_shift_work['start_time'][-1]) + datetime.timedelta(minutes=carer_shift_work['duration'][-1])).time()
            carer_shift_work['end_time'].append(end_time) # End time in hh:mm:ss
            end_dt = start_dt + datetime.timedelta(minutes=carer_shift_work['duration'][-1]) # datetime.datetime, end_dt is the timestamp, i.e. yyyy-mm-dd hh:mm:ss
            carer_work_end = time_dif_to_minutes(end_dt, start_of_day) # Get carer end time in minutes from midnight
            carer_shift_work['end'].append(carer_work_end)
            if k == 0:
                day_end_time = end_time
                day_end = carer_work_end
            # Link with carer details:
            carer_shift_work['grade'].append(carer_work_grade)
            carer_shift_work['job_function'].append(carer_work_job_function)
            carer_shift_work['postcode'].append(carer_work_postcode)
            carer_shift_work['not_on_rota'].append(carer_work_not_on_rota)
            carer_shift_work['driver'].append(carer_work_driver)
            carer_shift_work['area'].append(carer_work_area)
            carer_shift_work['longitude'].append(carer_work_longitude)
            carer_shift_work['latitude'].append(carer_work_latitude)

            # Fill in carer_day_work details:
            if k == 0:
                day_duration = day_end - day_start
                carer_day_work['carer'].append(str(carer_id[i]))
                carer_day_work['start'].append(day_start)
                carer_day_work['end'].append(day_end)
                carer_day_work['start_time'].append(day_start_time)
                carer_day_work['end_time'].append(day_end_time)
                carer_day_work['duration'].append(day_duration)
                carer_day_work['grade'].append(carer_work_grade)
                carer_day_work['job_function'].append(carer_work_job_function)
                carer_day_work['postcode'].append(carer_work_postcode)
                carer_day_work['not_on_rota'].append(carer_work_not_on_rota)
                carer_day_work['driver'].append(carer_work_driver)
                carer_day_work['area'].append(carer_work_area)
                carer_day_work['longitude'].append(carer_work_longitude)
                carer_day_work['latitude'].append(carer_work_latitude)
        # End of for k in range len(wh)-1 loop
    # End of main for loop - for i in range(len(carer_id))

    carershift_df = pd.DataFrame(carer_shift_work) # Convert dictionary into pd dataframe
    carerday_df = pd.DataFrame(carer_day_work) # Convert dictionary into pd dataframe

    if area == 'None':
        return client_df, carershift_df, carerday_df
    else:
        clientdf_area = client_df[client_df['area'] == area]
        carershiftdf_area = carershift_df[carershift_df['area'] == area]
        carerdaydf_area = carerday_df[carerday_df['area'] == area]
        clientdf_area.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area
        carershiftdf_area.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area
        carerdaydf_area.reset_index(inplace=True, drop=True) # reset the index of the dataframe now that we've filtered by area
        carer_count = 0
        for i in range(len(carershiftdf_area)):
            shift_number = carershiftdf_area.iloc[i]['shift']
            if shift_number == 1:
                carer_count += 1
        return clientdf_area, carershiftdf_area, carerdaydf_area
### --- End of def retrieve_info_dfs --- ###

# class POSTCODE_FINDER():
#     """
#     docstring
#     """
#     df = []
#     filename = []
#     def __init__(self, filename=r'data\temp\full_codepoint.csv'):
#         self.filename = filename
#         self.df = pd.read_csv(self.filename)
#         self.df['PC'] = self.df['PC'].str.replace('"', '')
#         self.df['PC'] = self.df['PC'].str.replace(' ', '') # Changed from self.adr_df['PC'].str.replace(' ', '') to self.adr_df['post'].str.replace(' ', '')
#         self.df['PC'] = self.df['PC'].str.lower()

#     def find_postcode_eastnorth(self, postcode): # Returns eastings and northings of given postcode
#         dfview = self.df[self.df['PC'] == postcode]
#         if dfview.empty:
#             print('ERROR Postcode does not exist:', postcode)
#             return [None, None]
#         else:
#             # Need to convert eastings and northings to lat and lon
#             return [dfview.iloc[0]['EA'], dfview.iloc[0]['NO']]
    
#     def find_postcode_n_addresses(self, postcode): # Returns number of registered properties at a given postcode.
#         dfview = self.df[self.df['PC'] == postcode]
#         if dfview.empty:
#             return -1
#         else:
#             return dfview.iloc[0]['RP']


