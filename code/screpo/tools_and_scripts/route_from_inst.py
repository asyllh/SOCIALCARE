import pickle
import folium
import numpy as np
# import osrm

# def route_these_points(p1, p2, goingThrough=None):
#     # Returns a list of coordinates with the route between the points, duration and distance
#     # [routeList, duration, distance]

#     # We give longlats
#     # node1 = osrm.Point(latitude=p1[1], longitude=p1[0])
#     # node2 = osrm.Point(latitude=p2[1], longitude=p2[0])

#     # result = osrm.simple_route(node1, node2)
#     # print(result)

#     rout = osrm.simple_route(p1, p2, output='route', overview="full", geometry='wkt', coord_intermediate=goingThrough)
#     # print(rout)
#     # print('From ' + str(p1) + ' to ' + str(p2) + ' and through ' + str(goingThrough) + '\nRoute:')
#     # print(rout)
#     rTemp = rout[0]['geometry'].replace("LINESTRING (", "")
#     rTemp = rTemp.replace(")", "")
#     rTemp = rTemp.split(",")
#     routeList = []
#     for coordPair in rTemp:
#         pList = coordPair.split(" ")
#         routeList.append([float(pList[1]), float(pList[0])])

#     dist = rout[0]['distance']
#     dur = rout[0]['duration']
#     # print('Distance ' + str(dist) + ', duration ' + str(dur))
#     return [routeList, dur, dist]



all_instances = pickle.load(open('all_inst_salisbury.p', 'rb'))

print(len(all_instances))

# inst = {
#         'name' : a_name + '_' + day.replace('-', '_'),
#         'area' : a_name,
#         'date' : day,
#         'stats' : {'ncarers' : 0, 'ntasks' : 0, 'ttraveltime' : 0, 'ttravelmiles' : 0, 'tservicetime' : 0, 'tgaptime' : 0},
#         'txtsummary' : '',
#         'rota' : {'carer' : [], 'postcode' : [], 'num_addresses' : [], 'lat': [], 'lon' : [], 'start' : [], 'finish' : []},
#         'tasks' : {'client' : [], 'postcode': [], 'num_addresses': [], 'lat': [], 'lon' : [], 'duration' : [], 'tw_start' : [], 'tw_end' : []},
#         'routes' : []
#         }

for inst in all_instances:

    webFilename = inst['name'] + '.html'
    centre = [inst['rota']['lat'].mean(), inst['rota']['lon'].mean()]
    m = folium.Map(location=centre, zoom_start=10, tiles='cartodbpositron')
    foliumRouteLayers = []


    # Add tasks:
    border_colour = '#000000'
    circle_colour = '#FF0000'
    foliumRouteLayers.append(folium.map.FeatureGroup(name='Clients'))
    for index, client in inst['tasks'].iterrows():
        loc = [client['lat'], client['lon']]
        client_colour = circle_colour
        if np.isnan(client['lat']):
            client_colour = '#990000'
            loc = centre
        foliumRouteLayers[-1].add_child(folium.Circle(loc,
                    radius=200,
                    popup=client['client'] + '/' + client['postcode'] + '/' + str(client['num_addresses']),
                    color=border_colour,
                    fill_color=circle_colour,
                    fill_opacity=0.5,
                    fill=True,
                    ))

    m.add_child(foliumRouteLayers[-1])
    # Add carers:
    circle_colour = '#0000FF'
    foliumRouteLayers.append(folium.map.FeatureGroup(name='Carers'))
    for index, carer in inst['rota'].iterrows():
        loc = [carer['lat'], carer['lon']]
        carer_colour = circle_colour
        if np.isnan(carer['lat']):
            carer_colour = '#990000'
            loc = centre
        foliumRouteLayers[-1].add_child(folium.Circle(loc,
                    radius=250,
                    popup=carer['carer'] + '/' + carer['postcode'] + '/' + str(carer['num_addresses']),
                    color=border_colour,
                    fill_color=circle_colour,
                    fill_opacity=0.5,
                    fill=True,
                    ))
    m.add_child(foliumRouteLayers[-1])    

    # Add routes:
    print(inst['routes'])
    print(len(inst['routes']))
    print('-----------')
    print(inst['routes'][0])
    print(len(inst['routes'][0]))

    for i, r in enumerate(inst['routes']):
        if len(r) < 1:
            continue
        foliumRouteLayers.append(folium.map.FeatureGroup(name='Route carer ' + str(i)))
        # routeList, dur, dist = route_these_points(r[0],
        #                                     r[-1],
        #                                     goingThrough=r[1:-2])

        # print('Nurse ' + str(nursej) + ' route: ')
        # print(routeList)
        # print('Provided was: ')
        # print(nRoute)
        foliumRouteLayers[-1].add_child(folium.PolyLine(r, color='#000000', weight=2.5, opacity=1))
        m.add_child(foliumRouteLayers[-1])
    
    
    
    m.add_child(folium.map.LayerControl())
    m.save(webFilename)

    f = open(webFilename.replace('html', 'txt'), 'w')
    f.write(inst['txtsummary'])
    f.close()
