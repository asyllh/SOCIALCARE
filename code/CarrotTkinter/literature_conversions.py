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

    inst.nurseWorkingTimes = np.zeros((inst.nNurses, 3), dtype=np.int)
    for i in range(inst.nNurses):
        inst.nurseWorkingTimes[i][0] = ldi.WindowsD[i][0]
        inst.nurseWorkingTimes[i][1] = ldi.WindowsD[i][1] - ldi.WindowsD[i][0]
        inst.nurseWorkingTimes[i][2] = ldi.WindowsD[i][1]

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

    return inst
# End def convert_aith_instance