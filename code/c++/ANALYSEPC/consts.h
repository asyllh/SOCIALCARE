/*------------------/
ALH
consts.h
ANALYSEPC: Program to read in csv file of postcodes and output csv file with status of postcodes.
02/12/2020
/-------------------*/

#ifndef ANALYSEPC_CONSTS_H
#define ANALYSEPC_CONSTS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>

extern std::set<std::string> invalidPCs; //Set of strings containing invalid postcodes - postcodes that comprise < 12 addresses, and so for privacy reasons cannot be used in the project.
extern std::set<std::string> unknownPCs; //Set of strings containing unknown postcodes - postcodes that cannot be identified.

#endif //ANALYSEPC_CONSTS_H
