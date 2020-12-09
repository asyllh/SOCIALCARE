import requests
server = r'localhost:5000'
coord1 = r'-1.813821,51.077959'
coord2 = r'-1.447691138804643,50.96380612421809'
str_call = 'http://' + server + '/route/v1/driving/' + coord1 + ';' + coord2 + '?overview=false'
r = requests.get(str_call)
d = r.json()

print(d)
print(d['routes'][0]['distance'], 'metres.')
print(d['routes'][0]['duration'], 'seconds.')

# 51.077959,-1.813821

# 50.96380612421809,-1.447691138804643

# 'http://localhost:5000/route/v1/driving/-1.813821,51.077959;-1.447691138804643,50.96380612421809?overview=false'