# Utils for working with OSRM
import osrm
# import cPickle as pickle
import pickle

def get_sol(filename):
	ad = pickle.load(open(filename, 'rb'))
	print('Solution: ' + filename)
	print('Delivers: ' + str(ad.nParcels) + ' parcels.')
	print('To ' + str(ad.nConsignees) + ' different consignees.')
	print('Total Weight: ' + str(ad.totalWeight) + ' kg.')
	print('Total Volume: ' + str(ad.totalVolume) + ' L.\n\n')

def switch_server_to(thisServer):
	osrm.RequestConfig.host = thisServer

def route_these_points(p1, p2, goingThrough=None):
	# print('Using server: ' + str(osrm.RequestConfig.host))
	# Returns a list of coordinates with the route between the points, duration and distance
	# [routeList, duration, distance]

	# We give longlats
	# node1 = osrm.Point(latitude=p1[1], longitude=p1[0])
	# node2 = osrm.Point(latitude=p2[1], longitude=p2[0])

	# result = osrm.simple_route(node1, node2)
	# print(result)

	rout = osrm.simple_route(p1, p2, output='route', overview="full", geometry='wkt', coord_intermediate=goingThrough)
	# print(rout)
	rTemp = rout[0]['geometry'].replace("LINESTRING (", "")
	rTemp = rTemp.replace(")", "")
	rTemp = rTemp.split(",")
	routeList = []
	for coordPair in rTemp:
		pList = coordPair.split(" ")
		routeList.append([float(pList[1]), float(pList[0])])

	dist = rout[0]['distance']
	dur = rout[0]['duration']

	return [routeList, dur, dist]

def point_list_to_leaflet_polyline(pointList, varname, col='red', mapvar='mymap'):
	output = ''
	polylineName = 'pl_' + varname
	output = output + '\nvar ' + varname + ' = [\n'
	for i,node in enumerate(pointList):
		if i >= len(pointList) - 1:
			output = output + "[%f, %f]\n" % (node[0], node[1])
		else:
			output = output + "[%f, %f],\n" % (node[0], node[1])

	output = output + '];\n'
	output = output + 'var ' + polylineName + ' = L.polyline(' + varname 
	output = output + ', {color: \'' + col + '\', opacity: 0.5}).addTo(' + mapvar + ');\n'
	return output

def point_list_to_gpx(pointList, style='track'):
	# Partly from pyroute

	# Point list is in Long - Lat

	description = 'ftc'
	# style = 'track' or 'route'
	output = ''
	output = output + "<?xml version='1.0'?>\n";
	
	output = output + "<gpx version='1.1' creator='automatic' xmlns='http://www.topografix.com/GPX/1/1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:schemaLocation='http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd'>\n"
		
	if(style == 'track'):
		output = output + " <trk>\n"
		output = output + "	<name>%s</name>\n" % description
		output = output + "	<trkseg>\n"
		count = 0;
		for node in pointList:
			output = output + "	 <trkpt lat='%f' lon='%f'>\n" % ( \
				node[0],
				node[1])
			output = output + "	 </trkpt>\n"
			count = count + 1
		output = output + "	</trkseg>\n	</trk>\n</gpx>\n"

	elif(style == 'route'):
		output = output + " <rte>\n"
		output = output + "	<name>%s</name>\n" % description
		
		count = 0;
		for node in pointList:
			output = output + "	 <rtept lat='%f' lon='%f'>\n" % ( \
				node[0],
				node[1])
			output = output + "		<name>%d</name>\n" % count
			output = output + "	 </rtept>\n"
			count = count + 1
		output = output + " </rte>\n</gpx>\n"
	
	return(output)

