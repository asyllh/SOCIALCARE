/*--------------/
EA_ALG
main.cpp
UoS
27/01/2022
* Change all int* integer function parameters to int& integer
* Change all arrays/mallocs to std::vector or appropriate container
* Change all var names to camelCase
* INSTANCE -> Instance
*
/--------------*/


#include <stdlib.h> // To have min in linux
#include "Python.h"
#include "inst.h"
#include "fns.h"
#include "getsetcheckfind.h"

//#include "grasp.h"


int main(int argc, char const* argv[]){
    struct Instance inst = GenerateInstance();
    struct Instance* ip = &inst;
    ip->verbose = 4;
    return MainWithOutput(ip, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

MODULE_API int PythonEntry(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data,
                            double* od_data, double* nurseTravelFromDepot_data, double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data,
                            int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* solMatrixPointer,
                            int* doubleService_data, int* dependsOn_data, int* mkMinD_data, int* mkMaxD_data, int* capabilityOfDoubleServices_data, double* prefScore_data,
                            double* algorithmOptions_data, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                            double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer, int randomSeed){


    double eps = 1e-6;
    int printInputData = (int)algorithmOptions_data[99] + eps;

    int qualityMeasureModAPI = (int)(algorithmOptions_data[0] + eps); // printf("Quality measure	= %d\n\n", qualityMeasureModAPI);
    int doGapNotPrecedence = (int)(algorithmOptions_data[12] + eps); // printf("Use gaps for dependent jobs (rather than precedence)	= %d\n", doGapNotPrecedence);

    if(qualityMeasureModAPI == 0 && doGapNotPrecedence == 0){
        std::cout << "WARNING: Solving with AIT H quality measure but with PRECEDENCE rather than GAP.\n" << std::endl;
    }


    // printf("In C.\nReading input...\n");
    struct Instance inst = InstanceFromPython(nJobs_data, nNurses_data, nSkills_data, verbose_data, MAX_TIME_SECONDS, twInterval_data, excludeNurseTravel_data, od_data, nurseTravelFromDepot_data,
                                              nurseTravelToDepot_data, unavailMatrix_data, nurseUnavail_data, nurseWorkingTimes_data, jobTimeInfo_data, jobRequirements_data, nurseSkills_data,
                                              doubleService_data, dependsOn_data, mkMinD_data, mkMaxD_data, capabilityOfDoubleServices_data, prefScore_data, algorithmOptions_data);


    //Calculating the minimum preference score
    double MMM = 10000;
    double minPref = 0.0;
    // First, ignore doubleServices (DS's)
    for(int i = 0; i < inst.nJobs; ++i){
        if(inst.doubleService[i] > 0){
            continue;
        }
        double best_a = MMM;
        for(int j = 0; j < inst.nNurses; ++j){
            if(CheckSkills(&inst, i, j)){
                if(inst.prefScore[i][j] < best_a){
                    best_a = inst.prefScore[i][j];
                }
            }
        }
        minPref += best_a; // Why +=? Why not just =?
    }
    // Add DS's now
    for(int i = 0; i < inst.nJobs; ++i){
        if(inst.doubleService[i] < 1){
            continue;
        }
        double best_a = MMM;
        for(int j = 0; j < inst.nNurses; ++j){
            for(int k = 0; k < inst.nNurses; ++k){
                if(CheckSkillsDS(&inst, i, j, k)){
                    double combPref = inst.prefScore[i][j] + inst.prefScore[i][k];
                    if(combPref < best_a)
                        best_a = combPref;
                }
            }
        }
        minPref += best_a; // Why +=? Why not just =?
    }
    std::cout << "THE MIN PREF OF THIS INSTANCE IS:\n" << std::endl;
    std::cout << "<-< minPref = %.2f >->\n" << minPref << std::endl;

    struct Instance* ip = &inst;
    if(randomSeed >= 0){
        // printf("Setting a fixed random seed: %d.\n", randomSeed);
        srand(randomSeed);
    }
    else{
        // printf("Setting a true random seed.\n");
        srand(time(NULL));
    }

    int thenum = rand();

    // Select the type of solver used:
    int retvalue = 0;
    double int_tol = 0.0001;
    retvalue = MainWithOutput(ip, od_data, solMatrixPointer, timeMatrixPointer, nurseWaitingTimePointer, nurseTravelTimePointer, violatedTWPointer, nurseWaitingMatrixPointer, nurseTravelMatrixPointer, totalsArrayPointer);

    return retvalue;

} // END OF MODULE_API int python_entry function
