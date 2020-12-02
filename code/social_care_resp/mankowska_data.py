import numpy as np
import instance_handler as hhc
import scipy.io
import math
import copy
import imp

def ait_h_to_c(filename, i):
    # i = hhc.INSTANCE()

    ldi = imp.load_source('ait_h', filename)


    
    # execfile(filename)
    # ldi = imp.load_source('module.name', '/path/to/file.py')
    # f = open(filename)
    # bb = f.read()
    # f.close()
    # print('About to execute\n' + bb)
    # eval(bb)
    # print(NbClients)
    # print(dir())
    # exit(-234235)
    # Need to set all this:
    # nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l

    # nbNodes = NbClients + 2
    # nbVehi = NbVehicules
    # nbServi = NbServices
    # r = Demandes
    # DS
    # a = Offres
    # x = range(NbClients)
    # y = range(NbClients)
    # # OD matrix:
    # rowSize = NbClients - 1

    # d = np.zeros(NbClients + 1)
    # for i in range(NbClients + 1):
    # 	dlRow = depl[rowSize*i:rowSize*(i + 1)]
    # 	map_i = i  - 1
    # 	for j in range (NbClients + 1):
    # 		if i == j:
    # 			continue
    # p = DureeDeVisite
    # mind
    # maxd
    # e
    # l
    # return([nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l])



    i.nJobs = ldi.NbClients


    i.nNurses = ldi.NbVehicules
    i.nSkills = ldi.NbServices

    i.nurseWorkingTimes = np.zeros((i.nNurses, 3), dtype=np.int)
    for ii in range(i.nNurses):
        i.nurseWorkingTimes[ii][0] = ldi.WindowsD[ii][0]
        i.nurseWorkingTimes[ii][1] = ldi.WindowsD[ii][1] - ldi.WindowsD[ii][0]
        i.nurseWorkingTimes[ii][2] = ldi.WindowsD[ii][1]

    i.nurseSkills = np.ascontiguousarray(ldi.Offres, dtype=np.int)


    i.jobTimeInfo = np.zeros((i.nJobs, 3), dtype=np.int)
    i.jobSkillsRequired = np.ascontiguousarray(ldi.Demandes, dtype=np.int)

    # print(ldi.WindowsC)
    # print(len(ldi.WindowsC))
    # print(i.nJobs)
    for job in range(i.nJobs):
        i.jobTimeInfo[job][0] = ldi.WindowsC[job][0]
        i.jobTimeInfo[job][1] = ldi.WindowsC[job][1]
        for sk in range(ldi.NbServices):
            if (i.jobSkillsRequired[job][sk] > 0):
                i.jobTimeInfo[job][2] = ldi.DureeDeVisite[job][sk]

    i.doubleService = np.zeros(i.nJobs, dtype=np.int)
    i.mk_mind = -1*np.ones(i.nJobs)
    i.mk_maxd = -1*np.ones(i.nJobs)

    for job,gapVal in enumerate(ldi.gap):
        if gapVal == 0:
            i.doubleService[job] = 1
        elif gapVal > 0:
            i.mk_mind[job] = gapVal
            i.mk_maxd[job] = gapVal

    # print(i.mk_mind)
    # To be filled later!
    i.dependsOn = -1*np.ones(i.nJobs, dtype=np.int)



    d = []
    normalRow = ldi.NbClients - 1
    extRow = ldi.NbClients
    finalRow = ldi.depl[-1*extRow:]
    # print(ldi.depl)
    # print(finalRow)
    for ii in range(ldi.NbClients + 1):
        if ii == 0:
            map_i = ldi.NbClients
            rowSize = extRow
        else:
            map_i = ii - 1
            rowSize = normalRow
        stLim = rowSize*map_i
        endLim = rowSize*(map_i + 1)
        dlRow = copy.copy(ldi.depl[stLim:endLim])

        if ii != 0:
            # print('Inserting ' + str(map_i) + ' / ' + str(len(finalRow)))
            dlRow.insert(0,finalRow[map_i])
            dlRow.insert(map_i + 1, 0)
        else:
            dlRow.insert(0, 0)

        d.append(copy.copy(dlRow))

    i.od = np.ascontiguousarray(d, dtype=np.float64)


    i.prefScore = np.ascontiguousarray(ldi.CoutPref, dtype=np.float64)

    # DUPLICATE SOME JOBS
    for job,jtd in enumerate(i.mk_mind):
        if jtd < 0:
            continue

        if (i.verbose > 5):
            print('Duplicating ' + str(job) + '...')

        i.nJobs += 1
        i.dependsOn[job] = len(i.dependsOn)
        i.dependsOn = np.append(i.dependsOn, job)
        i.doubleService = np.append(i.doubleService, 0)
        
        toAdd = np.expand_dims(i.jobTimeInfo[job,:], axis=0)
        i.jobTimeInfo = np.concatenate((i.jobTimeInfo,toAdd),0)

        skillToAdd = np.expand_dims(i.jobSkillsRequired[job,:], axis=0)
        i.jobSkillsRequired = np.concatenate((i.jobSkillsRequired,skillToAdd),0)
        i.jobSkillsRequired[job][1] = 0
        i.jobSkillsRequired[-1][0] = 0

        # i.mk_mind[job] *= -1
        # i.mk_maxd[job] *= -1

        i.mk_mind = np.append(i.mk_mind, i.mk_mind[job])
        i.mk_maxd = np.append(i.mk_maxd, i.mk_maxd[job])

        # Update OD matrix to include new point:
        i.od = np.concatenate((i.od,np.expand_dims(i.od[job+1,:], axis=0)),0)
        i.od = np.concatenate((i.od,np.expand_dims(i.od[:,job+1], axis=1)),1)


        prefToAdd = np.expand_dims(i.prefScore[job,:], axis=0)
        i.prefScore = np.concatenate((i.prefScore,prefToAdd),0)
        if (i.verbose > 5):
            print('Done.')

    # Duplicate preferences of the job!!!!!

    i.mk_mind = np.insert(i.mk_mind, 0, 0)
    i.mk_maxd = np.insert(i.mk_maxd, 0, 0)

    i.solMatrix = np.zeros((i.nNurses, i.nJobs), dtype=np.int)

    i.secondsPerTU = 60

    bestPref = 0
    for row in range(ldi.NbClients):
        rowScore = 1000000000000
        bestCol = -1
        for col in range(ldi.NbVehicules):
            if (i.doubleService[row] < 1) and ((i.jobSkillsRequired[row,0] > i.nurseSkills[col,0]) or
                (i.jobSkillsRequired[row,1] > i.nurseSkills[col,1])):
                # print('Rejected that nurse ' + str(col) + ' could do job ' + str(row))
                # print('\tRequired: ' + str(i.jobSkillsRequired[row,:]))
                # print('\tAvailable: ' + str(i.nurseSkills[col,:]))
                continue

            if (i.prefScore[row,col] < rowScore):
                rowScore = i.prefScore[row,col]
                bestCol = col

        bestPref += rowScore
        # print('For job: ' +str(row) + ' the best assignment is nurse ' + str(bestCol) + ' value: ' + str(rowScore))
    if (i.verbose > 5):
        print('For instance ' + str(filename) + ' the minimum pref score is: ' + str(bestPref) + '\n')

    # exit(-24235345)

    # Ait H has no support for heterogeneous depots,
    # so we get travel info and replicate it:
    i.nurse_travel_from_depot = np.empty((i.nNurses, i.nJobs), dtype=np.float64)
    i.nurse_travel_to_depot = np.empty((i.nNurses, i.nJobs), dtype=np.float64)
    for nurse in range(i.nNurses):
        i.nurse_travel_from_depot[nurse,:] = i.od[0,1:]
        i.nurse_travel_to_depot[nurse,:] = i.od[1:,0]

    i.nurse_travel_from_depot = np.ascontiguousarray(i.nurse_travel_from_depot)
    i.nurse_travel_to_depot = np.ascontiguousarray(i.nurse_travel_to_depot)

    i.init_job_and_nurse_objects()

    return i

