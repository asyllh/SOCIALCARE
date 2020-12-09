import os
import numpy as np
import pandas as pd
import ctypes
from numpy.ctypeslib import ndpointer
import matplotlib.pyplot as plt
import datetime
from datetime import timedelta
import time
import subprocess
from math import sqrt
import pickle
import osrm

import mankowska_data as Mk
# import osrm_extend as oe


def prepare_instance(file_to_run,
                     instance_type,
                     quality_measure='default',
                     ds_skill_type='strictly-shared',
                     max_time_seconds=-1,
                     verbose_level=-1,
                     load_from_disk=False,
                     mkVNS=False,
                     options_vector=[]):
    # This function is called in def worker_task, returns an instance 'i' of the class INSTANCE

    if (max_time_seconds < 0 or verbose_level < 0):
        print('Invalid parameters: ')
        print('max_time_seconds = ', max_time_seconds)
        print('verbose_level = ', verbose_level)
        return -1
    
    if not os.path.isfile(file_to_run):
        print('Error: File "' + str(file_to_run) + '" does not exist.')
        exit(-1)
    
    ############################################# INSTANCE i IS CREATED ######################################################
    i = INSTANCE()

    i.full_file_name = file_to_run
    i.name = os.path.basename(file_to_run)

    i.qualityMeasure = quality_measure

    # print('Skill type was', i.DSSkillType)
    i.DSSkillType = ds_skill_type
    # print('Skill type now is', ds_skill_type)
    # i.MAX_TIME_SECONDS = max_time_seconds
    # i.verbose = verbose_level
    i.loadFromDisk = load_from_disk
    # print('max time seconds ' + str(i.MAX_TIME_SECONDS))
    print('Before calling, i: ', i)
    print('i.DSSkillType ', i.DSSkillType)

    # Generate the instance from the file:
    if instance_type == 'excel':
        i.read_excel(file_to_run)
    elif instance_type == 'pfile':
        i = i.unpickle_instance(file_to_run)
        i.DSSkillType = 'strictly-shared'
        if len(i.capabilityOfDoubleServices) > 0:
            # print('i.capabilityOfDoubleServices\n', i.capabilityOfDoubleServices)
            # print('i.capabilityOfDoubleServices type\n', type(i.capabilityOfDoubleServices))
            i.capabilityOfDoubleServices = np.ascontiguousarray(i.capabilityOfDoubleServices.reshape(-1))
            # print('reshaped')
    elif instance_type == 'mankowska':
        if quality_measure == 'default':
            quality_measure = 'mankowska'
        i.DSSkillType = 'strictly-shared'
        if mkVNS:
            i = Mk.generate_Mk(i, matfile=file_to_run, filetype='large_vns')
        else:
            i = Mk.generate_Mk(i, file_to_run)
    elif instance_type == 'ait_h':
        if quality_measure == 'default':
            quality_measure = 'ait_h'
        i = Mk.ait_h_to_c(file_to_run, i)
    elif instance_type == 'tsp':
        i = Mk.generate_Mk(i, matfile=file_to_run, filetype='tsptw')
        print('TSP format implementation needs review')
        return(-1)

    elif instance_type == 'braekers':
        i.read_braekers(file_to_run)


    # Set lambdas:
    i.lambda_1 = 1
    i.lambda_2 = 1
    i.lambda_3 = 1
    i.lambda_4 = 1
    i.lambda_5 = 1
    i.lambda_6 = 10
    # print('Skill type still is', i.DSSkillType)

    # Initialise the rest:
    # print('BEFORE INIT ', i.MAX_TIME_SECONDS)
    i.init()
    if (len(options_vector) == 100):
        i.algorithmOptions = options_vector
    else:
        i.algorithmOptions = default_options_vector()

    i.qualityMeasure = quality_measure
    if quality_measure == 'ait_h':
        i.algorithmOptions[0] = 0.0
    elif quality_measure == 'mankowska':
        i.algorithmOptions[0] = 1.0    
    elif quality_measure == 'paper':
        i.algorithmOptions[0] = 6.0
    elif quality_measure == 'workload_balance':
        i.algorithmOptions[0] = 5.0
    elif quality_measure == 'balanced':
        i.algorithmOptions[0] = 5.0

    i.verbose = verbose_level 
    i.MAX_TIME_SECONDS = max_time_seconds
    i.solMatrix = np.zeros((i.nNurses, i.nJobs), dtype=np.int32)

    return i
### --- End def prepare_instance --- ###

def worker_task(options_tuple, rSeed):

    # Unpack all the options:
    file_to_run, instance_type, quality_measure, ds_skill_type,\
    max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector = options_tuple

    i = prepare_instance(file_to_run, instance_type, quality_measure, ds_skill_type,
                         max_time_seconds, verbose_level, load_from_disk, mkVNS, options_vector)

    saved_od_data = i.od[0][0]
    print_all_call_data_python = options_vector[99] > 0
    i.solve(randomSeed=rSeed, printAllCallData=print_all_call_data_python)
    i.solve = []
    i.lib = []
    i.fun = []
    i.Cquality = -1*i.od[0][0]
    i.od[0][0] = saved_od_data
    # print('WORKER QUALITY ' + str(i.c_quality))
    # quality = i.full_solution_report(doPlots=False, report=0)
    # print('Another quality ' + str(quality))
    # print('i.od[0][0]  ' + str(i.od[0][0]))
    return i
### --- End def worker_task --- ###

def get_all_files_in_directory(directory, matching_extension='.'):
    file_list = []
    for r, d, f in os.walk(directory):
        for fi in f:
            if matching_extension in fi:
                file_list.append(fi)

    return file_list
### --- End def get_all_files_in_directory --- ###

def reverse_list_coords(wrongList):
    rightList = []
    for ii in wrongList:
        rightList.append([ii[1], ii[0]])
    return rightList
### --- End def reverse_list_coords --- ###

def list_matching(list1, list2, metric, maxMat=50000):
    numberOfSources = len(list1)
    numberOfTargets = len(list2)
    if numberOfSources + numberOfTargets > maxMat:
        print('The list call needs to be split to fit the maximum allowed of ' + str(maxMat))
        print('(Calling with ' + str(numberOfSources) + ' sources and ' + str(numberOfTargets) + ' targets)')
        if numberOfTargets < maxMat - 1:
            nel1 = maxMat - numberOfTargets - 1
            print('Making calls with ' + str(nel1) + ' elements from list1 and ' + str(numberOfTargets) + ' from list2')
            nSplits = float(numberOfSources)/float(nel1)
            nSplits = int(np.floor(nSplits))
            print('Expecting ' + str(nSplits) + ' recursive calls.')
            matchListGo = []
            matchListReturn = []
            stored = 0
            nCalls = 0
            while stored < numberOfSources:
                nCalls += 1
                print('\tProcessing call number ' + str(nCalls) + '...')
                miniList1 = list1[stored:int(min(stored+nel1,numberOfSources))]
                print('\t(call with minlist: ' + str(len(miniList1)) + ' targets: ' + str(len(list2)) + ')')
                miniGo, miniReturn = list_matching(miniList1, list2, metric,maxMat=maxMat)
                matchListGo += copy.deepcopy(miniGo)
                matchListReturn += copy.deepcopy(miniReturn)
                stored += nel1
                print('\tDone.')
            print('Performed ' + str(nCalls) + ' calls.')
            print('> :split exit')
            return [matchListGo, matchListReturn]
        else:
            print('ERROR: Nedd to split second / both lists, not yet implemented!')
            exit(-1)

    # superList = reverse_list_coords(list1 + list2)
    superList = list1 + list2

    l2idx = len(list1)

    [theMatrix, dummy, dummy]  = osrm.table(superList, output='np', annotations=metric)


    matchListGo = []
    matchListReturn = []
    for i in range(len(list1)):
        matchListGo.append(theMatrix[i,l2idx:])
        matchListReturn.append(theMatrix[l2idx:,i])
    
    return [matchListGo, matchListReturn]
### --- End def list_matching --- ###

class geopoint(object):
    def __init__(self, latitude, longitude):
        self.lat = latitude
        self.long = longitude
    def latlong(self):
        return([self.lat, self.long])
    def longlat(self):
        return([self.long, self.lat])
### --- End class geopoint  --- ###

def default_options_vector():
    ov = np.zeros(100, dtype=np.float64)
    ov[1] = 0.0 # Quality meausre (might be modified automatically for MK, default: Ait H.)
    ov[1] = 1.0 # Two-opt active
    ov[2] = 1.0 # 2 exchange active
    ov[3] = 0.0 # Nurse order change active (neighbourhood in local search)
    ov[4] = 0.05 # GRASP delta low
    ov[5] = 0.25 # GRASP delta range
    ov[6] = 1.0 # Nurse order change active (In GRASP, between calls)
    ov[7] = 1.0 # performPathRelinking
    ov[8] = 10.0 # Solutions in pool
    ov[9] = 1.0 # Binary, perform path relinking for every solution with one random solution in the pool
    ov[10] = 1.0 # GRASP: RCL strategy (1 or 2)

    # Weights of objective function (for "paper" measure)
    # $\alpha_1$      &   1   &   Travel time \\
    # $\alpha_2$      &   1   &   Waiting time \\
    # $\alpha_3$      &   5   &   Tardiness \\
    # $\alpha_4$      &   5   &   Overtime \\   
    # $\alpha_5$      &   $\frac{1}{4}$   &   Workload balance \\       
    # $\alpha_6$      &   1   &   Preference score \\  

    ov[50] = 1 # 1 if tardiness and overtime are infeasible, 0 if feasible
    ov[51] = -1.0 # alpha_1 Travel time
    ov[52] = -1.0 # alpha_2 Waiting time
    ov[53] = -5.0 # alpha_3 Tardiness
    ov[54] = -5.0 # alpha_4 Overtime
    ov[55] = 0.50 # alpha_5 Workload balance
    ov[56] = 1 # alpha_6 Preference score
    ov[57] = 0 # alpha_7 Max tardiness (not in paper)

    ov[99] = 0.0 # print all input data
    return ov
### --- End def default_options_vector --- ###

class JOB(object):
    def __init__(self):
        # Characteristics
        self.ID = ''
        self.serviceTime = 0
        self.doubleService = False
        self.dependsOn = []
        self.postcode = 'Unknown'
        self.latlong = []
        self.priority = 0
        self.hasTimewindow = False
        self.timewindow = [0,86400]
        self.hasPreferredTimewindow = False
        self.preferredTimewindow = [0, 86400]
        self.skillsRequired = []
        self.features = []
        self.preferences = []
        self.preferredCarers = []
        self.dependsOn = -1
        self.minimumGap = 0
        self.maximumGap = 0
        # Calculated (solution)
        self.assignedNurse = -1
        self.positionInSchedule = 0
        self.tardiness = 0
        self.waitingToStart = 0
        self.arrivalTime = 0
        self.departureTime = 0
### --- End class JOB --- ###        

class NURSE(object):
    def __init__(self):
        # Characteristics
        self.ID = ''
        self.startLocation = []
        self.transportMode = 'car'
        self.shiftTimes = [0, 86400]
        self.maxWorking = 86400
        self.skills = []
        self.features = []
        self.preferences = []
        self.preferredJobs = []
        self.jobsToAvoid = []

        # Calculated (solution)
        self.jobsServed = 0
        self.startTime = 0
        self.finishTime = 0
        self.waitingTime = 0
        self.serviceTime = 0
        self.overtimeWork = 0
        self.travelTime = 0
        self.route = []
