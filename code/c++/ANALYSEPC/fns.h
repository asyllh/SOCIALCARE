/*------------------/
ALH
fns.h
ANALYSEPC: Program to read in csv file of postcodes and output csv file with status of postcodes.
02/12/2020
/-------------------*/

#ifndef ANALYSEPC_FNS_H
#define ANALYSEPC_FNS_H

#include "consts.h"

void ReadCSV(std::string inputFileName, std::string& inputColName, std::vector<std::string>& inputPCs);

void WriteCSV(std::string filename, std::vector<std::string>& inputPCs, std::set<std::string>& invalidPCs, std::set<std::string>& unknownPCs);


#endif //ANALYSEPC_FNS_H