def read_from_tsptw(tsptwfile='none'):
    import tsp_instance_reader
    [nbNodes, d, e, l] = tsp_instance_reader.mkformat_tsptwfile(tsptwfile)
    nbVehi = 1
    nbServi = 1
    mind = np.zeros(nbNodes)
    x = np.zeros(nbNodes)
    y = np.zeros(nbNodes)
    # x = [40.00, 25.00, 22.00, 22.00, 20.00, 20.00, 18.00, 15.00, 15.00, 10.00, 10.00, 8.00, 8.00, 5.00, 5.00, 2.00, 0.00, 0.00, 44.00, 42.00]
    # y = [50.00, 85.00, 75.00, 85.00, 80.00, 85.00, 75.00, 75.00, 80.00, 35.00, 40.00, 40.00, 45.00, 35.00, 45.00, 40.00, 40.00, 45.00, 5.00, 10.00]
    maxd = np.ones(nbNodes)*100000000
    DS = []
    p = np.zeros((nbNodes, nbVehi, nbServi))
    r = np.zeros((nbNodes, nbServi))
    a = np.ones((nbNodes, nbServi))
    return([nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l])

def read_from_tsp(tspfile='none'):
    if tspfile == 'none':
        import tsp_instance_reader
        x,y,d = tsp_instance_reader.mkformat_tsp()
    
    nbNodes = len(x)
    nbVehi = 1
    nbServi = 1
    mind = np.zeros(nbNodes)
    maxd = np.ones(nbNodes)*100000000
    DS = []
    e = maxd = np.ones(nbNodes)*-100000000
    l = maxd = np.ones(nbNodes)*100000000
    p = np.zeros((nbNodes, nbVehi, nbServi))
    r = np.zeros((nbNodes, nbServi))
    a = np.ones((nbNodes, nbServi))

    return([nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l])
