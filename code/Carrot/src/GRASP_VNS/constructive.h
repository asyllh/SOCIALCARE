/*--------------/
GRASP_VNS
constructive.h
UoS
10/07/2021
/--------------*/


#include <stdio.h>
#include <stdbool.h> // Required to use 'bool' variable type
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
// #  ifdef MODULE_API_EXPORTS
#    define MODULE_API __declspec(dllexport)
// #  else
// #    define MODULE_API __declspec(dllimport)
// #  endif
#else
#  define MODULE_API
#endif


#define bigM 9999999;

struct INSTANCE {
	int nJobs; // Number of jobs
    int nJobsIncDS; //Number of tasks in total, nJobs+nDS
	int nNurses; // Number of carers
	int nSkills; //Number of skills
	int verbose; // Additional information
	int isFeasible; // Feasibility (deprecated)
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
	double objAitHQuality;
	double objMKQuality;
	double objWBQuality;
	double objPaperQuality;
	double objQuality;
    double objMaxTravelTime;
    double objMaxWaitingTime;
    double objMaxDiffWorkload;
	float totalPref; // Total preference score
	int qualityMeasure; // Temp. Allow to choose (Mk, Ait h., etc.) Ideally we want a vector with weights! This should be set ONLY in "InstanceFromPython" (read_instance.c). The value is hardcoded at the moment.
	float MAX_TIME_SECONDS; // Maximum time limit in seconds
	int twInterval; // Time window interval
	bool excludeNurseTravel; // True if excluding the nurse travel time from home to the first job when updating current time in 'set_nurse_time' function, else false.
    int*** unavailMatrix; // 50 X 4 X nNurses 3d matrix, col[0] = unavailable shift number, col[1] = start of unavailable time, col[2] = end of unavailable time, col[3] = duration of unavailable time.
    int* nurseUnavail; // 1D array, size = nNurses, for each nurse contains the number of unavailable shifts.
	int** nurseWorkingTimes; // UPDATED: 2D array, size = nNurses x 5. For each nurse i (row): column[0] = start time, column[1] = finish time, column[2] = length of day for nurse, 
    // col[3] = # of shifts for that nurse, col[4] = duration of total shift working time. previously nNurses x 3, only first 3 columns used.
	int** solMatrix; // 2D array, size = nNurses x nJobs. For each nurse i (row), the element in solMatrix[i][j] gives the position in nurse i's route where nurse i visits job j.
	// E.g. if solMatrix[2][4] = 3 -> the third job in nurse 2's route is job 4. If solMatrix[i][j] = -1, then job j is not in nurse i's route.
	int** jobTimeInfo; // 2D array, size = nJobs x 3. For each job i (row): column[0] = start time window, column[1] = finish time window, column[2] = length of job.
	int** jobRequirements; // 2D array, size = nJobs x nSkills. Skill 0 means headcount needed. (deprecated)
	int** nurseSkills; // 2D array, size = nNurses x nSkills. Skill 0 is always 1 (one worker). (deprecated)
	int** nurseSkilled; // 2D array, size = nNurses x nJobs. For each nurse i (row) and each job j (column): nurseSkilled[i][j] = 1 if nurse i can do job j, = 0 otherwise.
	int* nurseRoute; // 1D array, size = 1 x nJobs. Used as temp. array to avoid allocations.
	int** allNurseRoutes; // 2D array, size = nNurses x nJobs. For each nurse i (row), the element in allNurseRoutes[i][j] gives the job in the POSITION j in nurse i's route.
	int* nurseOrder; // 1D array, size = 1 x nNurses. Integer values of order of nurses. Order in which nurses are considered (set randomly in the constructive).
	int* doubleService; // 1D array, size = 1 x nJobs. Indicates which jobs need more than one nurse. = 1 if job is DS, = 0 otherwise.
	int* dependsOn; // 1D array, size = 1 x nJobs. Indicates which jobs depend on another one. If job 2 depends on job 5, then dependsOn[2] = 5 and dependsOn[5] = 2.
	double** od; // 2D array, size = nJobs+1 x nJobs+1. Orgin-destination matrix: od[i][j] = time taken to travel from job i to job j. Note that i,j=0 was originally set to be used as depot, but is no longer used.
	double** nurseTravelFromDepot; // 2D array, size = nNurses x nJobs. For each nurse i (row): distance from nurse i's home to each of the job locations j = 1,..,nJobs.
	double** nurseTravelToDepot; // 2D array, size = nNurses x nJobs. For each nurse i (row), distance from each job location j=1,...,nJobs, to nurse i's home.
	double** timeMatrix; // 2D array, size = nNurses x nJobs. Time at which each nurse i does job j. If nurse i does not do job j, then timeMatrix[i][j] = -1.
	double* nurseWaitingTime; // 1D array, size = 1 x nNurses. Total waiting time for each nurse.
	double* nurseTravelTime; // 1D array, size = 1 x nNurses. Total travel time for each nurse.
	double* violatedTW; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
	double* violatedTWMK; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
	int* mkMinD; // 1D array, size = 1 x nJobs. Minimum gap with time of next job.
	int* mkMaxD; // 1D array, size = 1 x nJobs. Maximum gap with time of next job.
	int*** capabilityOfDoubleServices; // 3D array, size = nNurses x nNurses x nDoubleService. For each pair of nurses, are the nurses capable of performing the double service together?
	double** prefScore; // 2D array, size = nJobs x nNurses (note that the matrix dimension are the other way around compared to the others). Preference score: -ve if best to avoid, +ve if suitable.
	double* algorithmOptions; // 1D array, size = 1 x 100, set in python file instance_handler.py, def default_options_vector.
	double** nurseWaitingMatrix; //2D array, size nNurses x nJobs, contains waiting time for each job that nurse is assigned. If nurse is not assigned job, or there is no waiting time, then = 0
	double** nurseTravelMatrix; //2D array, size nNurses x nJobs, contains travel time to each job that nurse is assigned. If nurse is not assigned job, or there is no travel time, then = 0
    double totalServiceTime; // New 06/11/2021, moving total service time here as it should be fixed rather than calculating it every time in Objective function
    double totalServiceTimeIncDS; // New 06/11/2021, totalServiceTime but also including double time for the double services; this will therefore be the total working time of all nurses.
    double** arrivalTimes; //NEW 28/02/2022, size = nNurses x nJobs, keeps the original arrival time of nurse i at job j.
};

