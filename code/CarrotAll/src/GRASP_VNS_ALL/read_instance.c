/*--------------/
GRASP_VNS
read_instance.c
UoS
10/07/2021
/--------------*/


#include "constructive.h"
void OverwriteInstance(struct INSTANCE* ip, struct INSTANCE* oi) {
	/*
	Copies the contents of the solution of "oi" into "ip"
	without creating any new allocations.
	Only copies the solution, no algorithm options or data
	*/

	for (int i = 0; i < oi->nNurses; ++i){
        for(int j = 0; j < oi->nJobs; ++j){
            ip->solMatrix[i][j] = oi->solMatrix[i][j];
        }
    }

	for (int i = 0; i < oi->nNurses; ++i){
        for(int j = 0; j < oi->nJobs; ++j){
            ip->timeMatrix[i][j] = oi->timeMatrix[i][j];
        }
    }

	for (int i = 0; i < oi->nNurses; ++i){
        for(int j = 0; j < oi->nJobs; ++j){
            ip->allNurseRoutes[i][j] = oi->allNurseRoutes[i][j];
        }
    }

    for (int i = 0; i < oi->nNurses; ++i){ // NB: NEW: 03/06/2021
        for(int j = 0; j < oi->nJobs; ++j){
            ip->nurseWaitingMatrix[i][j] = oi->nurseWaitingMatrix[i][j];
        }
    }

    for (int i = 0; i < oi->nNurses; ++i){ // NB: NEW: 03/06/2021
        for(int j = 0; j < oi->nJobs; ++j){
            ip->nurseTravelMatrix[i][j] = oi->nurseTravelMatrix[i][j];
        }
    }

	for (int i = 0; i < oi->nNurses; ++i) {
		ip->nurseOrder[i] = oi->nurseOrder[i];
		ip->nurseWaitingTime[i] = oi->nurseWaitingTime[i];
		ip->nurseTravelTime[i] = oi->nurseTravelTime[i];
	}

	for (int i = 0; i < oi->nJobs; ++i) {
		ip->nurseRoute[i] = oi->nurseRoute[i];
		ip->violatedTW[i] = oi->violatedTW[i];
		ip->violatedTWMK[i] = oi->violatedTWMK[i];
	}

} //END OF OverwriteInstance function.

struct INSTANCE CopyInstance(struct INSTANCE* original_instance) {
	int LOCAL_VERBOSE = original_instance->verbose;

	int** solMatrix;
	int nRows = original_instance->nNurses;
	int nCols = original_instance->nJobs;
	solMatrix = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++){
        solMatrix[i] = malloc(nCols*sizeof(int)); // Cols
    }
	for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrix[i][j] = original_instance->solMatrix[i][j];
        }
    }
	if (LOCAL_VERBOSE > 5){
        printf("Allocated solMatrix. (copying)\n");
    }


	double** timeMatrix;
	nRows = original_instance->nNurses;
	nCols = original_instance->nJobs;
	timeMatrix = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++){
        timeMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
	for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            timeMatrix[i][j] = original_instance->timeMatrix[i][j];
        }
    }
	if (LOCAL_VERBOSE > 5){
        printf("Allocated timeMatrix. (copying)\n");
    }

	int** allNurseRoutes;
	nRows = original_instance->nNurses;
	nCols = original_instance->nJobs;
	allNurseRoutes = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++){
        allNurseRoutes[i] = malloc(nCols*sizeof(int)); // Cols
    }
	for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allNurseRoutes[i][j] = original_instance->allNurseRoutes[i][j];
        }
    }
	if (LOCAL_VERBOSE > 5){
        printf("Allocated allNurseRoutes. (copying)\n");
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
	double* nurseWaitingTime = malloc(original_instance->nNurses * sizeof(double));
	double* nurseTravelTime = malloc(original_instance->nNurses * sizeof(double));
	int* nurseOrder = malloc(original_instance->nNurses * sizeof(int));
	for (int i = 0; i < original_instance->nNurses; ++i) {
		nurseOrder[i] = original_instance->nurseOrder[i];
		nurseWaitingTime[i] = original_instance->nurseWaitingTime[i];
		nurseTravelTime[i] = original_instance->nurseTravelTime[i];
	}
	if (LOCAL_VERBOSE > 5){
        printf("Allocated vectors of size nNurses. (copying)\n");
    }

	// Allocate and populate vectors of size nJobs
	int* nurseRoute = malloc(original_instance->nJobs * sizeof(int));
	double* violatedTW = malloc(original_instance->nJobs * sizeof(double));
	double* violatedTWMK = malloc(original_instance->nJobs * sizeof(double));
	for (int i = 0; i < original_instance->nJobs; ++i) {
		nurseRoute[i] = original_instance->nurseRoute[i];
		violatedTW[i] = original_instance->violatedTW[i];
		violatedTWMK[i] = original_instance->violatedTWMK[i];
	}
	if (LOCAL_VERBOSE > 5){
        printf("Allocated vectors of size nJobs. (copying)\n");
    }

	if (LOCAL_VERBOSE > 5){
        printf("Creating instance struct... (copying)\n");
    }

	struct INSTANCE inst = { .nJobs = original_instance->nJobs,
                                    .nJobsIncDS = original_instance->nJobsIncDS, // NB: NEW 08/11/2021
									.nNurses = original_instance->nNurses,
									.nSkills = original_instance->nSkills,
									.verbose = original_instance->verbose,
									.qualityMeasure = original_instance->qualityMeasure,
									.MAX_TIME_SECONDS = original_instance->MAX_TIME_SECONDS,
                                    .twInterval = original_instance->twInterval,
                                    .excludeNurseTravel = original_instance->excludeNurseTravel,
									.od = original_instance->od,
									.nurseTravelFromDepot = original_instance->nurseTravelFromDepot,
									.nurseTravelToDepot = original_instance->nurseTravelToDepot,
                                    .unavailMatrix = original_instance->unavailMatrix,
                                    .nurseUnavail = original_instance->nurseUnavail,
									.nurseWorkingTimes = original_instance->nurseWorkingTimes,
									.solMatrix = solMatrix,
									.timeMatrix = timeMatrix,
									.jobTimeInfo = original_instance->jobTimeInfo,
									.jobRequirements = original_instance->jobRequirements,
									.nurseSkills = original_instance->nurseSkills,
									.nurseSkilled = original_instance->nurseSkilled,
									.nurseRoute = nurseRoute,
									.allNurseRoutes = allNurseRoutes,
									.nurseOrder = nurseOrder,
									.nurseWaitingTime = nurseWaitingTime,
									.nurseTravelTime = nurseTravelTime,
									.doubleService = original_instance->doubleService,
									.dependsOn = original_instance->dependsOn,
									.violatedTW = violatedTW,
									.violatedTWMK = violatedTWMK,
									.isFeasible = 0,
									.mkMinD = original_instance->mkMinD,
									.mkMaxD = original_instance->mkMaxD,
									.capabilityOfDoubleServices = original_instance->capabilityOfDoubleServices,
									.prefScore = original_instance->prefScore,
									.algorithmOptions = original_instance->algorithmOptions,
									.nurseWaitingMatrix = original_instance->nurseWaitingMatrix, //NB: NEW: 03/06/2021
									.nurseTravelMatrix = original_instance->nurseTravelMatrix, //NB: NEW: 03/06/2021
                                    .totalServiceTime = original_instance->totalServiceTime, //NB NEW: 06/11/2021
                                    .totalServiceTimeIncDS = original_instance->totalServiceTimeIncDS, //NB NEW: 06/11/2021
                                    .breakPoint = original_instance->breakPoint
	};

	if (LOCAL_VERBOSE > 5){
        printf("Done. (copying)\n");
    }
	return inst;

}