def read_vector_line(f, dtype_par=np.int):
        var_str = f.readline()
        var_str = var_str.strip()
        return np.array(var_str.split(' '), dtype=dtype_par)

def euclidean_matrices(x, y):
    totItems = len(x)
    dvm = np.ndarray((totItems, totItems))
    distanceScale = 1
    for i in range(totItems):
        for j in range(totItems):
            dvm[i][j] = distanceScale*math.sqrt((x[i] - x[j])**2 + (y[i] - y[j])**2)

    # wkm = 2*dvm
    # np.savetxt("dvm.csv", dvm, delimiter=",")
    # np.savetxt("wkm.csv", wkm, delimiter=",")
    return dvm

def read_vns_file_type(txtFile):
    nbNodes = -1
    nbVehi = -1
    nbServi = -1
    nbCust = -1
    nbSynch = -1
    e = []
    f = open(txtFile, 'r+')
    for i in range(4):

        s = f.readline()
        s = s.replace(';', '')
        s = s.replace(' ', '')
        ls = s.split('=')

        if ls[0] == 'nbServi':
            nbServi = int(ls[1])		
        elif ls[0] == 'nbNodes':
            nbNodes = int(ls[1])
        elif ls[0] == 'nbVehi':
            nbVehi = int(ls[1])
        elif ls[0] == 'nbCust':
            nbCust = int(ls[1])		
        elif ls[0] == 'nbSynch':
            nbSynch = int(ls[1])
        else:
            print('ERROR: Line ' + str(s) + ' not understood.')
            print(ls)

    nbNodes = nbCust + nbSynch + 2

    if nbServi < 0 or nbNodes < 0 or nbVehi < 0 or nbCust < 0 or nbSynch < 0:
        print('Error: There is a data requirement not found.')
        print('nbNodes = ' + str(nbNodes))
        print('nbVehi = ' + str(nbVehi))
        print('nbServi = ' + str(nbServi))
        print('nbCust = ' + str(nbCust))
        print('nbSynch = ' + str(nbSynch))

    nextLine = f.readline()
    if nextLine.strip() == 'e':
        e  = read_vector_line(f)
        e = np.append(e, 0)
    else:
        print('ERROR: This line is not what we expected: Expected "e" got " ' + str(nextLine) + '"')


    nextLine = f.readline()
    if nextLine.strip() == 'l':
        l = read_vector_line(f)
        l = np.append(l, 10000)
    else:
        print('ERROR: This line is not what we expected: Expected "l" got " ' + str(nextLine) + '"')

    nextLine = f.readline()
    varname = 'cx'
    if nextLine.strip() == varname:
        x = read_vector_line(f)
        x = np.append(x, x[0])
    else:
        print('ERROR: This line is not what we expected: Expected "' + varname + '" got " ' + str(nextLine) + '"')
    
    nextLine = f.readline()
    varname = 'cy'
    if nextLine.strip() == varname:
        y =  read_vector_line(f)
        y = np.append(y, y[0])
    else:
        print('ERROR: This line is not what we expected: Expected "' + varname + '" got " ' + str(nextLine) + '"')

    nextLine = f.readline()
    varname = 's'
    s_list = []
    if nextLine.strip() == varname:
        for i in range(nbNodes - 1):
            s_list.append(read_vector_line(f))
        s_list.append(np.ones(nbServi)) # To be consistent with the other files
        r = np.array(s_list)
    else:
        print('ERROR: This line is not what we expected: Expected "' + varname + '" got " ' + str(nextLine) + '"')

    nextLine = f.readline()
    varname = 'delta'
    s_list = []
    if nextLine.strip() == varname:
        for i in range(nbNodes - 1):
            s_list.append(read_vector_line(f))
        s_list.append(np.array([0, 1000])) # To be consistent with the other files
        delta = np.array(s_list)
    else:
        print('ERROR: This line is not what we expected: Expected "' + varname + '" got " ' + str(nextLine) + '"')
    # print(delta)
    mind = delta[:,0]
    mind = np.append(mind, mind[0])

    maxd = delta[:,1]
    maxd = np.append(maxd, maxd[0])

    # P
    nextLine = f.readline()
    varname = 'p'
    s_list = []
    pAux = []
    if nextLine.strip() == varname:
        for i in range((nbNodes - 1)*nbVehi):
            s_list.append(read_vector_line(f))
        for j in range(nbVehi):
            s_list.append(np.zeros(nbServi)) # To be consistent with the other files
        pAux = np.array(s_list)
    else:
        print('ERROR: This line is not what we expected: Expected "' + varname + '" got " ' + str(nextLine) + '"')

    p = np.zeros((nbNodes, nbVehi, nbServi))
    ct = 0
    for row,xv in enumerate(pAux):
        for col,v in enumerate(xv):
            ct = ct + 1
            realRow = row % nbVehi
            matx = int(np.floor(float(row) / float(nbVehi)))
            # print('\nCount = ' + str(ct))
            # print('row = ' + str(row) + ' col = ' + str(col))
            # print('mat = ' + str(matx) + ' row = ' + str(row) + ' col = ' + str(col))
            p[matx][realRow][col] = v

    nextLine = f.readline()
    varname = 'att'
    s_list = []
    a = []
    if nextLine.strip() == varname:
        for i in range(nbVehi):
            s_list.append(read_vector_line(f))

        a = np.array(s_list)
    else:
        print('ERROR: This line is not what we expected: Expected "' + varname + '" got " ' + str(nextLine) + '"')

    f.close()

    DS = np.array(range(nbCust + 2, nbCust + nbSynch + 2), dtype=np.int)
    # print(DS)
    # print(DS.shape)
    # print nbNodes
    # print nbCust
    # print nbSynch
    # exit(-123435)

    d = euclidean_matrices(x,y)

    if False:
        print('nbNodes = ' + str(nbNodes))
        print('nbVehi = ' + str(nbVehi))
        print('nbServi = ' + str(nbServi))
        print('nbCust = ' + str(nbCust))
        print('nbSynch = ' + str(nbSynch))
        print('r = ' + str(r))
        print('DS = ' + str(DS))
        print('a = ' + str(a))
        print('x = ' + str(x))
        print('y = ' + str(y))
        print('d = ' + str(d))
        print('p = ' + str(p))
        print('mind = ' + str(mind))
        print('maxd = ' + str(maxd))
        print('e (Shape = ' + str(e.shape) + ')\n' + str(e))
        print('l (Shape = ' + str(l.shape) + ')\n' + str(l))
        print('l = ' + str(l))

    return([nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l])

