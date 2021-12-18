import os, importlib
import numpy as np
import copy

def read_ait_h_instance(file_to_read, split=False):

    if split: # split instances being called, so need to use file in 'split_instances' folder and no need to call depl_to_od function
        folder = 'split_instances'
        inst = importlib.import_module(folder + '.' + file_to_read)
        inst.od = np.array(inst.od)
        inst.CoutPref = np.array(inst.CoutPref)
    else: # instance is in standard format, not split, call as normal.
        folder = 'ait _haddadene_et_al_2016'
        inst = importlib.import_module(folder + '.' + file_to_read)
        inst.od = depl_to_od(inst)
        inst.CoutPref = np.array(inst.CoutPref)
    return inst
### --- End def read_ait_h_instance --- ###

def depl_to_od(ldi):

    d = []
    normalRow = ldi.NbClients - 1
    extRow = ldi.NbClients
    finalRow = ldi.depl[-1*extRow:]
    # print('ldi.depl:\n')
    # print(ldi.depl)
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
    return np.array(d)
### --- End def depl_to_od --- ####


def split_instance(file_to_read, print_statements = True):

    folder = 'ait _haddadene_et_al_2016'
    inst = importlib.import_module(folder + '.' + file_to_read)
    inst.od = depl_to_od(inst)
    inst.CoutPref = np.array(inst.CoutPref)

    # Demandes clients x services
    Demandes1 = []
    Clients1 = []
    for i in range(len(inst.Demandes)):
        if inst.Demandes[i][0] == 1:
            Demandes1.append([1,0])
            Clients1.append(i)

    NumClients1 = len(Clients1)
    
    if print_statements:
        print('Demandes1: ', Demandes1)
        print('Clients1: ', Clients1)
        print('NumClients1: ', NumClients1)

    # Offres nurses x services
    Offres1 = []
    Nurses1 = []
    for i in range(len(inst.Offres)):
        if inst.Offres[i][0] == 1:
            Offres1.append([1,0])
            Nurses1.append(i)
    NumNurses1 = len(Nurses1)
    
    if print_statements:
        print('Offres1: ', Offres1)
        print('Nurses1: ', Nurses1)
        print('NumNurses1: ', NumNurses1)

    # Indices of clients that need to be removed
    removeClients1 = [] 
    for i in range(inst.NbClients):
        if i not in Clients1:
            removeClients1.append(i)

    removeClientsOD1 = copy.copy(removeClients1) # indices of the clients that need to be removed from the od matrix
    for i in range(len(removeClientsOD1)):
        removeClientsOD1[i] += 1

    if print_statements:
        print('removeClients1: ', removeClients1)
        print('removeClientsOD1: ', removeClientsOD1)

    # exit(-1)

    # Indices of nurses that need to be removed
    removeNurses1 = []
    for i in range(inst.NbVehicules):
        if i not in Nurses1:
            removeNurses1.append(i)

    if print_statements:
        print('removeNurses1: ', removeNurses1)

    # od matrix, clients+1 x clients+1
    odrows1 = np.delete(inst.od, removeClientsOD1, axis=0) # remove rows first (axis = 0)
    od1 = np.delete(odrows1, removeClientsOD1, axis=1) # now remove columns to get od1 (axis = 1)
    # print(od1)
    od1list = od1.tolist()
    # od1 = np.array(od1)

    # WindowsC clients x 2
    WindowsC1 = []
    for i in range(len(inst.WindowsC)):
        if i not in removeClients1:
            WindowsC1.append(inst.WindowsC[i])
    
    if print_statements:
        print('WindowsC1: ', WindowsC1)

    # WindowsD nurses x 2
    WindowsD1 = []
    for i in range(len(inst.WindowsD)):
        if i not in removeNurses1:
            WindowsD1.append(inst.WindowsD[i])
    
    if print_statements:
        print('WindowsD1: ', WindowsD1)

    # DureeDeVisite clients x services
    DureeDeVisite1 = []
    for i in range(len(inst.DureeDeVisite)):
        if i not in removeClients1:
            DureeDeVisite1.append([inst.DureeDeVisite[i][0], 0])
    
    if print_statements:
        print('DureeDeVisite1: ', DureeDeVisite1)       

    # CoutPref clients x nurses
    CoutPref1 = []
    for i in range(len(inst.CoutPref)):
        temp1 = []
        if i not in removeClients1:
            for j in range(len(inst.CoutPref[i])):
                if j not in removeNurses1:
                    temp1.append(inst.CoutPref[i][j])
            CoutPref1.append(temp1)

    if print_statements:
        print('CoutPref1: ', CoutPref1)

    # gap 1 x clients
    gap1 = []
    for i in range(len(inst.gap)):
        if i not in removeClients1:
            gap1.append(-1)
    
    if print_statements:
        print('gap1: ', gap1)


    cwd = os.getcwd()
    results_filename1 = str(file_to_read) + '-skill1.py'
    outputfiles_path = os.path.join(cwd, 'split_instances')
    resultsfile_path1 = os.path.join(outputfiles_path, results_filename1)

    f = open(resultsfile_path1, 'w')
    f.write('NbVehicules='+ str(NumNurses1) + '\n')
    f.write('NbClients='+ str(NumClients1) + '\n')
    f.write('NbServices='+ str(inst.NbServices) + '\n')
    f.write('Demandes='+str(Demandes1) + '\n')
    f.write('Offres='+str(Offres1) + '\n')
    f.write('od='+str(od1list) + '\n')
    f.write('WindowsC='+str(WindowsC1) + '\n')
    f.write('DureeDeVisite='+str(DureeDeVisite1) + '\n')
    f.write('WindowsD='+str(WindowsD1) + '\n')
    f.write('CoutPref='+str(CoutPref1) + '\n')
    f.write('gap='+str(gap1) + '\n')
    f.close()

    #-----------------------------------------------------------------------------------------------------#

    # Demandes clients x services
    Demandes2 = []
    Clients2 = []
    for i in range(len(inst.Demandes)):
        if inst.Demandes[i][1] == 1:
            Demandes2.append([0,1])
            Clients2.append(i)

    NumClients2 = len(Clients2)
    
    if print_statements:
        print('Demandes2: ', Demandes2)
        print('Clients2: ', Clients2)
        print('NumClients2: ', NumClients2)

    # Offres nurses x services
    Offres2 = []
    Nurses2 = []
    for i in range(len(inst.Offres)):
        if inst.Offres[i][1] == 1:
            Offres2.append([0,1])
            Nurses2.append(i)
    NumNurses2 = len(Nurses2)
    
    if print_statements:
        print('Offres2: ', Offres2)
        print('Nurses2: ', Nurses2)
        print('NumNurses2: ', NumNurses2)

    # Indices of clients that need to be removed
    removeClients2 = [] 
    for i in range(inst.NbClients):
        if i not in Clients2:
            removeClients2.append(i)

    removeClientsOD2 = copy.copy(removeClients2) # indices of the clients that need to be removed from the od matrix
    for i in range(len(removeClientsOD2)):
        removeClientsOD2[i] += 1

    if print_statements:
        print('removeClients2: ', removeClients2)
        print('removeClientsOD2: ', removeClientsOD2)

    # Indices of nurses that need to be removed
    removeNurses2 = []
    for i in range(inst.NbVehicules):
        if i not in Nurses2:
            removeNurses2.append(i)

    if print_statements:
        print('removeNurses2: ', removeNurses2)

    # od matrix, clients+1 x clients+1
    odrows2 = np.delete(inst.od, removeClientsOD2, axis=0)
    od2 = np.delete(odrows2, removeClientsOD2, axis=1)
    # print(od1)
    od2list = od2.tolist()

    # WindowsC clients x 2
    WindowsC2 = []
    for i in range(len(inst.WindowsC)):
        if i not in removeClients2:
            WindowsC2.append(inst.WindowsC[i])
    
    if print_statements:
        print('WindowsC2: ', WindowsC2)

    # WindowsD nurses x 2
    WindowsD2 = []
    for i in range(len(inst.WindowsD)):
        if i not in removeNurses2:
            WindowsD2.append(inst.WindowsD[i])
    
    if print_statements:
        print('WindowsD2: ', WindowsD2)

    # DureeDeVisite clients x services
    DureeDeVisite2 = []
    for i in range(len(inst.DureeDeVisite)):
        if i not in removeClients2:
            DureeDeVisite2.append([0, inst.DureeDeVisite[i][1]])

    if print_statements:
        print('DureeDeVisite2: ', DureeDeVisite2)

    # CoutPref clients x nurses
    CoutPref2 = []
    for i in range(len(inst.CoutPref)):
        temp2 = []
        if i not in removeClients2:
            for j in range(len(inst.CoutPref[i])):
                if j not in removeNurses2:
                    temp2.append(inst.CoutPref[i][j])
            CoutPref2.append(temp2)

    if print_statements:
        print('CoutPref2: ', CoutPref2)

    # gap 1 x clients
    gap2 = []
    for i in range(len(inst.gap)):
        if i not in removeClients2:
            gap2.append(-1)
    
    if print_statements:
        print('gap2: ', gap2)



    results_filename2 = file_to_read + '-skill2.py'
    # outputfiles_path = os.path.join(cwd, 'split_instances')
    resultsfile_path2 = os.path.join(outputfiles_path, results_filename2)

    f = open(resultsfile_path2, 'w')
    f.write('NbVehicules='+ str(NumNurses2) + '\n')
    f.write('NbClients='+ str(NumClients2) + '\n')
    f.write('NbServices='+ str(inst.NbServices) + '\n')
    f.write('Demandes='+str(Demandes2) + '\n')
    f.write('Offres='+str(Offres2) + '\n')
    f.write('od='+str(od2list) + '\n')
    f.write('WindowsC='+str(WindowsC2) + '\n')
    f.write('DureeDeVisite='+str(DureeDeVisite2) + '\n')
    f.write('WindowsD='+str(WindowsD2) + '\n')
    f.write('CoutPref='+str(CoutPref2) + '\n')
    f.write('gap='+str(gap2) + '\n')
    f.close()

    return inst
