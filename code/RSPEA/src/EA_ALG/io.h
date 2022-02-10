/*--------------/
EA_ALG
io.h
UoS
01/02/2022
/--------------*/

#ifndef EA_ALG_IO_H
#define EA_ALG_IO_H

#include "consts.h"
#include "inst.h"

void PrintSolMatrix(struct Instance* ip);

void PrintIntMatrix(std::vector<std::vector<int> >& matrix, int nRows, int nCols);

void PrintAllNurseRoutes(struct Instance* ip);

int CopyIntMatrix(std::vector<std::vector<int> >& source, std::vector<std::vector<int> >& destination, int nRows, int nCols);


#endif //EA_ALG_IO_H
