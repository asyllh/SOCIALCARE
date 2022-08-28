/*--------------/
GRASP_VNS
read_instance.c
UoS
10/07/2021
/--------------*/


#include "constructive.h"
void OverwriteInstance(struct INSTANCE* ip, struct INSTANCE* oi) {

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

}

struct INSTANCE CopyInstance(struct INSTANCE* original_instance) {
	int LOCAL_VERBOSE = original_instance->verbose;

	int** solMatrix;
	int nRows = original_instance->nNurses;
	int nCols = original_instance->nJobs;
	solMatrix = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++){
        solMatrix[i] = malloc(nCols*sizeof(int));
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
	timeMatrix = malloc(nRows * sizeof(double*));
	for (int i = 0; i < nRows; i++){
        timeMatrix[i] = malloc(nCols*sizeof(double));
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
	allNurseRoutes = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++){
        allNurseRoutes[i] = malloc(nCols*sizeof(int));
    }
	for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allNurseRoutes[i][j] = original_instance->allNurseRoutes[i][j];
        }
    }
	if (LOCAL_VERBOSE > 5){
        printf("Allocated allNurseRoutes. (copying)\n");
    }


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
                                    .nJobsIncDS = original_instance->nJobsIncDS,
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
									.nurseWaitingMatrix = original_instance->nurseWaitingMatrix,
									.nurseTravelMatrix = original_instance->nurseTravelMatrix,
                                    .totalServiceTime = original_instance->totalServiceTime,
                                    .totalServiceTimeIncDS = original_instance->totalServiceTimeIncDS,
                                    .arrivalTimes = original_instance->arrivalTimes
	};

	if (LOCAL_VERBOSE > 5){
        printf("Done. (copying)\n");
    }
	return inst;

}