// // Functions to read instances, and some hardcoded instances too
struct INSTANCE InstanceFromPython(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data,
                                   double* od_data, double* nurseTravelFromDepot_data, double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data,
                                   int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* doubleService_data, int* dependsOn_data,
                                   int* mkMinD_data, int* mkMaxD_data, int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data) {

    int printAllAllocations = 0;

    // qualityMeasure = 1 is Mk, 0 is Ait H.
    // Add very small number just to prevent accuracy errors
    int qualityMeasure = (int)(algorithmOptions_data[0] + 1e-6);

    if (printAllAllocations > 0){
        verbose_data = 6;
    }

    printAllAllocations = verbose_data;

    int ct = -1;
    int nRows, nCols;
    int nJobs = nJobs_data;
    int nNurses = nNurses_data;
    int nSkills = nSkills_data;
    if (verbose_data > 5){
        printf("Starting allocation of memory\n");
    }
    ///////////////////// ALLOCATE MEMORY /////////////////////

    // 1. double** od: nJobs+1 x nJobs+1 (using double* od_data)
    nRows = nJobs + 1;
    nCols = nJobs + 1;
    double** od;
    od = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        od[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            od[i][j] = (double) od_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated od.\n");
    }


    // 2. double** nurseTravelFromDepot: nNurses x nJobs (using double* carer_travel_from_depot_data)
    nRows = nNurses;
    nCols = nJobs;
    ct = -1;
    double** nurseTravelFromDepot;
    nurseTravelFromDepot = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseTravelFromDepot[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseTravelFromDepot[i][j] = (double) nurseTravelFromDepot_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseTravelFromDepot.\n");
    }

    // 3. double** nurseTravelToDepot: nNurses x nJobs (using double* nurse_travel_to_depot_data)
    double** nurseTravelToDepot;
    ct = -1;
    nurseTravelToDepot = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseTravelToDepot[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseTravelToDepot[i][j] = (double) nurseTravelToDepot_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseTravelToDepot.\n");
    }

    // 4. int** nurseWorkingTimes: nNurses x 5 (using int* carerWorkingTimes_data) Note: changed from nNurses x 3 to nNurses x 5 on 04/11/2021
    int** nurseWorkingTimes;
    nRows = nNurses;
    /*if(qualityMeasure == 0){
        nCols = 3; // NEW: 11/11/2021, this is for Ait H only, there are only 3 columns for ait h: col[0] = start time, col[1] = end time, col[2] = length of day
    }
    else if(qualityMeasure == 6){
        nCols = 5; // Changed from 3 to 5 on 04/11/2021 for abicare (paper)
    }*/
    nCols = 5;
    nurseWorkingTimes = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseWorkingTimes[i] = malloc(nCols*sizeof(int)); // Cols
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseWorkingTimes[i][j] = nurseWorkingTimes_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseWorkingTimes.\n");
    }

    // 5. int** nurseSkills: nNurses x nSkills (using int* nurseSkills_data)
    int** nurseSkills;
    nRows = nNurses;
    nCols = nSkills;
    nurseSkills = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseSkills[i] = malloc(nCols*sizeof(int)); // Cols
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            nurseSkills[i][j] = nurseSkills_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseSkills.\n");
    }

    // 6. int** jobRequirements: nJobs x nSkills (using int* jobRequirements_data)
    int** jobRequirements;
    nRows = nJobs;
    nCols = nSkills;
    jobRequirements = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        jobRequirements[i] = malloc(nCols*sizeof(int)); // Cols
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            jobRequirements[i][j] = jobRequirements_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated jobRequirements.\n");
    }

    // 7. int** jobTimeInfo: nJobs x 3 (using int* jobTimeInfo_data)
    int** jobTimeInfo;
    nRows = nJobs;
    nCols = 3;
    jobTimeInfo = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        jobTimeInfo[i] = malloc(nCols*sizeof(int)); // Cols
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            jobTimeInfo[i][j] = jobTimeInfo_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated jobTimeInfo.\n");
    }

    // 8. int** solMatrix: nNurses x nJobs (set to all -1)
    int** solMatrix;
    nRows = nNurses;
    nCols = nJobs;
    solMatrix = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        solMatrix[i] = malloc(nCols*sizeof(int)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrix[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated solMatrix.\n");
    }

    // 9. double** timeMatrix: nNurses x nJobs (set to all -1)
    double** timeMatrix;
    nRows = nNurses;
    nCols = nJobs;
    timeMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        timeMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            timeMatrix[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated timeMatrix.\n");
    }


    // 10. int* nurseRoute: 1 x nJobs
    int* nurseRoute = malloc(nJobs * sizeof(int));

    // 11. int** allNurseRoutes: nNurses x nJobs (set to all -1)
    int** allNurseRoutes;
    nRows = nNurses;
    nCols = nJobs;
    allNurseRoutes = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        allNurseRoutes[i] = malloc(nCols*sizeof(int)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allNurseRoutes[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated allNurseRoutes.\n");
    }

    // 12. double* nurseWaitingTime: 1 x nNurses
    double* nurseWaitingTime = malloc(nNurses * sizeof(double));

    // 13. double nurseTravelTime: 1 x nNurses
    double* nurseTravelTime = malloc(nNurses * sizeof(double));

    // 14. - 17. int* doubleService, dependsOn, mkMinD, mkMaxD: 1 x nJobs (using _data)
    int* doubleService = malloc(nJobs * sizeof(int));
    int* dependsOn = malloc(nJobs * sizeof(int));
    int* mkMinD = malloc(nJobs * sizeof(int));
    int* mkMaxD = malloc(nJobs * sizeof(int));
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
    int* nurseUnavail = malloc(nNurses * sizeof(int));
    for (int i = 0; i < nNurses; i++){
        nurseUnavail[i] = nurseUnavail_data[i];
    }

    // 19. double* violatedTW: 1 x nJobs
    double* violatedTW = malloc(nJobs * sizeof(double));

    // 20. double* violatedTWMK: 1 x nJobs
    double* violatedTWMK = malloc(nJobs * sizeof(double));

    // 21. int** nurseSkilled: nNurses x nJobs (set using nurseSkills, jobRequirements, doubleService)
    int** nurseSkilled;
    nRows = nNurses;
    nCols = nJobs;
    nurseSkilled = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseSkilled[i] = malloc(nCols*sizeof(int)); // Cols
    }
    NurseSkilledFromSkillsAndRequirements(nurseSkills, jobRequirements, nurseSkilled, doubleService, nJobs, nNurses, nSkills);
    if (verbose_data > 5){
        printf("Allocated nurseSkilled.\n");
    }

    // 22. double** prefScore: nJobs x nNurses (using double* prefScore_data)
    double** prefScore; // Job x Nurse
    nRows = nJobs;
    nCols = nNurses;
    prefScore = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        prefScore[i] = malloc(nCols*sizeof(double)); // Cols
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            prefScore[i][j] = prefScore_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated prefScore.\n");
    }

    // 23. double* algorithmOptions: 1 x 100 (using double* algorithmOptions_data)
    int nOptions = 100;
    double* algorithmOptions = malloc(nOptions * sizeof(double));
    for (int i = 0; i < nOptions; ++i) {
        algorithmOptions[i] = algorithmOptions_data[i];
    }
    if (verbose_data > 5){
        printf("Allocated algorithmOptions.\n");
    }

    // 24. int*** capabilityOfDoubleServices: nNurses x nNurses x nDoubleServices (using int* capabilityOfDoubleServices_data)
    int*** capabilityOfDoubleServices;
    nRows = nNurses;
    nCols = nNurses;
    // dim3 is nDoubleServices
    capabilityOfDoubleServices = malloc(nRows * sizeof(int**)); // Rows
    for (int i = 0; i < nRows; i++) {
        capabilityOfDoubleServices[i] = malloc(nCols * sizeof(int*)); // Cols
        for (int j = 0; j < nCols; j++){
            capabilityOfDoubleServices[i][j] = malloc(nDoubleServices*sizeof(int)); // Cols
        }
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            for (int k = 0; k < nDoubleServices; ++k) {
                ct++;
                capabilityOfDoubleServices[i][j][k] = capabilityOfDoubleServices_data[ct];
            }
        }
    }
    if (verbose_data > 5) {
        printf("Allocated capabilityOfDoubleServices.\n");
    }

    // 25. int*** unavailMatrix: 10 x 4 x nNurses (using int* unavail_matrix_data) (24/05/2021)
    int*** unavailMatrix;
    nRows = 50; /** NEED TO CHANGE THIS, currently just set to big number 50. **/
    nCols = 4;
    // dim3 is nNurses
    unavailMatrix = malloc(nRows * sizeof(int**)); // Rows
    for (int i = 0; i < nRows; i++) {
        unavailMatrix[i] = malloc(nCols * sizeof(int*)); // Cols
        for (int j = 0; j < nCols; j++){
            unavailMatrix[i][j] = malloc(nNurses*sizeof(int));
        }
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            for (int k = 0; k < nNurses; ++k) {
                ct++;
                unavailMatrix[i][j][k] = unavailMatrix_data[ct];
            }
        }
    }
    if (verbose_data > 5) {
        printf("Allocated unavailMatrix.\n");
    }

    // 26. double** nurseWaitingMatrix: nNurses x nJobs (set to all 0)
    // NB: NEW: 03/06/2021
    double** nurseWaitingMatrix;
    nRows = nNurses;
    nCols = nJobs;
    nurseWaitingMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseWaitingMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseWaitingMatrix[i][j] = 0;
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseWaitingMatrix.\n");
    }

    // 27. double** nurseWaitingMatrix: nNurses x nJobs (set to all 0)
    // NB: NEW: 03/06/2021
    double** nurseTravelMatrix;
    nRows = nNurses;
    nCols = nJobs;
    nurseTravelMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseTravelMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseTravelMatrix[i][j] = 0;
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseTravelMatrix.\n");
    }

    // 28. int* nurseOrder: 1 x nNurses (0... nNurses-1)
    int* nurseOrder = malloc(nNurses * sizeof(int));
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

    struct INSTANCE inst = { .nJobs = nJobs,
            .nJobsIncDS = nJobsIncDS, // NB: NEW 08/11/2021
            .nNurses = nNurses,
            .nSkills = nSkills,
            .verbose = verbose_data,
            .qualityMeasure = qualityMeasure,
            .MAX_TIME_SECONDS = MAX_TIME_SECONDS,
            .twInterval = twInterval_data,
            .excludeNurseTravel = excludeNurseTravel_data,
            .od = od,
            .nurseTravelFromDepot = nurseTravelFromDepot,
            .nurseTravelToDepot = nurseTravelToDepot,
            .unavailMatrix = unavailMatrix,
            .nurseUnavail = nurseUnavail,
            .nurseWorkingTimes = nurseWorkingTimes,
            .solMatrix = solMatrix,
            .timeMatrix = timeMatrix,
            .jobTimeInfo = jobTimeInfo,
            .jobRequirements = jobRequirements,
            .nurseSkills = nurseSkills,
            .nurseSkilled = nurseSkilled,
            .nurseRoute = nurseRoute,
            .allNurseRoutes = allNurseRoutes,
            .nurseOrder = nurseOrder,
            .nurseWaitingTime = nurseWaitingTime,
            .nurseTravelTime = nurseTravelTime,
            .doubleService = doubleService,
            .dependsOn = dependsOn,
            .violatedTW = violatedTW,
            .violatedTWMK = violatedTWMK,
            .isFeasible = 0,
            .mkMinD = mkMinD,
            .mkMaxD = mkMaxD,
            .capabilityOfDoubleServices = capabilityOfDoubleServices,
            .prefScore = prefScore,
            .algorithmOptions = algorithmOptions,
            .nurseWaitingMatrix = nurseWaitingMatrix, // NB: NEW: 03/06/2021
            .nurseTravelMatrix = nurseTravelMatrix, // NB: NEW: 03/06/2021
            .totalServiceTime = totalServiceTime, //NB: NEW: 06/11/2021
            .totalServiceTimeIncDS = totalServiceTimeIncDS,
            .breakPoint = 0//NB NEW 06/11/2021
    };

    return inst;

} // END OF InstanceFromPython