### --- End class NURSE --- ###                       

def reverse_latlong(latlong):
    return [latlong[1], latlong[0]]
### --- End def reverse_latlong --- ###    

class INSTANCE(object):
    def __init__(self):
        self.nJobs = -1
        self.nNurses = -1
        self.nSkills = -1
        self.nurseWorkingTimes = []
        self.nurseSkills = []
        self.jobTimeInfo = []
        self.jobSkillsRequired = []
        self.prefScore = []
        self.algorithmOptions = np.zeros(100, dtype=np.float64)
        self.doubleService = []
        self.dependsOn = []
        self.od = [] # Main od matrix, contains times in minutes - this will be used in C
        self.odDist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.nurse_travel_from_depot = [] # Main nurse_travel_from_depot matrix, contains times in minutes - this will be used in C
        self.nurse_travel_to_depot = [] # Main nurse_travel_to_depot matrix, contains times in minutes - this will be used in C
        self.nurse_travel_from_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.nurse_travel_to_depot_dist = [] #----NOTE- NEW VAR: contains distance matrix in metres
        self.xy = [] # x y coordinates for plotting routes on a map LAT-LONG
        self.solMatrix = []
        self.MAX_TIME_SECONDS = 30
        self.verbose = 5
        self.secondsPerTU = 1
        self.name = 'solution0'
        self.full_file_name = 'solution0'
        self.loadFromDisk = False
        self.mankowskaQuality = -1
        self.Cquality = -1
        self.DSSkillType = 'shared-duplicated' # See overleaf document for details
        self.capabilityOfDoubleServices = []

        # Post-processed solution:
        self.nurseRoute = []
        self.nurseWaitingTime = []
        self.nurseServiceTime = []
        self.nurseTravelTime = []
        self.nurseTime = []
        self.totalWaitingTime = -1
        self.totalServiceTime = -1
        self.totalTravelTime = -1
        self.totalTardiness = -1
        self.nLateJobs = 0
        self.totalTime = -1
        self.qualityMeasure = 'standard'
        self.maxTardiness = -1
        self.c_quality = 0
        self.mk_mind = []
        self.mk_maxd = []

        # Preferences weights and parameters:
        self.M = 1000000000 # Large value for unskilled jobs
        self.lambda_1 = 1
        self.lambda_2 = 1
        self.lambda_3 = 1
        self.lambda_4 = 1
        self.lambda_5 = 1
        self.lambda_6 = 1

        ### Define call to C ###
        if os.name == "nt":
            library_name = "./bin/constructive.dll"
            # library_name = "./bin/bnc_solver.dll"
            self.lib = ctypes.cdll.LoadLibrary(library_name)
        else:
            library_name = "./constructivelib.so"
            self.lib = ctypes.CDLL(library_name, mode=ctypes.RTLD_GLOBAL)

        self.fun = self.lib.python_entry
        self.fun.restype = None
        # self.fun.argtypes = [ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),
        #                 ctypes.c_size_t]
        # Need to pass all this:
        # 	int nJobs_data
        # 	int nNurses_data
        # 	int nSkills_data
        # 	int verbose_data
        # 	double * od_data
        # 	int * nurseWorkingTimes_data
        # 	int * jobTimeInfo_data
        # 	int * jobRequirements_data
        # 	int * nurseSkills_data
        #   int * solMatrixPointer
        self.fun.argtypes = [ctypes.c_int, # nJobs_data,
                        ctypes.c_int, # nNurses_data,
                        ctypes.c_int, # nSkills_data,
                        ctypes.c_int, # verbose_data,
                        ctypes.c_float, # MAX_TIME_SECONDS
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * od_data
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * nurse_travel_from_depot
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"), # * nurse_travel_to_depot
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # nurseWorkingTimes_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # jobTimeInfo_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # jobRequirements_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"), # nurseSkills_data
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# solMatrixPointer
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# doubleService
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# dependsOn
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# mk_mind
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# mk_maxd
                        ndpointer(ctypes.c_int, flags="C_CONTIGUOUS"),# capabilityOfDoubleServices
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),# prefScore
                        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),# algorithmOptions
                        ctypes.c_int] # Random seed
    ### --- End def  __init__ --- ###                    

    def pickle(self, name):
        # print('Saving instance as ' + str(name) + '...')
        f = open(name,'wb')
        # Some stuff cannot be pickled:
        temp_data = [self.lib, self.fun]
        self.lib = []
        self.fun = []
        pickle.dump(self, f)
        f.close()
        self.lib, self.fun = temp_data
        # print('Done.')
    ### --- End def pickle --- ###    

    def unpickle_instance(self, name):
        print('Reading instance from ' + str(name) + '...')
        f = open(name,'rb')
        # Some stuff cannot be pickled:
        instance_obj = pickle.load(f)
        f.close()
        # self.nJobs = instance_obj.nJobs
        # self.nNurses = instance_obj.nNurses
        # self.nSkills = instance_obj.nSkills
        # self.verbose = instance_obj.verbose
        # self.MAX_TIME_SECONDS = instance_obj.MAX_TIME_SECONDS
        # self.od = instance_obj.od
        # self.nurse_travel_from_depot = instance_obj.nurse_travel_from_depot
        # self.nurse_travel_to_depot = instance_obj.nurse_travel_to_depot
        # self.nurseWorkingTimes = instance_obj.nurseWorkingTimes
        # self.jobTimeInfo = instance_obj.jobTimeInfo
        # self.jobSkillsRequired = instance_obj.jobSkillsRequired
        # self.nurseSkills = instance_obj.nurseSkills
        # self.solMatrix = instance_obj.solMatrix
        # self.doubleService = instance_obj.doubleService
        # self.dependsOn = instance_obj.dependsOn
        # self.mk_mind = instance_obj.mk_mind
        # self.mk_maxd = instance_obj.mk_maxd
        # self.capabilityOfDoubleServices = instance_obj.capabilityOfDoubleServices
        # self.prefScore = instance_obj.prefScore
        # self.algorithmOptions = instance_obj.algorithmOptions
        instance_obj.lib = self.lib
        instance_obj.fun = self.fun
        instance_obj.DSSkillType = 'strictly-shared'
        instance_obj.init() # NOTE: CHECK THIS
        # self.init()
        # print('Done.')
        return instance_obj
    ### --- End def unpickle_instance --- ###

    def preference_score(self, job, nurse):
        # Function determines preference score of given job and given nurse, where 'job' is a JOB object and 'nurse' is a NURSE object.
        carerOK = 0
        if len(job.preferredCarers) > 0: # If the job does have preferred carers
            carerOK = -1 # Penalise if there is someone on the list, but it's not met
            for pc in job.preferredCarers: # For each 'value' (string?) pc in preferredCarers list
                if pc.lower() == nurse.ID.lower(): # If the 'pc' index matched the ID of the nurse (nurse called in as parameter), then it means that this given job would prefer to have this nurse.
                    carerOK = 1 # nurse is okay for this job!
                    break

        
        matchedPrefsJob = 0
        unmatchedPrefsJob = 0 # NOTE: Always zero??
        for pref in job.preferences: # For each value (job preference characteristic?) in job.preferences list
            if pref in nurse.features: # If the preference characteristic is one of the nurse's features, i.e. nurse has the desired characteristic
                matchedPrefsJob = matchedPrefsJob + 1 # Increase preference score for job
            else:
                matchedPrefsJob = matchedPrefsJob - 1 # Decrease preference score for job

        matchedPrefsNurse = 0
        unmatchedPrefsNurse = 0 # NOTE: Always zero??
        for pref in nurse.preferences: # For each value (nurse preference characteristic?) in nurse.preferences list
            if pref in job.features: # If the preference characteristic is one of the job's features, i.e. job has the desired characteristic
                matchedPrefsNurse = matchedPrefsNurse + 1 # Increase preference score for nurse
            else:
                matchedPrefsNurse = matchedPrefsNurse - 1 # Decrease preference score for nurse

        jobToAvoid = 0
        for jta in nurse.jobsToAvoid: # For each value (job ID?) in nurse.jobsToAvoid list
            if jta.lower() == job.ID.lower(): # If the ID of the current job is in the jobsToAvoid list, then jobToAvoid = -1
                jobToAvoid = -1
                break

        # Calculate preference score
        score = (
                self.lambda_1*carerOK +
                self.lambda_2*matchedPrefsJob + self.lambda_3*unmatchedPrefsJob + 
                self.lambda_4*matchedPrefsNurse + self.lambda_5*unmatchedPrefsNurse +
                self.lambda_6*jobToAvoid
                )

        return(score)
    ### --- End def preference_score --- ###    

    def fill_preferences(self):
        self.prefScore = np.zeros((self.nJobs, self.nNurses), dtype=np.float64) # Matrix, nJobs x nNurses (note that this is the only variable with jobxnurse, not nursexjob dimensions).
        for i,job in enumerate(self.jobObjs): # For each JOB object in the jobObjs list
            for j,nurse in enumerate(self.nurseObjs): # For each NURSE object in the nurseObjs list
                self.prefScore[i][j] = self.preference_score(job, nurse) # Set the preference score of job i and nurse j
    ### --- End def fill_preferences --- ###

    def init_job_and_nurse_objects(self):
        # Create two lists containing objects of the classes JOBS and NURSES respectively.
        self.jobObjs = []
        for ii in range(self.nJobs):
            self.jobObjs.append(JOB())
            self.jobObjs[-1].ID = ii # Set the ID for the object just added to jobObjs to be the index of the object (its position in the jobObjs list).

        self.nurseObjs = []
        for ii in range(self.nNurses):
            self.nurseObjs.append(NURSE())
            self.nurseObjs[-1].ID = ii # Set the ID for the object just added to nurseObjs to be the index of the object (its position in the nurseObjs list).
    ### --- End def init_job_and_nurse_objects --- ###

    def init(self):
        int_type = np.int32
        # self.jobObjs = []
        # for ii in range(self.nJobs):
        #     self.jobObjs.append(JOB())
        #     self.jobObjs[-1].ID = ii
        #     if self.doubleService[ii]:
        #         self.jobObjs[-1].doubleService = True

        # self.nurseObjs = []
        # for ii in range(self.nNurses):
        #     self.nurseObjs.append(NURSE())
        #     self.nurseObjs[-1].ID = ii

        if len(self.mk_mind) == 0:
            self.mk_mind = list(np.zeros(self.nJobs))
            self.mk_maxd = list(np.zeros(self.nJobs))

        # Create capabilityOfDoubleServices
        nDS = np.sum(self.doubleService)
        # print('Setting skill capabilities for ' + str(nDS) + ' jobs.')
        # for i in range(self.nNurses):
        # 	print('\tSkills Nurse ' + str(i) + ': ' + str(self.nurseSkills[i]))
        
        # self.DSSkillType = 'shared-duplicated' 
        self.capabilityOfDoubleServices = np.zeros((self.nNurses, self.nNurses, nDS), dtype=int_type)
        # print('Matrix size: ' + str((self.nNurses, self.nNurses, nDS)))
        # This for loop, for each double service job, goes through each jobSkillRequirement row, finds the indices of the jobs that require skilled nurses for double service,
        # then checks through each pair of nurses i and j to see if they are skilled enough together to do the double service job together.
        # If they are, then capabilityOfDoubleServices[i,j,k] = 1, else = 0 (where k = the number of the double service job).
        k = -1 # Counts through each double service (nDS)
        for k_all_job, dsjob in enumerate(self.doubleService): # k_all_job = index, dsjob = value at index (e.g. k_all_job = 0, dsjob = self.doubleService[0])
            if dsjob < 1: # if job is not double service, continue
                continue
            k += 1 
            reqSkills = [] # Contains indices of jobSkillsRequired of jobs that need skilled nurse, >0
            for s, req in enumerate(self.jobSkillsRequired[k_all_job]): # s = index, req = value at index s
                if req > 0: # if jobSkillsRequired > 0 (=1?), add index s to reqSkills
                    reqSkills.append(s)
            # print('* Job ' + str(k_all_job) + '(ds index ' + str(k) + ') required skills: ' + str(reqSkills))
            for i in range(self.nNurses):
                # print('\tSkills i ' + str(i) + ': ' + str(self.nurseSkills[i]))
                for j in range(self.nNurses):
                    if i == j:
                        continue
                    # print('\t\tSkills j ' + str(j) + ': ' + str(self.nurseSkills[j]))
                    if self.DSSkillType == 'shared-duplicated':
                        # Check for repeated skills
                        print('Warning: Skill type "shared-duplicated" not implemented yet')
                        exit(-21234)
                    elif self.DSSkillType == 'shared':
                        # print('SHSTRF')
                        self.capabilityOfDoubleServices[i,j,k] = 1
                        for s in reqSkills:
                            if self.nurseSkills[i][s] + self.nurseSkills[j][s] < 1:
                                self.capabilityOfDoubleServices[i,j,k] = 0
                                break
                    elif self.DSSkillType == 'duplicated':
                        # Nothing
                        print('Warning: Skill type "duplicated" not implemented yet')
                        exit(-21234)
                    elif self.DSSkillType == 'strictly-shared':
                        # print('strictlySHSTRF')

                        # Mankowska
                        somethingOnI = False
                        somethingOnJ = False
                        self.capabilityOfDoubleServices[i,j,k] = 1
                        # Go through each JOB s and check if current nurse i and current nurse j can do any of the jobs s in reqSkills
                        for s in reqSkills: #for each value in reqSkills
                            if self.nurseSkills[i][s] + self.nurseSkills[j][s] > 0: # If nurse i and nurse j can together do job s
                                if self.nurseSkills[i][s] > 0: # If nurse i can do job s
                                    somethingOnI = True
                                if self.nurseSkills[j][s] > 0: # If nurse j can do job s
                                    somethingOnJ = True
                            else:
                                self.capabilityOfDoubleServices[i,j,k] = 0 # If nurse i and nurse j cannot do job s together, not capable, so = 0
                                break # If nursei + nursej = 0, then neither of them have skill = 1, so there's no point in checking all the other jobs s in reqSkills, just break.
                        if (not (somethingOnJ and somethingOnI)): # If both are false, then nurse i and nurse j cannot do the double service, so set it to 0.
                            self.capabilityOfDoubleServices[i,j,k] = 0
                    else:
                        print('ERROR: Type of skill required for double services "' + self.DSSkillType + '" not understood.')
                        print('Please, use one of the following:')
                        print('\t"shared-duplicated"')
                        print('\t"shared"')
                        print('\t"duplicated"')
                        print('\t"strictly-shared"')
                        exit(-2000)
        # This for loop goes through all double services and checks that there exists a pair of nurses that are skilled enough to perform the double service together
        # This ensures that every job can be fulfilled successfully - otherwise there might be a job that cannot be fulfilled, that patient may have to be removed.
        k = -1
        for k_all_job, dsjob in enumerate(self.doubleService):
            if dsjob < 1:
                continue
            k += 1
            # print( 'Shape of self.capabilityOfDoubleServices[:] = ' + str(self.capabilityOfDoubleServices[:].shape))
            # print( 'Shape of self.capabilityOfDoubleServices[:][:] = ' + str(self.capabilityOfDoubleServices[:][:].shape))
            # print( 'Shape of self.capabilityOfDoubleServices[:][:][' + str(k) + '] = ' + str(self.capabilityOfDoubleServices[:][:][k].shape))
            jobServed = False
            for nursei in range(self.nNurses):
                for nursej in range(self.nNurses):
                    if self.capabilityOfDoubleServices[nursei][nursej][k] > 0.1:
                        jobServed = True
                        break
                if jobServed:
                    break

            # if not self.capabilityOfDoubleServices[:][:][k].any():
            if not jobServed:
                print('ERROR: There is no qualified pair of nurses for job ' + str(k_all_job))
                print('Required skills: ' + str(self.jobSkillsRequired[k_all_job]))
                print('Available nurses: ')
                for i in range(self.nNurses):
                    print('\tN' + str(i) + ' skills: ' + str(self.nurseSkills[i]))
                exit(-123234235346)
        # This should not be called if using 'unpickle_instance', as fill_preferences needs JOB and NURSE objects which are only created in init_job_and_nurse_objects function, which
        # is not called in unpickle_instance.
        if len(self.prefScore) < 1: 
            self.fill_preferences()
    ### --- End def init --- ###        

    def read_excel(self, excelFile):
        # Open file and check quality:
        xl = pd.ExcelFile(excelFile)

        # Check the sheet names
        if 'Jobs' not in xl.sheet_names or 'Staff' not in xl.sheet_names:
            print('ERROR: The program expects one sheet from ' + excelFile +
             ' to be called "Jobs" and another "Staff", instead we got:' + str(xl.sheet_names) + '.\nProgram terminated.')
            exit(-1)
        dfJobs = xl.parse('Jobs')
        dfStaff = xl.parse('Staff')

        self.nJobs = dfJobs.shape[0] - 1
        self.nNurses = dfStaff.shape[0] - 1
        self.nSkills = 0
        self.init_job_and_nurse_objects()
        print('There are ', self.nJobs, ' jobs and ', self.nNurses, ' nurses in ', excelFile)
        # print('There are %d jobs and %d nurses in %s' % (self.nJobs, self.nNurses, excelFile)) # C style?? remove.
        ii = -1
        for lineno,row in dfJobs.iterrows():
            # print(row)
            if lineno == 0:
                continue
            ii = ii + 1
            try:
                self.jobObjs[ii].ID = str(row[0])
            except Exception as e:
                ii = ii - 1
                print('Could not read line... It was:' + str(row))
                print(e)
                print(ii)
                print(self.jobObjs[ii])
                continue 
            self.jobObjs[ii].postcode = str(row[1])
            latLong = str(row[2]).split(',')
            # print(latLong)
            self.jobObjs[ii].location = geopoint(float(latLong[0]), float(latLong[1]))
            self.jobObjs[ii].priority = int(row[3])
            self.jobObjs[ii].serviceTime = float(row[4])
            if pd.isnull(row[5]) or pd.isnull(row[6]):
                self.jobObjs[ii].hasTimewindow = False
                self.jobObjs[ii].timewindow = [0, 24*3600]
            else:
                self.jobObjs[ii].hasTimewindow = True
                self.jobObjs[ii].timewindow = [float(row[5]), float(row[6])]

            if pd.isnull(row[7]) or pd.isnull(row[8]):
                self.jobObjs[ii].hasPreferredTimewindow = False
                self.jobObjs[ii].preferredTimewindow = [0, 24*3600]
            else:
                self.jobObjs[ii].hasPreferredTimewindow = True
                self.jobObjs[ii].preferredTimewindow = [float(row[7]), float(row[8])]
            
            self.jobObjs[ii].skillsRequired = []
            if not pd.isnull(row[9]):
                sklist = str(row[9]).split(',')
                for sk in sklist:
                    self.jobObjs[ii].skillsRequired.append(int(sk))
                self.nSkills = max(self.nSkills, max(self.jobObjs[ii].skillsRequired))

            if str(row[10]).lower() in ['yes', 'y', '1']:
                self.jobObjs[ii].doubleService = True
            else:
                self.jobObjs[ii].doubleService = False

            self.jobObjs[ii].features = []
            if not pd.isnull(row[11]):
                sklist = str(row[11]).split(',')
                for sk in sklist:
                    self.jobObjs[ii].features.append(int(sk))

            self.jobObjs[ii].preferences = []
            if not pd.isnull(row[12]):
                sklist = str(row[12]).split(',')
                for sk in sklist:
                    self.jobObjs[ii].preferences.append(int(sk))

            self.jobObjs[ii].preferredCarers = []
            if not pd.isnull(row[13]):
                sklist = str(row[13]).split(',')
                for sk in sklist:
                    self.jobObjs[ii].preferredCarers.append(str(sk))
        
            self.jobObjs[ii].dependsOn = -1
            if not pd.isnull(row[14]):
                sklist = str(row[14]).split(',')
                self.jobObjs[ii].dependsOn = str(sklist[0])
                # for sk in sklist:
                # 	self.jobObjs[ii].dependsOn.append(str(sk))

            self.jobObjs[ii].minimumGap = []
            if not pd.isnull(row[15]):
                sklist = str(row[15]).split(',')
                try:
                    self.jobObjs[ii].minimumGap = float(sklist[0])
                except Exception as e:
                    print('WARNING: ' + str(e))
                    print(sklist[0])
                # for sk in sklist:
                    # self.jobObjs[ii].minimumGap.append(float(sk))

            self.jobObjs[ii].maximumGap = []
            if not pd.isnull(row[16]):
                sklist = str(row[16]).split(',')
                try:
                    self.jobObjs[ii].maximumGap = float(sklist[0])
                except Exception as e:
                    print('WARNING: ' + str(e))
                    print(sklist[0])
                # for sk in sklist:
                # 	self.jobObjs[ii].maximumGap.append(float(sk))
        # Read nurses:
        ii = -1
        for lineno,row in dfStaff.iterrows():
            # print(row)
            if lineno == 0:
                continue
            ii = ii + 1
            try:
                self.nurseObjs[ii].ID = str(row[0])
            except Exception as e:
                print('Could not read line... It was:' + str(row))
                print(e)
                print(ii)
                print(self.nurseObjs[ii])
                continue	
            self.nurseObjs[ii].postcode = str(row[1])
            latLong = str(row[2]).split(',')
            self.nurseObjs[ii].startLocation = geopoint(float(latLong[0]), float(latLong[1]))
            self.nurseObjs[ii].transportMode = str(row[3]).lower()
            if self.nurseObjs[ii].transportMode not in ['car', 'bike', 'walk', 'public']:
                print('WARNING: Unknown transport mode: %s, should be one of the following: "car", "bike", "walk", "public"' % self.nurseObjs[ii].transportMode)
                print('WARNING: Transport mode set to "car" for nurse' + str(ii))
                self.nurseObjs[ii].transportMode = 'car'

            if pd.isnull(row[4]) or pd.isnull(row[5]):
                print('ERROR: Shift information missing for nurse ' + str(ii) + '\nProgram terminated.')
                exit(-1)
            else:
                self.nurseObjs[ii].shiftTimes = [float(row[4]), float(row[5])]

            self.nurseObjs[ii].maxWorking = float(row[6])

            self.nurseObjs[ii].skills = []
            if not pd.isnull(row[7]):
                sklist = str(row[7]).split(',')
                for sk in sklist:
                    self.nurseObjs[ii].skills.append(int(sk))
                self.nSkills = max(self.nSkills, max(self.nurseObjs[ii].skills))

            self.nurseObjs[ii].features = []
            if not pd.isnull(row[8]):
                sklist = str(row[8]).split(',')
                for sk in sklist:
                    self.nurseObjs[ii].features.append(int(sk))

            self.nurseObjs[ii].preferences = []
            if not pd.isnull(row[9]):
                sklist = str(row[9]).split(',')
                for sk in sklist:
                    self.nurseObjs[ii].preferences.append(int(sk))

            self.nurseObjs[ii].preferredJobs = []
            if not pd.isnull(row[10]):
                sklist = str(row[10]).split(',')
                for sk in sklist:
                    self.nurseObjs[ii].preferredJobs.append(str(sk))

            self.nurseObjs[ii].jobsToAvoid = []
            if not pd.isnull(row[11]):
                sklist = str(row[11]).split(',')
                for sk in sklist:
                    self.nurseObjs[ii].jobsToAvoid.append(str(sk))

        self.xy = []
        self.xy.append(self.nurseObjs[0].startLocation.longlat())
        for jo in self.jobObjs:
            self.xy.append(jo.location.longlat())
        # print('Number of skills: ' + str(self.nSkills))
        # print('Doing jobs:')
        # for jobb in self.jobObjs:
        # 	print('\n\nID = ' + str(jobb.ID))
        # 	print('serviceTime = ' + str(jobb.serviceTime))
        # 	print('doubleService = ' + str(jobb.doubleService))
        # 	print('postcode = ' + str(jobb.postcode))
        # 	print('latlong = ' + str(jobb.latlong))
        # 	print('priority = ' + str(jobb.priority))
        # 	print('hasTimewindow = ' + str(jobb.hasTimewindow))
        # 	print('timewindow = ' + str(jobb.timewindow))
        # 	print('hasPreferredTimewindow = ' + str(jobb.hasPreferredTimewindow))
        # 	print('preferredTimewindow = ' + str(jobb.preferredTimewindow))
        # 	print('skillsRequired = ' + str(jobb.skillsRequired))
        # 	print('features = ' + str(jobb.features))
        # 	print('preferences = ' + str(jobb.preferences))
        # 	print('preferredCarers = ' + str(jobb.preferredCarers))
        # 	print('dependsOn = ' + str(jobb.dependsOn))
        # 	print('minimumGap = ' + str(jobb.minimumGap))
        # 	print('maximumGap = ' + str(jobb.maximumGap))

        # print('Doing Nurses:')
        # for nurse in self.nurseObjs:
        # 	print('\nID = ' + str(nurse.ID))
        # 	print('startLocation = ' + str(nurse.startLocation))
        # 	print('transportMode = ' + str(nurse.transportMode))
        # 	print('shiftTimes = ' + str(nurse.shiftTimes))
        # 	print('maxWorking = ' + str(nurse.maxWorking))
        # 	print('skills = ' + str(nurse.skills))
        # 	print('features = ' + str(nurse.features))
        # 	print('preferences = ' + str(nurse.preferences))
        # 	print('preferredJobs = ' + str(nurse.preferredJobs))
        # 	print('jobsToAvoid = ' + str(nurse.jobsToAvoid))

        # print('Preference scores:')
        # print(self.prefScore)
        # print('Done.')
        self.fill_preferences()
        self.objs_info_to_c_format()
    ### --- End def read_excel --- ###

    def objs_info_to_c_format(self):
        int_type = np.int32
        loadFromDisk = self.loadFromDisk
        odMatrixName = self.full_file_name + '_' + 'driving' + '_times.npy'
        fromNurseDepotMatrixName = self.full_file_name + '_' + 'from_nurse_depot' + '_times.npy'
        toNurseDepotMatrixName = self.full_file_name + '_' + 'to_nurse_depot' + '_times.npy'
        self.nurseWorkingTimes = np.zeros((self.nNurses, 3), dtype=int_type)
        # self.jobSkillsRequired = np.zeros((self.nNurses, self.nSkills), dtype=int_type)
        self.nurseSkills = np.zeros((self.nNurses, self.nSkills), dtype=int_type)
    
        for n,nurse in enumerate(self.nurseObjs):
            self.nurseWorkingTimes[n][0] = nurse.shiftTimes[0]
            self.nurseWorkingTimes[n][1] = nurse.shiftTimes[1]
            self.nurseWorkingTimes[n][2] = nurse.maxWorking
            for ss in nurse.skills:
                s = int(ss) - 1
                self.nurseSkills[n][s] = 1
            # for s in range(self.nSkills):
            # 	if s in nurse.skills:
            # 		self.nurseSkills[n][s - 1] = 1
        self.jobTimeInfo = np.zeros((self.nJobs, 3), dtype=int_type)
        self.dependsOn = np.zeros(self.nJobs, dtype=int_type) - 1
        self.mk_maxd = np.zeros(self.nJobs, dtype=int_type)
        self.mk_mind = np.zeros(self.nJobs, dtype=int_type)

        self.jobPrefTime = np.zeros((self.nJobs, 3), dtype=int_type) - 1
        self.jobSkillsRequired = np.zeros((self.nJobs, self.nSkills), dtype=int_type)
        self.doubleService = np.zeros(self.nJobs, dtype=int_type)
        for i,job in enumerate(self.jobObjs):
            if job.doubleService:
                self.doubleService[i] = 1
            self.jobTimeInfo[i][0] = job.timewindow[0]
            self.jobTimeInfo[i][1] = job.timewindow[1]
            self.jobTimeInfo[i][2] = job.serviceTime
            try:
                basestring
            except NameError:
                basestring = str

            if isinstance(job.dependsOn, basestring):
                for ii,jobii in enumerate(self.jobObjs):
                    if job.dependsOn.lower() == jobii.ID.lower():
                        self.dependsOn[i] = ii
                self.mk_mind = job.minimumGap
                self.mk_maxd = job.maximumGap
                # self.jobDependency[i][1] = job.minimumGap
                # self.jobDependency[i][2] = job.maximumGap

            if job.hasPreferredTimewindow > -1:
                self.jobPrefTime[i][0] = 1
                self.jobPrefTime[i][1] = job.preferredTimewindow[0]
                self.jobPrefTime[i][2] = job.preferredTimewindow[1]

            for ss in job.skillsRequired:
                s = int(ss) - 1
                self.jobSkillsRequired[i][s] = 1
            # for s in range(self.nSkills):
            # 	if s in job.skillsRequired:
            # 		self.jobSkillsRequired[i][s] = 1
        
        # od = np.zeros((nJobs + 1, nJobs + 1))
        # toDepot = np.zeros(nJobs)
        # fromDepot = np.zeros(nJobs)
        # return([nJobs, nNurses, maxSkillLevel, nurseWorkingTimes, skArr, jobTimeInfo, jobSkillsRequired, od, secondsPerTU])
        # self.doubleService = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.solMatrix = np.zeros((self.nNurses, self.nJobs), dtype=int_type)

        # #### Deal with OD-Matrix:

        # print('Job skills req:')
        # print(self.jobSkillsRequired)

        # print('\nNurse skills:')
        # print(self.nurseSkills)
        # exit(-1)

        if loadFromDisk:
            odMatrix = np.load(odMatrixName)
            nursesGo = np.load(fromNurseDepotMatrixName)
            nursesReturn = np.load(toNurseDepotMatrixName)
        else:
            # Compute matrix:	
            longlats = self.xy
            ids = []
            for obj in range(self.nJobs + 1):
                ids.append(obj)

            print('Creating distance matrix...')
            # carServerHost = 'http://localhost:5000/'
            # walkServerHost = 'http://localhost:5001/'
            carServerHost = 'uos-212244.clients.soton.ac.uk:5000'
            walkServerHost = 'uos-212244.clients.soton.ac.uk:5001'

            osrm.RequestConfig.host = carServerHost
            
            # print(longlats)
            # travelTimes = osrm.table(longlats, ids_origin=ids, output='dataframe')
            # odMatrix = travelTimes[0].values
            [odMatrix, dummy, dummy]  = osrm.table(longlats, ids_origin=ids, output='np', annotations='duration')

            np.save(odMatrixName, odMatrix)
            print('Saved: ' + odMatrixName)

            # Create two lists with travel info of nurses from their starting location to/from other locations
            nurse_start_locations = []
            for no in self.nurseObjs:
                nurse_start_locations.append(no.startLocation.longlat())

            nursesGo, nursesReturn = list_matching(nurse_start_locations, self.xy[1:], 'duration')
            # print('Nurses going to depot')
            # print(nursesGo)
            # print('Nurses returning to depot')
            # print(nursesReturn)
            nursesGo = np.array(nursesGo)
            nursesReturn = np.array(nursesReturn)

            print('Saving ', fromNurseDepotMatrixName, ' and ', toNurseDepotMatrixName, '...')
            np.save(fromNurseDepotMatrixName, nursesGo)
            np.save(toNurseDepotMatrixName, nursesReturn)

        self.od = np.asarray(odMatrix, dtype=np.float64)	
        self.nurse_travel_from_depot = np.asarray(nursesGo, dtype=np.float64)
        self.nurse_travel_to_depot = np.asarray(nursesReturn, dtype=np.float64)

        self.prefScore = np.asarray(self.prefScore, dtype=np.float64)
        self.algorithmOptions = np.asarray(self.algorithmOptions, dtype=np.float64)
        
        self.capabilityOfDoubleServices = np.asarray(self.capabilityOfDoubleServices, dtype=int_type)
        self.dependsOn = np.asarray(self.dependsOn, dtype=int_type)

        # print('\nnurseWorkingTimes = ')
        # print(self.nurseWorkingTimes)

        # print('\nnurseSkills = ')
        # print(self.nurseSkills)

        # print('\njobTimeInfo = ')
        # print(self.jobTimeInfo)

        # print('\njobDependency = ')
        # print(self.jobDependency)

        # print('\njobPrefTime = ')
        # print(self.jobPrefTime)

        # print('\njobSkillsRequired = ')
        # print(self.jobSkillsRequired)

        # print('\nsolMatrix = ')
        # print(self.solMatrix)
    ### --- End def objs_info_to_c_format --- ###

    def time_to_string(self, quantity):
        seconds = quantity
        if seconds < 0:
            seconds = 0
        hours = seconds // 3600
        minutes = (seconds % 3600) // 60
        seconds = seconds % 60
        return('{0:0>2}:{1:0>2}:{2:0>2}'.format(int(hours), int(minutes), int(seconds)))

        # zeroDate = datetime.datetime(2018, 12, 31)
        # q_in_seconds = self.secondsPerTU*quantity
        # realTime = zeroDate + timedelta(seconds=q_in_seconds)
        # return(realTime.strftime("%H:%M:%S"))
    ### --- End def time_to_string --- ###

    def read_from_csvs(self, patientFile, nurseFile):
        self.nJobs = -1
        self.nNurses = -1
        self.nSkills = -1
        self.nurseWorkingTimes = []
        self.nurseSkills = []
        self.jobTimeInfo = []
        self.jobSkillsRequired = []
        self.od = []
        self.xy = [] # x y coordinates for plotting routes on a map

        self.secondsPerTU = 1
        # Count lines first:
        with open(patientFile) as csvfile:
            rwCter = csv.reader(csvfile)
            self.nJobs = sum(1 for row in rwCter) - 1

        # Count lines first:
        with open(nurseFile) as csvfile:
            rwCter = csv.reader(csvfile)
            self.nNurses = sum(1 for row in rwCter)

        # Count lines first:
        with open(patientFile) as csvfile:
            rwCter = csv.reader(csvfile)
            r0 = rwCter.readline()
            nSkillsJob = len(r0) - 7

        # Count lines first:
        with open(nurseFile) as csvfile:
            rwCter = csv.reader(csvfile)
            r0 = rwCter.readline()
            nSkillsNurse = len(r0) - 5

        if nSkillsJob != nSkillsNurse:
            print('ERROR: Nurses have ' + str(nSkillsNurse) + ' skills, but jobs require ' + str(nSkillsJob))
            exit()

        self.nSkills = nSkillsJob

        # COORDINATES	ServiceTIME	TW	TWS	TWD	TWE	SKILL1	SKILL2	SKILL3	DS
        # with open(patientFile) as csvfile:
        # 	reader = csv.reader(csvfile)
        # 	count = 0
        # 	for row in reader:
        # 		if count < 1: # Skip header
        # 			continue
        # 		if pitem.Type == row[6]:
    ### --- End def read_from_csvs --- ###

    def read_braekers(self, fname):
        [nJobs, nNurses, nSkills, nurseWorkingTimes, nurseSkills, jobTimeInfo, jobSkillsRequired, od, secondsPerTU] = read_braekers(fname)
        self.nJobs = nJobs
        self.nNurses = nNurses
        self.nSkills = nSkills
        self.nurseWorkingTimes = nurseWorkingTimes
        self.nurseSkills = nurseSkills
        self.jobTimeInfo = jobTimeInfo
        self.jobSkillsRequired = jobSkillsRequired
        self.od = od
        self.solMatrix = np.ones((nNurses, nJobs), dtype=np.int)
    ### --- End def read_braekers --- ###

    def simple_solution_plot(self, filename='none'):
        # Check if plots are available:
        if (self.nurseObjs[0].startLocation == []):
            for no in self.nurseObjs:
                no.startLocation = geopoint(self.xy[0][1], self.xy[0][0])
            # print('*** WARNING: Plots not available for this instance. Skipping... ***')
            # return

        fig, ax = plt.subplots()
        ncount = -1
        nLegends = []
        for nursej in range(self.nNurses):
            ncount += 1
            nLegends.append('R' + str(ncount))
            nLegends.append('N' + str(ncount) + ', S:' + str(self.nurseSkills[ncount]))
            routeCol = clusterColour(nursej)
            # foliumRouteLayers.append(folium.map.FeatureGroup(name='Nurse ' + str(nursej)))
            nRoute = []
            nRouteRev = []

            nRoute.append(self.nurseObjs[nursej].startLocation.longlat())
            nr = self.get_nurse_route(nursej)
            for pt in range(self.nJobs):
                if nr[pt] < -0.1:
                    break
                nRoute.append(self.xy[int(nr[pt] + 1)])
                # nRouteRev.append(reverse_latlong(rxy[int(nr[pt] + 1)]))
                # Add a Marker:
                # popupVal = 'Patient: ' + str(nr[pt]) + '<br>Service time: ' + str(self.jobTimeInfo[int(nr[pt])][2])
                # popupVal = popupVal + '<br>Nurse: ' + str(nursej)
                # foliumRouteLayers[-1].add_child(folium.CircleMarker(rxy[int(nr[pt] + 1)],
                # radius=10,
                # popup= popupVal,
                # color=routeCol,
                # fill_color=routeCol))

            nRoute.append(self.nurseObjs[nursej].startLocation.longlat())
            x,y = zip(*nRoute)
            ax.plot(x, y, 'ko', ms=10)
            ax.plot(x, y, routeCol)
            ax.plot(self.nurseObjs[nursej].startLocation.longlat()[0], self.nurseObjs[nursej].startLocation.longlat()[1], 'rs', ms=10)

        
        for	ptidx,pt in enumerate(self.xy):
            if ptidx > 0:
                textlabel = "P(%d) " % (ptidx - 1) #+ str(self.jobTimeInfo[ptidx - 1])
                if self.doubleService[ptidx - 1]:
                    textlabel += '\nDS ' + str(self.jobSkillsRequired[ptidx - 1])
            else:
                textlabel = 'DEPOT'
            plt.text(pt[0], pt[1], textlabel, rotation=0, verticalalignment='center',color='blue', fontsize=14)

        ax.legend(tuple(nLegends))
        ax.set_title("Routes for " + str(self.nNurses) + " nurses", fontsize=18)
        ax.grid(True, color='gray', alpha=0.5)
        ax.axis('equal')

        if filename == 'none':
            plt.show(block=False)
        else:
            plt.savefig(filename + '_route' + '.png', bbox_inches='tight')
    ### --- End def simple_solution_plot --- ###

    def solution_to_website(self, filename='sdfasdfasdfasdfas'):
        # Check if website generation is available:
        if (self.nurseObjs[0].startLocation == []):
            print('*** WARNING: Website generation is not available for this instance. Skipping... ***')
            return
        import folium
        webFilename = 'unknown.html'
        if filename == 'sdfasdfasdfasdfas':
            webFilename = self.name + '.html'
        webFilename = os.path.join(os.getcwd(), webFilename)
        print('Website filename: '  + str(webFilename))
        rxy = []
        for pt in self.xy:
            rxy.append(reverse_latlong(pt))
        m = folium.Map(location=rxy[0], zoom_start=14, tiles='cartodbpositron')
        # folium.TileLayer('openstreetmap').add_to(m)
        # folium.TileLayer('Mapbox Light', attr='Mapbox',tiles='https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw').add_to(m)
        # folium.TileLayer('Mapbox Control Room', attr='Mapbox',tiles='https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw').add_to(m)
        # folium.TileLayer('cartodbdark_matter').add_to(m)
        # folium.TileLayer('Stamen Toner').add_to(m)
        # m = folium.Map(location=rxy[0],
        # zoom_start=12,
        # tiles='https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw',
        # attr='Mapbox')
        # for i,pt in enumerate(rxy):
        # 	popupVal = 'Depot'
        # 	if i > 0:
        # 		popupVal = 'Patient: ' + str(i - 1) + '<br>Service time: ' + str(self.jobTimeInfo[i - 1][2])
        # 	folium.Marker(
        #     location=pt,
        #     popup= popupVal).add_to(m)#,
            #icon=folium.Icon(icon='cloud)'
        # folium.PolyLine(points).add_to(my_map)

        foliumRouteLayers = []
        for nursej in range(self.nNurses):
            routeCol = clusterColour(nursej)
            foliumRouteLayers.append(folium.map.FeatureGroup(name='Nurse ' + str(nursej)))
            nRoute = []
            nRouteRev = []
            # nRoute.append(tuple(rxy[0]))

            nr = self.get_nurse_route(nursej)
            for pt in range(self.nJobs):
                idxNRpt = int(nr[pt])
                if nr[pt] < -0.1:
                    break
                nRoute.append(tuple(rxy[idxNRpt + 1]))
                nRouteRev.append(reverse_latlong(rxy[idxNRpt + 1]))
                # Add a Marker:
                popupVal = '<b>Service ID:</b> ' + str(self.jobObjs[idxNRpt].ID) # +  '<br><b>Service time:</b> ' + str(self.jobTimeInfo[int(idxNRpt)][2])
                popupVal = popupVal + '<br><b>Service no.:</b> ' + str(idxNRpt) # +  '<br><b>Service time:</b> ' + str(self.jobTimeInfo[int(idxNRpt)][2])
                popupVal += '<br><b>Postcode:</b> ' + str(self.jobObjs[idxNRpt].postcode) # +  '<br><b>Service time:</b> ' + str(self.jobTimeInfo[int(idxNRpt)][2])
                if self.jobObjs[idxNRpt].doubleService:
                    popupVal = popupVal + ' (Double service)'
                popupVal = popupVal + '<br><b>Assigned nurse:</b> ' + str(nursej) # +  '(' + self.nurseObjs[nursej].ID + ')'
                popupVal = popupVal + '<br><b>Arrival time:</b> ' + self.time_to_string(self.jobObjs[idxNRpt].arrivalTime) # +  '(' + str(self.jobObjs[idxNRpt].arrivalTime) + ')'
                popupVal = popupVal + '<br><b>Departure Time:</b> ' + self.time_to_string(self.jobObjs[idxNRpt].departureTime) # +  '(' + str(self.jobObjs[idxNRpt].departureTime) + ')'
                popupVal = popupVal + '<br><b>Time Window:</b> ' + self.time_to_string(self.jobTimeInfo[int(idxNRpt)][0]) + ' - ' + self.time_to_string(self.jobTimeInfo[int(idxNRpt)][1])
                # popupVal = popupVal + '<br><b>Time Window:</b> ' + str(self.jobTimeInfo[int(idxNRpt)][0]) + ' - ' + str(self.jobTimeInfo[int(idxNRpt)][1])
                border_colour = routeCol
                if 	self.jobObjs[idxNRpt].tardiness > 0:
                    popupVal = popupVal + '<br><b>*** Missed by:</b> ' + self.time_to_string(self.jobObjs[idxNRpt].tardiness) # +  '(' + str(self.jobObjs[idxNRpt].tardiness) + ')'
                    border_colour = '#ff0000'

                if self.jobObjs[idxNRpt].waitingToStart	> 0:
                    popupVal = popupVal + '<br><b>Waiting before start:</b> ' + self.time_to_string(self.jobObjs[idxNRpt].waitingToStart) # +  '(' + str(self.jobObjs[idxNRpt].waitingToStart) + ')'
                popupVal = popupVal + '<br><b>serviceTime:</b> ' + self.time_to_string(self.jobObjs[idxNRpt].serviceTime) # +  '(' + str(self.jobObjs[idxNRpt].serviceTime) + ')'
                # popupVal = popupVal + '<br><b>assignedNurse:</b> ' + str(self.jobObjs[idxNRpt].assignedNurse)
                popupVal = popupVal + '<br><b>positionInSchedule:</b> ' + str(self.jobObjs[idxNRpt].positionInSchedule)
                popupVal = popupVal + '<br><b>skillsRequired:</b> ' + str(self.jobObjs[idxNRpt].skillsRequired)
                foliumRouteLayers[-1].add_child(folium.Circle(rxy[idxNRpt + 1],
                radius=30,
                popup= popupVal,
                color=border_colour,
                fill_color=routeCol,
                fill_opacity=0.5,
                fill=True,
                ))

            # nRoute.append(tuple(rxy[0]))
            # nRoute = tuple(nRoute)
            # [routeList, dur, dist] = route_these_points(tuple(rxy[0]), tuple(rxy[0]), goingThrough=nRoute)
            # routeList, dur, dist = route_these_points(reverse_latlong(rxy[0]), reverse_latlong(rxy[0]), goingThrough=nRouteRev)
            routeList, dur, dist = route_these_points(self.nurseObjs[nursej].startLocation.longlat(),
                                                        self.nurseObjs[nursej].startLocation.longlat(),
                                                        goingThrough=nRouteRev)

            # print('Nurse ' + str(nursej) + ' route: ')
            # print(routeList)
            # print('Provided was: ')
            # print(nRoute)
            foliumRouteLayers[-1].add_child(folium.PolyLine(routeList, color=routeCol, weight=2.5, opacity=1))
            m.add_child(foliumRouteLayers[-1])
        


        # Add some info bottom-left:
        lht =   '''
                <div style="position: fixed;
                            background-color: rgba(255, 255, 255, 0.6);
                            bottom: 50px; left: 50px; width: 350px; height: 600px; 
                            z-index:9999; font-size:14px;
                            border:1px solid grey;
                            overflow: auto;
                            ">              
                '''
                # &nbsp; Cool Legend <br>
                #               &nbsp; East &nbsp; <i class="fa fa-map-marker fa-2x" style="color:green"></i><br>
                #               &nbsp; West &nbsp; <i class="fa fa-map-marker fa-2x" style="color:red"></i>
        lht = lht + '''&nbsp; <b><u>Solution summary</u></b><br>'''
        lht = lht + '''&nbsp; <b>Total time: </b>''' + self.time_to_string(self.totalTime) + ''', of which:<br>'''
        lht = lht + '''&nbsp; <i> - Travel time: </i>''' + self.time_to_string(self.totalTravelTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Service time: </i>''' + self.time_to_string(self.totalServiceTime) + '''<br>'''
        lht = lht + '''&nbsp; <i> - Waiting time: </i>''' + self.time_to_string(self.totalWaitingTime) + '''<br><br>'''

        nursePart = '''&nbsp; <b><u>Nurse breakdown:</u> </b><br>'''
        for i, nn in enumerate(self.nurseObjs):
            nursePart = nursePart + '''<br>&nbsp; <b>Nurse ''' + str(i) + ' (' + str(nn.ID) + '''):</u> </b><br>'''
            nursePart = nursePart + '''&nbsp; <i>Skills: </i>''' + str(nn.skills) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Start time: </i>''' + self.time_to_string(self.nurseWorkingTimes[i][0]) + '''<br>'''
            nursePart = nursePart + '''&nbsp; <i>Finish time: </i>''' + self.time_to_string(nn.finishTime) + '''<br>'''
            nn.route = list(self.nurseRoute[i][:])
            nursePart = nursePart + '''&nbsp; <i>Number of services: </i>''' + str(len(nn.route)) + '''<br>'''
            if len(nn.route) > 0:
                nursePart = nursePart + '''&nbsp; <i>Service route: </i>[''' + str(self.jobObjs[int(nn.route[0])].ID)
                for kkk in range(1,len(nn.route)):
                    jobbb = self.jobObjs[int(nn.route[kkk])]
                    if jobbb.doubleService:
                        nursePart = nursePart + ', (' + str(jobbb.ID) + ')'
                    else:
                        nursePart = nursePart + ', ' + str(jobbb.ID)

                nursePart = nursePart + ''']<br>'''
    
        lht = lht + nursePart

        modalImages = [self.name + '_workload.png', self.name + '_time_info.png']
        modalCaptions = ['Workload distribution', 'Time distribution']

        for i,imn in enumerate(modalImages):
            lht = lht + self.hovering_image(imn, modalCaptions[i], i)

        # Add modal chart:
        lht = lht + '''
        <div id="myModal" class="modal">
          <span class="close">&times;</span>
          <img class="modal-content" id="img_of_modal">
          <div id="caption"></div>
        </div>

        <script>
        '''

        for i,imn in enumerate(modalImages):
            lht = lht + '''
            var img''' + str(i) + ''' = document.getElementById('myImg''' + str(i) + '''');
                img''' + str(i) + '''.onclick = function(){
                    just_display(this);
                }'''


        # Finish script:
        lht = lht + '''
        var modalImg = document.getElementById("img_of_modal");
        var modal = document.getElementById('myModal');
        var captionText = document.getElementById("caption");

        // Get the <span> element that closes the modal
        var span = document.getElementsByClassName("close")[0];

        // When the user clicks on <span> (x), close the modal
        span.onclick = function() { 
            modal.style.display = "none";
        }
        just_display = function(imag)
        {
            modal.style.display = "block";
            modalImg.src = imag.src;
            captionText.innerHTML = imag.alt;   
        }
        </script>'''

        lht = lht + '''</div>'''
        


        m.get_root().html.add_child(folium.Element(lht))


        f = open('modal_style.txt', 'r')
        styleStr = f.read()
        f.close()
        m.get_root().html.add_child(folium.Element(styleStr))

        # m
        # Add DEPOT:
        # html_code = '<div style="max-height:800px;max-width:800px;overflow:auto;">'
        # html_code = html_code + '<img src="' + self.name + '_workload.png" alt="Workload distribution"><br>'
        # html_code = html_code + '<img src="' + self.name + 'time_info.png" alt="Time distribution">'
        # html_code = html_code + '</div>'
        # html_code = 'Depot'
        # iframe = folium.IFrame(html=html_code, width=500, height=300)

        # popup = folium.Popup(html_code, max_width=800)

        # Depot, change to one per nurse!
        for nursej in range(self.nNurses):
            nurse_popup = 'Start location for nurse ' + str(nursej)

            folium.Circle(self.nurseObjs[nursej].startLocation.latlong(),
                            radius=50,
                            popup=nurse_popup,
                            color='black',
                            fill_color='black',
                            fill_opacity=0.5,
                            fill=True).add_to(m)
        # folium.Circle(rxy[0],
        #         radius=50,
        #         popup=html_code,
        #         color='black',
        #         fill_color='black',
        #         fill_opacity=0.5,
        #         fill=True).add_to(m)

        m.add_child(folium.map.LayerControl())
        m.save(webFilename)
    ### --- End def solution_to_website --- ###

    ##################################################
    # xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    ##################################################
    def hovering_image(self, imName, altText, idd):
        return('''<img id="myImg''' + str(idd) + '''" src="''' + imName + '''"
                 alt="''' + altText + '''" width="300" height="200">
                <style>
                #myImg''' + str(idd) + ''' {
                    border-radius: 5px;
                    cursor: pointer;
                    transition: 0.3s;
                }
                #myImg''' + str(idd) + ''':hover {opacity: 0.7;}
                </style>
                ''')
    ### --- End def hovering_image --- ###           

    def solve(self, randomSeed=0, printAllCallData=False):
        if self.verbose > 0:
            print('Calling C function for a max. of ' + str(self.MAX_TIME_SECONDS) + ' seconds , random seed is ' + str(randomSeed) + '.')

        # Prepare some data: 
        self.mk_mind = np.asarray(self.mk_mind, dtype=np.int32)
        self.mk_maxd = np.asarray(self.mk_maxd, dtype=np.int32)

        self.nJobs = int(self.nJobs)
        self.nNurses = int(self.nNurses)
        self.nSkills = int(self.nSkills)
        self.od = np.ascontiguousarray(self.od)
        self.nurse_travel_from_depot = np.ascontiguousarray(self.nurse_travel_from_depot)
        self.nurse_travel_to_depot = np.ascontiguousarray(self.nurse_travel_to_depot)
        self.nurseWorkingTimes = np.ascontiguousarray(self.nurseWorkingTimes, dtype=np.int32)
        self.jobTimeInfo = np.ascontiguousarray(self.jobTimeInfo, dtype=np.int32)
        self.jobSkillsRequired = np.ascontiguousarray(self.jobSkillsRequired, dtype=np.int32)
        self.nurseSkills = np.ascontiguousarray(self.nurseSkills, dtype=np.int32)
        self.solMatrix = np.ascontiguousarray(self.solMatrix, dtype=np.int32)
        self.doubleService = np.ascontiguousarray(self.doubleService, dtype=np.int32)		
        self.dependsOn = np.ascontiguousarray(self.dependsOn, dtype=np.int32)
        self.mk_mind = np.ascontiguousarray(self.mk_mind, dtype=np.int32)
        self.mk_maxd = np.ascontiguousarray(self.mk_maxd, dtype=np.int32)
        self.capabilityOfDoubleServices = np.ascontiguousarray(self.capabilityOfDoubleServices.reshape(-1))
        self.prefScore = np.ascontiguousarray(self.prefScore)
        self.algorithmOptions = np.ascontiguousarray(self.algorithmOptions)

        if	(self.verbose > 5 or printAllCallData):
            print('nJobs (type ' + str(type(self.nJobs)) + ')')
            print(self.nJobs)
            print('nNurses (type ' + str(type(self.nNurses)) + ')')
            print(self.nNurses)
            print('nSkills (type ' + str(type(self.nSkills)) + ')')
            print(self.nSkills)
            print('verbose (type ' + str(type(self.verbose)) + ')')
            print(self.verbose)
            print('MAX_TIME_SECONDS (type ' + str(type(self.MAX_TIME_SECONDS)) + ')')
            print(self.MAX_TIME_SECONDS)
            
            print('od (type ' + str(type(self.od)) + ')')
            print('dtype = ' + str(self.od.dtype))
            print('Shape = ' + str(self.od.shape))
            print(self.od)

            print('nurse_travel_from_depot (type ' + str(type(self.nurse_travel_from_depot)) + ')')
            print('dtype = ' + str(self.nurse_travel_from_depot.dtype))
            print('Shape = ' + str(self.nurse_travel_from_depot.shape))
            print(self.nurse_travel_from_depot)

            print('nurse_travel_to_depot (type ' + str(type(self.nurse_travel_to_depot)) + ')')
            print('dtype = ' + str(self.nurse_travel_to_depot.dtype))
            print('Shape = ' + str(self.nurse_travel_to_depot.shape))
            print(self.nurse_travel_to_depot)
        
            print('nurseWorkingTimes (type ' + str(type(self.nurseWorkingTimes)) + ')')
            print('dtype = ' + str(self.nurseWorkingTimes.dtype))
            print('Shape = ' + str(self.nurseWorkingTimes.shape))
            print(self.nurseWorkingTimes)
            
            print('jobTimeInfo (type ' + str(type(self.jobTimeInfo)) + ')')
            print('dtype = ' + str(self.jobTimeInfo.dtype))
            print('Shape = ' + str(self.jobTimeInfo.shape))

            print(self.jobTimeInfo)
            
            print('jobSkillsRequired (type ' + str(type(self.jobSkillsRequired)) + ')')
            print('dtype = ' + str(self.jobSkillsRequired.dtype))
            print('Shape = ' + str(self.jobSkillsRequired.shape))

            print(self.jobSkillsRequired)
            
            print('nurseSkills (type ' + str(type(self.nurseSkills)) + ')')
            print('dtype = ' + str(self.nurseSkills.dtype))
            print('Shape = ' + str(self.nurseSkills.shape))

            print(self.nurseSkills)
            
            print('solMatrix (type ' + str(type(self.solMatrix)) + ')')
            print('dtype = ' + str(self.solMatrix.dtype))
            print('Shape = ' + str(self.solMatrix.shape))

            print(self.solMatrix)
            
            print('doubleService (type ' + str(type(self.doubleService)) + ')')
            print('dtype = ' + str(self.doubleService.dtype))
            print('Shape = ' + str(self.doubleService.shape))			
            print(self.doubleService)

            print('dependsOn (type ' + str(type(self.dependsOn)) + ')')
            print('dtype = ' + str(self.dependsOn.dtype))
            print('Shape = ' + str(self.dependsOn.shape))
            print(self.dependsOn)
            
            print('mk_mind (type ' + str(type(self.mk_mind)) + ')')
            print('dtype = ' + str(self.mk_mind.dtype))
            print('Shape = ' + str(self.mk_mind.shape))

            print(self.mk_mind)
            
            print('mk_maxd (type ' + str(type(self.mk_maxd)) + ')')
            print('dtype = ' + str(self.mk_maxd.dtype))
            print('Shape = ' + str(self.mk_maxd.shape))

            print(self.mk_maxd)

            print('capabilityOfDoubleServices (type ' + str(type(self.capabilityOfDoubleServices)) + ')')
            print('dtype = ' + str(self.capabilityOfDoubleServices.dtype))
            print('Shape = ' + str(self.capabilityOfDoubleServices.shape))

            print(self.capabilityOfDoubleServices)
            

            print('prefScore (type ' + str(type(self.prefScore)) + ')')
            print('dtype = ' + str(self.prefScore.dtype))
            print('Shape = ' + str(self.prefScore.shape))
            print(self.prefScore)

            print('algorithmOptions (type ' + str(type(self.algorithmOptions)) + ')')
            print('dtype = ' + str(self.algorithmOptions.dtype))
            print('Shape = ' + str(self.algorithmOptions.shape))
            print(self.algorithmOptions)
            
            print('\n ---------------- end of python data - start call to C ----------------\n\n\n ')

        # Call:
        self.fun(self.nJobs,
            self.nNurses,
            self.nSkills,
            self.verbose,
            self.MAX_TIME_SECONDS,
            self.od,
            self.nurse_travel_from_depot,
            self.nurse_travel_to_depot,
            self.nurseWorkingTimes,
            self.jobTimeInfo,
            self.jobSkillsRequired,
            self.nurseSkills,
            self.solMatrix,
            self.doubleService,
            self.dependsOn,
            self.mk_mind,
            self.mk_maxd,
            self.capabilityOfDoubleServices,
            self.prefScore,
            self.algorithmOptions,
            randomSeed)

        if self.verbose > 10:
            print('Returned this matrix: ')
            print(self.solMatrix)
            print('Postprocessing...')
        # time.sleep(2)
        # print('Done.')
        self.post_process_solution()
    ### --- End def solve --- ###  

    def post_process_solution(self):
        # Generate nurse routes:
        self.nurseWaitingTime = np.zeros(self.nNurses)
        self.nurseServiceTime = np.zeros(self.nNurses)
        self.nurseTravelTime = np.zeros(self.nNurses)
        self.nurseTime = np.zeros(self.nNurses)
        self.totalWaitingTime = 0
        self.totalServiceTime = 0
        self.totalTravelTime = 0
        self.totalTime = 0
        self.nurseRoute = []
        # self.c_quality = np.min(self.solMatrix)

        for nurse in range(self.nNurses):
            # howMany = 0
            try:
                howMany = max(self.solMatrix[nurse]) + 1 # howMany = largest value in self.solMatrix[nurse], which is equivalent to the number of jobs in the nurse's route. Add one because job positions start at 0.
            except Exception as e:
                print('ERROR in post_process_solution()')
                print(e)
                print('Nurse: ' + str(nurse))
                print('solMatrix: \n' + str(self.solMatrix))
            
            self.nurseRoute.append(np.zeros(howMany)) # add array of size 'number of jobs' containing all zeros to the nurseRoute list.
            for i,sp in enumerate(self.solMatrix[nurse,:]): # For each index, value in solMatrix[nurse], all jobs
                if sp > -1: # If > -1 then job i is in position sp of nurse's route
                    self.nurseRoute[nurse][sp] = i # set nurseRoute[nurse][position] = job (like allNurseRoutes)
    ### --- End def post_process_solution --- ###

    def pie_chart_how_is_time_spent(self):
        labels = 'Travel', 'Waiting', 'Service'
        times = [self.totalTravelTime, self.totalWaitingTime, self.totalServiceTime]
        # colours = ['#2dbe60', '#7b868a', '#166a8f']
        # plt.pie(times, labels=labels, colors=colours, autopct='%1.1f%%')
        plt.pie(times, labels=labels, autopct='%1.1f%%')
        plt.title('Total time for ' + str(self.nNurses) + ' nurses: ' + self.time_to_string(self.totalTime))
        plt.axis('equal')
        plt.draw()
    ### --- End def pie_chart_how_is_time_spent --- ###

    def bar_stacked_chart_work_times_per_nurse(self):
        xpos = np.arange(self.nNurses)
        width = 0.35

        p1 = plt.bar(xpos, self.nurseServiceTime, width)
        p2 = plt.bar(xpos, self.nurseTravelTime, width, bottom=self.nurseServiceTime)
        p3 = plt.bar(xpos, self.nurseWaitingTime, width, bottom=(self.nurseServiceTime + self.nurseTravelTime))

        plt.ylabel('Time ')
        plt.title('Distribution of working time per nurse')
        ticksNames = []
        for i in xpos:
            ticksNames.append(str(self.nurseObjs[i].ID))
            # ticksNames.append('Nurse ' + str(i))
        maxYtick = 8*3600/self.secondsPerTU
        tmarks = np.arange(0, maxYtick, 1800/self.secondsPerTU)
        tt = 0
        tticks = []
        for x in tmarks:
            tticks.append(str(np.round(tt, 1)) + ' h')
            tt = tt + 0.5
        plt.yticks(tmarks, tuple(tticks))
        plt.xticks(xpos, tuple(ticksNames))
        plt.legend((p1[0], p2[0], p3[0]), ('Service time', 'Travel time', 'Waiting time'))

        plt.draw()
    ### --- End def bar_stacked_chart_work_times_per_nurse --- ###


