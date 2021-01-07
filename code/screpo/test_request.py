import requests
import pandas as pd
# server = r'localhost:5000'
# coord1 = r'-1.813821,51.077959'
# coord2 = r'-1.447691138804643,50.96380612421809'
# coord1 = r'-1.1265538656195921,51.25167783293762'
# coord2 = r'-1.0542636173681512,51.29435018372664'
# str_call = 'http://' + server + '/route/v1/driving/' + coord1 + ';' + coord2 + ';' + coord1 + '?overview=full&geometries=geojson'
str_call = 'http://localhost:5000/route/v1/driving/-1.0932713683349489,51.26244013391559;-1.1223542469709538,51.2517453438589;-1.1108495601022277,51.247216458376776;-1.1066389846881866,51.27818905207848;-1.1195947760811282,51.27919535669177;-1.0993976532712253,51.262109743031516;-1.0932713683349489,51.26244013391559?overview=full&geometries=geojson'
# str_call = 'http://' + server + '/table/v1/driving/' + coord1 + ';' + coord2
r = requests.get(str_call)
d = r.json()
# print(d)

# print(d)
dist = d['routes'][0]['distance']
dur = d['routes'][0]['duration']
# print(d['routes'][0]['legs'][0]['distance'])
firstlegdist = d['routes'][0]['legs'][0]['distance']
lastlegdist = d['routes'][0]['legs'][-1]['distance']
distance_jobs = dist - (firstlegdist + lastlegdist)
print(distance_jobs)
# print(dist)
# print(dur)
# # d['routes'] = pd.DataFrame(d['routes'][0])
# # print(type(d['routes'][0]['geometry']))
# # print(d['routes'])
# # print(len(d['routes']))
# a = d['routes'][0]['geometry']
# # print(a)
# print(len(a))
# # print(a['coordinates'])
# ac = d['routes'][0]['geometry']['coordinates']
# print(type(ac))
# print(len(ac))
# # exit(-1)
# # geo = d['routes'][0]['geometry']
# # geotemp = d['routes'][0]['geometry'].replace('LINESTRING (', '')
# # geotemp = geotemp.replace(')', '')
# # geotemp = geotemp.split(',')
# routeList = []
# for coordPair in ac:
#     # pList = coordPair.split(' ')
#     pList = coordPair
#     routeList.append([float(pList[1]), float(pList[0])])
# # print('dist: ', dist, ' dur: ', dur)
# # print(geo)
# print(routeList)
# print(len(routeList))
# # print(d['routes'][0]['distance'], 'metres.')
# # print(d['routes'][0]['duration'], 'seconds.')
# exit(-1)
# 51.077959,-1.813821

# 50.96380612421809,-1.447691138804643

# 'http://localhost:5000/route/v1/driving/-1.813821,51.077959;-1.447691138804643,50.96380612421809?overview=false'

# ll1:  [-1.1190442126942648, 51.281744953988415]
# ll2:  [-1.1259240333048044, 51.26676181184375]

# lon, lat = cpo_inst.find_postcode_lonlat('rg226ly') # -1.1265538656195921 51.25167783293762
# lon, lat = cpo_inst.find_postcode_lonlat('rg248ex') # -1.0542636173681512 51.29435018372664

# goingThrough = [[-1.1,6.6], [-2.2,7.7], [-3.3,8.8], [-4.4, 9.9], [-5.5, 10.1]]

# stringGoingThrough = ';'.join([','.join([str(i), str(j)]) for i, j in goingThrough])

# print('stringGoingThrough: ', stringGoingThrough)

# coord_one = [-1.1, 2.2]

# string_coord_one = ','.join([str(coord_one[0]), str(coord_one[1])])

# print(string_coord_one)
# print(type(string_coord_one))

# coord_two = [-8.8, 9.9]

# string_coord_two = ','.join([str(coord_two[0]), str(coord_two[1])])

# print(string_coord_two)
# print(type(string_coord_two))

# string_all = ';'.join([string_coord_one, stringGoingThrough, string_coord_two])
# print(string_all)
# print(type(string_all))