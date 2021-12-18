#-----------------------#
# CARROT - CARe ROuting Tool
# literature_conversions.py
# file containing functions to convert literature instances (ait_h, mankowska etc) to instance for the program.
# 09/11/2021
#-----------------------#

import numpy as np
import imp
import copy

def convert_aith_instance(filename, inst):

    # Load Ait Hadadene file
    ldi = imp.load_source('ait_h', filename)

    inst.nJobs = ldi.NbClients
    inst.nNurses = ldi.NbVehicules
    inst.nSkills = ldi.NbServices

    # inst.nurseWorkingTimes = np.zeros((inst.nNurses, 3), dtype=np.int)
    inst.nurseWorkingTimes = np.zeros((inst.nNurses, 5), dtype=np.int)# [0] = start time, [1] = end time, [2] = total length of day, [3] = number of shifts (always 1) [4] = duration of actual working times (always the same as total length of day [2])
    for i in range(inst.nNurses):
        inst.nurseWorkingTimes[i][0] = ldi.WindowsD[i][0]
        inst.nurseWorkingTimes[i][1] = ldi.WindowsD[i][1]
        inst.nurseWorkingTimes[i][2] = ldi.WindowsD[i][1] - ldi.WindowsD[i][0]
        inst.nurseWorkingTimes[i][3] = 1
        inst.nurseWorkingTimes[i][4] = inst.nurseWorkingTimes[i][2]

    inst.nurseSkills = np.ascontiguousarray(ldi.Offres, dtype=np.int)
    inst.jobTimeInfo = np.zeros((inst.nJobs, 3), dtype=np.int)
    inst.jobSkillsRequired = np.ascontiguousarray(ldi.Demandes, dtype=np.int)

    # print(ldi.WindowsC)
    # print(len(ldi.WindowsC))
    # print(inst.nJobs)
    for job in range(inst.nJobs):
        inst.jobTimeInfo[job][0] = ldi.WindowsC[job][0]
        inst.jobTimeInfo[job][1] = ldi.WindowsC[job][1]
        for sk in range(ldi.NbServices):
            if (inst.jobSkillsRequired[job][sk] > 0):
                inst.jobTimeInfo[job][2] = ldi.DureeDeVisite[job][sk]

    inst.doubleService = np.zeros(inst.nJobs, dtype=np.int)
    inst.mk_mind = -1*np.ones(inst.nJobs)
    inst.mk_maxd = -1*np.ones(inst.nJobs)

    for job,gapVal in enumerate(ldi.gap):
        if gapVal == 0:
            inst.doubleService[job] = 1
        elif gapVal > 0:
            inst.mk_mind[job] = gapVal
            inst.mk_maxd[job] = gapVal

    # print(inst.mk_mind)
    # To be filled later!
    inst.dependsOn = -1*np.ones(inst.nJobs, dtype=np.int)

    d = []
    normalRow = ldi.NbClients - 1
    extRow = ldi.NbClients
    finalRow = ldi.depl[-1*extRow:]
    # print(ldi.depl)
    # print(finalRow)
    for i in range(ldi.NbClients + 1):
        if i == 0:
            map_i = ldi.NbClients
            rowSize = extRow
        else:
            map_i = i - 1
            rowSize = normalRow
        stLim = rowSize*map_i
        endLim = rowSize*(map_i + 1)
        dlRow = copy.copy(ldi.depl[stLim:endLim])

        if i != 0:
            # print('Inserting ' + str(map_i) + ' / ' + str(len(finalRow)))
            dlRow.insert(0,finalRow[map_i])
            dlRow.insert(map_i + 1, 0)
        else:
            dlRow.insert(0, 0)

        d.append(copy.copy(dlRow))

    inst.od = np.ascontiguousarray(d, dtype=np.float64)


    inst.prefScore = np.ascontiguousarray(ldi.CoutPref, dtype=np.float64)

    # DUPLICATE SOME JOBS
    for job,jtd in enumerate(inst.mk_mind):
        if jtd < 0:
            continue

        if (inst.verbose > 5):
            print('Duplicating ' + str(job) + '...')

        inst.nJobs += 1
        inst.dependsOn[job] = len(inst.dependsOn)
        inst.dependsOn = np.append(inst.dependsOn, job)
        inst.doubleService = np.append(inst.doubleService, 0)
        
        toAdd = np.expand_dims(inst.jobTimeInfo[job,:], axis=0)
        inst.jobTimeInfo = np.concatenate((inst.jobTimeInfo,toAdd),0)

        skillToAdd = np.expand_dims(inst.jobSkillsRequired[job,:], axis=0)
        inst.jobSkillsRequired = np.concatenate((inst.jobSkillsRequired,skillToAdd),0)
        inst.jobSkillsRequired[job][1] = 0
        inst.jobSkillsRequired[-1][0] = 0

        # inst.mk_mind[job] *= -1
        # inst.mk_maxd[job] *= -1

        inst.mk_mind = np.append(inst.mk_mind, inst.mk_mind[job])
        inst.mk_maxd = np.append(inst.mk_maxd, inst.mk_maxd[job])

        # Update OD matrix to include new point:
        inst.od = np.concatenate((inst.od,np.expand_dims(inst.od[job+1,:], axis=0)),0)
        inst.od = np.concatenate((inst.od,np.expand_dims(inst.od[:,job+1], axis=1)),1)


        prefToAdd = np.expand_dims(inst.prefScore[job,:], axis=0)
        inst.prefScore = np.concatenate((inst.prefScore,prefToAdd),0)
        if (inst.verbose > 5):
            print('Done.')

    # Duplicate preferences of the job!!!!!

    inst.mk_mind = np.insert(inst.mk_mind, 0, 0)
    inst.mk_maxd = np.insert(inst.mk_maxd, 0, 0)

    inst.solMatrix = np.zeros((inst.nNurses, inst.nJobs), dtype=np.int)

    inst.secondsPerTU = 60

    bestPref = 0
    for row in range(ldi.NbClients):
        rowScore = 1000000000000
        bestCol = -1
        for col in range(ldi.NbVehicules):
            if (inst.doubleService[row] < 1) and ((inst.jobSkillsRequired[row,0] > inst.nurseSkills[col,0]) or
                (inst.jobSkillsRequired[row,1] > inst.nurseSkills[col,1])):
                # print('Rejected that nurse ' + str(col) + ' could do job ' + str(row))
                # print('\tRequired: ' + str(inst.jobSkillsRequired[row,:]))
                # print('\tAvailable: ' + str(inst.nurseSkills[col,:]))
                continue

            if (inst.prefScore[row,col] < rowScore):
                rowScore = inst.prefScore[row,col]
                bestCol = col

        bestPref += rowScore
        # print('For job: ' +str(row) + ' the best assignment is nurse ' + str(bestCol) + ' value: ' + str(rowScore))
    if (inst.verbose > 5):
        print('For instance ' + str(filename) + ' the minimum pref score is: ' + str(bestPref) + '\n')

    # exit(-24235345)

    # Ait H has no support for heterogeneous depots,
    # so we get travel info and replicate it:
    inst.nurse_travel_from_depot = np.empty((inst.nNurses, inst.nJobs), dtype=np.float64)
    inst.nurse_travel_to_depot = np.empty((inst.nNurses, inst.nJobs), dtype=np.float64)
    for nurse in range(inst.nNurses):
        inst.nurse_travel_from_depot[nurse,:] = inst.od[0,1:]
        inst.nurse_travel_to_depot[nurse,:] = inst.od[1:,0]

    inst.nurse_travel_from_depot = np.ascontiguousarray(inst.nurse_travel_from_depot)
    inst.nurse_travel_to_depot = np.ascontiguousarray(inst.nurse_travel_to_depot)

    # inst.init_job_and_nurse_objects()
    # inst.instantiate_job_nurse_objects_aith()

    int_type = np.int32

    if len(inst.mk_mind) == 0:
        inst.mk_mind = list(np.zeros(inst.nJobs))
        inst.mk_maxd = list(np.zeros(inst.nJobs))

    # Create capabilityOfDoubleServices
    nDS = np.sum(inst.doubleService)
    # print('Setting skill capabilities for ' + str(nDS) + ' jobs.')
    # for i in range(self.nNurses):
    # 	print('\tSkills Nurse ' + str(i) + ': ' + str(self.nurseSkills[i]))
    
    # self.DSSkillType = 'shared-duplicated' 
    inst.capabilityOfDoubleServices = np.zeros((inst.nNurses, inst.nNurses, nDS), dtype=int_type)
    # print('Matrix size: ' + str((self.nNurses, self.nNurses, nDS)))
    k = -1
    for k_all_job,dsjob in enumerate(inst.doubleService):
        if dsjob < 1:
            continue
        k += 1
        reqSkills = []
        for s,req in enumerate(inst.jobSkillsRequired[k_all_job]):
            if req > 0:
                reqSkills.append(s)
        # print('* Job ' + str(k_all_job) + '(ds index ' + str(k) + ') required skills: ' + str(reqSkills))
        for i in range(inst.nNurses):
            # print('\tSkills i ' + str(i) + ': ' + str(self.nurseSkills[i]))
            for j in range(inst.nNurses):
                if i == j:
                    continue
                # print('\t\tSkills j ' + str(j) + ': ' + str(self.nurseSkills[j]))
                if inst.DSSkillType == 'shared-duplicated':
                    # Check for repeated skills
                    print('Warning: Skill type "shared-duplicated" not implemented yet')
                    exit(-21234)
                elif inst.DSSkillType == 'shared':
                    # print('SHSTRF')
                    inst.capabilityOfDoubleServices[i,j,k] = 1
                    for s in reqSkills:
                        if inst.nurseSkills[i][s] + inst.nurseSkills[j][s] < 1:
                            inst.capabilityOfDoubleServices[i,j,k] = 0
                            break
                elif inst.DSSkillType == 'duplicated':
                    # Nothing
                    print('Warning: Skill type "duplicated" not implemented yet')
                    exit(-21234)
                elif inst.DSSkillType == 'strictly-shared':
                    # print('strictlySHSTRF')

                    # Mankowska
                    somethingOnI = False
                    somethingOnJ = False
                    inst.capabilityOfDoubleServices[i,j,k] = 1
                    for s in reqSkills:
                        if inst.nurseSkills[i][s] + inst.nurseSkills[j][s] > 0:
                            if inst.nurseSkills[i][s] > 0:
                                somethingOnI = True
                            if inst.nurseSkills[j][s] > 0:
                                somethingOnJ = True
                        else:
                            inst.capabilityOfDoubleServices[i,j,k] = 0
                            break
                    if (not (somethingOnJ and somethingOnI)):
                        inst.capabilityOfDoubleServices[i,j,k] = 0

                    # print('\t\tSetting ' + str((i,j,k)) + ' to ' + str(self.capabilityOfDoubleServices[i,j,k]))
                    # print('\t\tNurse i ' + str(somethingOnI))
                    # print('\t\tNurse j ' + str(somethingOnJ))
                else:
                    print('ERROR: Type of skill required for double services "' + inst.DSSkillType + '" not understood.')
                    print('Please, use one of the following:')
                    print('\t"shared-duplicated"')
                    print('\t"shared"')
                    print('\t"duplicated"')
                    print('\t"strictly-shared"')
                    exit(-2000)
    k = -1
    for k_all_job,dsjob in enumerate(inst.doubleService):
        if dsjob < 1:
            continue
        k += 1
        # print( 'Shape of self.capabilityOfDoubleServices[:] = ' + str(self.capabilityOfDoubleServices[:].shape))
        # print( 'Shape of self.capabilityOfDoubleServices[:][:] = ' + str(self.capabilityOfDoubleServices[:][:].shape))
        # print( 'Shape of self.capabilityOfDoubleServices[:][:][' + str(k) + '] = ' + str(self.capabilityOfDoubleServices[:][:][k].shape))
        jobServed = False
        for nursei in range(inst.nNurses):
            for nursej in range(inst.nNurses):
                if inst.capabilityOfDoubleServices[nursei][nursej][k] > 0.1:
                    jobServed = True
                    break
            if jobServed:
                break

        # if not self.capabilityOfDoubleServices[:][:][k].any():
        if not jobServed:
            print('ERROR: There is no qualified pair of nurses for job ' + str(k_all_job))
            print('Required skills: ' + str(inst.jobSkillsRequired[k_all_job]))
            print('Available nurses: ')
            for i in range(inst.nNurses):
                print('\tN' + str(i) + ' skills: ' + str(inst.nurseSkills[i]))
            exit(-123234235346)

    if len(inst.prefScore) < 1:
        inst.fill_preferences()

    return inst
### --- End def convert_aith_instance --- ###