void FreeInstanceCopy(struct INSTANCE* ip) {

    // 1. solMatrix
	for (int i = 0; i < ip->nNurses; ++i){
        free(ip->solMatrix[i]);
    }
	free(ip->solMatrix);
	if (ip->verbose > 10){
        printf("Freed solMatrix. (copy)\n");
    }

    // 2. timeMatrix
	for (int i = 0; i < ip->nNurses; ++i){
        free(ip->timeMatrix[i]);
    }
	free(ip->timeMatrix);
	if (ip->verbose > 10){
        printf("Freed timeMatrix. (copy)\n");
    }

    // 3. allNurseRoutes
	for (int i = 0; i < ip->nNurses; ++i){
        free(ip->allNurseRoutes[i]);
    }
	free(ip->allNurseRoutes);
	if (ip->verbose > 10){
        printf("Freed allNurseRoutes. (copy)\n");
    }

    // 4. violatedTW
	free(ip->violatedTW);
	if (ip->verbose > 10){
        printf("freed ip->violatedTW (copy)\n");
    }

    // 5. violatedTWMK
	free(ip->violatedTWMK);
	if (ip->verbose > 10){
        printf("freed ip->violatedTWMK (copy)\n");
    }

    // 6. nurseRoute
	free(ip->nurseRoute);
	// ip->nurseRoute = NULL;
	if (ip->verbose > 10){
        printf("freed ip->nurseRoute (copy)\n");
    }

    // 7. nurseOrder
	free(ip->nurseOrder);
	if (ip->verbose > 10){
        printf("freed ip->nurseOrder (copy)\n");
    }

    // 8. nurseWaitingTime
	free(ip->nurseWaitingTime);
	if (ip->verbose > 10){
        printf("freed ip->nurseWaitingTime (copy)\n");
    }

    // 9. nurseTravelTime
	free(ip->nurseTravelTime);
	if (ip->verbose > 10){
        printf("freed ip->nurseTravelTime (copy)\n");
    }

    /*// 10. nurseWaitingMatrix (nNurses x nJobs)
    for (int i = 0; i < ip->nNurses; ++i){
        free(ip->nurseWaitingMatrix[i]);
    }
    free(ip->nurseWaitingMatrix);
    if (ip->verbose > 10){
        printf("Freed nurseWaitingMatrix. (copy)\n");
    }

    // 11. nurseTravelMatrix (nNurses x nJobs)
    for (int i = 0; i < ip->nNurses; ++i){
        free(ip->nurseTravelMatrix[i]);
    }
    free(ip->nurseTravelMatrix);
    if (ip->verbose > 10){
        printf("Freed nurseTravelMatrix. (copy)\n");
    }

    // 12. od (nJobs+1 x nJobs+1)
    for(int i = 0; i < ip->nJobs + 1; ++i){
        free(ip->od[i]);
    }
    free(ip->od);
    if (ip->verbose > 10){
        printf("Freed od. (copy)\n");
    }

    // 13. nurseTravelFromDepot (nNurses x nJobs)
    for(int i = 0; i < ip->nNurses; ++i){
        free(ip->nurseTravelFromDepot[i]);
    }
    free(ip->nurseTravelFromDepot);
    if (ip->verbose > 10){
        printf("Freed nurseTravelFromDepot. (copy)\n");
    }

    // 14. nurseTravelToDepot (nNurses x nJobs)
    for(int i = 0; i < ip->nNurses; ++i){
        free(ip->nurseTravelToDepot[i]);
    }
    free(ip->nurseTravelToDepot);
    if (ip->verbose > 10){
        printf("Freed nurseTravelToDepot. (copy)\n");
    }

    // 15. unavailMatrix (50 x 4 x nNurses)
    for(int i = 0; i < 50; ++i){
        for(int j = 0; j < 4; ++j){
            free(ip->unavailMatrix[i][j]);
        }
        free(ip->unavailMatrix[i]);
    }
    free(ip->unavailMatrix);
    if (ip->verbose > 10){
        printf("Freed unavailMatrix. (copy)\n");
    }

    // 16. nurseUnavail (1 x nNurses)
    free(ip->nurseUnavail);
    if (ip->verbose > 10){
        printf("Freed nurseUnavail. (copy)\n");
    }

    // 17. nurseWorkingTimes (nNurses x 5)
    for(int i = 0; i < ip->nNurses; ++i){
        free(ip->nurseWorkingTimes[i]);
    }
    free(ip->nurseWorkingTimes);
    if (ip->verbose > 10){
        printf("Freed nurseWorkingTimes. (copy)\n");
    }

    // 18. jobTimeInfo (nJobs x 3)
    for(int i = 0; i > ip->nJobs; ++i){
        free(ip->jobTimeInfo[i]);
    }
    free(ip->jobTimeInfo);
    if (ip->verbose > 10){
        printf("Freed jobTimeInfo. (copy)\n");
    }

    // 19. jobRequirements (nJobs x nSkills)
    for(int i = 0; i < ip->nJobs; ++i){
        free(ip->jobRequirements[i]);
    }
    free(ip->jobRequirements);
    if (ip->verbose > 10){
        printf("Freed jobRequirements. (copy)\n");
    }

    // 20. nurseSkills (nNurses x nSkills)
    for(int i = 0; i < ip->nNurses; ++i){
        free(ip->nurseSkills[i]);
    }
    free(ip->nurseSkills);
    if (ip->verbose > 10){
        printf("Freed nurseSkills. (copy)\n");
    }

    // 21. nurseSkilled (nNurses x nJobs)
    for(int i = 0; i < ip->nNurses; ++i){
        free(ip->nurseSkilled[i]);
    }
    free(ip->nurseSkilled);
    if (ip->verbose > 10){
        printf("Freed nurseSkilled. (copy)\n");
    }

    // 22. doubleService (1 x nJobs)
    free(ip->doubleService);
    if (ip->verbose > 10){
        printf("Freed doubleService. (copy)\n");
    }

    // 23. dependsOn (1 x nJobs)
    free(ip->dependsOn);
    if (ip->verbose > 10){
        printf("Freed dependsOn. (copy)\n");
    }

    // 24. capabilitiesOfDoubleServices (nNurses x nNurses x nDS)
    for(int i = 0; i < ip->nNurses; ++i){
        for(int j = 0; j < ip->nNurses; ++j){
            free(ip->capabilityOfDoubleServices[i][j]);
        }
        free(ip->capabilityOfDoubleServices[i]);
    }
    free(ip->capabilityOfDoubleServices);
    if (ip->verbose > 10){
        printf("Freed capabilityOfDoubleServices. (copy)\n");
    }

    // 25 prefScore (nJobs x nNurses)
    for(int i = 0; i < ip->nJobs; ++i){
        free(ip->prefScore[i]);
    }
    free(ip->prefScore);
    if (ip->verbose > 10){
        printf("Freed prefScore. (copy)\n");
    }*/

	if (ip->verbose > 10){
        printf("Finished freeing memory of instance copy.\n");
    }
}

