/*------------------
ANALYSEPC
main.cpp
02/12/2020
Program to read in csv file of postcodes and output csv file with status of postcodes.
-------------------*/

#include <iostream>

#include "fns.h"

int main() {

    std::string inputColName;
    std::vector<std::string> inputPCs;
    ReadCSV("input.csv", inputColName, inputPCs);

    WriteCSV("output.csv", inputPCs, invalidPCs, unknownPCs);

}