/*--------------/
GRASP_VNS
grasp.h
UoS
10/07/2021
/--------------*/

#include "math.h"


int GRASP(struct INSTANCE * ip);
void randomised_constructive(struct INSTANCE * ip, int randomness, double delta, int rcl_strategy);
void grasp_ls(struct INSTANCE * ip, int LS_ITERS);
void generate_rcl(double delta, int * rcl_size, int * RCL_seeds, double * rankValues, int rv_size, double bestRank, double worstRank, int strategy);
void rcl_pick(int ** bestIndices, int * RCL_seeds, int rcl_size, int * cCarer, int * cJob);
void best_index_pick(int ** bestIndices, int GRASP_param, int * cCarer, int * cJob);
int pick_integer(int max_int);
void update_top_values(int GRASP_param, int ** bestIndices, double * bestValues, double newValue, int bestCarer, int bestJob);
void clean_solution_from_struct(struct INSTANCE * ip);
int identify_early_insert(struct INSTANCE * ip, struct INSTANCE * guiding, int guiding_carer, int guiding_carer_pos);
void find_best_worst_pool_qualities(double * pool_best, double * pool_worst, struct INSTANCE ** pool, double * pool_quality, int pool_size);
double solution_dissimilarity(struct INSTANCE * input1, struct INSTANCE * input2);
int find_arc_destination(int source_carer, int source_job, struct INSTANCE * ip);
void print_and_recalculate_pool_contents(struct INSTANCE ** pool, double * pool_quality,  int solutions_in_pool);
void calculate_and_print_dissimilartiy_matrix(struct INSTANCE ** pool, int solutions_in_pool);
double forward_path_relinking(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output);
double forward_and_backward_path_relinking(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output1);
double backward_path_relinking(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output);
double directed_path_relinking(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output, int direction);
double path_relinking(struct INSTANCE * ip, struct INSTANCE * guiding);
void get_carer_and_position_of_job(struct INSTANCE * ip, int job, int * carer, int * position);
void get_carer_and_position_of_job_ds(struct INSTANCE * ip, int job, int * carer1, int * position1, int * carer2, int * position2);
void standard_local_search(struct INSTANCE * ip, int MAX_ITERATIONS, double MAX_TIME);
double standard_local_search_test(struct INSTANCE * ip, int MAX_ITERATIONS, double MAX_TIME, int TEST_ITERATIONS);
double solution_distance(struct INSTANCE * input1, struct INSTANCE * input2);
// void standard_local_search_two_opt_first(struct INSTANCE * ip, int MAX_ITERATIONS, double MAX_TIME);
