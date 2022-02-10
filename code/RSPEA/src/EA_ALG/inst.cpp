/*--------------/
EA_ALG
inst.cpp
UoS
27/01/2022
/--------------*/
#include "inst.h"


struct Instance InstanceFromPython(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data,
                                   double* od_data, double* nurseTravelFromDepot_data, double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data,
                                   int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* doubleService_data, int* dependsOn_data,
                                   int* mkMinD_data, int* mkMaxD_data, int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data) {

    int printAllAllocations = 0;

    // qualityMeasure = 1 is Mk, 0 is Ait H.
    // Add very small number just to prevent accuracy errors
    int quality_measure = (int)(algorithmOptions_data[0] + 1e-6);

    if (printAllAllocations > 0){
        verbose_data = 6;
    }

    printAllAllocations = verbose_data;

    int ct = -1;
    int nRows, nCols;
    int nJobs = nJobs_data;
    int nNurses = nNurses_data;
    int nSkills = nSkills_data;

    ///////////////////// ALLOCATE MEMORY /////////////////////

    // 1. double** od: nJobs+1 x nJobs+1 (using double* od_data)
    std::vector<std::vector<double> > od(nJobs+1, std::vector<double>(nJobs+1));
    //nRows = nJobs + 1;
    //nCols = nJobs + 1;
    //double** od;
    //od = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    od[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            od[i][j] = (double) od_data[ct];
        }
    }

    // 2. double** nurseTravelFromDepot: nNurses x nJobs (using double* carer_travel_from_depot_data)
    std::vector<std::vector<double> > nurseTravelFromDepot(nNurses, std::vector<double>(nJobs));
    //nRows = nNurses;
    //nCols = nJobs;
    ct = -1;
    //double** nurseTravelFromDepot;
    //nurseTravelFromDepot = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    nurseTravelFromDepot[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseTravelFromDepot[i][j] = (double) nurseTravelFromDepot_data[ct];
        }
    }

    // 3. double** nurseTravelToDepot: nNurses x nJobs (using double* nurse_travel_to_depot_data)
    std::vector<std::vector<double> > nurseTravelToDepot(nNurses, std::vector<double>(nJobs));
    //double** nurseTravelToDepot;
    ct = -1;
    //nurseTravelToDepot = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    nurseTravelToDepot[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseTravelToDepot[i][j] = (double) nurseTravelToDepot_data[ct];
        }
    }


    // 4. int** nurseWorkingTimes: nNurses x 5 (using int* carerWorkingTimes_data) Note: changed from nNurses x 3 to nNurses x 5 on 04/11/2021
    std::vector<std::vector<int> > nurseWorkingTimes(nNurses, std::vector<int>(5));
    //int** nurseWorkingTimes;
    //nRows = nNurses;
    //nCols = 5; // Changed from 3 to 5 on 04/11/2021
    //nurseWorkingTimes = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    nurseWorkingTimes[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseWorkingTimes[i][j] = nurseWorkingTimes_data[ct];
        }
    }


    // 5. int** nurseSkills: nNurses x nSkills (using int* nurseSkills_data)
    std::vector<std::vector<int> > nurseSkills(nNurses, std::vector<int>(nSkills));
    //int** nurseSkills;
    //nRows = nNurses;
    //nCols = nSkills;
    //nurseSkills = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    nurseSkills[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseSkills[i][j] = nurseSkills_data[ct];
        }
    }


    // 6. int** jobRequirements: nJobs x nSkills (using int* jobRequirements_data)
    std::vector<std::vector<int> > jobRequirements(nJobs, std::vector<int>(nSkills));
    //int** jobRequirements;
    //nRows = nJobs;
    //nCols = nSkills;
    //jobRequirements = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    jobRequirements[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            jobRequirements[i][j] = jobRequirements_data[ct];
        }
    }

    // 7. int** jobTimeInfo: nJobs x 3 (using int* jobTimeInfo_data)
    std::vector<std::vector<int> > jobTimeInfo(nJobs, std::vector<int>(3));
    //int** jobTimeInfo;
    //nRows = nJobs;
    //nCols = 3;
    //jobTimeInfo = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    jobTimeInfo[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            jobTimeInfo[i][j] = jobTimeInfo_data[ct];
        }
    }

    // 8. int** solMatrix: nNurses x nJobs (set to all -1)
    std::vector<std::vector<int> > solMatrix(nNurses, std::vector<int>(nJobs));
    //int** solMatrix;
    //nRows = nNurses;
    //nCols = nJobs;
    //solMatrix = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    solMatrix[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrix[i][j] = -1;
        }
    }


    // 9. double** timeMatrix: nNurses x nJobs (set to all -1)
    std::vector<std::vector<double> > timeMatrix(nNurses, std::vector<double>(nJobs));
    //double** timeMatrix;
    //nRows = nNurses;
    //nCols = nJobs;
    //timeMatrix = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    timeMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            timeMatrix[i][j] = -1;
        }
    }



    // 10. int* nurseRoute: 1 x nJobs
    std::vector<int> nurseRoute(nJobs);
    //int* nurseRoute = malloc(nJobs * sizeof(int));


    // 11. int** allNurseRoutes: nNurses x nJobs (set to all -1)
    std::vector<std::vector<int> > allNurseRoutes(nNurses, std::vector<int>(nJobs));
    //int** allNurseRoutes;
    //nRows = nNurses;
    //nCols = nJobs;
    //allNurseRoutes = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    allNurseRoutes[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allNurseRoutes[i][j] = -1;
        }
    }


    // 12. double* nurseWaitingTime: 1 x nNurses
    std::vector<double> nurseWaitingTime(nNurses);
    //double* nurseWaitingTime = malloc(nNurses * sizeof(double));

    // 13. double nurseTravelTime: 1 x nNurses
    std::vector<double> nurseTravelTime(nNurses);
    //double* nurseTravelTime = malloc(nNurses * sizeof(double));

    // 14. - 17. int* doubleService, dependsOn, mkMinD, mkMaxD: 1 x nJobs (using _data)
    std::vector<int> doubleService(nJobs);
    std::vector<int> dependsOn(nJobs);
    std::vector<int> mkMinD(nJobs);
    std::vector<int> mkMaxD(nJobs);
    //int* doubleService = malloc(nJobs * sizeof(int));
    //int* dependsOn = malloc(nJobs * sizeof(int));
    //int* mkMinD = malloc(nJobs * sizeof(int));
    //int* mkMaxD = malloc(nJobs * sizeof(int));
    int nDoubleServices = 0;
    for (int i = 0; i < nJobs; ++i) {
        doubleService[i] = doubleService_data[i];
        dependsOn[i] = dependsOn_data[i];
        mkMinD[i] = mkMinD_data[i + 1];
        mkMaxD[i] = mkMaxD_data[i + 1];
        nDoubleServices += doubleService_data[i];
    }
    int nJobsIncDS = nJobs + nDoubleServices;

    // 18. int* nurseUnavail: 1 x nNurses (using int* carer_unavail_data) (24/05/2021)
    std::vector<int> nurseUnavail(nNurses);
    //int* nurseUnavail = malloc(nNurses * sizeof(int));
    for (int i = 0; i < nNurses; i++){
        nurseUnavail[i] = nurseUnavail_data[i];
    }

    // 19. double* violatedTW: 1 x nJobs
    std::vector<double> violatedTW(nJobs);
    //double* violatedTW = malloc(nJobs * sizeof(double));

    // 20. double* violatedTWMK: 1 x nJobs
    std::vector<double> violatedTWMK(nJobs);
    //double* violatedTWMK = malloc(nJobs * sizeof(double));

    // 21. int** nurseSkilled: nNurses x nJobs (set using nurseSkills, jobRequirements, doubleService)
    std::vector<std::vector<int> > nurseSkilled(nNurses, std::vector<int>(nJobs));
    //int** nurseSkilled;
    //nRows = nNurses;
    //nCols = nJobs;
    //nurseSkilled = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    nurseSkilled[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    NurseSkilledFromSkillsAndRequirements(nurseSkills, jobRequirements, nurseSkilled, doubleService, nJobs, nNurses, nSkills);


    // 22. double** prefScore: nJobs x nNurses (using double* prefScore_data)
    std::vector<std::vector<double> > prefScore(nJobs, std::vector<double>(nNurses));
    //double** prefScore; // Job x Nurse
    //nRows = nJobs;
    //nCols = nNurses;
    //prefScore = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    prefScore[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            prefScore[i][j] = prefScore_data[ct];
        }
    }


    // 23. double* algorithmOptions: 1 x 100 (using double* algorithmOptions_data)
    int nOptions = 100;
    std::vector<double> algorithmOptions(nOptions);
    //double* algorithmOptions = malloc(nOptions * sizeof(double));
    for (int i = 0; i < nOptions; ++i) {
        algorithmOptions[i] = algorithmOptions_data[i];
    }

    // 24. int*** capabilityOfDoubleServices: nNurses x nNurses x nDoubleServices (using int* capabilityOfDoubleServices_data)
    std::vector<std::vector<std::vector<int> > > capabilityOfDoubleServices(nNurses, std::vector<std::vector<int> >(nNurses, std::vector<int>(nDoubleServices)));
    //int*** capabilityOfDoubleServices;
    //nRows = nNurses;
    //nCols = nNurses;
    // dim3 is nDoubleServices
    //capabilityOfDoubleServices = malloc(nRows * sizeof(int**)); // Rows
    //for (int i = 0; i < nRows; i++) {
    //    capabilityOfDoubleServices[i] = malloc(nCols * sizeof(int*)); // Cols
    //    for (int j = 0; j < nCols; j++){
    //        capabilityOfDoubleServices[i][j] = malloc(nDoubleServices*sizeof(int)); // Cols
    //    }
    //}
    ct = -1;
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            for (int k = 0; k < nDoubleServices; ++k) {
                ct++;
                capabilityOfDoubleServices[i][j][k] = capabilityOfDoubleServices_data[ct];
            }
        }
    }

    // 25. int*** unavailMatrix: 10 x 4 x nNurses (using int* unavail_matrix_data) (24/05/2021)
    std::vector<std::vector<std::vector<int> > > unavailMatrix(50, std::vector<std::vector<int> >(4, std::vector<int>(nNurses)));
    //int*** unavailMatrix;
    //nRows = 50; /** NEED TO CHANGE THIS **/
    //nCols = 4;
    // dim3 is nNurses
    //unavailMatrix = malloc(nRows * sizeof(int**)); // Rows
    //for (int i = 0; i < nRows; i++) {
    //    unavailMatrix[i] = malloc(nCols * sizeof(int*)); // Cols
    //    for (int j = 0; j < nCols; j++){
    //        unavailMatrix[i][j] = malloc(nNurses*sizeof(int));
    //    }
    //}
    ct = -1;
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            for (int k = 0; k < nNurses; ++k) {
                ct++;
                unavailMatrix[i][j][k] = unavailMatrix_data[ct];
            }
        }
    }

    // 26. double** nurseWaitingMatrix: nNurses x nJobs (set to all 0)
    std::vector<std::vector<double> > nurseWaitingMatrix(nNurses, std::vector<double>(nJobs));
    // NB: NEW: 03/06/2021
    //double** nurseWaitingMatrix;
    //nRows = nNurses;
    //nCols = nJobs;
    //nurseWaitingMatrix = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    nurseWaitingMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseWaitingMatrix[i][j] = 0;
        }
    }


    // 27. double** nurseWTravelMatrix: nNurses x nJobs (set to all 0)
    std::vector<std::vector<double> > nurseTravelMatrix(nNurses, std::vector<double>(nJobs));
    // NB: NEW: 03/06/2021
    //double** nurseTravelMatrix;
    //nRows = nNurses;
    //nCols = nJobs;
    //nurseTravelMatrix = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    nurseTravelMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseTravelMatrix[i][j] = 0;
        }
    }


    // 28. int* nurseOrder: 1 x nNurses (0... nNurses-1)
    std::vector<int> nurseOrder(nNurses);
    //int* nurseOrder = malloc(nNurses * sizeof(int));
    for (int i = 0; i < nNurses; ++i){
        nurseOrder[i] = i;
    }

    //29. NB NEW 06/11/2021 - calculate totalServiceTime
    double totalServiceTime = 0.0;
    for (int j = 0; j < nJobs; ++j){
        totalServiceTime += jobTimeInfo[j][2];
    }

    //30. NB NEW 06/11/2021 - calculate totalServiceTimeIncDS - which is totalServiceTime plus double the time for double services.
    double totalServiceTimeIncDS = totalServiceTime;
    for (int j = 0; j < nJobs; ++j){
        if(doubleService[j] == 1){
            totalServiceTimeIncDS += jobTimeInfo[j][2];
        }
    }

    verbose_data = printAllAllocations;	// Return this to what it was

    struct Instance inst = { .nJobs = nJobs,
            .nJobsIncDS = nJobsIncDS, // ints // NB: NEW 08/11/2021
            .nNurses = nNurses,
            .nSkills = nSkills,
            .verbose = verbose_data,
            .isFeasible = 0,
            .qualityMeasure = quality_measure,
            .twInterval = twInterval_data,
            .totalServiceTime = totalServiceTime, // double//NB: NEW: 06/11/2021
            .totalServiceTimeIncDS = totalServiceTimeIncDS, //NB NEW 06/11/2021
            .MAX_TIME_SECONDS = MAX_TIME_SECONDS,
            .excludeNurseTravel = excludeNurseTravel_data, // bool
            .nurseOrder = nurseOrder, // vector int 1d
            .nurseRoute = nurseRoute,
            .doubleService = doubleService,
            .dependsOn = dependsOn,
            .mkMinD = mkMinD,
            .mkMaxD = mkMaxD,
            .nurseUnavail = nurseUnavail,
            .solMatrix = solMatrix, // vector int 2d
            .nurseWorkingTimes = nurseWorkingTimes,
            .jobTimeInfo = jobTimeInfo,
            .jobRequirements = jobRequirements,
            .nurseSkills = nurseSkills,
            .nurseSkilled = nurseSkilled,
            .allNurseRoutes = allNurseRoutes,
            .unavailMatrix = unavailMatrix, // vector int 3d
            .capabilityOfDoubleServices = capabilityOfDoubleServices,
            .nurseWaitingTime = nurseWaitingTime, // vector double 1d
            .nurseTravelTime = nurseTravelTime,
            .violatedTW = violatedTW,
            .violatedTWMK = violatedTWMK,
            .algorithmOptions = algorithmOptions,
            .od = od, // vector double 2d
            .nurseTravelFromDepot = nurseTravelFromDepot,
            .nurseTravelToDepot = nurseTravelToDepot,
            .timeMatrix = timeMatrix,
            .prefScore = prefScore,
            .nurseWaitingMatrix = nurseWaitingMatrix, // NB: NEW: 03/06/2021
            .nurseTravelMatrix = nurseTravelMatrix // NB: NEW: 03/06/2021

    };

    return inst;

} // End of InstanceFromPython function