def read_from_matfile(matFile):
    c = scipy.io.loadmat(str(matFile))
    print('Loaded ' + str(matFile))
    # print('Start loading vars...')
    nbNodes = c.get('nbNodes')[0][0]
    DS = np.zeros(nbNodes)
    x = np.zeros(nbNodes)
    y = np.zeros(nbNodes)
    xaux = c.get('x')[0]
    DSaux = c.get('DS')[0]
    yaux = c.get('y')[0]
    doDS = True
    for	i in range(nbNodes):
        x[i] = xaux[i][0][0]
        y[i] = yaux[i][0][0]
        if doDS:
            try:
                DS[i] = DSaux[i][0][0]
            except:
                doDS = False
        
    d = c.get('d')
    maxd = c.get('maxd')[0]
    nbVehi = c.get('nbVehi')[0][0]
    r = np.asarray(c.get('r'))
    a = np.asarray(c.get('a'))
    e = np.asarray(c.get('e'))[0]
    l = np.asarray(c.get('l'))[0]
    mind = np.asarray(c.get('mind'))[0]
    nbServi = c.get('nbServi')[0][0]
    pAux = c.get('p')
    p = np.zeros((nbNodes, nbVehi, nbServi))
    ct = 0
    for row,xv in enumerate(pAux):
        for col,v in enumerate(xv):
            ct = ct + 1
            realRow = row % nbVehi
            matx = int(np.floor(float(row) / float(nbVehi)))
            # print('\nCount = ' + str(ct))
            # print('row = ' + str(row) + ' col = ' + str(col))
            # print('mat = ' + str(matx) + ' row = ' + str(row) + ' col = ' + str(col))
            p[matx][realRow][col] = v

    if False:
        print('Done.')
        print('\nnbNodes: ')
        print(nbNodes)
        print('\nnbVehi: ')
        print(nbVehi)
        print('\nnbServi: ')
        print(nbServi)
        # r = list(r)
        print('\nr: ')
        print(r)
        # DS = list(DS)
        print('\nDS: ')
        print(DS)
        # a = list(a)
        print('\na: ')
        print(a)
        # x = list(x)
        print('\nx: ')
        print(x)
        # y = list(y)
        print('\ny: ')
        print(y)
        # d = list(d)
        print('\nd: ')
        print(d)
        # p = list(p)
        print('\np: ')
        print(p)
        # mind = list(mind)
        print('\nmind: ')
        print(mind)
        # maxd = list(maxd)
        print('\nmaxd: ')
        print(maxd)
        # e = list(e)
        print('\ne: ')
        print(e)
        # l = list(l)
        print('\nl: ')
        print(l)
    return([nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l])