########################################################
#### Brought from C #####
    def get_nurse_route(self, ni):
        nurseRoute = np.zeros(self.nJobs)
        for ii in range(self.nJobs):
            nurseRoute[ii] = -1

        for ii in range(self.nJobs):
            if (self.solMatrix[ni][ii] >= 0):
                nurseRoute[self.solMatrix[ni][ii]] = ii

        return nurseRoute

    def get_travel_time(self, i, j):
        return(self.od[int(i + 1)][int(j + 1)])

    def full_solution_report(self, doPlots=True, report=2):
        # for nurse in range(self.nNurses):
        # 	print('Nurse 1 starts at ')
        # report = 1
        self.totalTardiness = 0
        self.totalWaitingTime = 0
        self.totalTime = 0
        self.nLateJobs = 0
         # * nurseRoute = self.nurseRoute  # malloc(self.nJobs * sizeof(int))
        onlyTravelTime = 0
        arriveAt = 0
        leaveAt = 0
        self.maxTardiness = 0
        for j in range(self.nNurses):
            job = -1
            prevPt = -1
            currentTime = self.nurseWorkingTimes[j][0]
            if (report > 1):
                print("Nurse " + str(j) + " starts at " + " (" + str(currentTime) + ")" + self.time_to_string(currentTime))

            # get_nurse_route(ip, j, self.nurseRoute)
            self.nurseTravelTime[j] = 0
            self.nurseWaitingTime[j] = 0
            self.nurseServiceTime[j] = 0

            for jobb in self.nurseRoute[j]:
                job = int(jobb)
                    # if (self.nurseRoute[i] < 0)
                    # break
                # job = self.nurseRoute[i]
                 # Trip from depot:
                tTime = self.get_travel_time(prevPt, job)
                # onlyTravelTime = onlyTravelTime  + tTime
                self.nurseTravelTime[j] = self.nurseTravelTime[j] + tTime

                currentTime = currentTime + tTime #self.od[prevPt][job]

                self.nurseServiceTime[j] = self.nurseServiceTime[j] + self.jobTimeInfo[job][2]

                arriveAt = currentTime
                waitingTime = 0
                if (arriveAt < self.jobTimeInfo[job][0]):
                    waitingTime = self.jobTimeInfo[job][0] - arriveAt
                self.nurseWaitingTime[j] = self.nurseWaitingTime[j] + waitingTime

                tardiness = 0
                if (arriveAt > self.jobTimeInfo[job][1]):
                    tardiness = arriveAt - self.jobTimeInfo[job][1]

                if tardiness > self.maxTardiness:
                    self.maxTardiness = tardiness
                self.totalTardiness = self.totalTardiness  + tardiness

                prevPt = job
                currentTime = currentTime + self.jobTimeInfo[job][2] + waitingTime
                leaveAt = currentTime

                # self.totalWaitingTime = self.totalWaitingTime + waitingTime
                # self.totalServiceTime = self.totalServiceTime + self.jobTimeInfo[job][2]
                # self.nurseTravelTime[j] = self.nurseTravelTime[j] + tTime

                # self.totalTime = self.totalTime  + leaveAt - self.nurseWorkingTimes[j][0]
                 # Add penalty for potential lateness and breaching of normal working hours
                # SET JOB OBJECT INFO:
                # print('SETTING JOBOBJ INFO for: ' + str(job))
                self.jobObjs[job].arrivalTime = arriveAt
                self.jobObjs[job].departureTime = leaveAt
                self.jobObjs[job].serviceTime = self.jobTimeInfo[job][2]
                self.jobObjs[job].tardiness = tardiness
                self.jobObjs[job].waitingToStart = waitingTime
                if self.jobObjs[job].assignedNurse is list:
                    self.jobObjs[job].assignedNurse.append(j)
                else:
                    self.jobObjs[job].assignedNurse = [j]
                self.jobObjs[job].positionInSchedule = self.solMatrix[j][job]
                # print(self.jobObjs[job])
                if (report > 1):
                    # self.jobObjs[job].skillsRequired = []
                    print("\tArrives at job " + str(job) + " at " + " (" + str(arriveAt) + ")" + self.time_to_string(arriveAt) + " and leaves at " + " (" + str(leaveAt) + ")" + self.time_to_string(leaveAt))
                    if (waitingTime > 0):
                        print("\t\tNeeds to wait for " + " (" + str(waitingTime) + ")" + self.time_to_string(waitingTime) + " before starting the job")
                    if (tardiness > 0):
                        print("\t\t*** Misses the time window by " + " (" + str(tardiness) + ")" + self.time_to_string(tardiness) + "! ***")
                        self.nLateJobs = self.nLateJobs + 1
                
             # Return to depot:
            if job > -1:
                tTime = self.get_travel_time(job, -1)
                self.nurseTravelTime[j] = self.nurseTravelTime[j] + tTime
                finishShiftAt = leaveAt + tTime
            else:
                finishShiftAt = self.nurseWorkingTimes[j][0]


            # Update totals:
            # self.totalTime = self.totalTime  + tTime
            self.totalServiceTime = self.totalServiceTime + self.nurseServiceTime[j]
            onlyTravelTime = onlyTravelTime  + self.nurseTravelTime[j]
            self.totalWaitingTime = self.totalWaitingTime + self.nurseWaitingTime[j]

            self.nurseObjs[j].finishTime = finishShiftAt
            if (report > 1):
                print("\tFinishes at the depot at " + " (" + str(finishShiftAt) + ")" + self.time_to_string(finishShiftAt) + ".")
                if (finishShiftAt > self.nurseWorkingTimes[j][1]):
                    print("\t\t*** This nurse is finishing late (by " + " (" + str(finishShiftAt - self.nurseWorkingTimes[j][1]) + ")" + self.time_to_string(finishShiftAt - self.nurseWorkingTimes[j][1]) + ")\n")
                         # else
                 # {
                 # 	print("\t\t Finishing before " + self.time_to_string() + " (end of shift)\n", )self.nurseWorkingTimes[j][1])
                 # }

        
        if (report > 0):
            print("\nTotal travel time: " + str(onlyTravelTime) + '(' + self.time_to_string(onlyTravelTime) + ')')
        self.totalTravelTime = onlyTravelTime

        self.totalTime = self.totalTravelTime + self.totalServiceTime + self.totalWaitingTime

            # print("\n")
         # free(nurseRoute)
        if self.qualityMeasure == 'mankowska':
            quality = (self.totalTravelTime + self.totalTardiness + self.maxTardiness)/3 # Mankowska
            print('Quality returned from C DLL: ' + str(self.Cquality))
            print("Mankowska measure (python computed) = " + str(quality))
            print("\ttotalTravelTime = " + str(self.totalTravelTime))
            print("\ttotalTardiness = " + str(self.totalTardiness))
            print("\tmaxTardiness = " + str(self.maxTardiness))
        else:
            quality = -1000000 * self.totalTardiness - self.totalTime

        self.mankowskaQuality = quality
        # print('Solution quality: ' + self.time_to_string(quality))
        if report > 0:
            print('Computed quality: ' + str(quality))
            print('From: ' + 'totalTime = ' + str(self.totalTime) + '\ntotalTardiness = ' + str(self.totalTardiness) + '\nmaxTardiness = ' + str(self.maxTardiness))
            print('Total time required to complete the solution: ' + self.time_to_string(self.totalTime))
            if self.nLateJobs > 0:
                print('The solution is infeasible. Nurses arrive late to ' + str(self.nLateJobs) + ' services.')
                print('The total tardiness time is ' + self.time_to_string(self.totalTardiness))
            else:
                print('All jobs are served on time. Nurses need to wait a total of ' + self.time_to_string(self.totalWaitingTime) + ' because of early arrivals.')
            
        if doPlots:
            self.pie_chart_how_is_time_spent()
            plt.savefig(self.name + '_time_info' + '.png', bbox_inches='tight')
            plt.show()
            self.bar_stacked_chart_work_times_per_nurse()
            plt.savefig(self.name + '_workload' + '.png', bbox_inches='tight')
            plt.show()

        return quality



