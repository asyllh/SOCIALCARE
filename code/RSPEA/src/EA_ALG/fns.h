/*--------------/
EA_ALG
fns.h
UoS
27/01/2022
/--------------*/

#ifndef EA_ALG_FNS_H
#define EA_ALG_FNS_H

#include "consts.h"
#include "inst.h"
#include "io.h"
#include "ls.h"
#include "getsetcheckfind.h"

int MainWithOutput(struct Instance* ip, double* odmat_pointer, int* solMatrixPointer, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                   double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer);

void GRASP(struct Instance* ip);

void RandomisedConstructive(struct Instance* ip, int randomness, double delta, int rclStrategy);

void GenerateRCL(double delta, std::vector<int>& rclSeeds, int& rclSize, std::vector<double>& rankValues, int rvSize, double bestRank, double worstRank, int rclStrategy);

void RCLPick(std::vector<std::vector<int> >& bestIndices,std::vector<int>& RCL, int rclSize, int& cNurse, int& cJob);

int PickInteger(int maxInt);

int BestJobInsertion(struct Instance* ip, int job, int ni);

int InsertJobAtPosition(struct Instance* ip, int job, int ni, int posi);

int RemoveJob(struct Instance* ip, int job, int ni);

double SolutionDissimilarity(struct Instance* input1, struct Instance* input2);

int FindArcDestination(struct Instance* ip, int sourceNurse, int sourceJob);

double SolutionQuality(struct Instance* ip, int report);

double SolutionQualityLight(struct Instance* ip);

double ObjectiveFunction(struct Instance* ip, int report);

void RandomTwoExchange(std::vector<int>& array, size_t n, int* i, int* j);

void TwoExchange(std::vector<int>& array, int i, int j);

void Shuffle(std::vector<int>& array, size_t n); /** check this size_t n **/

int RandomInteger(int minVal, int maxVal);

void CleanSolutionFromStruct(struct Instance* ip);

int* MinsToTime(double time);

int* MinsToMinSecs(double time);

#endif //EA_ALG_FNS_H
