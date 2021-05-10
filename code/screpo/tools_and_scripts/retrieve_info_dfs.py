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
# --- End def --- #
##################################################


# filename = 'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\abicare\clientcarerdetails.xlsx' #File from Abicare

def retrieve_dfs(area = 'None', print_statements=True):

    pdfinder = POSTCODE_FINDER() # Instantiate object
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

    client_work = {'client_id' : [], 'job_function' : [], 'area' : [], 
                'county' : [], 'postcode' : [], 'date' : [], 'start_time' : [], 
                'duration' : [], 'end_time' : [], 'start' : [], 'end' : [], 'tw_start' : [], 'tw_end' : [], 'exception' : [], 
                'eastings' : [], 'northings' : [], 'longitude' : [], 'latitude' : []}

    start_of_day = clientdf_hours.iloc[0]['From'].replace(hour=0, minute=0, second=0) # Set the start time of that DAY (day_df) to 00:00:00 for the first job.
    # print('start_of_day: ', start_of_day)
    # exit(-1)
    # print(type(clientdf_hours['Start'][0]))
    # print(type(clientdf_hours['Duration'][0]))
    # print(clientdf_hours)

    # exit(-1)
    # print(len(clientdf_hours))

    no_details = 0
    multiple_details = 0
    # timewindow_interval = datetime.timedelta(minutes = 15) # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
    timewindow_interval = 15 # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
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
        client_work_eastings = np.nan
        client_work_northings = np.nan 
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
            client_work_eastings, client_work_northings = pdfinder.find_postcode_eastnorth(client_work_postcode)
            client_work_date = clientdf_hours['Date'][i]
            client_work_from = clientdf_hours['From'][i] # NOTE: this will not be in the final df
            client_work_to = clientdf_hours['To'][i] # NOTE: this will not be in the final df
            client_work_starttime = clientdf_hours['Start Time'][i]
            client_work_duration = clientdf_hours['Duration'][i]
            client_work_endtime = clientdf_hours['End Time'][i]
            client_work_exception = clientdf_hours['Exception'][i]
            client_work_longitude, client_work_latitude = cpo_inst.find_postcode_lonlat(client_work_postcode)
            client_work_start = time_dif_to_minutes(client_work_from, start_of_day)
            client_work_end = time_dif_to_minutes(client_work_to, start_of_day)
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
            client_work['tw_start'].append(client_work_start - timewindow_interval)
            client_work['tw_end'].append(client_work_end + timewindow_interval)
            # print('start client:', client_work['start'][-1])
            # print('tw_start client:', client_work['tw_start'][-1])
            # print('end client:', client_work['end'][-1])
            # print('tw_end client:', client_work['tw_end'][-1])
            # exit(-1)
            client_work['exception'].append(client_work_exception)
            client_work['eastings'].append(client_work_eastings)
            client_work['northings'].append(client_work_northings)
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
    carer_work = {'unique_id' : [], 'carer' : [], 
                'shift' : [], 'start' : [], 
                'duration' : [], 'end' : [],
                'grade' : [], 'job_function' : [],
                'postcode' : [], 'not_on_rota' : [],
                'driver' : [], 'area' : [], 'eastings' : [], 'northings' : [], 'longitude' : [], 'latitude' : []}

    for i in range(len(carer_id)): # For each carer id number in the Carer Availability sheet (first row, under 'Nights')
        # Extract and save carer info from the other DF:
        carer_details_row = carerdf_details[carerdf_details[key_carer_id] == carer_id[i]] # carer_details_row is the dataframe containing details for only the current carer_id in the Carer Details sheet
        carer_work_grade = np.nan
        carer_work_job_function = np.nan
        carer_work_postcode = np.nan
        carer_work_not_on_rota = np.nan
        carer_work_driver = np.nan
        carer_work_area = np.nan
        carer_work_eastings = np.nan
        carer_work_northings = np.nan
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
        else:
            # This is the expected case, only 1 entry:
            carer_work_grade = carer_details_row['Grade'].values[0]
            carer_work_job_function = carer_details_row['Primary Job Function'].values[0]
            carer_work_postcode = carer_details_row['Postcode'].values[0]
            carer_work_not_on_rota = carer_details_row['Not on Rota'].values[0]
            carer_work_driver = carer_details_row['Driver'].values[0]
            carer_work_area = carer_details_row['Primary Area'].values[0]
            carer_work_eastings, carer_work_northings = pdfinder.find_postcode_eastnorth(carer_work_postcode)
            carer_work_longitude, carer_work_latitude = cpo_inst.find_postcode_lonlat(carer_work_postcode)

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

        # Update carer_work dictionary for current carer i
        shift_count = 0
        for k in range(len(wh) - 1, -1, -1): #for k in range (start from len(wh)-1, end at -1 (so go up to 0), and step decrement by -1 each time)
            shift_count += 1 
            carer_work['unique_id'].append(str(carer_id[i]) + '___' + str(shift_count))
            carer_work['carer'].append(str(carer_id[i]))
            carer_work['shift'].append(shift_count)

            # print(wh[k]['start'])
            # print(type(wh[k]['start']))
            # print(wh[k]['duration'])
            # print(type(wh[k]['duration']))
            # exit(-1)
            # carer_work['start'].append(wh[k]['start'])
            # start_dt = datetime.datetime.combine(date_pd, carer_work['start'][-1])
            start_dt = datetime.datetime.combine(date_pd, wh[k]['start']) # wh[k]['start'] is datetime.time, need to combine with date_pd to get datetime.datetime
            # print('start_dt:', start_dt)
            # print('type start_dt:', type(start_dt))
            carer_work_start = time_dif_to_minutes(start_dt, start_of_day) # Get carer start time in minutes from midnight
            # print('carer_work_start: ', carer_work_start)    
            # print('type carer_work_start: ', type(carer_work_start))    
            carer_work['start'].append(carer_work_start)

            # exit(-1)
            carer_work['duration'].append(wh[k]['duration']) # type is int
            # endtime = (datetime.datetime.combine(datetime.date(1,1,1), carer_work['start'][-1]) + datetime.timedelta(minutes=carer_work['duration'][-1])).time()
            end_dt = start_dt + datetime.timedelta(minutes=carer_work['duration'][-1]) # datetime.datetime, end_dt is the timestamp
            # print('end_dt:', end_dt)
            # print('type end_dt:', type(end_dt))
            carer_work_end = time_dif_to_minutes(end_dt, start_of_day) # Get carer end time in minutes from midnight
            # print('carer_work_end: ', carer_work_end)
            # print('type carer_work_end: ', type(carer_work_end))
            carer_work['end'].append(carer_work_end)
            # carer_work['end'].append(endtime)
            # print('end: ', carer_work['end'][-1])
            # print('end type carer:', type(carer_work['end'][-1]))
            # exit(-1)
            # Link with carer details:
            carer_work['grade'].append(carer_work_grade)
            carer_work['job_function'].append(carer_work_job_function)
            carer_work['postcode'].append(carer_work_postcode)
            carer_work['not_on_rota'].append(carer_work_not_on_rota)
            carer_work['driver'].append(carer_work_driver)
            carer_work['area'].append(carer_work_area)
            carer_work['eastings'].append(carer_work_eastings)
            carer_work['northings'].append(carer_work_northings)
            carer_work['longitude'].append(carer_work_longitude)
            carer_work['latitude'].append(carer_work_latitude)
    # End of main for loop - for i in range(len(carer_id))

    carer_df = pd.DataFrame(carer_work) # Convert dictionary into pd dataframe

    # print(carer_df)
    # exit(-1)

    if area == 'None':
        return client_df, carer_df
    else:
        # print('Len client_df: ', len(client_df))
        # print('Len carer_df: ', len(carer_df))
        clientdf_area = client_df[client_df['area'] == area]
        carerdf_area = carer_df[carer_df['area'] == area]
        clientdf_area.reset_index(inplace=True) # reset the index of the dataframe now that we've filtered by area
        carerdf_area.reset_index(inplace=True) # reset the index of the dataframe now that we've filtered by area
        # print(clientdf_area)
        # print(carerdf_area)
        # exit(-1)
        return clientdf_area, carerdf_area

    # print(carer_df)
    
### --- End retrieve_dfs function

# client_df, carer_df = retrieve_dfs(print_statements=False)
# print('Len client_df: ', len(client_df))
# print('Len carer_df: ', len(carer_df))



# df_to_inst(carer_df, clientdf_hours)