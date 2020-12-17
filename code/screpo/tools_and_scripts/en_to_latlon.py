# A bunch of Northings and Eastings from the en.csv in Codepoint-Open
# Corresponding to these postcodes (in order):

import pandas as pd
import geopandas
postcodes = ['SO52 9LP', 'EN1 1AA', 'EN1 1BA', 'EN1 1BB', 'EN1 1BE', 'EN1 1BG', 'EN1 1BJ', 'EN1 1BL', 'EN1 1BN']
eastings = [439024, 533124, 533716, 533827, 533875, 533837, 533794, 533693, 533642]
northings = [118424, 196542, 195439, 195494, 195672, 195730, 195497, 195464, 195556]
en_df = pd.DataFrame({'postcodes' : postcodes, 'eastings' : eastings, 'northings' : northings})
# print(type(en_df))
# print(en_df)
# exit(-1)

# Now we create a GeoDataFrame from it, with the geometry being northings and eastings:
gdf = geopandas.GeoDataFrame(en_df, geometry=geopandas.points_from_xy(en_df.eastings, en_df.northings))

# Our reference coordinate is: EPSG:7405 see: https://spatialreference.org/ref/epsg/7405/ 
gdf = gdf.set_crs("EPSG:7405") # Set our current reference coordinates (eastings and northings)

# Change it to standard lat-long
gdf = gdf.to_crs("EPSG:4326")

# Put the data back in our original en_df DataFrame (swap the rows around if we want lat then lon in the en_df)
en_df['lon'] = gdf['geometry'][:].x
en_df['lat'] = gdf['geometry'][:].y
print(en_df)