struct Instance GenerateInstance() {

    int nRows, nCols;
    int verbose_data = 110;
    int nJobs = 10;
    int nNurses = 2;
    int nSkills = 4;

    int nurseWorkingTimes_data[2][3] = { {72, 276, 96}, {72, 276, 96} };


    int jobTimeInfo_data[10][3] = { {96, 131, 18},
                                    {132, 155, 18},
                                    {192, 240, 6},
                                    {192, 240, 6},
                                    {132, 155, 12},
                                    {96, 131, 24},
                                    {96, 131, 18},
                                    {96, 131, 21},
                                    {72, 95, 12},
                                    {210, 210, 9} };

    int nurseSkills_data[2][4] = { {1, 1, 1, 1}, {1, 1, 0, 0} };

    int jobRequirements_data[10][4] = { {0, 0, 0, 0},
                                        {0, 0, 0, 0},
                                        {1, 1, 1, 1},
                                        {0, 0, 0, 0},
                                        {1, 1, 0, 0},
                                        {1, 1, 0, 0},
                                        {1, 1, 0, 0},
                                        {0, 0, 0, 0},
                                        {1, 1, 0, 0},
                                        {1, 1, 0, 0} };

    int od_data[11][11] = { {0, 1, 2, 0, 2, 2, 3, 3, 1, 1, 1},
                            {1, 0, 2, 1, 1, 4, 4, 3, 3, 3, 1},
                            {1, 3, 0, 2, 2, 4, 4, 1, 1, 1, 0},
                            {0, 0, 3, 0, 0, 3, 3, 3, 3, 3, 2},
                            {2, 0, 3, 0, 0, 3, 3, 3, 3, 3, 2},
                            {2, 3, 4, 3, 3, 0, 0, 3, 3, 3, 3},
                            {3, 3, 4, 3, 3, 0, 0, 3, 3, 3, 3},
                            {3, 4, 1, 3, 3, 4, 4, 0, 0, 0, 1},
                            {1, 4, 1, 3, 3, 4, 4, 0, 0, 0, 1},
                            {1, 4, 1, 3, 3, 4, 4, 0, 0, 0, 1},
                            {1, 2, 0, 2, 2, 3, 3, 1, 1, 1, 0} };

