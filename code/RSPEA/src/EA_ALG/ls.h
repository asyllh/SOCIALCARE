/*--------------/
EA_ALG
ls.h
UoS
01/02/2022
/--------------*/

#ifndef EA_ALG_LS_H
#define EA_ALG_LS_H

#include "consts.h"
#include "inst.h"
#include "fns.h"
#include "getsetcheckfind.h"

double StandardLocalSearchTest(struct Instance* ip, int MAX_ITERATIONS, double MAX_TIME, int TEST_ITERATIONS);

int BestSwitch(struct Instance* ip, int onlyInfeasible, double MAX_TIME);

int RouteTwoExchange(struct Instance* ip, int firstImprovement);

int NurseTwoExchange(struct Instance* ip);

int BestSyncDoubleSwitch(struct Instance* ip);

int TwoOptMove(struct Instance* ip, int ni, int pos1, int pos2);

int SwitchNurse(struct Instance* ip, int ni, int nj, int pi);

int ExchangeJobsInRoute(struct Instance* ip, int ni, int jobFromNi, int nj, int jobFromNj);

#endif //EA_ALG_LS_H