####################################################

def np_array_to_c_format(nparr, arrname='Array', floatVar=False):
    rows,cols = nparr.shape
    astr = ''
    if floatVar:
        astr = astr + 'float '
    else:
        astr = astr + 'int '
    astr = astr + arrname + '[' + str(rows) + '][' + str(cols) + ']' 
    astr = astr + ' = {\n'
    for i in range(rows):
        astr = astr + '{'
        for j in range(cols-1):
            if floatVar:
                astr = astr + str(nparr[i][j]) + ', '
            else:
                astr = astr + str(int(nparr[i][j])) + ', '
        if floatVar:
            astr = astr + str(nparr[i][cols-1])
        else:
            astr = astr + str(int(nparr[i][cols-1]))

        if i < rows -1:
            astr = astr + '},\n'
        else:
            astr = astr + '}\n};\n'
    return(astr)

def read_braekers(filename):
    int_type = np.int32
    f = open(filename, 'r')
    welcome_line = f.readline()
    welcome_line = welcome_line.split()
    nJobs = int(welcome_line[1])
    nNurses = int(welcome_line[2])
    transportMode = int(welcome_line[7])
    if transportMode == 2:
        print('// WARNING: This instance originally uses a mixed transport mode.')
    matrixSource = int(welcome_line[7])
    secondsPerTU = 0
    if matrixSource < 5:
        print('// Information is expressed on a 5 minute level.')
        secondsPerTU = 300
    else:
        print('// Information is expressed on a 1 minute level.')
        secondsPerTU = 60

    # The following lines contain information on the nurses:
    # 1 earliest_working_time latest_working_time regular_working_time maximum_working_time cost_per_unit_of_overtime mode skill_level
    # .
    # N
    maxSkillLevel = 0
    nurseWorkingTimes = np.zeros((nNurses, 3), dtype=int_type)
    nSkills = []
    for r in range(nNurses):
        line = f.readline().split()

        nurseWorkingTimes[r][0] = int(line[0])
        nurseWorkingTimes[r][1] = int(line[1])
        nurseWorkingTimes[r][2] = int(line[2])
        skl = int(line[6])
        nSkills.append(skl)
        if skl > maxSkillLevel:
            maxSkillLevel = skl

    # Generate skills array:
    skArr = np.ones((nNurses, maxSkillLevel), dtype=int_type)
    for n,skl in enumerate(nSkills):
        if skl < maxSkillLevel:
            skArr[n][skl:] = 0

    jobTimeInfo = np.zeros((nJobs, 3), dtype=int_type)
    jobSkillsRequired = np.ones((nJobs, maxSkillLevel), dtype=int_type)

    # The following lines contain information on the jobs:
    # 1 start_time_window end_time_window service_duration preferred_visit_time required_skill_level
    # .
    # I
    for j in range(nJobs):
        line = f.readline().split()
        jobTimeInfo[j][0] = int(line[0])
        jobTimeInfo[j][1] = int(line[1])
        jobTimeInfo[j][2] = int(line[2])
        skl = int(line[4])
        if skl < maxSkillLevel:
            jobSkillsRequired[j][skl:] = 0

    # Skip some lines regarding preferences (2*nJobs)
    for j in range(2*nJobs):
        line = f.readline()

    # The following lines indicate the travel cost and travel time matrices for the network of N+I vertices. The matrices are in the following order:
    # travel cost by car, travel time by car, travel cost by public transport, travel time by public transport
    # (when a mode of transportation is not used, all values are set to 100000)
    #  	1 . N N+1 . N+I
    # 1
    # .
    # N
    # N+1
    # .
    # N+I 

    # Skip first matrix: cost
    for j in range(nNurses + nJobs):
        line = f.readline()

    if transportMode > 0.5:
        # Transport mode is walking, skip two more matrices
        for j in range(2*(nNurses + nJobs)):
            line = f.readline()

    od = np.zeros((nJobs + 1, nJobs + 1))
    toDepot = np.zeros(nJobs)
    fromDepot = np.zeros(nJobs)
    # rowCt = 0
    for rows in range(nJobs + 1):
        line = f.readline().split()

        # Depot is last in these instances
        # rowCt = (rowCt + 1) % (nJobs + 1)

        # colCt = 0
        for cols in range(nJobs + 1):
            # colCt = (colCt + 1) % (nJobs + 1)
            od[rows][cols] = float(line[cols])

    # Rearrange rows and cols to have depot first:
    fromDepot = od[nJobs,0:-1]
    toDepot = od[0:-1,nJobs]
    depotItself = od[nJobs][nJobs]
    # Move OD part down
    od[1:,1:] = od[0:-1,0:-1]
    od[0,1:] = fromDepot
    od[1:,0] = toDepot
    od[0][0] = depotItself

    return([nJobs, nNurses, maxSkillLevel, nurseWorkingTimes, skArr, jobTimeInfo, jobSkillsRequired, od, secondsPerTU])