void FreeInstanceMemory(struct INSTANCE* ip) {
	free(ip->nurseRoute);
	ip->nurseRoute = NULL; // Used as temp. array to avoid allocations
	if (ip->verbose > 10)
		printf("Set nurseRoute to NULL.\n");

	// printf("\nWarning: There are more things to free!\n");
	/* deallocate the array */
	for (int i = 0; i < (ip->nJobs + 1); i++)
		free(ip->od[i]);
	free(ip->od);
	if (ip->verbose > 10)
		printf("Freed od.\n");

	for (int i = 0; i < (ip->nNurses); i++)
		free(ip->nurseTravelFromDepot[i]);
	free(ip->nurseTravelFromDepot);
	if (ip->verbose > 10)
		printf("Freed nurseTravelFromDepot.\n");

	for (int i = 0; i < (ip->nNurses); i++)
		free(ip->nurseTravelToDepot[i]);
	free(ip->nurseTravelToDepot);
	if (ip->verbose > 10)
		printf("Freed nurseTravelToDepot.\n");

	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->solMatrix[i]);
	free(ip->solMatrix);
	if (ip->verbose > 10)
		printf("Freed solMatrix.\n");

	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->timeMatrix[i]);
	free(ip->timeMatrix);
	if (ip->verbose > 10)
		printf("Freed timeMatrix.\n");


	for (int i = 0; i < ip->nJobs; ++i)
		free(ip->jobTimeInfo[i]);
	free(ip->jobTimeInfo);
	if (ip->verbose > 10)
		printf("Freed jobTimeInfo.\n");

	for (int i = 0; i < ip->nJobs; ++i)
		free(ip->jobRequirements[i]);
	free(ip->jobRequirements);
	if (ip->verbose > 10)
		printf("Freed jobRequirements.\n");

	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->nurseSkills[i]);
	free(ip->nurseSkills);
	if (ip->verbose > 10)
		printf("Freed nurseSkills.\n");


	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->nurseSkilled[i]);
	free(ip->nurseSkilled);
	if (ip->verbose > 10)
		printf("Freed nurseSkilled.\n");

	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->nurseWorkingTimes[i]);
	free(ip->nurseWorkingTimes);
	if (ip->verbose > 10)
		printf("Freed nurseWorkingTimes.\n\n");

	free(ip->violatedTWMK);
	if (ip->verbose > 10)
		printf("freed ip->violatedTWMK\n");

	free(ip->violatedTW);
	if (ip->verbose > 10)
		printf("freed ip->violatedTW\n");

	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->allNurseRoutes[i]);
	free(ip->allNurseRoutes);
	if (ip->verbose > 10)
		printf("Freed allNurseRoutes.\n");


	// free(ip->nurseRoute);
	// ip->nurseRoute = NULL;
	// if(ip->verbose > 10)
	// 	printf("freed ip->nurseRoute (set to NULL)\n");

	free(ip->nurseOrder);
	ip->nurseOrder = NULL;
	if (ip->verbose > 10)
		printf("freed ip->nurseOrder\n");

	free(ip->doubleService);
	if (ip->verbose > 10)
		printf("freed ip->doubleService\n");

	free(ip->dependsOn);
	if (ip->verbose > 10)
		printf("freed ip->dependsOn\n");

	free(ip->nurseWaitingTime);
	if (ip->verbose > 10)
		printf("freed ip->nurseWaitingTime\n");

	free(ip->nurseTravelTime);
	if (ip->verbose > 10)
		printf("freed ip->nurseTravelTime\n");

	free(ip->algorithmOptions);
	if (ip->verbose > 10)
		printf("freed ip->algorithmOptions\n");

	free(ip->mkMinD);
	if (ip->verbose > 10)
		printf("freed ip->mk_mind\n");

	free(ip->mkMaxD);
	if (ip->verbose > 10)
		printf("freed ip->mkMaxD\n");

	if (ip->verbose > 10)
		printf("Finished freeing memory.\n");

}




