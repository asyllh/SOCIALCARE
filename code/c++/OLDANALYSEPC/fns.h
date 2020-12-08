/*------------------
ANALYSEPC
fns.h
02/12/2020
Program to read in csv file of postcodes and output csv file with status of postcodes.
-------------------*/

#ifndef ANALYSEPC_FNS_H
#define ANALYSEPC_FNS_H

#include "consts.h"

void ReadCSV(std::string inputFileName, std::string& inputColName, std::vector<std::string>& inputPCs);

void ReadTXT(std::string filename, std::set<std::string>& PCs);

void WriteCSV(std::string filename, std::vector<std::string>& inputPCs, std::set<std::string>& invalidPCs, std::set<std::string>& unknownPCs);

void WriteTXT(std::string filename, std::set<std::string>& PCs);


#endif //ANALYSEPC_CONSTS_H