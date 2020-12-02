import pandas as pd
from math import acos, cos, sin, pi
import subprocess 
import time
import osrm_extend as oe

class geopoint(object):
    def __init__(self, latitude, longitude):
        self.lat = latitude
        self.long = longitude
    def latlong(self):
        return([self.lat, self.long])
    def longlat(self):
        return([self.long, self.lat])

def PI():
    return pi
def get_distance_fake(point1, point2):
    distanceScale = 100
    return distanceScale*sqrt((point1[0] - point2[0])**2 + (point1[1] - point2[1])**2)
def get_crow_flight_distance(point1, point2):
    # From https://stackoverflow.com/questions/13845152/calculate-as-the-crow-flies-distance-php
    lat1 = point1[0]
    lon1 = point1[1]
    lat2 = point2[0]
    lon2 = point2[1]
    distance = acos(
        cos(lat1 * (PI()/180)) *
        cos(lon1 * (PI()/180)) *
        cos(lat2 * (PI()/180)) *
        cos(lon2 * (PI()/180))
        +
        cos(lat1 * (PI()/180)) *
        sin(lon1 * (PI()/180)) *
        cos(lat2 * (PI()/180)) *
        sin(lon2 * (PI()/180))
        +
        sin(lat1 * (PI()/180)) *
        sin(lat2 * (PI()/180))
        ) * 6371
    return distance*1000 # In m.

def excel_sheet_to_df(excelFile, sheetName='Sheet1'):
    # name = excelFile.split('.')[0]

    # Open file and check quality:
    xl = pd.ExcelFile(excelFile)

    # Check the sheet names
    if sheetName not in xl.sheet_names:
        print('ERROR: The program expects one sheet from ' + excelFile +
         ' to be called "' + sheetName + '", instead we got:' + str(xl.sheet_names) + '.\nProgram terminated.')
        exit(-1)

    pdDataFrame = xl.parse(sheetName)
    return(pdDataFrame)
    # nJobs = dfJobs.shape[0] - 1
    # nSkills = 0

    # jobObjs = []
    # for ii in range(nJobs):
    #   jobObjs.append(JOB())

    # nurseObjs = []
    # for ii in range(nNurses):
    #   nurseObjs.append(NURSE())   

    # print('There are %d jobs and %d nurses in %s' % (nJobs, nNurses, excelFile))
    # ii = -1
    # for lineno,row in dfJobs.iterrows():
    #   # print(row)
    #   if lineno == 0:
    #       continue
    #   ii = ii + 1
    #   try:
    #       jobObjs[ii].ID = str(row[0])
    #   except Exception as e:
    #       ii = ii - 1
    #       print('Could not read line... It was:' + str(row))
    #       print(e)
    #       print(ii)
    #       print(jobObjs[ii])
    #       continue

class OSRM_SERVER(object):
    """docstring for osrm_server"""
    def __init__(self):
        baseDir = 'C:\\Users\\clf1u13\\Docs\\01.HHCRSP\\Code\\HHCRSP\\'
        # Start OSRM server...
        # print('Starting server for distances ' + matrixType + ' matrix...')
        # if matrixType == 'driving':
        self.demoServerHost = 'https://router.project-osrm.org/'
        self.carserver = subprocess.Popen(baseDir + 'runCarServer.bat', shell=False,stdout=subprocess.PIPE)

        self.carServerHost = 'http://localhost:5000/'
        self.walkServerHost = 'http://localhost:5001/'
            
        # self.carServerHost = 'uos-212244.clients.soton.ac.uk:5000'
        # self.walkServerHost = 'uos-212244.clients.soton.ac.uk:5001'
        # else:
        self.walkserver = subprocess.Popen(baseDir + 'runWalkServer.bat', shell=False,stdout=subprocess.PIPE)

        time.sleep(3) # Give it 3 secs to initialise
        # self.carPID = self.carserver.pid
        # self.walkPID = self.walkserver.pid
        # print('PID: ' + str(popenConnection.pid))
        # print('Done.')
    def switch_to_walkserver(self):
        oe.switch_server_to(self.walkServerHost)

    def switch_to_carserver(self):
        oe.switch_server_to(self.carServerHost)

    def switch_to_demo_driving(self):
        oe.switch_server_to(self.demoServerHost)
        
    def switch_to_demo_walking(self):
        oe.switch_server_to(self.demoServerHost)


    def get_dist_latlon(self, point1, point2):
        [dur, dist, routeList] = self.get_time_distance_steps_latlon(point1, point2)
        return dist

    def get_time_distance_steps_latlon(self, point1, point2):
        # Points need to be in [LAT, LONG] format
        point1 = [point1[1], point1[0]]
        point2 = [point2[1], point2[0]]        
        [routeList, dur, dist] = oe.route_these_points(point1, point2)
        return([dur, dist, routeList])

    def get_time_distance_steps_lonlat(self, point1, point2):
        # Points need to be in [LONG, LAT] format
        [routeList, dur, dist] = oe.route_these_points(point1, point2)
        return([dur, dist, routeList])
        
    def kill_server(self):
        # Close servers:
        # print('Killing server (' + matrixType + ' profile)...')
        self.carserver.kill()
        self.walkserver.kill()
        print('Done.')
        