struct INSTANCE GenerateInstance() {
	int nRows, nCols;
	int verbose_data = 110;

	// INSTANCE: bihcrsp_1.txt
	// Information is expressed on a 5 minute level.

	int nJobs = 10;
	int nNurses = 2;
	int nSkills = 4;

	int nurseWorkingTimes_data[2][3] = {
	{72, 276, 96},
	{72, 276, 96}
	};



	int jobTimeInfo_data[10][3] = {
	{96, 131, 18},
	{132, 155, 18},
	{192, 240, 6},
	{192, 240, 6},
	{132, 155, 12},
	{96, 131, 24},
	{96, 131, 18},
	{96, 131, 21},
	{72, 95, 12},
	{210, 210, 9}
	};

	int nurseSkills_data[2][4] = {
	{1, 1, 1, 1},
	{1, 1, 0, 0}
	};

	int jobRequirements_data[10][4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{1, 1, 1, 1},
	{0, 0, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{0, 0, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0}
	};

	int od_data[11][11] = {
	{0, 1, 2, 0, 2, 2, 3, 3, 1, 1, 1},
	{1, 0, 2, 1, 1, 4, 4, 3, 3, 3, 1},
	{1, 3, 0, 2, 2, 4, 4, 1, 1, 1, 0},
	{0, 0, 3, 0, 0, 3, 3, 3, 3, 3, 2},
	{2, 0, 3, 0, 0, 3, 3, 3, 3, 3, 2},
	{2, 3, 4, 3, 3, 0, 0, 3, 3, 3, 3},
	{3, 3, 4, 3, 3, 0, 0, 3, 3, 3, 3},
	{3, 4, 1, 3, 3, 4, 4, 0, 0, 0, 1},
	{1, 4, 1, 3, 3, 4, 4, 0, 0, 0, 1},
	{1, 4, 1, 3, 3, 4, 4, 0, 0, 0, 1},
	{1, 2, 0, 2, 2, 3, 3, 1, 1, 1, 0}
	};

	int nurseSkilled_data[2][10];

	// End of instance bihcrsp_1.txt




		// SIMPLE DEBUG:
		/*
		int nJobs = 12;
		int nNurses = 3;
		int startTime = 0;

		int od_data[13][13] = {
			{0, 361, 500, 566, 500, 600, 608, 509, 400, 500, 539, 509, 316},
			{361, 0, 141, 224, 200, 500, 566, 500, 361, 707, 800, 806, 608},
			{500, 141, 0, 100, 141, 500, 583, 539, 412, 800, 906, 922, 728},
			{566, 224, 100, 0, 100, 447, 539, 509, 400, 806, 922, 949, 762},
			{500, 200, 141, 100, 0, 361, 447, 412, 300, 707, 825, 853, 671},
			{600, 500, 500, 447, 361, 0, 100, 141, 200, 500, 640, 707, 583},
			{608, 566, 583, 539, 447, 100, 0, 100, 224, 424, 566, 640, 539},
			{509, 500, 539, 509, 412, 141, 100, 0, 141, 361, 500, 566, 447},
			{400, 361, 412, 400, 300, 200, 224, 141, 0, 412, 539, 583, 424},
			{500, 707, 800, 806, 707, 500, 424, 361, 412, 0, 141, 224, 224},
			{539, 800, 906, 922, 825, 640, 566, 500, 539, 141, 0, 100, 224},
			{509, 806, 922, 949, 853, 707, 640, 566, 583, 224, 100, 0, 200},
			{316, 608, 728, 762, 671, 583, 539, 447, 424, 224, 224, 200, 0}};

	int jobTimeWindow_data[12][2] = {
	{0, 2400, 0},
	{0, 2400, 0},
	{500, 700, 0},
	{0, 2400, 0},
	{500, 700, 0},
	{0, 2400, 0},
	{0, 2400, 0},
	{0, 2400, 0},
	{0, 2400, 0},
	{500, 700, 0},
	{0, 2400, 0},
	{0, 2400, 0}};

	int nurseSkilled_data[3][12] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

	// int jobDuration_data[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
		*/
		// MANKOWSKA
			/*
		int nJobs = 10;
		int nNurses = 3;
		int nSkills = 6;
		int startTime = 9;
		int od_data[11][11]={
			{ 0, 38, 34, 55, 7, 23, 71, 32, 13, 26, 88},// 0},
			{ 38, 0, 23, 21, 32, 31, 34, 32, 45, 57, 56},// 38},
			{ 34, 23, 0, 43, 27, 15, 53, 10, 46, 42, 54},// 34},
			{ 55, 21, 43, 0, 50, 53, 17, 53, 59, 77, 59},// 55},
			{ 7, 32, 27, 50, 0, 17, 65, 26, 19, 28, 81},// 7}}//,
			{ 23, 31, 15, 53, 17, 0, 65, 9, 36, 27, 69},// 23},
			{ 71, 34, 53, 17, 65, 65, 0, 63, 75, 91, 50},// 71},
			{ 32, 32, 10, 53, 26, 9, 63, 0, 45, 34, 62},// 32}}//,
			{ 13, 45, 46, 59, 19, 36, 75, 45, 0, 35, 99},// 13},
			{ 26, 57, 42, 77, 28, 27, 91, 34, 35, 0, 96},// 26},
			{ 88, 56, 54, 59, 81, 69, 50, 62, 99, 96, 0}};// 88}};//,
			// { 0, 38, 34, 55, 7, 23, 71, 32, 13, 26, 88, 0}};

		int jobTimeWindow_data[10][2] = {//{0, 600},
										{345, 465, 14},
										{268, 388, 14},
										{247, 367, 14},
										{393, 513, 14},
										{254, 374, 14},
										{184, 304, 14},
										{434, 554, 14},
										{46, 166, 14},
										{298, 418, 14},
										{148, 268, 14}};//,									{0, 1000}};

		// int nSkills = 6;
		int a[3][6] =	{{1, 1, 1, 0, 0, 0},
			{0, 0, 0, 0, 1, 1},
			{0, 0, 0, 1, 1, 1}};

		int r[10][6] ={//{0, 0, 0, 0, 0, 0},//{1,1,1,1,1,1},
				{0,0,0,1,0,0},
				{0,0,0,0,1,0},
				{0,1,0,0,0,0},
				{0,0,0,1,0,0},
				{0,0,1,0,0,0},
				{0,0,0,0,1,0},
				{0,0,1,0,0,0},
				{0,0,0,0,1,1},
				{1,0,0,1,0,0},
				{0,0,1,0,0,1}};//,
				//{0, 0, 0, 0, 0, 0}};
				// {1,1,1,1,1,1}};

		int nurseSkilled_data[3][10];

		// int jobDuration_data[10] = {14,14,14,14,14,14,14,14,14,14};
		*/

		/*
			// SIMPLE EXAMPLE
			int nJobs = 5;
			int nNurses = 2;
			int startTime = 9;

			int od_data[5][5] = {{0, 1, 2, 3, 4},
							{1, 0, 1, 2, 3},
							{2, 1, 0, 1, 2},
							{3, 2, 1, 0, 1},
							{4, 3, 2, 1, 0}};

			int jobTimeWindow_data[5][2] = {
				{9,17},
				{12,13},
				{9,17},
				{9,17},
				{12,16}};

			int nurseSkilled_data[2][5] = {{0, 1, 1, 1, 1},
									{1, 0, 1, 0, 1}};

		*/

		///////////////// FAKE START TIME WIP ////////////////
		// int startTime = nurseWorkingTimes_data[0][0];


		///////////////////// SEE WHAT CAN EACH NURSE DO ///////////////
	int canDoIt = 1;
	for (int i = 0; i < nNurses; ++i) {
		for (int j = 0; j < nJobs; ++j) {
			canDoIt = 1;
			for (int k = 0; k < nSkills; ++k) {
				if ((float)nurseSkills_data[i][k] < ((float)(jobRequirements_data[j][k]) - 0.5)) {
					// if (j == 9)
					// {
					// 	printf("a[%d][%d] = %d, r[%d][%d] = %d\n", i,k,a[i][k], j, k, r[j][k]);
					// }
					canDoIt = 0;
					break;
				}
			}
			nurseSkilled_data[i][j] = canDoIt;
		}
	}

	// // Debug
	// for (int i = 0; i < nNurses; ++i)
	// {
	// 	for (int j = 0; j < nJobs; ++j)
	// 	{
	// 		printf("%d\t", nurseSkilled_data[i][j]);
	// 	}
	// 	printf("\n");
	// }

///////////////////// ALLOCATE MEMORY /////////////////////

	// int **od;.
	nRows = nJobs + 1;
	nCols = nJobs + 1;
	double** od;
	// double ** od_cost = NULL; // Not used at the moment
	od = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++)
		od[i] = malloc(nCols * sizeof(double)); // Cols
		// od_cost[i] = malloc(nCols * sizeof(int)); // No need to allocate this at the moment;

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			od[i][j] = (double)od_data[i][j];

