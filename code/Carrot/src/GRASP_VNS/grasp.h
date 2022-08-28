/*--------------/
GRASP_VNS
grasp.h
UoS
10/07/2021
/--------------*/

#include "math.h"


int GRASP(struct INSTANCE * ip);
void RandomisedConstructive(struct INSTANCE * ip, int randomness, double delta, int rclStrategy);
void GenerateRCL(double delta, int* rclSeeds, int* rclSize, double* rankValues, int rvSize, double bestRank, double worstRank, int strategy);
void RCLPick(int ** bestIndices, int * rclSeeds, int rclSize, int * cNurse, int * cJob);
int PickInteger(int max_int);
void CleanSolutionFromStruct(struct INSTANCE * ip);
int IdentifyEarlyInsert(struct INSTANCE * ip, struct INSTANCE * guiding, int guidingNurse, int guidingNursePos);
double SolutionDissimilarity(struct INSTANCE * input1, struct INSTANCE * input2);
int FindArcDestination(int sourceNurse, int sourceJob, struct INSTANCE * ip);
void PrintRecalculatePoolContents(struct INSTANCE ** pool, double * poolQuality, int solutionsInPool);
void CalculatePrintDissimilarityMatrix(struct INSTANCE ** pool, int solutionsInPool);
double ForwardPR(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output);
double ForwardBackwardPR(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output1);
double BackwardPR(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output);
double DirectedPR(struct INSTANCE * input1, struct INSTANCE * input2, double q1, double q2, struct INSTANCE * output, int direction);
double PathRelinking(struct INSTANCE * ip, struct INSTANCE * guiding);
void NurseAndJobPosition(struct INSTANCE * ip, int job, int * nurse, int * position);
void NurseAndJobPositionDS(struct INSTANCE * ip, int job, int * nurse1, int * position1, int * nurse2, int * position2);
void StandardLocalSearch(struct INSTANCE * ip, int MAX_ITERATIONS, double MAX_TIME);
double StandardLocalSearchTest(struct INSTANCE * ip, int MAX_ITERATIONS, double MAX_TIME, int TEST_ITERATIONS);
