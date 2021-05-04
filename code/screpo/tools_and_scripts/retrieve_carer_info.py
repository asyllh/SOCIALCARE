import pandas as pd
import numpy as np
import datetime

def carer_works_this_slot(slot): # Function finds out whether there is 'Unavailable' in the given cell 'slot' in Carer Availability
    works = True
    if type(slot) == str: # If there is the string 'Unavailable' in the given 'slot', then the carer does not work that time
        works = False
    else: # Else, the 'slot' is empty, and so the carer is available to work that time.
        works = np.isnan(slot)
    return works
# --- End def --- #

cdetails_sheetname = 'Carer Details'
chours_sheetname = 'Carer Availability'
data_filename = r'../data/abicare/clientcarerdetails.xlsx'

# Carer details sheet
dfdetails = pd.read_excel(data_filename, sheet_name=cdetails_sheetname, header=2)
key_carer_id = 'On-line Identity'
dfdetails['Postcode'] = dfdetails['Postcode'].str.replace(' ', '')
dfdetails['Postcode'] = dfdetails['Postcode'].str.lower()

# Carer availability sheet
dfhours = pd.read_excel(data_filename, sheet_name=chours_sheetname, header=2)
h_keys = dfhours.keys()

#ID's are located under the "Nights" column
carer_id = dfhours['Nights']

# Split by even/odd rows
carer_id = carer_id[::2].values # start:stop:step, so from the beginning to the end of all rows, but every other step
carer_hours = dfhours[1::2].values # start:stop:step, so from row 1 to the end but every other step

slot_duration = 15
# Nights is from 23:00 to 6:00am, 7 hours
first_slot_duration = 60*7 # Double check this, how long is "nights"?
carer_work = {'unique_id' : [], 'carer' : [], 
              'shift' : [], 'start' : [], 
              'duration' : [], 'end' : [],
             'grade' : [], 'p_job_function' : [],
             'postcode' : [], 'not_on_rota' : [],
             'driver' : [], 'p_area' : []}

for i in range(len(carer_id)): # For each carer id number in the Carer Availability sheet (first row, under 'Nights')
    # Extract and save carer info from the other DF:
    carer_details_row = dfdetails[dfdetails[key_carer_id] == carer_id[i]] # carer_details_row is the dataframe containing details for only the current carer_id in the Carer Details sheet
    carer_work_grade = np.nan
    carer_work_p_job_function = np.nan
    carer_work_postcode = np.nan
    carer_work_not_on_rota = np.nan
    carer_work_driver = np.nan
    carer_work_p_area = np.nan

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
# End of main for loop - for i in range(len(carer_id))

work_df = pd.DataFrame(carer_work) # Convert dictionary into pd dataframe


print(work_df)