### -- End of def split_instance --- ###

# print('type od: ', type(inst.od), ' shape od: ', inst.od.shape)
# # inst.Demandes = np.array(inst.Demandes)
# print('type demandes: ', type(inst.Demandes))
# # print('shape demandes: ', inst.Demandes.shape)
# # inst.Offres = np.array(inst.Offres)
# print('type Offres: ', type(inst.Offres))
# # print('shape offres: ', inst.Offres.shape)
# # inst.WindowsC = np.array(inst.WindowsC)
# print('type WindowsC: ', type(inst.WindowsC))
# # print('shape WindowsC: ', inst.WindowsC.shape)
# # inst.WindowsD = np.array(inst.WindowsD)
# print('type WindowsD: ', type(inst.WindowsD))
# # print('shape WindowsD: ', inst.WindowsD.shape)
# # inst.DureeDeVisite = np.array(inst.DureeDeVisite)
# print('type Duree: ', type(inst.DureeDeVisite))
# # print('shape Duree: ', inst.DureeDeVisite.shape)
# print('type coutpref: ', type(inst.CoutPref), ' shape coutpref: ', inst.CoutPref.shape)
# inst.gap = np.array(inst.gap)
# print('type gap: ', type(inst.gap))
# # print('shape gap: ', inst.gap.shape)
# exit(-1)