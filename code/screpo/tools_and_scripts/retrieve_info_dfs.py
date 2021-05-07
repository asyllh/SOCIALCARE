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
from df_to_inst_test import *

# retrieve_info_dfs.py: main full codepoint analyse mileage, using abicare's data

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

def retrieve_dfs():

    pdfinder = POSTCODE_FINDER() # Instantiate object
    data_filename = r'../data/abicare/clientcarerdetails.xlsx'

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

    clientdf_hours['Client'] = clientdf_hours['Client'].str.replace(' ', '')
    clientdf_hours['Client'] = clientdf_hours['Client'].str.lower()
    clientdf_hours['Duration TD'] = pd.to_timedelta(clientdf_hours['Duration'], unit='min')
    clientdf_hours['From'] = pd.to_datetime(clientdf_hours['From'], format='%d/%m/%Y %H:%M')
    clientdf_hours['Date'] = clientdf_hours['From'].dt.date
    clientdf_hours['Start Time'] = clientdf_hours['From'].dt.time
    clientdf_hours['To'] = clientdf_hours['From'] + clientdf_hours['Duration TD']
    # clientdf_hours['To'] = clientdf_hours['To'].dt.time
    clientdf_hours['End Time'] = clientdf_hours['To'].dt.time
    clientdf_hours['Area'] = clientdf_hours['Employee']
    clientdf_hours['Postcode'] = clientdf_hours['Employee']
    clientdf_hours['Eastings'] = clientdf_hours['Employee']
    clientdf_hours['Northings'] = clientdf_hours['Employee']

    client_work = {'client_id' : [], 'job_function' : [], 'area' : [], 
                'county' : [], 'postcode' : [], 'date' : [], 'start' : [], 
                'duration' : [], 'end' : [], 'exception' : [], 
                'eastings' : [], 'northings' : []}

    # print(type(clientdf_hours['Start'][0]))
    # print(type(clientdf_hours['Duration'][0]))
    # print(clientdf_hours)

    # exit(-1)
    # print(len(clientdf_hours))

    no_details = 0
    multiple_details = 0
    timewindow_interval = datetime.timedelta(minutes = 15) # Timewindow generated as plus-minus these minutes of the start date, change this value 30, 15, etc.
    client_id = clientdf_hours['Clients']
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
        client_work_start = np.nan
        client_work_duration = np.nan
        client_work_end = np.nan 
        client_work_exception = np.nan
        # print(clientdf_details.loc[clientdf_details['Client ID'] == clientid])
        # print(row)
        # Check there is no missing information or too much information
        # if len(row) > 1:
        if len(client_details_row) > 1: # client_id[i] has multiple rows in the client details list
            # print('WARNING: Client "', clientid, '" has duplicated details')
            print('WARNING: Client ', client_id[i], ' has duplicated details')
            print(client_details_row)
            multiple_details += 1
        # elif len(row) < 1: 
        elif len(client_details_row) < 1: # client_id[i] is not in the client details list
            # print('DATAFRAME EMPTY')
            # print('WARNING: Client "', clientid, '" has no details')
            print('WARNING: Client ', client_id[i], ' has no details')
            no_details += 1
        # else:
        #     area = row.iloc[0]['Area']
        #     postcode = row.iloc[0]['Postcode']
        #     eastings, northings = pdfinder.find_postcode_eastnorth(postcode)
        #     clientdf_hours['Area'][i] = area
        #     clientdf_hours['Postcode'][i] = postcode
        #     clientdf_hours['Eastings'][i] = eastings
        #     clientdf_hours['Northings'][i] = northings
        else:
            client_work_job_function = client_details_row['Job Function'].values[0]
            client_work_area = client_details_row['Area'].values[0]
            client_work_county = client_details_row['County'].values[0] 
            client_work_postcode = client_details_row['Postcode'].values[0] 
            client_work_eastings, client_work_northings = pdfinder.find_postcode_eastnorth(client_work_postcode)
            client_work_date = clientdf_hours['Date'][i]
            client_work_start = clientdf_hours['Start Time'][i]
            client_work_duration = clientdf_hours['Duration TD'][i]
            client_work_end = clientdf_hours['End Time'][i]
            client_work_exception = clientdf_hours['Exception'][i]

            client_work['client_id'].append(str(client_id[i]))
            client_work['job_function'].append(client_work_job_function)
            client_work['area'].append(client_work_area)
            client_work['county'].append(client_work_county)
            client_work['postcode'].append(client_work_postcode)
            client_work['date'].append(client_work_date)
            client_work['start'].append(client_work_start)
            client_work['duration'].append(client_work_duration)
            client_work['end'].append(client_work_end)
            client_work['exception'].append(client_work_exception)
            client_work['eastings'].append(client_work_eastings)
            client_work['northings'].append(client_work_northings)
    # --- End of for loop --- #
        

    print('Number of clients with no details: ', no_details)
    print('Number of clients with multiple details: ', multiple_details)
    # print(clientdf_hours)

    client_df = pd.DataFrame(client_work)
    print(client_df)
    # u_areas = clientdf_details['Area'].unique() # List of Areas in df
    # print('There are', len(u_areas), 'areas:', u_areas)
    # exit(-1)

    ### -------------------------------- Carer -------------------------------- ###

    carerdetails_sheetname = 'Carer Details'
    carerhours_sheetname = 'Carer Availability'


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
                'grade' : [], 'p_job_function' : [],
                'postcode' : [], 'not_on_rota' : [],
                'driver' : [], 'p_area' : [], 'eastings' : [], 'northings' : []}

    for i in range(len(carer_id)): # For each carer id number in the Carer Availability sheet (first row, under 'Nights')
        # Extract and save carer info from the other DF:
        carer_details_row = carerdf_details[carerdf_details[key_carer_id] == carer_id[i]] # carer_details_row is the dataframe containing details for only the current carer_id in the Carer Details sheet
        carer_work_grade = np.nan
        carer_work_p_job_function = np.nan
        carer_work_postcode = np.nan
        carer_work_not_on_rota = np.nan
        carer_work_driver = np.nan
        carer_work_p_area = np.nan
        carer_work_eastings = np.nan
        carer_work_northings = np.nan

        # Check there is no missing/too much info
        if len(carer_details_row) > 1: # If the carer_id[i] has more than one row in Carer Details, then there are duplicate entries
            print('WARNING: Carer "', carer_id[i], '" has duplicated details.')
            print(carer_details_row)
            print('Using only first entry.')
            
        if len(carer_details_row) < 1: # If the carer_id[i] has no rows in Carer Details, then there is no information for that carer.
            print('WARNING: Carer "', carer_id[i], '" has no details.')
        else:
            # This is the expected case, only 1 entry:
            carer_work_grade = carer_details_row['Grade'].values[0]
            carer_work_p_job_function = carer_details_row['Primary Job Function'].values[0]
            carer_work_postcode = carer_details_row['Postcode'].values[0]
            carer_work_not_on_rota = carer_details_row['Not on Rota'].values[0]
            carer_work_driver = carer_details_row['Driver'].values[0]
            carer_work_p_area = carer_details_row['Primary Area'].values[0]
            carer_work_eastings, carer_work_northings = pdfinder.find_postcode_eastnorth(carer_work_postcode)


        wh = []
        in_shift = carer_works_this_slot(carer_hours[i,0]) # in_shift = true if carer i is available for column 0 (Nights), else = false if cell contains 'Unavailable'
        if in_shift: # If carer i is available for Night shifts, then append 'Nights' and duration to wh list.
            wh.append({'start' : h_keys[0], 'duration' : 15})
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
            carer_work['start'].append(wh[k]['start'])
            carer_work['duration'].append(wh[k]['duration'])
            endtime = (datetime.datetime.combine(datetime.date(1,1,1), carer_work['start'][-1]) + datetime.timedelta(minutes=carer_work['duration'][-1])).time()
            carer_work['end'].append(endtime)
            # Link with carer details:
            carer_work['grade'].append(carer_work_grade)
            carer_work['p_job_function'].append(carer_work_p_job_function)
            carer_work['postcode'].append(carer_work_postcode)
            carer_work['not_on_rota'].append(carer_work_not_on_rota)
            carer_work['driver'].append(carer_work_driver)
            carer_work['p_area'].append(carer_work_p_area)
            carer_work['eastings'].append(carer_work_eastings)
            carer_work['northings'].append(carer_work_northings)
    # End of main for loop - for i in range(len(carer_id))

    carer_df = pd.DataFrame(carer_work) # Convert dictionary into pd dataframe


    print(carer_df)

    return clientdf_hours, carer_df
### --- End retrieve_dfs function


# df_to_inst(carer_df, clientdf_hours)