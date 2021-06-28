#--------------------#
# retrieve_info_dfs.py
# 21/04/2021
# 
#--------------------#

import math
import pickle
import datetime
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats
import openpyxl
# import class_cpo_df as ccd
import tools_and_scripts.class_cpo_df as ccd
# from df_to_inst_test import *

# retrieve_info_dfs.py: main full codepoint analyse mileage, using abicare's data

class POSTCODE_FINDER():
    """
    docstring
    """
    df = []
    filename = []
    def __init__(self, filename=r'data\temp\full_codepoint.csv'):
        self.filename = filename
        self.df = pd.read_csv(self.filename)
        self.df['PC'] = self.df['PC'].str.replace('"', '')
        self.df['PC'] = self.df['PC'].str.replace(' ', '') # Changed from self.adr_df['PC'].str.replace(' ', '') to self.adr_df['post'].str.replace(' ', '')
        self.df['PC'] = self.df['PC'].str.lower()

    def find_postcode_eastnorth(self, postcode): # Returns eastings and northings of given postcode
        dfview = self.df[self.df['PC'] == postcode]
        if dfview.empty:
            print('ERROR Postcode does not exist:', postcode)
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

def carer_works_this_slot(slot): # Function finds out whether there is 'Unavailable' in the given cell 'slot' in Carer Availability
    works = True
    if type(slot) == str: # If there is the string 'Unavailable' in the given 'slot', then the carer does not work that time
        works = False
    else: # Else, the 'slot' is empty, and so the carer is available to work that time.
        works = np.isnan(slot)
    return works

# filename = 'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\abicare\clientcarerdetails.xlsx' #File from Abicare