def clusterColour(clusterNumber):
    # From: https://www.w3schools.com/tags/ref_colornames.asp
    clusterNumber = (clusterNumber % 146)
    col = '#000000'
    if clusterNumber == 0:
        col = '#000000' #   Black
    elif clusterNumber == 1:
        col = '#1E90FF' #   DodgerBlue
    elif clusterNumber == 2:
        col = '#FF8C00' #   DarkOrange
    elif clusterNumber == 3:
        col = '#00FFFF' #    Aqua
    elif clusterNumber == 4:
        col = '#0000FF' #   Blue
    elif clusterNumber == 5:
        col = '#006400' #   DarkGreen
    elif clusterNumber == 6:
        col = '#FFD700' #   Gold
    elif clusterNumber == 7:
        col = '#808000' #  Olive
    elif clusterNumber == 8:
        col = '#B22222' #    FireBrick
    elif clusterNumber == 9:
        col = '#FF1493' #    DeepPink
    elif clusterNumber == 10:
        col = '#8B008B' #   DarkMagenta
    elif clusterNumber == 11:
        col = '#8A2BE2' #   BlueViolet
    elif clusterNumber == 12:
        col = '#7FFF00' #   Chartreuse
    elif clusterNumber == 13:
        col = '#DEB887' #   BurlyWood
    elif clusterNumber == 14:
        col = '#5F9EA0' #   CadetBlue
    elif clusterNumber == 15:
        col = '#A52A2A' #   Brown
    elif clusterNumber == 16:
        col = '#D2691E' #   Chocolate
    elif clusterNumber == 17:
        col = '#FF7F50' #   Coral
    elif clusterNumber == 18:
        col = '#6495ED' #   CornflowerBlue
    elif clusterNumber == 19:
        col = '#FF69B4' #   HotPink
    elif clusterNumber == 20:
        col = '#DC143C' #   Crimson
    elif clusterNumber == 21:
        col = '#00FFFF' #   Cyan
    elif clusterNumber == 22:
        col = '#00008B' #   DarkBlue
    elif clusterNumber == 23:
        col = '#008B8B' #   DarkCyan
    elif clusterNumber == 24:
        col = '#B8860B' #   DarkGoldenRod
    elif clusterNumber == 25:
        col = '#A9A9A9' #   DarkGray
    elif clusterNumber == 26:
        col = '#A9A9A9' #   DarkGrey
    elif clusterNumber == 270:
        col = '#F0FFFF' #    Azure
    elif clusterNumber == 28:
        col = '#BDB76B' #   DarkKhaki
    elif clusterNumber == 290:
        col = '#F5F5DC' #    Beige
    elif clusterNumber == 30:
        col = '#556B2F' #   DarkOliveGreen
    elif clusterNumber == 31:
        col = '#FAEBD7' #    AntiqueWhite
    elif clusterNumber == 32:
        col = '#9932CC' #   DarkOrchid
    elif clusterNumber == 33:
        col = '#8B0000' #   DarkRed
    elif clusterNumber == 34:
        col = '#E9967A' #   DarkSalmon
    elif clusterNumber == 35:
        col = '#8FBC8F' #   DarkSeaGreen
    elif clusterNumber == 36:
        col = '#483D8B' #   DarkSlateBlue
    elif clusterNumber == 37:
        col = '#2F4F4F' #   DarkSlateGray
    elif clusterNumber == 38:
        col = '#2F4F4F' #   DarkSlateGrey
    elif clusterNumber == 39:
        col = '#00CED1' #   DarkTurquoise
    elif clusterNumber == 40:
        col = '#9400D3' #   DarkViolet
    elif clusterNumber == 41:
        col = '#FFEBCD' #   BlanchedAlmond
    elif clusterNumber == 42:
        col = '#00BFFF' #   DeepSkyBlue
    elif clusterNumber == 43:
        col = '#696969' #   DimGray
    elif clusterNumber == 44:
        col = '#696969' #   DimGrey
    elif clusterNumber == 45:
        col = '#FFFF00' #  Yellow
    elif clusterNumber == 46:
        col = '#F0F8FF' #    AliceBlue
    elif clusterNumber == 47:
        col = '#FFFAF0' #   FloralWhite
    elif clusterNumber == 48:
        col = '#228B22' #   ForestGreen
    elif clusterNumber == 49:
        col = '#FF00FF' #   Fuchsia
    elif clusterNumber == 50:
        col = '#DCDCDC' #   Gainsboro
    elif clusterNumber == 51:
        col = '#F8F8FF' #   GhostWhite
    elif clusterNumber == 52:
        col = '#7FFFD4' #    Aquamarine
    elif clusterNumber == 53:
        col = '#DAA520' #   GoldenRod
    elif clusterNumber == 54:
        col = '#808080' #   Gray
    elif clusterNumber == 55:
        col = '#808080' #   Grey
    elif clusterNumber == 56:
        col = '#008000' #   Green
    elif clusterNumber == 57:
        col = '#ADFF2F' #   GreenYellow
    elif clusterNumber == 58:
        col = '#F0FFF0' #   HoneyDew
    elif clusterNumber == 59:
        col = '#FFF8DC' #   Cornsilk
    elif clusterNumber == 60:
        col = '#CD5C5C' #   IndianRed
    elif clusterNumber == 61:
        col = '#4B0082' #   Indigo
    elif clusterNumber == 62:
        col = '#FFFFF0' #   Ivory
    elif clusterNumber == 63:
        col = '#F0E68C' #   Khaki
    elif clusterNumber == 64:
        col = '#E6E6FA' #   Lavender
    elif clusterNumber == 65:
        col = '#FFF0F5' #   LavenderBlush
    elif clusterNumber == 66:
        col = '#7CFC00' #   LawnGreen
    elif clusterNumber == 67:
        col = '#FFFACD' #   LemonChiffon
    elif clusterNumber == 68:
        col = '#ADD8E6' #   LightBlue
    elif clusterNumber == 69:
        col = '#F08080' #   LightCoral
    elif clusterNumber == 70:
        col = '#E0FFFF' #   LightCyan
    elif clusterNumber == 71:
        col = '#FAFAD2' #   LightGoldenRodYellow
    elif clusterNumber == 72:
        col = '#D3D3D3' #   LightGray
    elif clusterNumber == 73:
        col = '#D3D3D3' #   LightGrey
    elif clusterNumber == 74:
        col = '#90EE90' #   LightGreen
    elif clusterNumber == 75:
        col = '#FFB6C1' #   LightPink
    elif clusterNumber == 76:
        col = '#FFA07A' #   LightSalmon
    elif clusterNumber == 77:
        col = '#20B2AA' #   LightSeaGreen
    elif clusterNumber == 78:
        col = '#87CEFA' #   LightSkyBlue
    elif clusterNumber == 79:
        col = '#00FF7F' #    SpringGreen
    elif clusterNumber == 80:
        col = '#778899' #   LightSlateGrey
    elif clusterNumber == 81:
        col = '#B0C4DE' #   LightSteelBlue
    elif clusterNumber == 82:
        col = '#FFFFE0' #   LightYellow
    elif clusterNumber == 83:
        col = '#00FF00' #   Lime
    elif clusterNumber == 84:
        col = '#32CD32' #   LimeGreen
    elif clusterNumber == 85:
        col = '#FAF0E6' #   Linen
    elif clusterNumber == 86:
        col = '#FF00FF' #   Magenta
    elif clusterNumber == 87:
        col = '#800000' #   Maroon
    elif clusterNumber == 88:
        col = '#66CDAA' #   MediumAquaMarine
    elif clusterNumber == 89:
        col = '#0000CD' #   MediumBlue
    elif clusterNumber == 90:
        col = '#BA55D3' #   MediumOrchid
    elif clusterNumber == 91:
        col = '#9370DB' #   MediumPurple
    elif clusterNumber == 92:
        col = '#3CB371' #   MediumSeaGreen
    elif clusterNumber == 93:
        col = '#7B68EE' #   MediumSlateBlue
    elif clusterNumber == 94:
        col = '#00FA9A' #   MediumSpringGreen
    elif clusterNumber == 95:
        col = '#48D1CC' #   MediumTurquoise
    elif clusterNumber == 96:
        col = '#C71585' #   MediumVioletRed
    elif clusterNumber == 97:
        col = '#191970' #   MidnightBlue
    elif clusterNumber == 98:
        col = '#F5FFFA' #   MintCream
    elif clusterNumber == 99:
        col = '#FFE4E1' #   MistyRose
    elif clusterNumber == 100:
        col = '#FFE4B5' #  Moccasin
    elif clusterNumber == 101:
        col = '#FFDEAD' #  NavajoWhite
    elif clusterNumber == 102:
        col = '#000080' #  Navy
    elif clusterNumber == 103:
        col = '#FDF5E6' #  OldLace
    elif clusterNumber == 104:
        col = '#778899' #   LightSlateGray
    elif clusterNumber == 105:
        col = '#6B8E23' #  OliveDrab
    elif clusterNumber == 106:
        col = '#FFA500' #  Orange
    elif clusterNumber == 107:
        col = '#FF4500' #  OrangeRed
    elif clusterNumber == 108:
        col = '#DA70D6' #  Orchid
    elif clusterNumber == 109:
        col = '#EEE8AA' #  PaleGoldenRod
    elif clusterNumber == 110:
        col = '#98FB98' #  PaleGreen
    elif clusterNumber == 111:
        col = '#AFEEEE' #  PaleTurquoise
    elif clusterNumber == 112:
        col = '#DB7093' #  PaleVioletRed
    elif clusterNumber == 113:
        col = '#FFEFD5' #  PapayaWhip
    elif clusterNumber == 114:
        col = '#FFDAB9' #  PeachPuff
    elif clusterNumber == 115:
        col = '#CD853F' #  Peru
    elif clusterNumber == 116:
        col = '#FFC0CB' #  Pink
    elif clusterNumber == 117:
        col = '#DDA0DD' #  Plum
    elif clusterNumber == 118:
        col = '#B0E0E6' #  PowderBlue
    elif clusterNumber == 119:
        col = '#800080' #  Purple
    elif clusterNumber == 120:
        col = '#663399' #  RebeccaPurple
    elif clusterNumber == 121:
        col = '#FF0000' #  Red
    elif clusterNumber == 122:
        col = '#BC8F8F' #  RosyBrown
    elif clusterNumber == 123:
        col = '#4169E1' #  RoyalBlue
    elif clusterNumber == 124:
        col = '#8B4513' #  SaddleBrown
    elif clusterNumber == 125:
        col = '#FA8072' #  Salmon
    elif clusterNumber == 126:
        col = '#F4A460' #  SandyBrown
    elif clusterNumber == 127:
        col = '#2E8B57' #  SeaGreen
    elif clusterNumber == 128:
        col = '#FFF5EE' #  SeaShell
    elif clusterNumber == 129:
        col = '#A0522D' #  Sienna
    elif clusterNumber == 130:
        col = '#C0C0C0' #  Silver
    elif clusterNumber == 131:
        col = '#87CEEB' #  SkyBlue
    elif clusterNumber == 132:
        col = '#6A5ACD' #  SlateBlue
    elif clusterNumber == 133:
        col = '#708090' #  SlateGray
    elif clusterNumber == 134:
        col = '#708090' #  SlateGrey
    elif clusterNumber == 135:
        col = '#FFFAFA' #  Snow
    elif clusterNumber == 136:
        col = '#FFE4C4' #  Bisque
    elif clusterNumber == 137:
        col = '#4682B4' #  SteelBlue
    elif clusterNumber == 138:
        col = '#D2B48C' #  Tan
    elif clusterNumber == 139:
        col = '#008080' #  Teal
    elif clusterNumber == 140:
        col = '#D8BFD8' #  Thistle
    elif clusterNumber == 141:
        col = '#FF6347' #  Tomato
    elif clusterNumber == 142:
        col = '#40E0D0' #  Turquoise
    elif clusterNumber == 143:
        col = '#EE82EE' #  Violet
    elif clusterNumber == 144:
        col = '#F5DEB3' #  Wheat
    elif clusterNumber == 145:
        col = '#FFFFFF' #  White
    elif clusterNumber == 146:
        col = '#F5F5F5' #  WhiteSmoke
    return(col)