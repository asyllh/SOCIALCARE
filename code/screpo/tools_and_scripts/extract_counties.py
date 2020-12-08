import os, glob
import pandas as pd

MIN_ADDRESSES = 12

# Step 1 - Get from CP Open the postcodes in the counties of interest:
CP_Open_Folder = r'..\data\codepo_gb'
all_cpopen_csvs = glob.glob(os.path.join(CP_Open_Folder, 'data\CSV', "*.csv"))

# Get the headers in a list:
with open(os.path.join(CP_Open_Folder, 'Doc', 'Code-Point_Open_Column_Headers.csv')) as f:
    raw_headers = f.readline()
    cp_open_headers = raw_headers.split(',')

# We are interested in these areas:
# 'Wiltshire', 'Hampshire', 'Monmouthshire'
# Which we search with the codes below (county our unitary authority ('DC'))
utas = [['DC', 'E06000054'], ['CC', 'E10000014'], ['DC', 'W06000021']]

postcode_list = []
for csvfile in all_cpopen_csvs:
    csvpart = pd.read_csv(csvfile, names=cp_open_headers)
    for place_type, place_code in utas:
        cp_rest = csvpart[csvpart[place_type] == place_code]
        postcode_list += list(cp_rest['PC'])

print(len(postcode_list))
print(postcode_list[:5])
# Step 2 - Open the full Codepoint dataset and check the postcodes in "postcode_list"
full_codepoint_filename = r'..\data\codepoint\codepoint_3765705\full_codepoint.csv'
full_cp_df = pd.read_csv(full_codepoint_filename)
forbidden_list = []
for p in postcode_list:
    postcode_row = full_cp_df[full_cp_df['PC'] == p]
    if len(postcode_row) >= 1:
        if len(postcode_row) > 1:
            print('WARNING: Postcode "', p, '" found more than one ocurrenc:')
            print(postcode_row)
        try:
            if postcode_row.iloc[0]['RP'] < MIN_ADDRESSES:
                forbidden_list.append(p)
        except Exception as e:
            print('WARNING: Could not double check postcode "', p, '" with the following error:')
            print(e)
            print('Row was:')
            print(postcode_row)

    else:
        print('WARNING: Postcode "', p, '" was not found.')


f = open('output_forbidden_postcodes.txt', 'w')
for p in forbidden_list:
    f.write(p + '\n')
f.close()