def read_InstanzCPLEX_HCSRP_10_1():
    nbNodes=12
    nbVehi=3
    nbServi=6
    r=[
    [1,1,1,1,1,1],
    [0,0,0,1,0,0],
    [0,0,0,0,1,0],
    [0,1,0,0,0,0],
    [0,0,0,1,0,0],
    [0,0,1,0,0,0],
    [0,0,0,0,1,0],
    [0,0,1,0,0,0],
    [0,0,0,0,1,1],
    [1,0,0,1,0,0],
    [0,0,1,0,0,1],
    [1,1,1,1,1,1]
    ]
    DS=[11,10,9]


    #  This is new fake data
    # for i in range(20):
    # print("WARNING - THIS DATA IS NOT CORRECT - DS and some SKILLS ARE REMOVED!!!!")
    # DS = []
    # r=[
    # [1,1,1,1,1,1],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [0,0,0,0,0,0],
    # [1,1,1,1,1,1]
    # ]
    # End of fake data
    a=[
    [0, 1, 1, 0, 1, 1],
    [1, 1, 1, 0, 1, 0],
    [0, 1, 1, 1, 1, 0]
    ]
    x=[85,47,54,34,78,69,17,62,92,96,0,85]
    y=[26,32,10,49,24,9,48,3,37,2,0,26]
    d=[ [ 0.0,38.470768,34.88553,55.946404,7.28011,23.345236,71.470276,32.526913,13.038404,26.400757,88.88757,0.0],
     [ 38.470768,0.0,23.086792,21.400934,32.01562,31.827662,34.0,32.649654,45.276924,57.45433,56.859474,38.470768],
     [ 34.88553,23.086792,0.0,43.829212,27.784887,15.033297,53.037724,10.630146,46.615448,42.755116,54.91812,34.88553],
     [ 55.946404,21.400934,43.829212,0.0,50.606323,53.15073,17.029387,53.851646,59.22837,77.801025,59.64059,55.946404],
     [ 7.28011,32.01562,27.784887,50.606323,0.0,17.492855,65.551506,26.400757,19.104973,28.42534,81.608826,7.28011],
     [ 23.345236,31.827662,15.033297,53.15073,17.492855,0.0,65.0,9.219544,36.23534,27.89265,69.58448,23.345236],
     [ 71.470276,34.0,53.037724,17.029387,65.551506,65.0,0.0,63.63961,75.802376,91.416626,50.92151,71.470276],
     [ 32.526913,32.649654,10.630146,53.851646,26.400757,9.219544,63.63961,0.0,45.343136,34.0147,62.072536,32.526913],
     [ 13.038404,45.276924,46.615448,59.22837,19.104973,36.23534,75.802376,45.343136,0.0,35.22783,99.16148,13.038404],
     [ 26.400757,57.45433,42.755116,77.801025,28.42534,27.89265,91.416626,34.0147,35.22783,0.0,96.02083,26.400757],
     [ 88.88757,56.859474,54.91812,59.64059,81.608826,69.58448,50.92151,62.072536,99.16148,96.02083,0.0,88.88757],
     [ 0.0,38.470768,34.88553,55.946404,7.28011,23.345236,71.470276,32.526913,13.038404,26.400757,88.88757,0.0]
    ]

    p=[ [ [0,0,0,0,0,0],
     [0,0,0,0,0,0],
     [0,0,0,0,0,0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0],
     [14.0,14.0,14.0,14.0,14.0,14.0]
     ],
     [
     [0,0,0,0,0,0],
     [0,0,0,0,0,0],
     [0,0,0,0,0,0]
     ]]
    mind=[ 14,24,36,38,46,10,27,16,0,51,8,36]
    maxd=[ 28,48,72,76,92,20,54,32,0,102,16,72]
    e=[0,0,0,0,0,0,0,0,0,0,0,0]
    l=[10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000]
    return([nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l])