def retrieve_dfs(area = 'None', tw_interval = 15, print_statements=True):

    # pdfinder = POSTCODE_FINDER() # Instantiate object
    cpo_inst = ccd.CPO_DF()

    data_filename = r'data\abicare\clientcarerdetails.xlsx'
    carerdetails_sheetname = 'Carer Details'
    carerhours_sheetname = 'Carer Availability'

    # Read date of the rota
    book = openpyxl.load_workbook(data_filename)
    sheet = book.active
    sheet = book[carerhours_sheetname]
    date_txt = sheet.cell(row=2, column=1).value
    date_txt = date_txt.split(' ')[1]
    date_pd = pd.to_datetime(date_txt, format='%d/%m/%Y').date()

    ### --- Client --- ###

    clientdetails_sheetname = 'Clients Details'
    clienthours_sheetname = 'Client Contracts'
    # This is the first sheet, 'Clients Details', could also use sheet_name=0.
    # clientdf_details = pd.read_excel(data_filename, sheet_name='Clients Details', header=2)
    clientdf_details = pd.read_excel(data_filename, sheet_name=clientdetails_sheetname, header=2)
    # clientdf_details.head(-1)
    clientdf_details.drop(clientdf_details.tail(1).index,inplace=True) # Remove the last row from the dataframe as it is just a row of NaN values.

    clientdf_details['Client ID'] = clientdf_details['Client ID'].str.replace(' ', '')
    clientdf_details['Client ID'] = clientdf_details['Client ID'].str.lower()
    clientdf_details['Postcode'] = clientdf_details['Postcode'].str.replace(' ', '')
    clientdf_details['Postcode'] = clientdf_details['Postcode'].str.lower()
    # print(clientdf_details)

    # clientdf_hours = pd.read_excel(data_filename, sheet_name='Client Contracts', header=5)
    clientdf_hours = pd.read_excel(data_filename, sheet_name=clienthours_sheetname, header=5)
    clientdf_hours['From'] = pd.to_datetime(clientdf_hours['From'], format='%d/%m/%Y %H:%M')
    clientdf_hours['Date'] = clientdf_hours['From'].dt.date
    
    # Filter date by date_pd
    clientdf_hours = clientdf_hours[clientdf_hours['Date'] == date_pd] # filter by date_pd
    clientdf_hours.reset_index(inplace=True) # reset the index of the dataframe now that we've filtered by data_pd

    clientdf_hours['Client'] = clientdf_hours['Client'].str.replace(' ', '')
    clientdf_hours['Client'] = clientdf_hours['Client'].str.lower()
    clientdf_hours['Duration TD'] = pd.to_timedelta(clientdf_hours['Duration'], unit='min')
    
    clientdf_hours['Start Time'] = clientdf_hours['From'].dt.time
    clientdf_hours['To'] = clientdf_hours['From'] + clientdf_hours['Duration TD']
    # clientdf_hours['To'] = clientdf_hours['To'].dt.time
    clientdf_hours['End Time'] = clientdf_hours['To'].dt.time
    clientdf_hours['Area'] = clientdf_hours['Employee']
    clientdf_hours['Postcode'] = clientdf_hours['Employee']
    clientdf_hours['Eastings'] = clientdf_hours['Employee']
    clientdf_hours['Northings'] = clientdf_hours['Employee']

    # client_work = {'client_id' : [], 'postcode' : [], 'area' : [], 'date' : [], 
    #             'start' : [],  'duration' : [], 'end' : [],  'tw_start' : [], 'tw_end' : [], 
    #             'longitude' : [], 'latitude' : [], 'eastings' : [], 'northings' : [],
    #             'start_time' : [], 'end_time' : [], 'exception' : [], 'county' : [], 'job_function' : []}

    # NOTE: This version of the dictionary doesn't have eastings and northings, means we don't have to use POSTCODE_FINDER which uses full codepoint
    client_work = {'client_id' : [], 'postcode' : [], 'area' : [], 'date' : [], 
                'start' : [],  'duration' : [], 'end' : [],  'tw_start' : [], 'tw_end' : [], 
                'longitude' : [], 'latitude' : [], 
                'start_time' : [], 'end_time' : [], 'exception' : [], 'county' : [], 'job_function' : []}

    start_of_day = clientdf_hours.iloc[0]['From'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.
    print('start_of_day: ', start_of_day)
    print('type start_of_day: ', type(start_of_day))
    # exit(-1)
    # print(type(clientdf_hours['Start'][0]))
    # print(type(clientdf_hours['Duration'][0]))
    # print(clientdf_hours)

    # exit(-1)
    # print(len(clientdf_hours))

    no_details = 0
    multiple_details = 0
    # timewindow_interval = datetime.timedelta(minutes = 15) # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
    # timewindow_interval = 15 # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
    # print(clientdf_hours.keys())
    # client_id = clientdf_hours['Clients']
    client_id = clientdf_hours['Client']

    # for i in range(len(clientdf_hours)):
    for i in range(len(client_id)):
        # clientid = clientdf_hours['Client'][i]
        # print(clientid)
        # row = clientdf_details.loc[clientdf_details['Client ID'] == clientid]
        client_details_row = clientdf_details.loc[clientdf_details['Client ID'] == client_id[i]]
        client_work_job_function = np.nan
        client_work_area = np.nan
        client_work_county = np.nan 
        client_work_postcode = np.nan 
        # client_work_eastings = np.nan
        # client_work_northings = np.nan 
        client_work_date = np.nan
        client_work_starttime = np.nan
        client_work_duration = np.nan
        client_work_endtime = np.nan 
        client_work_exception = np.nan
        client_work_longitude = np.nan
        client_work_latitude = np.nan
        client_work_start = np.nan 
        client_work_end = np.nan 
        # print(clientdf_details.loc[clientdf_details['Client ID'] == clientid])
        # print(row)
        # Check there is no missing information or too much information
        # if len(row) > 1:
        if len(client_details_row) > 1: # client_id[i] has multiple rows in the client details list
            # print('WARNING: Client "', clientid, '" has duplicated details')
            multiple_details += 1
            if print_statements:
                print('WARNING: Client ', client_id[i], ' has duplicated details')
                print(client_details_row)
        # elif len(row) < 1: 
        elif len(client_details_row) < 1: # client_id[i] is not in the client details list
            # print('DATAFRAME EMPTY')
            # print('WARNING: Client "', clientid, '" has no details')
            no_details += 1
            if print_statements:
                print('WARNING: Client ', client_id[i], ' has no details')
        else:
            client_work_job_function = client_details_row['Job Function'].values[0]
            client_work_area = client_details_row['Area'].values[0]
            client_work_county = client_details_row['County'].values[0] 
            client_work_postcode = client_details_row['Postcode'].values[0] 
            # client_work_eastings, client_work_northings = pdfinder.find_postcode_eastnorth(client_work_postcode)
            client_work_date = clientdf_hours['Date'][i]
            client_work_from = clientdf_hours['From'][i] # NOTE: this will not be in the final df
            client_work_to = clientdf_hours['To'][i] # NOTE: this will not be in the final df
            print('client-work_from: ', client_work_from, ' type: ', type(client_work_from))
            print('client-work_to: ', client_work_to, ' type: ', type(client_work_to))
            client_work_starttime = clientdf_hours['Start Time'][i]
            client_work_duration = clientdf_hours['Duration'][i]
            client_work_endtime = clientdf_hours['End Time'][i]
            print('client_work_starttime: ', client_work_starttime, ' type: ', type(client_work_starttime))
            print('client_work_endtime: ', client_work_endtime, ' type: ', type(client_work_endtime))
            print('client_work_duration: ', client_work_duration, ' type: ', type(client_work_duration))

            client_work_exception = clientdf_hours['Exception'][i]
            client_work_longitude, client_work_latitude = cpo_inst.find_postcode_lonlat(client_work_postcode)
            client_work_start = time_dif_to_minutes(client_work_from, start_of_day)
            client_work_end = time_dif_to_minutes(client_work_to, start_of_day)
            print('client_work_start: ', client_work_start, ' type: ', type(client_work_start))
            print('client_work_end: ', client_work_end, ' type: ', type(client_work_end))
            exit(-1)
            # print('from: ', client_work_from)
            # print('starttime: ', client_work_starttime)
            # print('start:', client_work_start)
            # print('to: ', client_work_to)
            # print('endtime: ', client_work_endtime)
            # print('end:', client_work_end)
            # exit(-1)

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
            # print('start client:', client_work['start'][-1])
            # print('tw_start client:', client_work['tw_start'][-1])
            # print('end client:', client_work['end'][-1])
            # print('tw_end client:', client_work['tw_end'][-1])
            # exit(-1)
            client_work['exception'].append(client_work_exception)
            # client_work['eastings'].append(client_work_eastings)
            # client_work['northings'].append(client_work_northings)
            client_work['longitude'].append(client_work_longitude)
            client_work['latitude'].append(client_work_latitude)
    # --- End of for loop --- #
        
    if print_statements:
        print('Number of clients with no details: ', no_details)
        print('Number of clients with multiple details: ', multiple_details)
    # print(clientdf_hours)

    client_df = pd.DataFrame(client_work)
    # print(client_df)
    # u_areas = clientdf_details['Area'].unique() # List of Areas in df
    # print('There are', len(u_areas), 'areas:', u_areas)
    # exit(-1)

    ### -------------------------------- Carer -------------------------------- ###

    # Carer details sheet
    carerdf_details = pd.read_excel(data_filename, sheet_name=carerdetails_sheetname, header=2)

    key_carer_id = 'On-line Identity'
    carerdf_details['Postcode'] = carerdf_details['Postcode'].str.replace(' ', '')
    carerdf_details['Postcode'] = carerdf_details['Postcode'].str.lower()

    # Carer availability sheet
    carerdf_hours = pd.read_excel(data_filename, sheet_name=carerhours_sheetname, header=2)
    h_keys = carerdf_hours.keys()

    #ID's are located under the "Nights" column
    carer_id = carerdf_hours['Nights']

    # Split by even/odd rows
    carer_id = carer_id[::2].values # start:stop:step, so from the beginning to the end of all rows, but every other step
    carer_hours = carerdf_hours[1::2].values # start:stop:step, so from row 1 to the end but every other step
    slot_duration = 15
    # Nights is from 23:00 to 6:00am, 7 hours
    first_slot_duration = 60*7 # Double check this, how long is "nights"?

    # carer_shift_work = {'unique_id' : [], 'carer' : [], 'postcode' : [], 'area' : [],
    #             'shift' : [], 'start' : [], 'duration' : [], 'end' : [], 
    #             'longitude' : [], 'latitude' : [], 'eastings' : [], 'northings' : [],
    #             'start_time' : [], 'end_time' : [],
    #             'grade' : [], 'job_function' : [],
    #             'not_on_rota' : [], 'driver' : []}
    
    # carer_day_work = {'carer' : [], 'postcode' : [], 'area' : [], 'start' : [], 'duration' : [], 'end' : [], 
    #             'longitude' : [], 'latitude' : [], 'eastings' : [], 'northings' : [],
    #             'start_time' : [], 'end_time' : [],
    #             'grade' : [], 'job_function' : [],
    #             'not_on_rota' : [], 'driver' : []}

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
        carer_details_row = carerdf_details[carerdf_details[key_carer_id] == carer_id[i]] # carer_details_row is the dataframe containing details for only the current carer_id in the Carer Details sheet
        carer_work_grade = np.nan
        carer_work_job_function = np.nan
        carer_work_postcode = np.nan
        carer_work_not_on_rota = np.nan
        carer_work_driver = np.nan
        carer_work_area = np.nan
        # carer_work_eastings = np.nan
        # carer_work_northings = np.nan
        carer_work_longitude = np.nan
        carer_work_latitude = np.nan

        # Check there is no missing/too much info
        if len(carer_details_row) > 1: # If the carer_id[i] has more than one row in Carer Details, then there are duplicate entries
            if print_statements:
                print('WARNING: Carer "', carer_id[i], '" has duplicated details.')
                print(carer_details_row)
                print('Using only first entry.')
            
        if len(carer_details_row) < 1: # If the carer_id[i] has no rows in Carer Details, then there is no information for that carer.
            if print_statements:
                print('WARNING: Carer "', carer_id[i], '" has no details.')
            continue
        else:
            # This is the expected case, only 1 entry:
            carer_work_grade = carer_details_row['Grade'].values[0]
            carer_work_job_function = carer_details_row['Primary Job Function'].values[0]
            carer_work_postcode = carer_details_row['Postcode'].values[0]
            carer_work_not_on_rota = carer_details_row['Not on Rota'].values[0]
            carer_work_driver = carer_details_row['Driver'].values[0]
            carer_work_area = carer_details_row['Primary Area'].values[0]
            # carer_work_eastings, carer_work_northings = pdfinder.find_postcode_eastnorth(carer_work_postcode)
            carer_work_longitude, carer_work_latitude = cpo_inst.find_postcode_lonlat(carer_work_postcode)
            # print(carer_work_longitude)
            # print(type(carer_work_longitude))
            if (carer_work_longitude == None):
                # print('carer_work_eastings', carer_work_eastings)
                # print('carer_work_northings', carer_work_northings)
                # print('carer_work_longitude', carer_work_longitude)
                # print('carer_work_latitude', carer_work_latitude)
                print('No latitude and longitude found for carer_work_postcode: ', carer_work_postcode)
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

        # if carer_id[i] == 209608 and i != 140:
        #     print('i: ', i)
        #     print('carer_id: ', carer_id[i])
        #     print('len wh: ', len(wh))
        #     print(wh)
        #     exit(-1)
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
            # carer_shift_work['eastings'].append(carer_work_eastings)
            # carer_shift_work['northings'].append(carer_work_northings)
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
                # carer_day_work['eastings'].append(carer_work_eastings)
                # carer_day_work['northings'].append(carer_work_northings)
                carer_day_work['longitude'].append(carer_work_longitude)
                carer_day_work['latitude'].append(carer_work_latitude)
        # End of for k in range len(wh)-1 loop
        # if carer_id[i] == 209515:
        #     print('i: ', i)
        #     print('carer_id: ', carer_id[i])
        #     print('len wh: ', len(wh))
        #     print(wh)
        #     print('day_start: ', day_start)
        #     print('day_start_time: ', day_start_time)
        #     print('day_end: ', day_end)
        #     print('day_end_time: ', day_end_time)
        #     day_duration = day_end - day_start
        #     print('day_duration: ', day_duration)
        #     exit(-1)
    # End of main for loop - for i in range(len(carer_id))

    carershift_df = pd.DataFrame(carer_shift_work) # Convert dictionary into pd dataframe
    carerday_df = pd.DataFrame(carer_day_work) # Convert dictionary into pd dataframe

    # print(carershift_df)
    # exit(-1)

    if area == 'None':
        return client_df, carershift_df, carerday_df
    else:
        # print('Len client_df: ', len(client_df))
        # print('Len carershift_df: ', len(carershift_df))
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
        # print('Carer_count: ', carer_count)
        # print(clientdf_area)
        # print(carerdf_area)
        # exit(-1)
        return clientdf_area, carershiftdf_area, carerdaydf_area

    # print(carershift_df)
### --- End of def retrieve_info_dfs --- ###

    