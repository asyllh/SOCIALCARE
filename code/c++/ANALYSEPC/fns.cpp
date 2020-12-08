/*------------------/
ALH
fns.cpp
ANALYSEPC: Program to read in csv file of postcodes and output csv file with status of postcodes.
02/12/2020
/-------------------*/

#include "fns.h"

void ReadCSV(std::string inputFileName, std::string& inputColName, std::vector<std::string>& inputPCs) {

    //This function reads in input.csv file and stores each postcode as a string in vector inputPCs, *without* spaces.

    std::ifstream ifs(inputFileName);
    if(!ifs){
        std::cerr << "[ERROR]: Input CSV file name must be 'input.csv'." << std::endl;
        exit(1);
    }

    //Read the column header name
    std::string firstLine;
    std::getline(ifs, firstLine);
    inputColName = firstLine;

    //Read each line of the file (each postcode) as a string, removing any spaces, and add to inputPCs.
    std::string line;
    while(std::getline(ifs, line)) {
        std::erase(line, ' ');
        inputPCs.push_back(line);
    }

} //End ReadCSV function.


void WriteCSV(std::string filename, std::vector<std::string>& inputPCs, std::set<std::string>& invalidPCs, std::set<std::string>& unknownPCs) {

    /* This function assesses each postcode in inputPCs to see if the postcode is invalid or cannot be identified.
     * The function then creates an ouput.csv file, and writes to it the postcode from inputPCs and the status of the postcode (two columns):
     * "OK": the postcode is not in invalidPCs or unknownPCs, and so it is fine to use;
     * "Remove": the postcode has been found in invalidPCs, and so there are < 12 addresses at that postcode - the postcode cannot be used for privacy reasons;
     * "Not Found": the postcode has been found in unknownPCs, which means that the postcode cannot be identified.
     * Commas must be used to separate data into two columns. */

    std::ofstream ofs(filename);
    if(!ofs){
        std::cerr << "[ERROR]: Cannot write to output file." << std::endl;
        exit(1);
    }
    ofs << "Postcodes" << "," << "Status" << std::endl; //Write the two column headers.

    for(int i = 0; i < inputPCs.size(); ++i) {
        std::set<std::string>::iterator iti = invalidPCs.find(inputPCs[i]); //Check if inputPC[i] is in invalidPCs, returns iterator
        if(iti != invalidPCs.end()) {
            ofs << *iti << "," << "Remove" << std::endl;
        }
        else {
            std::set<std::string>::iterator itu = unknownPCs.find(inputPCs[i]); //Check if inputPC[i] is in unknownPCs, returns iterator
            if(itu != unknownPCs.end()) {
                ofs << *itu << "," << "Not Found" << std::endl;
            }
            else{ //inputPC[i] postcode is not in either set, so postcode is OK to use.
                ofs << inputPCs[i] << "," << "OK" << std::endl;
            }
        }
    }

    ofs.close();

} //End WriteCSV function.