def remove_time_info(inst):
    inst.jobTimeInfo[:,0] = -10000


def mk_to_c(nbNodes, nbVehi, nbServi, r, DS, dependsOn, a, x, y, d, p, mind, maxd, e, l, i=0, maxworkingtime=1000000,pureMK = True,orJobs=1000000000):
    
    # The instace given. If it's a number, it was not given:
    if type(i) == int:
        i = hhc.INSTANCE()
        
    # offset = 0
    # if pureMK:
    # 	offset = 1
    if pureMK:
        i.nJobs = nbNodes - 1
    else:
        i.nJobs = nbNodes - 1 # - offset

    i.nNurses = nbVehi
    i.nSkills = nbServi
    
    i.init_job_and_nurse_objects()

    i.nurseWorkingTimes = np.zeros((i.nNurses, 3), dtype=np.int)
    for ii in range(i.nNurses):
        i.nurseWorkingTimes[ii][0] = e[0]
        i.nurseWorkingTimes[ii][1] = maxworkingtime
        i.nurseWorkingTimes[ii][2] = maxworkingtime - e[0]

    i.nurseSkills = np.ascontiguousarray(a, dtype=np.int)


    i.jobTimeInfo = np.zeros((i.nJobs, 3), dtype=np.int)
    i.jobSkillsRequired = np.zeros((i.nJobs, i.nSkills), dtype=np.int)
    for job,mkJob in enumerate(range(1,nbNodes)):
        i.jobTimeInfo[job][0] = e[mkJob]
        i.jobTimeInfo[job][1] = l[mkJob]
        # Duration:
        for ii in range(i.nSkills):
            if (r[mkJob][ii] < 0.1):
                continue
            else:
                i.jobSkillsRequired[job][ii] = 1
            if pureMK:
                i.jobTimeInfo[job][2] = p[1][1][1]
            else:
                i.jobTimeInfo[job][2] = max(int(p[mkJob][0][ii]), i.jobTimeInfo[job][2]) # Take the longest
            for jj in range(1,i.nNurses):
                if p[mkJob][0][ii] != p[mkJob][jj][ii]:
                    print('ERROR: Cannot solve this Mankowska instance, processing times are different for each staff member.')
                    exit()

    i.doubleService = np.zeros(i.nJobs, dtype=np.int)
    for job in DS:
        i.doubleService[int(job) - 2] = 1

    i.dependsOn = np.array(dependsOn, dtype=np.int)
    # print(i.dependsOn)
    i.od = np.zeros((i.nJobs + 1, i.nJobs + 1), dtype=np.float64)
    i.xy = np.zeros((i.nJobs + 1, 2))
    for jj in range(i.nJobs + 1):
        i.xy[jj][0] = x[jj]
        i.xy[jj][1] = y[jj]
        for kk in range(i.nJobs + 1):
            i.od[jj][kk] = d[jj][kk]

    i.solMatrix = np.zeros((i.nNurses, i.nJobs), dtype=np.int)
    i.MAX_TIME_SECONDS = 30
    i.verbose = 5
    i.secondsPerTU = 60
    i.mk_mind = mind
    i.mk_maxd = maxd
    # print('------------------------------\njobSkillsRequired: ')
    # print(i.jobSkillsRequired)
    # print('------------------------------\nr: ')
    # print(r)
    # print('------------------------------\n')

    # Assume these instances have no support for different depots:
    i.nurse_travel_from_depot = np.empty((i.nNurses, i.nJobs), dtype=np.float64)
    i.nurse_travel_to_depot = np.empty((i.nNurses, i.nJobs), dtype=np.float64)
    for nurse in range(i.nNurses):
        i.nurse_travel_from_depot[nurse,:] = i.od[0,1:]
        i.nurse_travel_to_depot[nurse,:] = i.od[1:,0]

    return i