def read_data(filename):
	print('WARNING: Information is hardcoded!')
	longlats = [
		[-0.0959901, 51.5036133],
		[-0.1010152, 51.5019757],
		[-0.1027236, 51.5022542],
		[-0.1033619, 51.5018751],
		[-0.0989531, 51.501136],
		[-0.0997711, 51.5029403],
		[-0.0967363, 51.5031382],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.1011336, 51.5004285],
		[-0.096829, 51.504885],
		[-0.0997477, 51.5055323],
		[-0.0997477, 51.5055323],
		[-0.1027729, 51.5062882],
		[-0.0992189, 51.5007313],
		[-0.0992189, 51.5007313],
		[-0.1018024, 51.5037113],
		[-0.0963154, 51.5043999],
		[-0.0963154, 51.5043999],
		[-0.0985502, 51.5045618],
		[-0.100024, 51.505149],
		[-0.100216, 51.504796],
		[-0.1010036, 51.5007925],
		[-0.101577, 51.5013303],
		[-0.101362, 51.5001409],
		[-0.1025556, 51.5058694],
		[-0.1029305, 51.505127],
		[-0.1036361, 51.5047179],
		[-0.102583, 51.503205],
		[-0.0960342, 51.5034772],
		[-0.0996685, 51.502877],
		[-0.098287, 51.5030345],
		[-0.0970693, 51.5045858],
		[-0.1017325, 51.5013843],
		[-0.1027035, 51.5066003],
		[-0.1027236, 51.5022542],
		[-0.1027236, 51.5022542],
		[-0.1033619, 51.5018751],
		[-0.1033619, 51.5018751],
		[-0.0997477, 51.5055323],
		[-0.0992189, 51.5007313],
		[-0.0992189, 51.5007313],
		[-0.0963154, 51.5043999],
		[-0.0963154, 51.5043999],
		[-0.0985502, 51.5045618],
		[-0.100024, 51.505149],
		[-0.100024, 51.505149],
		[-0.1025556, 51.5058694],
		[-0.1025556, 51.5058694],
		[-0.1036361, 51.5047179],
		[-0.1036361, 51.5047179],
		[-0.1020862, 51.498224],
		[-0.1020862, 51.498224],
		[-0.1006747, 51.4995052],
		[-0.1026206, 51.499216],
		[-0.1003479, 51.4992685],
		[-0.1003479, 51.4992685],
		[-0.0972657, 51.4992547],
		[-0.0972657, 51.4992547],
		[-0.1052038, 51.4990149],
		[-0.1052038, 51.4990149],
		[-0.0960227, 51.5035259],
		[-0.0965077, 51.5035968],
		[-0.0985533, 51.5011675],
		[-0.1009932, 51.5029652],
		[-0.099348, 51.5004668],
		[-0.1000904, 51.5005189],
		[-0.1000904, 51.5005189],
		[-0.1035171, 51.4992882],
		[-0.1035171, 51.4992882],
		[-0.0998992, 51.5034974],
		[-0.0974842, 51.5051949],
		[-0.0975718, 51.505038],
		[-0.0975726, 51.5050443],
		[-0.0977079, 51.505071],
		[-0.097066, 51.504629],
		[-0.097066, 51.504629],
		[-0.0973549, 51.5043618],
		[-0.0981187, 51.5037975],
		[-0.1003153, 51.5037929],
		[-0.098593, 51.503647],
		[-0.1010562, 51.5036482],
		[-0.0962785, 51.5044433],
		[-0.0962785, 51.5044433],
		[-0.0962785, 51.5044433],
		[-0.1013195, 51.5047523],
		[-0.1012406, 51.504882],
		[-0.0971372, 51.5039781],
		[-0.0997958, 51.5051792],
		[-0.1042178, 51.5029303],
		[-0.1025893, 51.5027063],
		# [-0.1021889, 51.5016564],
		# [-0.1024071, 51.5018113],
		# [-0.1011353, 51.5009212],
		# [-0.1012943, 51.5008925],
		# [-0.1030846, 51.5005705],
		# [-0.1024855, 51.4999141],
		# [-0.1016352, 51.4995658],
		# [-0.1015541, 51.5006278],
		# [-0.1017673, 51.5046329],
		# [-0.1014167, 51.5044369],
		# [-0.102853, 51.504914],
		# [-0.1021803, 51.5046882],
		# [-0.1031335, 51.5059129],
		# [-0.1031166, 51.5051229],
		# [-0.103344, 51.5046173],
		# [-0.1040966, 51.5066188],
		# [-0.1040966, 51.5066188],
		# [-0.1040966, 51.5066188],
		# [-0.1040803, 51.5065223],
		# [-0.1040497, 51.5058029]
		]
	ids = ['P1_SE10AS',
		'P2_SE10BE',
		'P3_SE10BH',
		'P4_SE10BW',
		'P5_SE10BX',
		'P6_SE10EH',
		'P7_SE10ES',
		'P8_SE10FN',
		'P9_SE10FN',
		'P10_SE10FN',
		'P11_SE10FP',
		'P12_SE10FP',
		'P13_SE10FP',
		'P14_SE10FQ',
		'P15_SE10FQ',
		'P16_SE10GB',
		'P17_SE10HL',
		'P18_SE10HX',
		'P19_SE10HX',
		'P20_SE10JF',
		'P21_SE10JN',
		'P22_SE10JP',
		'P23_SE10LX',
		'P24_SE10NE',
		'P25_SE10NE',
		'P26_SE10NR',
		'P27_SE10NZ',
		'P28_SE10NZ',
		'P29_SE10QW',
		'P30_SE10RB',
		'P31_SE10SE',
		'P32_SE10UJ',
		'P33_SE10UX',
		'P34_SE10XD',
		'P35_SE10LR',
		'P36_SE10AS',
		'P37_SE10EH',
		'P38_SE10EN',
		'P39_SE10HS',
		'P40_SE10RB',
		'P41_SE10UP',
		'P42_SE10BH',
		'P43_SE10BH',
		'P44_SE10BW',
		'P45_SE10BW',
		'P46_SE10HX',
		'P47_SE10JN',
		'P48_SE10JP',
		'P49_SE10NE',
		'P50_SE10NE',
		'P51_SE10NR',
		'P52_SE10NZ',
		'P53_SE10NZ',
		'P54_SE10UJ',
		'P55_SE10UJ',
		'P56_SE10XD',
		'P57_SE10XD',
		'P58_SE10AA',
		'P59_SE10AA',
		'P60_SE10AD',
		'P61_SE10AG',
		'P62_SE10AJ',
		'P63_SE10AJ',
		'P64_SE10AJ',
		'P65_SE10AJ',
		'P66_SE10AP',
		'P67_SE10AP',
		'P68_SE10AT',
		'P69_SE10AT',
		'P70_SE10BQ',
		'P71_SE10DB',
		'P72_SE10DG',
		'P73_SE10DQ',
		'P74_SE10DQ',
		'P75_SE10FH',
		'P76_SE10FH',
		'P77_SE10HE',
		'P78_SE10HR',
		'P79_SE10HR',
		'P80_SE10HR',
		'P81_SE10HR',
		'P82_SE10HS',
		'P83_SE10HS',
		'P84_SE10HT',
		'P85_SE10LF',
		'P86_SE10LH',
		'P87_SE10LL',
		'P88_SE10LN',
		'P89_SE10NE',
		'P90_SE10NE',
		'P91_SE10NE',
		'P92_SE10NS',
		'P93_SE10NS',
		'P94_SE10NW',
		'P95_SE10NZ',
		'P96_SE10PY',
		'P97_SE10PZ',
		# 'P98_SE10RB',
		# 'P99_SE10RB',
		# 'P100_SE10RB',
		# 'P101_SE10RB',
		# 'P102_SE10RJ',
		# 'P103_SE10RW',
		# 'P104_SE10SA',
		# 'P105_SE10SE',
		# 'P106_SE10UE',
		# 'P107_SE10UN',
		# 'P108_SE10UQ',
		# 'P109_SE10UQ',
		# 'P110_SE10UR',
		# 'P111_SE10UX',
		# 'P112_SE10XE',
		# 'P113_SE18NW',
		# 'P114_SE18NW',
		# 'P115_SE18NW',
		# 'P116_SE18NW',
		# 'P117_SE18PJ'
		]
	return [longlats, ids]







def read_data_20(filename):
	print('WARNING: Information is hardcoded!')
	longlats = [[-0.0959901, 51.5036133],
		[-0.1010152, 51.5019757],
		[-0.1027236, 51.5022542],
		[-0.1033619, 51.5018751],
		[-0.0989531, 51.501136],
		[-0.0997711, 51.5029403],
		[-0.0967363, 51.5031382],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.0997915, 51.5021403],
		[-0.1011336, 51.5004285],
		[-0.096829, 51.504885],
		[-0.0997477, 51.5055323],
		[-0.0997477, 51.5055323],
		[-0.1027729, 51.5062882]]
	ids = ['P1_SE10AS',
		'P2_SE10BE',
		'P3_SE10BH',
		'P4_SE10BW',
		'P5_SE10BX',
		'P6_SE10EH',
		'P7_SE10ES',
		'P8_SE10FN',
		'P9_SE10FN',
		'P10_SE10FN',
		'P11_SE10FP',
		'P12_SE10FP',
		'P13_SE10FP',
		'P14_SE10FQ',
		'P15_SE10FQ',
		'P16_SE10GB',
		'P17_SE10HL',
		'P18_SE10HX',
		'P19_SE10HX',
		'P20_SE10JF']
	return [longlats, ids]