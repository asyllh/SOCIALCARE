import os
import glob
import pickle
import datetime
import numpy as np
import pandas as pd
import geopandas
import matplotlib.pyplot as plt
from scipy import stats

class CPO_DF(): # Codepoint Open DataFrame
    """
    docstring
    """
    # df = []
    def __init__(self, foldername=r'data\codepo_gb'):
        self.foldername = foldername # CP_Open_Folder = r'..\data\codepo_gb'
        # self.foldername = r'..\data\codepo_gb'
        all_cpopen_csvs = glob.glob(os.path.join(self.foldername, 'data\\CSV', '*.csv'))

        # Get the headers in a list:
        with open(os.path.join(self.foldername, 'Doc', 'Code-Point_Open_Column_Headers.csv')) as f:
            raw_headers = f.readline()
            cp_open_headers = raw_headers.split(',') # cp_open_headers:  ['PC', 'PQ', 'EA', 'NO', 'CY', 'RH', 'LH', 'CC', 'DC', 'WC\n'], it's a list of strings.
        
        # We are interested in the areas 'Wiltshire', 'Hampshire', and 'Monmouthshire', which we search with the codes below (county code 'CC', district code 'DC')
        utas = [['DC', 'E06000054'], ['CC', 'E10000014'], ['DC', 'W06000021']]
        # utas = [['DC', 'E06000054']]

        count = 0
        dict_dfs = {}
        for csvfile in all_cpopen_csvs:
            csvpart = pd.read_csv(csvfile, names=cp_open_headers) # Read csvfile with cp_open_headers as the column names.
            for place_type, place_code in utas:
                cp_rest = csvpart[csvpart[place_type] == place_code] # cp_rest is csvpart but only containing information for the given place_type and place_code (e.g. only contains info for rows where in E06000054 in 'DC' column)
                if cp_rest.empty:
                    continue
                else:
                    dict_dfs[count] = cp_rest
                    count += 1

        frames = []
        for i in range(len(dict_dfs)):
            frames.append(dict_dfs[i])
        self.df = pd.concat(frames)
        self.df = self.df.reset_index(drop=True) # Reset index to 0, remove the old index column.
        self.df['PC'] = self.df['PC'].str.replace(' ', '')
        self.df['PC'] = self.df['PC'].str.lower()

        gdf = geopandas.GeoDataFrame(self.df, geometry=geopandas.points_from_xy(self.df.EA, self.df.NO))
        
        # Our reference coordinate is: EPSG:7405 see: https://spatialreference.org/ref/epsg/7405/ 
        gdf = gdf.set_crs("EPSG:7405") # Set our current reference coordinates (eastings and northings)
        # Change it to standard lat-long
        gdf = gdf.to_crs("EPSG:4326")
        # Put the data back in our original df DataFrame (swap the rows around if we want lat then lon in the df)
        self.df['LON'] = gdf['geometry'][:].x
        self.df['LAT'] = gdf['geometry'][:].y
    # --- End of def __init__ --- #

    def find_postcode_index(self, postcode): # Return the index of the df (the row number) where the postcode is located. 'no match' means postcode is not in df.
        dfview = next(iter(self.df[self.df['PC'] == postcode].index), 'no match')
        return dfview
    # --- End of def find_postcode_index --- #
        
    def find_postcode_lonlat(self, postcode): # Returns Eastings (EA) and Northings (NO) of given postcode (PC).
        dfview = self.df[self.df['PC'] == postcode]
        if dfview.empty:
            return [None, None]
        else:
            # return [dfview.iloc[0]['EA'], dfview.iloc[0]['NO']]
            return [dfview.iloc[0]['LON'], dfview.iloc[0]['LAT']]
    # --- End of def find_postcode_lonlat --- #
    
    # def find_postcode_n_addresses(self, postcode): # Returns number of registered properties at a given postcode.
    #     dfview = self.adr_df[self.adr_df['post'] == postcode]
    #     if dfview.empty:
    #         return -1
    #     else:
    #         return dfview.iloc[0]['RP']
    ## --- End of def find_postcode_n_addresses --- #
### --- End of class CPO_DF --- ###

# cpodfinst = CPO_DF()

# coord = cpodfinst.find_postcode_lonlat('so529lp')
# print(coord)
# index = cpodfinst.find_postcode_index('so529lp')
# print(index)