    int nurseSkilled_data[2][10];


    ///////////////////// SEE WHAT CAN EACH NURSE DO ///////////////

    int canDoIt = 1;
    for (int i = 0; i < nNurses; ++i) {
        for (int j = 0; j < nJobs; ++j) {
            canDoIt = 1;
            for (int k = 0; k < nSkills; ++k) {
                if ((double)nurseSkills_data[i][k] < ((double)(jobRequirements_data[j][k]) - 0.5)) {
                    canDoIt = 0;
                    break;
                }
            }
            nurseSkilled_data[i][j] = canDoIt;
        }
    }


    ///////////////////// ALLOCATE MEMORY /////////////////////

    // int **od;.
    std::vector<std::vector<double> > od(nJobs+1, std::vector<double>(nJobs+1));
    //nRows = nJobs + 1;
    //nCols = nJobs + 1;
    //double** od;
    // double ** od_cost = NULL; // Not used at the moment
    //od = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    od[i] = malloc(nCols * sizeof(double)); // Cols
    // od_cost[i] = malloc(nCols * sizeof(int)); // No need to allocate this at the moment;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            od[i][j] = (double) od_data[i][j];
        }
    }

    std::vector<std::vector<int> > nurseSkilled(nNurses, std::vector<int>(nJobs));
    //int** nurseSkilled;
    //nRows = nNurses;
    //nCols = nJobs;
    //nurseSkilled = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    nurseSkilled[i] = malloc(nCols * sizeof(int)); // Cols
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseSkilled[i][j] = nurseSkilled_data[i][j];
        }
    }




    // nurseWorkingTimes_data
    std::vector<std::vector<int> > nurseWorkingTimes(nNurses, std::vector<int>(3));
    //int** nurseWorkingTimes;
    //nRows = nNurses;
    //nCols = 3;
    //nurseWorkingTimes = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    nurseWorkingTimes[i] = malloc(nCols * sizeof(int)); // Cols
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseWorkingTimes[i][j] = nurseWorkingTimes_data[i][j];
        }
    }



    // nurseSkills_data
    std::vector<std::vector<int> > nurseSkills(nNurses, std::vector<int>(nSkills));
    //int** nurseSkills;
    //nRows = nNurses;
    //nCols = nSkills;
    //nurseSkills = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    nurseSkills[i] = malloc(nCols * sizeof(int)); // Cols
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseSkills[i][j] = nurseSkills_data[i][j];
        }
    }


    // jobRequirements_data
    std::vector<std::vector<int> > jobRequirements(nJobs, std::vector<int>(nSkills));
    //int** jobRequirements;
    //nRows = nJobs;
    //nCols = nSkills;
    //jobRequirements = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    jobRequirements[i] = malloc(nCols * sizeof(int)); // Cols
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            jobRequirements[i][j] = jobRequirements_data[i][j];
        }
    }



    std::vector<std::vector<int> > jobTimeInfo(nJobs, std::vector<int>(3));
    //int** jobTimeInfo;
    //nRows = nJobs;
    //nCols = 3;
    //jobTimeInfo = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    jobTimeInfo[i] = malloc(nCols * sizeof(int)); // Cols

    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            jobTimeInfo[i][j] = jobTimeInfo_data[i][j];
        }
    }


    std::vector<std::vector<int> > solMatrix(nNurses, std::vector<int>(nJobs));
    //int** solMatrix;
    //nRows = nNurses;
    //nCols = nJobs;
    //solMatrix = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    solMatrix[i] = malloc(nCols * sizeof(int)); // Cols
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrix[i][j] = -1;
        }
    }


    // int * nurseRoute = 	malloc(nJobs * sizeof(int));
    std::vector<std::vector<double> > timeMatrix(nNurses, std::vector<double>(nJobs));
    //double** timeMatrix;
    //nRows = nNurses;
    //nCols = nJobs;
    //timeMatrix = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++)
    //    timeMatrix[i] = malloc(nCols * sizeof(double)); // Cols
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            timeMatrix[i][j] = -1;
        }
    }


    std::vector<int> nurseRoute(nJobs);
    std::vector<double> nurseWaitingTime(nNurses);
    std::vector<int> doubleService(nJobs);
    std::vector<double> violatedTW(nJobs);
    //int* nurseRoute = malloc(nJobs * sizeof(int));
    //double* nurseWaitingTime = malloc(nNurses * sizeof(double));
    //int* doubleService = malloc(nJobs * sizeof(int));
    //double* violatedTW = malloc(nJobs * sizeof(double));

    double MAX_TIME_SECONDS = 600;

    struct Instance inst = { .nJobs = nJobs,
            .nNurses = nNurses,
            .nSkills = nSkills,
            .verbose = verbose_data,
            .isFeasible = 0,
            .MAX_TIME_SECONDS = MAX_TIME_SECONDS,
            .nurseRoute = nurseRoute,
            .doubleService = doubleService,
            .solMatrix = solMatrix,
            .nurseWorkingTimes = nurseWorkingTimes,
            .jobTimeInfo = jobTimeInfo,
            .jobRequirements = jobRequirements,
            .nurseSkills = nurseSkills,
            .nurseSkilled = nurseSkilled,
            .nurseWaitingTime = nurseWaitingTime,
            .violatedTW = violatedTW,
            .od = od,
            .timeMatrix = timeMatrix
    };

    return inst;

} // End GenerateInstance function

