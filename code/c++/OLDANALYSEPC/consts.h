/*------------------
ANALYSEPC
consts.h
02/12/2020
Program to read in csv file of postcodes and output csv file with status of postcodes.
-------------------*/

#ifndef ANALYSEPC_CONSTS_H
#define ANALYSEPC_CONSTS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>

extern std::set<std::string> invalidPCs;
extern std::set<std::string> unknownPCs;

#endif //ANALYSEPC_CONSTS_H
