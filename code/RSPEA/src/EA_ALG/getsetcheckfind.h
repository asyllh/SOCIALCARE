/*--------------/
EA_ALG
getset.h
UoS
01/02/2022
/--------------*/

#ifndef EA_ALG_GETSETCHECKFIND_H
#define EA_ALG_GETSETCHECKFIND_H

#include "consts.h"
#include "inst.h"
#include "fns.h"

void SetNurseRoute(struct Instance* ip, int ni);

void SetAllNurseRoutes(struct Instance* ip);

void SetNurseTime(struct Instance* ip, int nursei);

void SetTimesFrom(struct Instance* ip, int firstNurse);

void SetTimesFull(struct Instance* ip);

int CheckSkills(struct Instance* ip, int job, int nurse);

int CheckSkillsDS(struct Instance* ip, int job, int nursei, int nursej);

int CheckSkillsDSFirst(struct Instance* ip, int job, int nursei);

double GetTravelTime(struct Instance* ip, int i, int j);

double TravelTimeFromDepot(struct Instance* ip, int nurse, int job);

double TravelTimeToDepot(struct Instance* ip, int nurse, int job);

int GetNurseJobCount(struct Instance* ip, int nurse);

void GetNurseRoute(struct Instance* ip, int ni, std::vector<int>& nurseRoute);

int FindSecondNurseDS(struct Instance* ip, int job, int currentNurse);



#endif //EA_ALG_GETSETCHECKFIND_H
