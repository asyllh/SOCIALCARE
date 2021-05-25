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
	int nNurses; // Number of nurses
	int nSkills; //Number of skills
	int verbose; // Additional information
	int isFeasible; // Feasibility (deprecated)
	float objTime; // Objective time
	float objTravel; // Objective travel time
	float objWaiting; // Objective waiting time, NEW 07/01/2021
	float objTardiness; // Objective tardiness
	float objShortestDay; // Objective shortest day, NEW 07/01/2021
	float objLongestDay; // Objective longest day
	float objOvertime; // Objective overtime
	float totPref; // Total preference score
	int quality_measure; // Temp. Allow to choose (Mk, Ait h., etc.) Ideally we want a vector with weights! This should be set ONLY in "instance_from_python" (read_instance.c). The value is hardcoded at the moment.
	float MAX_TIME_SECONDS; // Maximum time limit in seconds
	int tw_interval; // Time window interval
	bool exclude_nurse_travel; // True if excluding the nurse travel time from home to the first job when updating current time in 'set_nurse_time' function, else false.
	// double **od_cost;
	int** nurseWorkingTimes; // 2D array, size = nNurses x 3. For each nurse i (row): column[0] = start time, column[1] = finish time, column[2] = max working time. (column[2] not used?)
	int** solMatrix; // 2D array, size = nNurses x nJobs. For each nurse i (row), the element in solMatrix[i][j] gives the position in nurse i's route where nurse i visits job j.
	// E.g. if solMatrix[2][4] = 3 -> the third job in nurse 2's route is job 4. If solMatrix[i][j] = -1, then job j is not in nurse i's route.
	int** jobTimeInfo; // 2D array, size = nJobs x 3. For each job i (row): column[0] = start time window, column[1] = finish time window, column[2] = length of job.
	int** jobRequirements; // 2D array, size = nJobs x nSkills. Skill 0 means headcount needed. (deprecated)
	int** nurseSkills; // 2D array, size = nNurses x nSkills. Skill 0 is always 1 (one worker). (deprecated)
	int** nurseSkilled; // 2D array, size = nNurses x nJobs. For each nurse i (row) and each job j (column): nurseSkilled[i][j] = 1 if nurse i can do job j, = 0 otherwise.
	int* nurseRoute; // 1D array, size = 1 x nJobs. Used as temp. array to avoid allocations.
	int** allNurseRoutes; // 2D array, size = nNurses x nJobs (dynamic). Same as solMatrix, but removing all -1 elements in route. Keep all nurse routes here, only update if necessary.
	int* nurseOrder; // 1D array, size = 1 x nNurses. Integer values of order of nurses. Order in which nurses are considered (set randomly in the constructive).
	int* doubleService; // 1D array, size = 1 x nJobs. Indicates which jobs need more than one nurse. = 1 if job is DS, = 0 otherwise.
	int* dependsOn; // 1D array, size = 1 x nJobs. Indicates which jobs depend on another one. If job 2 depends on job 5, then dependsOn[2] = 5 and dependsOn[5] = 2.
	double** od; // 2D array, size = nJobs+1 x nJobs+1. Orgin-destination matrix: od[i][j] = time taken to travel from job i to job j. Note that i,j=0 was originally set to be used as depot, but is no longer used.
	double** nurse_travel_from_depot; // 2D array, size = nNurses x nJobs. For each nurse i (row): distance from nurse i's home to each of the job locations j = 1,..,nJobs.
	double** nurse_travel_to_depot; // 2D array, size = nNurses x nJobs. For each nurse i (row), distance from each job location j=1,...,nJobs, to nurse i's home.
	double** timeMatrix; // 2D array, size = nNurses x nJobs. Time at which each nurse i does job j. If nurse i does not do job j, then timeMatrix[i][j] = -1.
	double* nurseWaitingTime; // 1D array, size = 1 x nNurses. Total waiting time for each nurse.
	double* nurseTravelTime; // 1D array, size = 1 x nNurses. Total travel time for each nurse.
	double* violatedTW; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
	double* violatedTWMK; // 1D array, size = 1 x nJobs. How late does each job start. If not late then 0, else +ve value.
	int* MK_mind; // 1D array, size = 1 x nJobs. Minimum gap with time of next job.
	int* MK_maxd; // 1D array, size = 1 x nJobs. Maximum gap with time of next job.
	int*** capabilityOfDoubleServices; // 3D array, size = nNurses x nNurses x nDoubleService. For each pair of nurses, are the nurses capable of performing the double service together?
	double** prefScore; // 2D array, size = nJobs x nNurses (note that the matrix dimension are the other way around compared to the others). Preference score: -ve if best to avoid, +ve if suitable.
	double* algorithmOptions; // 1D array, size = 1 x 100, set in python file instance_handler.py, def default_options_vector.
    int*** unavailMatrix; // 10 X 4 X nNurses 3d matrix, col[0] = unavailable shift number, col[1] = start of unavailable time, col[2] = end of unavailable time, col[3] = duration of unavailable time.
    int* nurseUnavail; // 1D array, size = nNurses, for each nurse contains the number of unavailable shifts.
	// int startTime;
	// int ** jobTimeWindow;
	// int * jobDuration;
	// int **nurseSkilled;

};

MODULE_API int python_entry(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int tw_interval_data, bool exclude_nurse_travel_data, double* od_data, double* nurse_travel_from_depot,
	double* nurse_travel_to_depot, int* unavail_matrix_data, int* nurse_unavail_data, int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* solMatrixPointer, int* doubleService,
	int* dependsOn, int* mk_mind, int* mk_maxd, int* capabilityOfDoubleServices, double* prefScore, double* algorithmOptions_data, int randomSeed);