struct INSTANCE InstanceFromPython(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data,
                                   double* od_data, double* nurseTravelFromDepot_data, double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data,
                                   int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* doubleService_data, int* dependsOn_data,
                                   int* mkMinD_data, int* mkMaxD_data, int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data) {

    int printAllAllocations = 0;

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
    if (verbose_data > 5){
        printf("Starting allocation of memory\n");
    }

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

    nRows = nNurses;
    nCols = nJobs;
    ct = -1;
    double** nurseTravelFromDepot;
    nurseTravelFromDepot = malloc(nRows * sizeof(double*));
    for (int i = 0; i < nRows; i++){
        nurseTravelFromDepot[i] = malloc(nCols*sizeof(double));
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

    double** nurseTravelToDepot;
    ct = -1;
    nurseTravelToDepot = malloc(nRows * sizeof(double*));
    for (int i = 0; i < nRows; i++){
        nurseTravelToDepot[i] = malloc(nCols*sizeof(double));
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

    int** nurseWorkingTimes;
    nRows = nNurses;
    nCols = 5;
    nurseWorkingTimes = malloc(nRows * sizeof(int*));
    for (int i = 0; i < nRows; i++){
        nurseWorkingTimes[i] = malloc(nCols*sizeof(int));
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

    int** nurseSkills;
    nRows = nNurses;
    nCols = nSkills;
    nurseSkills = malloc(nRows * sizeof(int*));
    for (int i = 0; i < nRows; i++){
        nurseSkills[i] = malloc(nCols*sizeof(int));
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

    int** jobRequirements;
    nRows = nJobs;
    nCols = nSkills;
    jobRequirements = malloc(nRows * sizeof(int*));
    for (int i = 0; i < nRows; i++){
        jobRequirements[i] = malloc(nCols*sizeof(int));
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


    int** jobTimeInfo;
    nRows = nJobs;
    nCols = 3;
    jobTimeInfo = malloc(nRows * sizeof(int*));
    for (int i = 0; i < nRows; i++){
        jobTimeInfo[i] = malloc(nCols*sizeof(int));
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

    int** solMatrix;
    nRows = nNurses;
    nCols = nJobs;
    solMatrix = malloc(nRows * sizeof(int*));
    for (int i = 0; i < nRows; i++){
        solMatrix[i] = malloc(nCols*sizeof(int));
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrix[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated solMatrix.\n");
    }

    double** timeMatrix;
    nRows = nNurses;
    nCols = nJobs;
    timeMatrix = malloc(nRows * sizeof(double*));
    for (int i = 0; i < nRows; i++){
        timeMatrix[i] = malloc(nCols*sizeof(double));
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            timeMatrix[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated timeMatrix.\n");
    }

    int* nurseRoute = malloc(nJobs * sizeof(int));

    int** allNurseRoutes;
    nRows = nNurses;
    nCols = nJobs;
    allNurseRoutes = malloc(nRows * sizeof(int*));
    for (int i = 0; i < nRows; i++){
        allNurseRoutes[i] = malloc(nCols*sizeof(int));
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allNurseRoutes[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated allNurseRoutes.\n");
    }


    double* nurseWaitingTime = malloc(nNurses * sizeof(double));

    double* nurseTravelTime = malloc(nNurses * sizeof(double));

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

    int* nurseUnavail = malloc(nNurses * sizeof(int));
    for (int i = 0; i < nNurses; i++){
        nurseUnavail[i] = nurseUnavail_data[i];
    }

    double* violatedTW = malloc(nJobs * sizeof(double));

    double* violatedTWMK = malloc(nJobs * sizeof(double));

    int** nurseSkilled;
    nRows = nNurses;
    nCols = nJobs;
    nurseSkilled = malloc(nRows * sizeof(int*));
    for (int i = 0; i < nRows; i++){
        nurseSkilled[i] = malloc(nCols*sizeof(int));
    }
    NurseSkilledFromSkillsAndRequirements(nurseSkills, jobRequirements, nurseSkilled, doubleService, nJobs, nNurses, nSkills);
    if (verbose_data > 5){
        printf("Allocated nurseSkilled.\n");
    }

    double** prefScore;
    nRows = nJobs;
    nCols = nNurses;
    prefScore = malloc(nRows * sizeof(double*));
    for (int i = 0; i < nRows; i++){
        prefScore[i] = malloc(nCols*sizeof(double));
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

    int nOptions = 100;
    double* algorithmOptions = malloc(nOptions * sizeof(double));
    for (int i = 0; i < nOptions; ++i) {
        algorithmOptions[i] = algorithmOptions_data[i];
    }
    if (verbose_data > 5){
        printf("Allocated algorithmOptions.\n");
    }

    int*** capabilityOfDoubleServices;
    nRows = nNurses;
    nCols = nNurses;
    capabilityOfDoubleServices = malloc(nRows * sizeof(int**));
    for (int i = 0; i < nRows; i++) {
        capabilityOfDoubleServices[i] = malloc(nCols * sizeof(int*));
        for (int j = 0; j < nCols; j++){
            capabilityOfDoubleServices[i][j] = malloc(nDoubleServices*sizeof(int));
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

    int*** unavailMatrix;
    nRows = 50;
    nCols = 4;
    unavailMatrix = malloc(nRows * sizeof(int**));
    for (int i = 0; i < nRows; i++) {
        unavailMatrix[i] = malloc(nCols * sizeof(int*));
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

    double** nurseWaitingMatrix;
    nRows = nNurses;
    nCols = nJobs;
    nurseWaitingMatrix = malloc(nRows * sizeof(double*));
    for (int i = 0; i < nRows; i++){
        nurseWaitingMatrix[i] = malloc(nCols*sizeof(double));
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseWaitingMatrix[i][j] = 0;
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseWaitingMatrix.\n");
    }

    double** nurseTravelMatrix;
    nRows = nNurses;
    nCols = nJobs;
    nurseTravelMatrix = malloc(nRows * sizeof(double*));
    for (int i = 0; i < nRows; i++){
        nurseTravelMatrix[i] = malloc(nCols*sizeof(double));
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseTravelMatrix[i][j] = 0;
        }
    }
    if (verbose_data > 5){
        printf("Allocated nurseTravelMatrix.\n");
    }

    int* nurseOrder = malloc(nNurses * sizeof(int));
    for (int i = 0; i < nNurses; ++i){
        nurseOrder[i] = i;
    }

    double totalServiceTime = 0.0;
    for (int j = 0; j < nJobs; ++j){
        totalServiceTime += jobTimeInfo[j][2];
    }

    double totalServiceTimeIncDS = totalServiceTime;
    for (int j = 0; j < nJobs; ++j){
        if(doubleService[j] == 1){
            totalServiceTimeIncDS += jobTimeInfo[j][2];
        }
    }

    double** arrivalTimes;
    nRows = nNurses;
    nCols = nJobs;
    arrivalTimes = malloc(nRows * sizeof(double*));
    for (int i = 0; i < nRows; i++){
        arrivalTimes[i] = malloc(nCols*sizeof(double));
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            arrivalTimes[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated arrivalTimes.\n");
    }

    verbose_data = printAllAllocations;

    struct INSTANCE inst = { .nJobs = nJobs,
            .nJobsIncDS = nJobsIncDS, // NB: NEW 08/11/2021
            .nNurses = nNurses,
            .nSkills = nSkills,
            .verbose = verbose_data,
            .qualityMeasure = quality_measure,
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
            .totalServiceTimeIncDS = totalServiceTimeIncDS, //NB NEW 06/11/2021
            .arrivalTimes = arrivalTimes // NB NEW 28/02/2022
    };

    return inst;

}

void FreeInstanceCopy(struct INSTANCE* ip) {
	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->solMatrix[i]);
	free(ip->solMatrix);
	if (ip->verbose > 10)
		printf("Freed solMatrix. (copy)\n");

	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->timeMatrix[i]);
	free(ip->timeMatrix);
	if (ip->verbose > 10)
		printf("Freed timeMatrix. (copy)\n");

	for (int i = 0; i < ip->nNurses; ++i)
		free(ip->allNurseRoutes[i]);
	free(ip->allNurseRoutes);
	if (ip->verbose > 10)
		printf("Freed allNurseRoutes. (copy)\n");

	free(ip->violatedTW);
	if (ip->verbose > 10)
		printf("freed ip->violatedTW (copy)\n");

	free(ip->violatedTWMK);
	if (ip->verbose > 10)
		printf("freed ip->violatedTWMK (copy)\n");

	free(ip->nurseRoute);
	if (ip->verbose > 10)
		printf("freed ip->nurseRoute (copy)\n");

	free(ip->nurseOrder);
	if (ip->verbose > 10)
		printf("freed ip->nurseOrder (copy)\n");

	free(ip->nurseWaitingTime);
	if (ip->verbose > 10)
		printf("freed ip->nurseWaitingTime (copy)\n");

	free(ip->nurseTravelTime);
	if (ip->verbose > 10)
		printf("freed ip->nurseTravelTime (copy)\n");

	if (ip->verbose > 10)
		printf("Finished freeing memory of instance copy.\n");
}

void FreeInstanceMemory(struct INSTANCE* ip) {
	free(ip->nurseRoute);
	ip->nurseRoute = NULL;
	if (ip->verbose > 10)
		printf("Set nurseRoute to NULL.\n");

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


	nRows = nJobs + 1;
	nCols = nJobs + 1;
	double** od;
	od = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++)
		od[i] = malloc(nCols * sizeof(double)); // Cols

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			od[i][j] = (double)od_data[i][j];


	int** nurseSkilled;
	nRows = nNurses;
	nCols = nJobs;
	nurseSkilled = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++)
		nurseSkilled[i] = malloc(nCols * sizeof(int));
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			nurseSkilled[i][j] = nurseSkilled_data[i][j];

	printf("Allocated jobDuration.\n");



	int** nurseWorkingTimes;
	nRows = nNurses;
	nCols = 3;
	nurseWorkingTimes = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++)
		nurseWorkingTimes[i] = malloc(nCols * sizeof(int));

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			nurseWorkingTimes[i][j] = nurseWorkingTimes_data[i][j];

	int** nurseSkills;
	nRows = nNurses;
	nCols = nSkills;
	nurseSkills = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++)
		nurseSkills[i] = malloc(nCols * sizeof(int));
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			nurseSkills[i][j] = nurseSkills_data[i][j];

	int** jobRequirements;
	nRows = nJobs;
	nCols = nSkills;
	jobRequirements = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++)
		jobRequirements[i] = malloc(nCols * sizeof(int)); // Cols

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			jobRequirements[i][j] = jobRequirements_data[i][j];



	int** jobTimeInfo;
	nRows = nJobs;
	nCols = 3;
	jobTimeInfo = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++)
		jobTimeInfo[i] = malloc(nCols * sizeof(int));

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			jobTimeInfo[i][j] = jobTimeInfo_data[i][j];



	int** solMatrix;
	nRows = nNurses;
	nCols = nJobs;
	solMatrix = malloc(nRows * sizeof(int*));
	for (int i = 0; i < nRows; i++)
		solMatrix[i] = malloc(nCols * sizeof(int));
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			solMatrix[i][j] = -1;


	double** timeMatrix;
	nRows = nNurses;
	nCols = nJobs;
	timeMatrix = malloc(nRows * sizeof(double*));
	for (int i = 0; i < nRows; i++)
		timeMatrix[i] = malloc(nCols * sizeof(double));

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			timeMatrix[i][j] = -1;
	if (verbose_data > 5)
		printf("Allocated timeMatrix.\n");


	int* nurseRoute = malloc(nJobs * sizeof(int));
	double* nurseWaitingTime = malloc(nNurses * sizeof(double));
	int* doubleService = malloc(nJobs * sizeof(int));
	double* violatedTW = malloc(nJobs * sizeof(double));

	float MAX_TIME_SECONDS = 600;
	struct INSTANCE inst = { .nJobs = nJobs,
									.nNurses = nNurses,
									.nSkills = nSkills,
									.verbose = verbose_data,
									.MAX_TIME_SECONDS = MAX_TIME_SECONDS,
									.od = od,
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
		.isFeasible = 0
    };

	return inst;
}

void NurseSkilledFromSkillsAndRequirements(int** nurseSkills, int** jobRequirements, int** nurseSkilled, int* doubleService, int nJobs, int nNurses, int nSkills) {
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
			exit(-2343);
		}
	}

}