void NurseSkilledFromSkillsAndRequirements(std::vector<std::vector<int> >& nurseSkills, std::vector<std::vector<int> >& jobRequirements, std::vector<std::vector<int> >& nurseSkilled,
                                           std::vector<int>& doubleService, int nJobs, int nNurses, int nSkills) {
    int canDoIt = 1;
    for (int i = 0; i < nNurses; ++i) {
        for (int j = 0; j < nJobs; ++j) {
            canDoIt = 1;
            for (int k = 0; k < nSkills; ++k) {
                if (nurseSkills[i][k] < jobRequirements[j][k]) {
                    canDoIt = 0;
                    break;
                }
            }
            nurseSkilled[i][j] = canDoIt;
        }
    }


    int nursesThatCanDoIt = 0;
    for (int j = 0; j < nJobs; ++j) {
        nursesThatCanDoIt = 0;
        for (int i = 0; i < nNurses; ++i) {
            if (nurseSkilled[i][j] > 0)
                nursesThatCanDoIt++;
        }
        if ((nursesThatCanDoIt < 1) && (doubleService[j] < 1)) {
            printf("ERROR: there are no single nurses skilled for job %d (not a DS)\n", j);
            // if (doubleService_data[j] > 0)
            // 	printf("(DS) ");
            // printf("can be done by %d nurses!!!\n", nursesThatCanDoIt);
            exit(-2343);
        }
    }

}// End of NurseSkilledFromSkillsAndRequirements function