int main_with_output(struct INSTANCE* ip, int* solMatrixPointer, double* odmat_pointer);
void solmatrix_to_python_format(struct INSTANCE* ip, int* solMatrixPointer);
void constructive_basic(struct INSTANCE* ip);
int check_skills(struct INSTANCE* ip, int job, int nurse);
int check_skills_ds_first(struct INSTANCE* ip, int job, int nursei);
int check_skills_ds(struct INSTANCE* ip, int job, int nursei, int nursej);
void initial_job_assignment(struct INSTANCE* ip);
int find_closest_unallocated_point_for_nurse(struct INSTANCE* ip, int* allocatedJobs, int job, int nurse);
int find_furthest_unallocated_point_for_nurse(struct INSTANCE* ip, int* allocatedJobs, int nurse);
void print_int_matrix(int** matrix, int nRows, int nCols);
void print_int_matrix_one(int* matrix, int nRows, int nCols);
void print_double_matrix_one(double* matrix, int nRows, int nCols);
void print_double_matrix(double** matrix, int nRows, int nCols);
void print_solmatrix(struct INSTANCE* ip);
void print_timeMatrix(struct INSTANCE* ip);
double get_travel_time(struct INSTANCE* ip, int i, int j);
double get_travel_time_from_depot(struct INSTANCE* ip, int nurse, int job);
double get_travel_time_to_depot(struct INSTANCE* ip, int nurse, int job);
double get_travel_time_bnc(struct INSTANCE* ip, int nodei, int nodej);
int report_solution(struct INSTANCE* ip);
int swap_points(struct INSTANCE* ip, int ni, int nj, int pi, int pj);
int remove_job(struct INSTANCE* ip, int job, int ni);
int job_insertion_last(struct INSTANCE* ip, int job, int ni);
int best_job_insertion(struct INSTANCE* ip, int job, int ni);
int insert_job_at_position(struct INSTANCE* ip, int job, int ni, int posi);
int best_sync_double_switch(struct INSTANCE* ip);
int route_two_exchange(struct INSTANCE* ip, int firstImprovement);
int exchange_jobs_in_route(struct INSTANCE* ip, int ni, int job_from_ni, int nj, int job_from_nj);
int best_switch(struct INSTANCE* ip, int onlyInfeasible, double MAX_TIME);
void free_matrix_int(int** matrix, int nRows);
int copy_int_matrix(int** source, int** destination, int nRows, int nCols);
int nurse_two_exchange(struct INSTANCE* ip);
int two_opt_move(struct INSTANCE* ip, int ni, int pos1, int pos2);
int find_second_nurse_ds(struct INSTANCE* ip, int job, int currentNurse);
int switch_nurse(struct INSTANCE* ip, int ni, int nj, int pi);
int repair(struct INSTANCE* ip, int ni);
int get_job_count(struct INSTANCE* ip, int ni);
int get_nurse_job_count(struct INSTANCE* ip, int nurse);
void set_all_nurse_routes(struct INSTANCE* ip);
void set_nurse_route(struct INSTANCE* ip, int ni);
void get_nurse_route(struct INSTANCE* ip, int ni, int* nurseRoute);
void print_nurse_route(struct INSTANCE* ip, int ni, int* nurseRoute);
void set_nurse_time(struct INSTANCE* ip, int nursej);
void new_set_nurse_time(struct INSTANCE* ip, int nursej);
void set_times_full(struct INSTANCE* ip);
void set_times_from(struct INSTANCE* ip, int first_nurse);
int synchronise_job_i(struct INSTANCE* ip, int job, int nurse1, int nurse2);
double sol_quality_global(struct INSTANCE* ip, int report);
double sol_quality_light(struct INSTANCE* ip);
double sol_quality_optimised(struct INSTANCE* ip, int n1, int n2);
double sol_quality(struct INSTANCE* ip, int report);
int synchronise_jobs(struct INSTANCE* ip);
// void mins_to_time(double time, int* hours, int* minutes, int* seconds);
int* mins_to_time(double time);
// double mins_to_minsecs(double time);
int* mins_to_minsecs(double time);
double obj_from_times(struct INSTANCE* ip, int report);
double alternative_quality(struct INSTANCE* ip, int report);
int random_integer(int min_val, int max_val);
void two_exchange(int* array, int i, int j);
void random_two_exchange(int* array, size_t n, int* i, int* j);
void shuffle(int* array, size_t n); // From Ben Pfaff's Writings, see below
void print_vector(int* array, size_t n);


// Not in constructive.c
void access_nurse_route(struct INSTANCE* ip, int ni, int* nurseRoute);
int schedule_quality(struct INSTANCE* ip, int ni);
void free_instance_memory(struct INSTANCE* ip);
void free_instance_copy(struct INSTANCE* ip);


// Reading data functions: read_instance.c
struct INSTANCE instance_from_python(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int tw_interval_data, bool exclude_nurse_travel_data, double* od_data, double* nurse_travel_from_depot,
	double* nurse_travel_to_depot, int* unavail_matrix_data, int* nurse_unavail_data, int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* doubleService_data,
	int* dependsOn_data, int* mk_mind_data, int* mk_maxd_data, int* capabilityOfDoubleServices, double* prefScore, double* algorithmOptions);
struct INSTANCE generate_instance();
struct INSTANCE copy_instance(struct INSTANCE* original_instance);
void overwrite_instance(struct INSTANCE* ip, struct INSTANCE* oi);
void nurse_skilled_from_skills_and_requirements(int** nurseSkills, int** jobRequirements, int** nurseSkilled, int* doubleService, int nJobs, int nNurses, int nSkills);