def generate_Mk(i, matfile='none', filetype='tsplib'):
    print('arg i', i)
    pureMK = False
    if filetype == 'tspLib':
        [nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l] = read_from_tsp(matfile)
    elif filetype == 'tsptw':
        [nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l] = read_from_tsptw(matfile)
    elif filetype == 'large_vns':
        pureMK = True
        [nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l] = read_vns_file_type(matfile)
    else:
        pureMK = True
        if matfile == 'none':
            print('WARNING: Using instance with fake data!!!!!!!!!!!!')
            [nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l] = read_InstanzCPLEX_HCSRP_10_1()
        else:
            print('Reading from matfile: ' + matfile + '...')
            [nbNodes, nbVehi, nbServi, r, DS, a, x, y, d, p, mind, maxd, e, l] = read_from_matfile(matfile)
            print('Done.')

    # See below for modification, we can follow paper, or intuition from "Instance"
    # We cannot replicate results if we follow instance...
    maxworkingtime = 1000000
    useInstanceWorkingTime = True

    if pureMK:
        # Remove last bit of most variables:
        orJobs = nbNodes
        nbNodes -= 1
        dependsOn = list(np.ones(nbNodes - 1)*-1)
        d = np.delete(d, -1, 1)
        d = np.delete(d, -1, 0)
        r = np.delete(r, -1, 0)

        # Choose whether using large working time
        if useInstanceWorkingTime:
            maxworkingtime = l[-1]

        l = np.delete(l, -1, 0)
        e = np.delete(e, -1, 0)
        x = np.delete(x, -1, 0)
        y = np.delete(y, -1, 0)
        mind = np.delete(mind, -1, 0)
        maxd = np.delete(maxd, -1, 0)

        # Duplicate those DS that are not simultaneous
        DSb = copy.copy(DS)
        TOL = 0.1
        DS = []

        # for idxx,rrr in enumerate(r):
        #     print('Job ', idxx, ')\t', rrr)
        # print('Ds jobs: ' + str(DSb))

        for dsind in DSb:
            ds = int(dsind) - 1
            if ds < 1:
                break
            # If mind and maxd are not 0, then it's not DS it's synchronised and we should split it
            if not ((abs(mind[ds]) < TOL) and (abs(maxd[ds]) < TOL)):
                # print('Duplicating job: ' + str(ds))
                # This is not simultaneous, split in two
                nbNodes += 1
                x = np.append(x, x[ds])
                y = np.append(y, y[ds])
                r_old = copy.copy(r[ds])
                # print('reqs: ' + str(r_old))
                # print('r starts as \n' + str(r))
                r = np.append(r, np.expand_dims(copy.copy(r_old), axis=0), axis=0)
                # print('r continues \n' + str(r))+

                # Mankowska jobs that have dependency require two skills.
                # The lowest index nurse should do the lowest index skill (check model in paper)
                # Here, we split the job in two, and then we remove one skill in the old job (the skill that appears later)
                # and one skill in the new one (the skill that appears first)
                # print('----------------------')
                # print('Prev job: ', r[ds - 1])
                # print('Splitting job ' + str(ds) + ' with skills: ', r_old)
                # print('Next job: ', r[ds + 1])

                isFirst = True
                for skill_in_old in range(len(r_old) - 1,-1,-1):
                    if r_old[skill_in_old] > 0:
                        if isFirst:
                            r[ds][skill_in_old] = 0
                            isFirst = False
                        else:
                            r[-1][skill_in_old] = 0
                            break
                # print('r mod \n' + str(r) + '\n----\n')
                # print('Ended up with r[' + str(ds) + ']: ', r[ds])
                # print('Ended up with r[-1]: ', r[-1])
                # print('----------------------')
                # Sort out mutual dependency
                # print('\tmind: ' + str(mind) + ' shape: ' + str(mind.shape))
                # print('\tThe relevant mind element is: ' + str(mind[ds]))
                dependsOn.append(ds - 1)
                dependsOn[ds - 1] = len(dependsOn) - 1
                print('Duplicating ' + str(ds - 1) + ' - linked with ' + str(len(dependsOn) - 1))

                # mindb = list(mind)
                # mindb[ds] = -1*mindb[ds]
                # newVal = -1*mindb[ds]
                # mindb.append(newVal)
                # mind = np.array(mindb, dtype=np.int)

                # maxdb = list(maxd)
                # maxdb[ds] = -1*maxdb[ds]
                # newVal = -1*maxdb[ds]
                # maxdb.append(newVal)
                # maxd = np.array(maxdb, dtype=np.int)

                mindb = list(mind)
                maxdb = list(maxd)


                oldMin = mindb[ds]
                oldMax = maxdb[ds]
                
                mindb[ds] = -1*oldMax
                maxdb[ds] = -1*oldMin
                
                maxdb.append(oldMax)
                mindb.append(oldMin)

                mind = np.array(mindb, dtype=np.int)
                maxd = np.array(maxdb, dtype=np.int)

                # mind = copy.copy(mind)
                # mind[ds] = -1*int(mind[ds])
                # mind = np.append(mind, -1*copy.copy(mind[ds]))
                # mind = copy.copy(mind)
                # print('\tmind is, after appending: ' + str(mind) + ' shape: ' + str(mind.shape))

                # newVal = -1*mind[ds]
                # print('\tTrying to switch ' + str(mind[ds]) + ' by ' + str(newVal))
                # mind[ds] = newVal
                # print('\tmind is, after the change: (element ' + str(mind[ds]) + ') ' + str(mind) + ' shape: ' + str(mind.shape))

                # maxd = np.append(maxd, copy.copy(maxd[ds]))
                # maxd[ds] = -1*copy.copy(maxd[ds])

                # print('--')

                # Copy attributes		
                e = np.append(e, e[ds])
                l = np.append(l, l[ds])
                p = np.concatenate((p,np.expand_dims(p[ds,:,:], axis=0)),0)

                # Update OD matrix to include new point:
                d = np.concatenate((d,np.expand_dims(d[ds,:], axis=0)),0)
                d = np.concatenate((d,np.expand_dims(d[:,ds], axis=1)),1)
            else:
                DS.append(dsind)

    # print(dependsOn)
    print('mktc i skilltype ', i.DSSkillType)
    return mk_to_c(nbNodes, nbVehi, nbServi, r, DS, dependsOn, a, x, y, d, p, mind, maxd, e, l, i=i, pureMK=pureMK, maxworkingtime=maxworkingtime, orJobs=orJobs)