# fLocation = 'C:\\Users\\clf1u13\\Docs\\01.HHCRSP\\Instances\\Braekers\\bihcrsp_instances\\'
# fName = 'bihcrsp_14.txt'

# print('/*\n// INSTANCE: ' + fName)
# [nJobs, nNurses, nSkills, nurseWorkingTimes, skArr, jobTimeInfo, jobSkillsRequired, od] = read_braekers(fLocation + fName)
# print('int nJobs = ' + str(nJobs) + ';')
# print('int nNurses = ' + str(nNurses) + ';')
# print('int nSkills = ' + str(nSkills) + ';')
# print(np_array_to_c_format(nurseWorkingTimes, 'nurseWorkingTimes_data'))
# print(np_array_to_c_format(jobTimeInfo, 'jobTimeWindow_data'))
# print(np_array_to_c_format(skArr, 'a'))
# print(np_array_to_c_format(jobSkillsRequired, 'r'))

# print(np_array_to_c_format(od, 'od_data'))
# print('int nurseSkilled_data[' + str(nNurses) + '][' + str(nJobs) + '];')
# print('\n// End of instance ' + fName + '\n*/\n')

################################### From the FTC project, do not update here ###############

def route_these_points(p1, p2, goingThrough=None):
    # Returns a list of coordinates with the route between the points, duration and distance
    # [routeList, duration, distance]

    # We give longlats
    # node1 = osrm.Point(latitude=p1[1], longitude=p1[0])
    # node2 = osrm.Point(latitude=p2[1], longitude=p2[0])

    # result = osrm.simple_route(node1, node2)
    # print(result)

    rout = osrm.simple_route(p1, p2, output='route', overview="full", geometry='wkt', coord_intermediate=goingThrough)
    # print(rout)
    # print('From ' + str(p1) + ' to ' + str(p2) + ' and through ' + str(goingThrough) + '\nRoute:')
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
    # print('Distance ' + str(dist) + ', duration ' + str(dur))
    return [routeList, dur, dist]