struct Instance CopyInstance(struct Instance* originalInst) {
    int LOCAL_VERBOSE = originalInst->verbose;

    std::vector<std::vector<int> > solMatrix(originalInst->nNurses, std::vector<int>(originalInst->nJobs));
    //int** solMatrix;
    int nRows = originalInst->nNurses;
    int nCols = originalInst->nJobs;
    //solMatrix = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    solMatrix[i] = malloc(nCols*sizeof(int)); // Col
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrix[i][j] = originalInst->solMatrix[i][j];
        }
    }



    std::vector<std::vector<double> > timeMatrix(originalInst->nNurses, std::vector<double>(originalInst->nJobs));
    //double** timeMatrix;
    nRows = originalInst->nNurses;
    nCols = originalInst->nJobs;
    //timeMatrix = malloc(nRows * sizeof(double*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    timeMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            timeMatrix[i][j] = originalInst->timeMatrix[i][j];
        }
    }

    std::vector<std::vector<int> > allNurseRoutes(originalInst->nNurses, std::vector<int>(originalInst->nJobs));
    //int** allNurseRoutes;
    nRows = originalInst->nNurses;
    nCols = originalInst->nJobs;
    //allNurseRoutes = malloc(nRows * sizeof(int*)); // Rows
    //for (int i = 0; i < nRows; i++){
    //    allNurseRoutes[i] = malloc(nCols*sizeof(int)); // Cols
    //}
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allNurseRoutes[i][j] = originalInst->allNurseRoutes[i][j];
        }
    }


    /*double** nurseWaitingMatrix; //NB: NEW: 03/06/2021
    nRows = original_instance->nNurses;
    nCols = original_instance->nJobs;
    nurseWaitingMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseWaitingMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseWaitingMatrix[i][j] = original_instance->nurseWaitingMatrix[i][j];
        }
    }
    if (LOCAL_VERBOSE > 5){
        printf("Allocated nurseWaitingMatrix. (copying)\n");
    }

    double** nurseTravelMatrix; //NB: NEW: 03/06/2021
    nRows = original_instance->nNurses;
    nCols = original_instance->nJobs;
    nurseTravelMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseTravelMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseTravelMatrix[i][j] = original_instance->nurseTravelMatrix[i][j];
        }
    }
    if (LOCAL_VERBOSE > 5){
        printf("Allocated nurseTravelMatrix. (copying)\n");
    }*/

    // Allocate and populate vectors of size nNurses
    std::vector<double> nurseWaitingTime(originalInst->nNurses);
    std::vector<double> nurseTravelTime(originalInst->nNurses);
    std::vector<int> nurseOrder(originalInst->nNurses);
    //double* nurseWaitingTime = malloc(originalInst->nNurses * sizeof(double));
    //double* nurseTravelTime = malloc(originalInst->nNurses * sizeof(double));
    //int* nurseOrder = malloc(originalInst->nNurses * sizeof(int));
    for (int i = 0; i < originalInst->nNurses; ++i) {
        nurseOrder[i] = originalInst->nurseOrder[i];
        nurseWaitingTime[i] = originalInst->nurseWaitingTime[i];
        nurseTravelTime[i] = originalInst->nurseTravelTime[i];
    }


    // Allocate and populate vectors of size nJobs
    std::vector<int> nurseRoute(originalInst->nJobs);
    std::vector<double> violatedTW(originalInst->nJobs);
    std::vector<double> violatedTWMK(originalInst->nJobs);
    //int* nurseRoute = malloc(originalInst->nJobs * sizeof(int));
    //double* violatedTW = malloc(originalInst->nJobs * sizeof(double));
    //double* violatedTWMK = malloc(originalInst->nJobs * sizeof(double));
    for (int i = 0; i < originalInst->nJobs; ++i) {
        nurseRoute[i] = originalInst->nurseRoute[i];
        violatedTW[i] = originalInst->violatedTW[i];
        violatedTWMK[i] = originalInst->violatedTWMK[i];
    }


    if (LOCAL_VERBOSE > 5){
        printf("Creating instance struct... (copying)\n");
    }

    struct Instance inst = { .nJobs = originalInst->nJobs,
            .nJobsIncDS = originalInst->nJobsIncDS, // ints // NB: NEW 08/11/2021
            .nNurses = originalInst->nNurses,
            .nSkills = originalInst->nSkills,
            .verbose = originalInst->verbose,
            .isFeasible = 0,
            .qualityMeasure = originalInst->qualityMeasure,
            .twInterval = originalInst->twInterval,
            .totalServiceTime = originalInst->totalServiceTime, // double//NB: NEW: 06/11/2021
            .totalServiceTimeIncDS = originalInst->totalServiceTimeIncDS, //NB NEW 06/11/2021
            .MAX_TIME_SECONDS = originalInst->MAX_TIME_SECONDS,
            .excludeNurseTravel = originalInst->excludeNurseTravel, // bool
            .nurseOrder = nurseOrder, // vector int 1d
            .nurseRoute = nurseRoute,
            .doubleService = originalInst->doubleService,
            .dependsOn = originalInst->dependsOn,
            .mkMinD = originalInst->mkMinD,
            .mkMaxD = originalInst->mkMaxD,
            .nurseUnavail = originalInst->nurseUnavail,
            .solMatrix = solMatrix, // vector int 2d
            .nurseWorkingTimes = originalInst->nurseWorkingTimes,
            .jobTimeInfo = originalInst->jobTimeInfo,
            .jobRequirements = originalInst->jobRequirements,
            .nurseSkills = originalInst->nurseSkills,
            .nurseSkilled = originalInst->nurseSkilled,
            .allNurseRoutes = allNurseRoutes,
            .unavailMatrix = originalInst->unavailMatrix, // vector int 3d
            .capabilityOfDoubleServices = originalInst->capabilityOfDoubleServices,
            .nurseWaitingTime = nurseWaitingTime, // vector double 1d
            .nurseTravelTime = nurseTravelTime,
            .violatedTW = violatedTW,
            .violatedTWMK = violatedTWMK,
            .algorithmOptions = originalInst->algorithmOptions,
            .od = originalInst->od, // vector double 2d
            .nurseTravelFromDepot = originalInst->nurseTravelFromDepot,
            .nurseTravelToDepot = originalInst->nurseTravelToDepot,
            .timeMatrix = timeMatrix,
            .prefScore = originalInst->prefScore,
            .nurseWaitingMatrix = originalInst->nurseWaitingMatrix, // NB: NEW: 03/06/2021
            .nurseTravelMatrix = originalInst->nurseTravelMatrix // NB: NEW: 03/06/2021
    };


    return inst;

}// End of CopyInstance function