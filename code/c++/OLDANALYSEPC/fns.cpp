/*------------------
ANALYSEPC
fns.cpp
02/12/2020
Program to read in csv file of postcodes and output csv file with status of postcodes.
-------------------*/

#include "fns.h"

void ReadCSV(std::string inputFileName, std::string& inputColName, std::vector<std::string>& inputPCs) {

    std::ifstream ifs(inputFileName);
    if(!ifs){
        std::cerr << "[ERROR]: Input CSV file name must be 'input.csv'." << std::endl;
        exit(1);
    }
    std::string firstLine;
    std::getline(ifs, firstLine);
    inputColName = firstLine;

    std::string line;
    while(std::getline(ifs, line)) {
        std::erase(line, ' ');
        inputPCs.push_back(line);
    }

}

void ReadTXT(std::string filename, std::set<std::string>& PCs) {

    std::ifstream ifs(filename);

    if (!ifs){
        std::cerr << "[ERROR]: Cannot read from input text file." << std::endl;
        exit(1);
    }

    if (ifs.good()) {
        std::string line;
        while(std::getline(ifs, line)) {
            std::erase(line, ' ');
            PCs.insert(line);
        }
    }

    ifs.close();

}

void WriteCSV(std::string filename, std::vector<std::string>& inputPCs, std::set<std::string>& invalidPCs, std::set<std::string>& unknownPCs) {

    std::ofstream ofs(filename);
    ofs << "Postcodes" << "," << "Status" << std::endl;

    for(int i = 0; i < inputPCs.size(); ++i) {
        std::set<std::string>::iterator iti = invalidPCs.find(inputPCs[i]);
        if(iti != invalidPCs.end()) {
            ofs << *iti << "," << "Remove" << std::endl;
        }
        else {
            std::set<std::string>::iterator itu = unknownPCs.find(inputPCs[i]);
            if(itu != unknownPCs.end()) {
                ofs << *itu << "," << "Not Found" << std::endl;
            }
            else{
                ofs << inputPCs[i] << "," << "OK" << std::endl;
            }
        }
    }

    ofs.close();
}

void WriteTXT(std::string filename, std::set<std::string>& PCs){

    std::ofstream ofs(filename);
    for(const auto& i : PCs){
        ofs << "\"" << i << "\", ";
    }

}


