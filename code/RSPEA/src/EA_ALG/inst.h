/*--------------/
EA_ALG
inst.h
UoS
27/01/2022
/--------------*/



#ifndef EA_ALG_INST_H
#define EA_ALG_INST_H

#include "consts.h"

struct Instance {
    int nJobs; // Number of jobs
    int nJobsIncDS; //Number of tasks in total, nJobs+nDS
    int nNurses; // Number of carers
    int nSkills; //Number of skills
    int verbose; // Additional information
    int isFeasible; // Feasibility (deprecated)
    int qualityMeasure; // Temp. Allow to choose (Mk, Ait h., etc.) Ideally we want a vector with weights! This should be set ONLY in "InstanceFromPython" (read_instance.c). The value is hardcoded at the moment.
    int twInterval; // Time window interval
    double objTime; // Objective time
    double objWaiting; // Objective waiting time, NEW 07/01/2021
    double objTravel; // Objective travel time
    double objService; // Objective service time
    double objTardiness; // Objective tardiness
    double objMaxTardiness; // Objective max tardiness
    double objMKTardiness; // Objective total MK tardiness
    double objMKAllowedTardiness; // Objective mk_allowed_tardiness
    double objOvertime; // Objective overtime
    double objMaxOvertime; // Objective max overtime
    double objMinSpareTime;
    double objMaxSpareTime;
    double objShortestDay; // Objective shortest day, NEW 07/01/2021
    double objLongestDay; // Objective longest day
    //double objAitHQuality;
    //double objMKQuality;
    //double objWBQuality;
    //double objPaperQuality;
    double objQuality;
    double objMaxTravelTime; // NB new 16/11/2021
    double objMaxWaitingTime; // NB new 16/11/2021
    double objMaxDiffWorkload; // NB new 16/11/2021
    double totalServiceTime; // New 06/11/2021, moving total service time here as it should be fixed rather than calculating it every time in Objective function
    double totalServiceTimeIncDS; // New 06/11/2021, totalServiceTime but also including double time for the double services; this will therefore be the total working time of all nurses.
    //float totalPref; // Total preference score
    double totalPref; // Total preference score
    //float MAX_TIME_SECONDS; // Maximum time limit in seconds
    double MAX_TIME_SECONDS; // Maximum time limit in seconds
    bool excludeNurseTravel; // True if excluding the nurse travel time from home to the first job when updating current time in 'set_nurse_time' function, else false.
    // double **od_cost;
    //int* nurseOrder; // 1D array, size = 1 x nNurses. Integer values of order of nurses. Order in which nurses are considered (set randomly in the constructive).
    std::vector<int> nurseOrder; // 1D array, size = 1 x nNurses. Integer values of order of nurses. Order in which nurses are considered (set randomly in the constructive).
    //int* nurseRoute; // 1D array, size = 1 x nJobs. Used as temp. array to avoid allocations.
    std::vector<int> nurseRoute; // 1D array, size = 1 x nJobs. Used as temp. array to avoid allocations.
    //int* doubleService; // 1D array, size = 1 x nJobs. Indicates which jobs need more than one nurse. = 1 if job is DS, = 0 otherwise.
    std::vector<int> doubleService; // 1D array, size = 1 x nJobs. Indicates which jobs need more than one nurse. = 1 if job is DS, = 0 otherwise.
    //int* dependsOn; // 1D array, size = 1 x nJobs. Indicates which jobs depend on another one. If job 2 depends on job 5, then dependsOn[2] = 5 and dependsOn[5] = 2.
    std::vector<int> dependsOn; // 1D array, size = 1 x nJobs. Indicates which jobs depend on another one. If job 2 depends on job 5, then dependsOn[2] = 5 and dependsOn[5] = 2.
    //int* mkMinD; // 1D array, size = 1 x nJobs. Minimum gap with time of next job.
    std::vector<int> mkMinD; // 1D array, size = 1 x nJobs. Minimum gap with time of next job.
    //int* mkMaxD; // 1D array, size = 1 x nJobs. Maximum gap with time of next job.
    std::vector<int> mkMaxD; // 1D array, size = 1 x nJobs. Maximum gap with time of next job.
    //int* nurseUnavail; // 1D array, size = nNurses, for each nurse contains the number of unavailable shifts.
    std::vector<int> nurseUnavail; // 1D array, size = nNurses, for each nurse contains the number of unavailable shifts.
    //int** solMatrix; // 2D array, size = nNurses x nJobs. For each nurse i (row), the element in solMatrix[i][j] gives the position in nurse i's route where nurse i visits job j.
    std::vector<std::vector<int> > solMatrix; // 2D array, size = nNurses x nJobs. For each nurse i (row), the element in solMatrix[i][j] gives the position in nurse i's route where nurse i visits job j.
    // E.g. if solMatrix[2][4] = 3 -> the third job in nurse 2's route is job 4. If solMatrix[i][j] = -1, then job j is not in nurse i's route.
    //int** nurseWorkingTimes; // UPDATED: 2D array, size = nNurses x 5. For each nurse i (row): column[0] = start time, column[1] = finish time, column[2] = length of day for nurse,
    std::vector<std::vector<int> > nurseWorkingTimes; // UPDATED: 2D array, size = nNurses x 5. For each nurse i (row): column[0] = start time, column[1] = finish time, column[2] = length of day for nurse,
    // col[3] = # of shifts for that nurse, col[4] = duration of total shift working time. previously nNurses x 3, only first 3 columns used.
    //int** jobTimeInfo; // 2D array, size = nJobs x 3. For each job i (row): column[0] = start time window, column[1] = finish time window, column[2] = length of job.
    std::vector<std::vector<int> > jobTimeInfo; // 2D array, size = nJobs x 3. For each job i (row): column[0] = start time window, column[1] = finish time window, column[2] = length of job.
    //int** jobRequirements; // 2D array, size = nJobs x nSkills. Skill 0 means headcount needed. (deprecated)
    std::vector<std::vector<int> > jobRequirements; // 2D array, size = nJobs x nSkills. Skill 0 means headcount needed. (deprecated)
    //int** nurseSkills; // 2D array, size = nNurses x nSkills. Skill 0 is always 1 (one worker). (deprecated)
    std::vector<std::vector<int> > nurseSkills; // 2D array, size = nNurses x nSkills. Skill 0 is always 1 (one worker). (deprecated)
    //int** nurseSkilled; // 2D array, size = nNurses x nJobs. For each nurse i (row) and each job j (column): nurseSkilled[i][j] = 1 if nurse i can do job j, = 0 otherwise.
    std::vector<std::vector<int> > nurseSkilled; // 2D array, size = nNurses x nJobs. For each nurse i (row) and each job j (column): nurseSkilled[i][j] = 1 if nurse i can do job j, = 0 otherwise.
    //int** allNurseRoutes; // 2D array, size = nNurses x nJobs. For each nurse i (row), the element in allNurseRoutes[i][j] gives the job in the POSITION j in nurse i's route.
    std::vector<std::vector<int> > allNurseRoutes; // 2D array, size = nNurses x nJobs. For each nurse i (row), the element in allNurseRoutes[i][j] gives the job in the POSITION j in nurse i's route.
    //int*** unavailMatrix; // 50 X 4 X nNurses 3d matrix, col[0] = unavailable shift number, col[1] = start of unavailable time, col[2] = end of unavailable time, col[3] = duration of unavailable time.
    std::vector<std::vector<std::vector<int> > > unavailMatrix; // 50 X 4 X nNurses 3d matrix, col[0] = unavailable shift number, col[1] = start of unavailable time, col[2] = end of unavailable time, col[3] = duration of unavailable time.
    //int*** capabilityOfDoubleServices; // 3D array, size = nNurses x nNurses x nDoubleService. For each pair of nurses, are the nurses capable of performing the double service together?
    std::vector<std::vector<std::vector<int> > > capabilityOfDoubleServices; // 3D array, size = nNurses x nNurses x nDoubleService. For each pair of nurses, are the nurses capable of performing the double service together?
    //double* nurseWaitingTime; // 1D array, size = 1 x nNurses. Total waiting time for each nurse.
    std::vector<double> nurseWaitingTime; // 1D array, size = 1 x nNurses. Total waiting time for each nurse.
    //double* nurseTravelTime; // 1D array, size = 1 x nNurses. Total travel time for each nurse.
    std::vector<double> nurseTravelTime; // 1D array, size = 1 x nNurses. Total travel time for each nurse.
    //double* violatedTW; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
    std::vector<double> violatedTW; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
    //double* violatedTWMK; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
    std::vector<double> violatedTWMK; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
    //double* algorithmOptions; // 1D array, size = 1 x 100, set in python file instance_handler.py, def default_options_vector.
    std::vector<double> algorithmOptions; // 1D array, size = 1 x 100, set in python file instance_handler.py, def default_options_vector.
    //double** od; // 2D array, size = nJobs+1 x nJobs+1. Orgin-destination matrix: od[i][j] = time taken to travel from job i to job j. Note that i,j=0 was originally set to be used as depot, but is no longer used.
    std::vector<std::vector<double> > od; // 2D array, size = nJobs+1 x nJobs+1. Orgin-destination matrix: od[i][j] = time taken to travel from job i to job j. Note that i,j=0 was originally set to be used as depot, but is no longer used.
    //double** nurseTravelFromDepot; // 2D array, size = nNurses x nJobs. For each nurse i (row): distance from nurse i's home to each of the job locations j = 1,..,nJobs.
    std::vector<std::vector<double> > nurseTravelFromDepot; // 2D array, size = nNurses x nJobs. For each nurse i (row): distance from nurse i's home to each of the job locations j = 1,..,nJobs.
    //double** nurseTravelToDepot; // 2D array, size = nNurses x nJobs. For each nurse i (row), distance from each job location j=1,...,nJobs, to nurse i's home.
    std::vector<std::vector<double> > nurseTravelToDepot; // 2D array, size = nNurses x nJobs. For each nurse i (row), distance from each job location j=1,...,nJobs, to nurse i's home.
    //double** timeMatrix; // 2D array, size = nNurses x nJobs. Time at which each nurse i does job j. If nurse i does not do job j, then timeMatrix[i][j] = -1.
    std::vector<std::vector<double> > timeMatrix; // 2D array, size = nNurses x nJobs. Time at which each nurse i does job j. If nurse i does not do job j, then timeMatrix[i][j] = -1.
    //double** prefScore; // 2D array, size = nJobs x nNurses (note that the matrix dimension are the other way around compared to the others). Preference score: -ve if best to avoid, +ve if suitable.
    std::vector<std::vector<double> > prefScore; // 2D array, size = nJobs x nNurses (note that the matrix dimension are the other way around compared to the others). Preference score: -ve if best to avoid, +ve if suitable.
    //double** nurseWaitingMatrix; //2D array, size nNurses x nJobs, contains waiting time for each job that nurse is assigned. If nurse is not assigned job, or there is no waiting time, then = 0
    std::vector<std::vector<double> > nurseWaitingMatrix; //2D array, size nNurses x nJobs, contains waiting time for each job that nurse is assigned. If nurse is not assigned job, or there is no waiting time, then = 0
    //double** nurseTravelMatrix; //2D array, size nNurses x nJobs, contains travel time to each job that nurse is assigned. If nurse is not assigned job, or there is no travel time, then = 0
    std::vector<std::vector<double> > nurseTravelMatrix; //2D array, size nNurses x nJobs, contains travel time to each job that nurse is assigned. If nurse is not assigned job, or there is no travel time, then = 0
    // int startTime;
    // int ** jobTimeWindow;
    // int * jobDuration;
    // int **nurseSkilled;

};

struct Instance InstanceFromPython(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data, double* od_data, double* nurseTravelFromDepot_data,
                                   double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data, int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* doubleService_data,
                                   int* dependsOn_data, int* mkMinD_data, int* mkMaxD_data, int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data);

struct Instance GenerateInstance();

void NurseSkilledFromSkillsAndRequirements(std::vector<std::vector<int> >& nurseSkills, std::vector<std::vector<int> >& jobRequirements, std::vector<std::vector<int> >& nurseSkilled,
                                           std::vector<int>& doubleService, int nJobs, int nNurses, int nSkills);

struct Instance CopyInstance(struct Instance* originalInst);

#endif //EA_ALG_READINST_H