# def point_list_to_leaflet_polyline(pointList, varname, col='red', mapvar='mymap'):
# 	output = ''
# 	polylineName = 'pl_' + varname
# 	output = output + '\nvar ' + varname + ' = [\n'
# 	for i,node in enumerate(pointList):
# 		if i >= len(pointList) - 1:
# 			output = output + "[%f, %f]\n" % (node[0], node[1])
# 		else:
# 			output = output + "[%f, %f],\n" % (node[0], node[1])

# 	output = output + '];\n'
# 	output = output + 'var ' + polylineName + ' = L.polyline(' + varname 
# 	output = output + ', {color: \'' + col + '\', opacity: 0.5}).addTo(' + mapvar + ');\n'
# 	return output

def clusterColour(clusterNumber):
    # From: https://www.w3schools.com/tags/ref_colornames.asp
    clusterNumber = clusterNumber % 18
    if clusterNumber == 0:
        col = '#1E90FF' #   DodgerBlue
    elif clusterNumber == 1:
        col = '#FF7F50' #   Coral
    elif clusterNumber == 2:
        col = '#FF1493' #    DeepPink
    elif clusterNumber == 3:
        col = '#00FFFF' #    Aqua
    elif clusterNumber == 4:
        col = '#808000' #  Olive
    elif clusterNumber == 5:
        col = '#006400' #   DarkGreen
    elif clusterNumber == 6:
        col = '#FFD700' #   Gold
    elif clusterNumber == 7:
        col = '#0000FF' #   Blue
    elif clusterNumber == 8:
        col = '#B22222' #    FireBrick
    elif clusterNumber == 9:
        col = '#FF8C00' #   DarkOrange
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
        col = '#000000' #   Black

    return(col)