	// printf("Allocated od.\n");

	// inst.od = od;


	int** nurseSkilled;
	nRows = nNurses;
	nCols = nJobs;
	nurseSkilled = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurseSkilled[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			nurseSkilled[i][j] = nurseSkilled_data[i][j];
	//end populating
	// printf("Allocated nurseSkilled.\n");



	// Should include eventually working times here as well

	// nCols = nJobs;
	// int * jobDuration = malloc(nCols * sizeof(int));
	// //start single vector pop
	// for (int i = 0; i < nCols; ++i)
	// 	jobDuration[i] = jobTimeWindow_data[i][2];
	//end populating

	printf("Allocated jobDuration.\n");


	// nurseWorkingTimes_data
	int** nurseWorkingTimes;
	nRows = nNurses;
	nCols = 3;
	nurseWorkingTimes = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurseWorkingTimes[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			nurseWorkingTimes[i][j] = nurseWorkingTimes_data[i][j];
	//end populating
	// printf("Allocated nurseWorkingTimes.\n");

// nurseSkills_data
	int** nurseSkills;
	nRows = nNurses;
	nCols = nSkills;
	nurseSkills = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurseSkills[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			nurseSkills[i][j] = nurseSkills_data[i][j];
	//end populating
	// printf("Allocated nurseSkills.\n");

// jobRequirements_data
	int** jobRequirements;
	nRows = nJobs;
	nCols = nSkills;
	jobRequirements = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		jobRequirements[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			jobRequirements[i][j] = jobRequirements_data[i][j];
	//end populating
	// printf("Allocated jobRequirements.\n");


	int** jobTimeInfo;
	nRows = nJobs;
	nCols = 3;
	jobTimeInfo = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		jobTimeInfo[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			jobTimeInfo[i][j] = jobTimeInfo_data[i][j];
	//end populating

	// printf("Allocated jobTimeInfo.\n");


	int** solMatrix;
	nRows = nNurses;
	nCols = nJobs;
	solMatrix = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		solMatrix[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			solMatrix[i][j] = -1;
	//end populating
	// printf("Allocated solMatrix.\n");

	// int * nurseRoute = 	malloc(nJobs * sizeof(int));
	double** timeMatrix;
	nRows = nNurses;
	nCols = nJobs;
	timeMatrix = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++)
		timeMatrix[i] = malloc(nCols * sizeof(double)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			timeMatrix[i][j] = -1;
	//end populating
	if (verbose_data > 5)
		printf("Allocated timeMatrix.\n");


	int* nurseRoute = malloc(nJobs * sizeof(int));
	double* nurseWaitingTime = malloc(nNurses * sizeof(double));
	int* doubleService = malloc(nJobs * sizeof(int));
	double* violatedTW = malloc(nJobs * sizeof(double));
	// Group all data in the instance:
	// Old version:
	// struct INSTANCE inst = {.od = od, .startTime = startTime, .nJobs = nJobs, .nNurses = nNurses, .solMatrix = solMatrix, .jobTimeWindow = jobTimeWindow, .jobDuration = jobDuration, .nurseSkilled = nurseSkilled};

	// struct INSTANCE inst = {.nJobs = nJobs,
	// 						.nNurses = nNurses,
	// 						.nSkills = nSkills,
	// 						.verbose = 5,
	// 						.MAX_TIME_SECONDS = 600,
	// 						.od = od,
	// 						// .od_cost = od_cost,
	// 						.nurseWorkingTimes = nurseWorkingTimes,
	// 						.solMatrix = solMatrix,
	// 						.jobTimeInfo = jobTimeInfo,
	// 						.jobRequirements = jobRequirements,
	// 						.nurseSkills = nurseSkills,
	// 						.nurseSkilled = nurseSkilled,
	// 						.nurseRoute = nurseRoute};
	// New version:
	float MAX_TIME_SECONDS = 600;
	struct INSTANCE inst = { .nJobs = nJobs,
									.nNurses = nNurses,
									.nSkills = nSkills,
									.verbose = verbose_data,
									.MAX_TIME_SECONDS = MAX_TIME_SECONDS,
									.od = od,
		// .od_cost = od_cost,
		.nurseWorkingTimes = nurseWorkingTimes,
		.solMatrix = solMatrix,
		.timeMatrix = timeMatrix,
		.jobTimeInfo = jobTimeInfo,
		.jobRequirements = jobRequirements,
		.nurseSkills = nurseSkills,
		.nurseSkilled = nurseSkilled,
		.nurseRoute = nurseRoute,
		.nurseWaitingTime = nurseWaitingTime,
		.doubleService = doubleService,
		.violatedTW = violatedTW,
		.isFeasible = 0 };


	// ip = &inst;
	return inst;
}

void NurseSkilledFromSkillsAndRequirements(int** nurseSkills, int** jobRequirements, int** nurseSkilled, int* doubleService, int nJobs, int nNurses, int nSkills) {
	int canDoIt = 1;
	for (int i = 0; i < nNurses; ++i) {
		for (int j = 0; j < nJobs; ++j) {
			canDoIt = 1;
			for (int k = 0; k < nSkills; ++k) {
				if (nurseSkills[i][k] < jobRequirements[j][k]) {
					// if (j == 9)
					// {
					// 	printf("a[%d][%d] = %d, r[%d][%d] = %d\n", i,k,a[i][k], j, k, r[j][k]);
					// }
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

	// // Debug
	// for (int i = 0; i < nNurses; ++i)
	// {
	// 	for (int j = 0; j < nJobs; ++j)
	// 	{
	// 		printf("%d\t", nurseSkilled[i][j]);
	// 	}
	// 	printf("\n");
	// }


}