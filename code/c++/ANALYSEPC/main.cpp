/*------------------/
ALH
main.cpp
ANALYSEPC: Program to read in csv file of postcodes and output csv file with status of postcodes.
02/12/2020
/-------------------*/

/* The input and output files have been hardcoded to "input.csv" and "output.csv" respectively.
 * invalidPCs and unknownPCs are hardcoded (sets of strings, see consts.h/.cpp) to prevent having to provide a database along with this program.
 * Note that this program is only suitable for Wiltshire, Hampshire, and Monmouthshire. */

#include <iostream>

#include "fns.h"

int main() {

    std::string inputColName; //Stores name of column/column header (cell A1) from input.csv file.
    std::vector<std::string> inputPCs; //Input Postcodes - stores postcodes from input.csv.

    //Read in the data from input.csv:
    ReadCSV("input.csv", inputColName, inputPCs);

    //Create an output.csv file containing the status of each postcode in input.csv:
    WriteCSV("output.csv", inputPCs, invalidPCs, unknownPCs);

}