MODULE_API int python_entry(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data,
                            double* od_data, double* nurseTravelFromDepot_data, double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data, int* nurseWorkingTimes_data,
                            int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* solMatrixPointer, int* doubleService_data, int* dependsOn_data, int* mkMinD_data, int* mkMaxD_data,
                            int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data, double* timeMatrixPointer, double* nurseWaitingTimePointer,
                            double* nurseTravelTimePointer, double* violatedTWPointer, double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer, int randomSeed);

int MainWithOutput(struct INSTANCE* ip, double* odmat_pointer, int* solMatrixPointer, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                   double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer);
void SolnToPythomFormat(struct INSTANCE* ip, int* solMatrixPointer, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                        double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer);
int CheckSkills(struct INSTANCE* ip, int job, int nurse);
int CheckSkillsDSFirst(struct INSTANCE* ip, int job, int nursei);
int CheckSkillsDS(struct INSTANCE* ip, int job, int nursei, int nursej);
void InitialJobAssignment(struct INSTANCE* ip);
int ClosestUnallocatedPointNurse(struct INSTANCE* ip, int* allocatedJobs, int job, int nurse);
int FurthestUnallocatedPointNurse(struct INSTANCE* ip, int* allocatedJobs, int nurse);
void PrintIntMatrix(int** matrix, int nRows, int nCols);
void PrintIntMatrixOne(int* matrix, int nRows, int nCols);
void PrintDoubleMatrixOne(double* matrix, int nRows, int nCols);
void PrintSolMatrix(struct INSTANCE* ip);
void PrintAllNurseRoutes(struct INSTANCE* ip);
double GetTravelTime(struct INSTANCE* ip, int i, int j);
double TravelTimeFromDepot(struct INSTANCE* ip, int nurse, int job);
double TravelTimeToDepot(struct INSTANCE* ip, int nurse, int job);
int ReportSolution(struct INSTANCE* ip);
int SwapPoints(struct INSTANCE* ip, int ni, int nj, int pi, int pj);
int RemoveJob(struct INSTANCE* ip, int job, int ni);
int BestJobInsertion(struct INSTANCE* ip, int job, int ni);
int InsertJobAtPosition(struct INSTANCE* ip, int job, int ni, int posi);
int BestSyncDoubleSwitch(struct INSTANCE* ip);
int RouteTwoExchange(struct INSTANCE* ip, int firstImprovement);
int ExchangeJobsInRoute(struct INSTANCE* ip, int ni, int job_from_ni, int nj, int job_from_nj);
int BestSwitch(struct INSTANCE* ip, int onlyInfeasible, double MAX_TIME);
void FreeMatrixInt(int** matrix, int nRows);
int CopyIntMatrix(int** source, int** destination, int nRows, int nCols);
int NurseTwoExchange(struct INSTANCE* ip);
int TwoOptMove(struct INSTANCE* ip, int ni, int pos1, int pos2);
int FindSecondNurseDS(struct INSTANCE* ip, int job, int currentNurse);
int SwitchNurse(struct INSTANCE* ip, int ni, int nj, int pi);
int GetJobCount(struct INSTANCE* ip, int ni);
int GetNurseJobCount(struct INSTANCE* ip, int nurse);
void SetAllNurseRoutes(struct INSTANCE* ip);
void SetNurseRoute(struct INSTANCE* ip, int ni);
void GetNurseRoute(struct INSTANCE* ip, int ni, int* nurseRoute);
void CalculateJobTimes(struct INSTANCE* ip, int nursei);
double* FindValidTime(struct INSTANCE* ip, int f, double currentTime, int currentNurse, int job, int considerDependency, int otherNurseDJ, int otherJobDJ, int considerDoubleService, int otherNurseDS, double startTWMK, double endTWMK);
double* OriginalFindValidTime(struct INSTANCE* ip, int f, double currentTime, int currentNurse, int job, int considerDependency, int otherNurseDJ, int otherJobDJ, int considerDoubleService, int otherNurseDS, double startTWMK, double endTWMK);
void SetTimesFull(struct INSTANCE* ip);
void SetTimesFrom(struct INSTANCE* ip, int firstNurse);
int SynchroniseJobi(struct INSTANCE* ip, int job, int nurse1, int nurse2);
double SolutionQualityLight(struct INSTANCE* ip);
double SolutionQuality(struct INSTANCE* ip, int report);
int* MinsToTime(double time);
int* MinsToMinSecs(double time);
double ObjectiveNew(struct INSTANCE* ip, int report);
int RandomInteger(int min_val, int max_val);
void TwoExchange(int* array, int i, int j);
void RandomTwoExchange(int* array, size_t n, int* i, int* j);
void Shuffle(int* array, size_t n); // From Ben Pfaff's Writings, see below
void PrintVector(int* array, size_t n);
double MaxNumDouble(double num1, double num2);
void RemoveBreaksWaitingTime(struct INSTANCE* ip);


// Not in constructive.c
void FreeInstanceMemory(struct INSTANCE* ip);
void FreeInstanceCopy(struct INSTANCE* ip);


// Reading data functions: read_instance.c
struct INSTANCE InstanceFromPython(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data, double* od_data, double* nurseTravelFromDepot_data,
                                   double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data, int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* doubleService_data,
                                   int* dependsOn_data, int* mkMinD_data, int* mkMaxD_data, int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data);
struct INSTANCE GenerateInstance();
struct INSTANCE CopyInstance(struct INSTANCE* original_instance);
void OverwriteInstance(struct INSTANCE* ip, struct INSTANCE* oi);
void NurseSkilledFromSkillsAndRequirements(int** nurseSkills, int** jobRequirements, int** nurseSkilled, int* doubleService, int nJobs, int nNurses, int nSkills);


