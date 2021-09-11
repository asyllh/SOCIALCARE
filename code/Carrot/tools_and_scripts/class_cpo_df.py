#--------------------#
# CARROT - CARe ROuting Tool
# class_cpo_df.py
# class CPO_DF(): CodepointOpen Dataframe class
# This class opens all csv files in codepo_gb folder, and goes through all files to find postcodes for the given CC/DC codes.
# These dataframes are put into a dictionary, which are then put into a list and merged together into a single dataframe, df.
# As codepoint open only contains eastings and northings, geopandas is used to convert these to longitudes and latitudes, which are added as columns to df.
# This class is used to obtain the longitudes and latitudes of any postcode in the given CC/DC areas, directly from codepoint open rather than using postcodelookup and fullcodepoint files.
# Note: codepoint open does not contain the number of addresses per postcode (i.e. 'RP' column).
# Used in convert_dict_inst.pu and compre_abicare_osrm.py.
# 16/12/2020
#--------------------#

import os
import glob
import numpy as np
import pandas as pd
import geopandas

class CPO_DF(): # Codepoint Open DataFrame
    def __init__(self, foldername):
    # def __init__(self, foldername=r'C:\Users\ah4c20\Asyl\PostDoc\SOCIALCARE\code\screpo\data\codepo_gb'):
    # def __init__(self, foldername=r'data\codepo_gb'):
    # def __init__(self, foldername=r'..\data\codepo_gb'):
        self.foldername = foldername # CP_Open_Folder = r'..\data\codepo_gb'
        all_cpopen_csvs = glob.glob(os.path.join(self.foldername, 'data\\CSV', '*.csv'))

        # Get the headers in a list:
        with open(os.path.join(self.foldername, 'Doc', 'Code-Point_Open_Column_Headers.csv')) as f:
            raw_headers = f.readline()
            cp_open_headers = raw_headers.split(',') # cp_open_headers:  ['PC', 'PQ', 'EA', 'NO', 'CY', 'RH', 'LH', 'CC', 'DC', 'WC\n'], it's a list of strings.
        
        # We are interested in the areas 'Wiltshire', 'Hampshire', 'Berkshire/Buckinghamshire', and 'Monmouthshire', which we search with the codes below (CC county code, DC district code).
        # Wiltshire: DC E06000054, Hampshire: CC E10000014, Berkshire/Buckinghamshire: DC E06000060, Monmouthshire: DC W06000019, W06000020 and W06000021, W06000022
        utas = [['CC', 'E10000030'], 
                ['CC', 'E10000032'], 
                ['DC', 'E06000037'],
                ['DC', 'E06000045'], 
                ['DC', 'E06000054'], 
                ['CC', 'E10000014'], 
                ['DC', 'E07000092'], 
                ['DC', 'E06000060'], 
                ['DC', 'W06000019'], 
                ['DC', 'W06000020'], 
                ['DC', 'W06000021'], 
                ['DC', 'W06000022']]

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

        # Now we convert eastings/northings to longitude/latitude and add these new columns to the datafram df. Our reference coordinate is EPSG:7405: https://spatialreference.org/ref/epsg/7405/ 
        gdf = geopandas.GeoDataFrame(self.df, geometry=geopandas.points_from_xy(self.df.EA, self.df.NO)) 

        # TO DO: Make this a try/except statement        
        gdf.crs = {"init":"epsg:7405"}
        # gdf = gdf.set_crs("EPSG:7405") # Set our current reference coordinates (eastings and northings)

        gdf = gdf.to_crs("EPSG:4326") # Change it to standard lat-long

        # Put the data back in our original df DataFrame (swap the rows around if we want lat then lon in the df)
        self.df['LON'] = gdf['geometry'][:].x
        self.df['LAT'] = gdf['geometry'][:].y
        # print(self.df.head())
    
    # --- End of def __init__ --- #

    def find_postcode_index(self, postcode): # Return the index of the df (the row number) where the postcode is located. 'no match' means postcode is not in df.
        dfview = next(iter(self.df[self.df['PC'] == postcode].index), 'no match')
        return dfview
    # --- End of def find_postcode_index --- #

    def find_postcode_latlon(self, postcode): # Returns Latitude and Longitude of given postcode (PC)
        dfview = self.df[self.df['PC'] == postcode]
        if dfview.empty:
            return [None, None]
        else:
            return [dfview.iloc[0]['LAT'], dfview.iloc[0]['LON']]
        
    def find_postcode_lonlat(self, postcode): # Returns Longitude and Latitude of given postcode (PC).
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