# class OSRM_SERVER(object):
# 	"""docstring for osrm_server"""
# 	def __init__(self):
# 		# Start OSRM server...
# 		# print('Starting server for distances ' + matrixType + ' matrix...')
# 		# if matrixType == 'driving':
# 		serverHost = 'http://localhost:5000/'
# 		self.carserver = subprocess.Popen('C:\\Users\\clf1u13\\Docs\\01.HHCRSP\\Code\\HHCRSP\\runCarServer.bat', 
# 			shell=False,stdout=subprocess.PIPE)
            
# 		# else:
# 		serverHost = 'http://localhost:5001/'
# 		self.walkserver = subprocess.Popen('C:\\Users\\clf1u13\\Docs\\01.HHCRSP\\Code\\HHCRSP\\runWalkServer.bat', 
# 			shell=False,stdout=subprocess.PIPE)

# 		time.sleep(3) # Give it 3 secs to initialise
# 		# self.carPID = self.carserver.pid
# 		# self.walkPID = self.walkserver.pid
# 		# print('PID: ' + str(popenConnection.pid))
# 		# print('Done.')


# 	def kill_server(self):
# 		# Close servers:
# 		# print('Killing server (' + matrixType + ' profile)...')
# 		self.carserver.kill()
# 		self.walkserver.kill()
# 		print('Done.')

