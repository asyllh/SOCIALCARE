/*--------------/
GRASP_VNS
read_instance.c
UoS
10/07/2021
/--------------*/


#include "constructive.h"
void overwrite_instance(struct INSTANCE* ip, struct INSTANCE* oi) {
	/*
	Copies the contents of the solution of "oi" into "ip"
	without creating any new allocations.
	Only copies the solution, no algorithm options or data
	*/

	for (int i = 0; i < oi->nCarers; ++i){
        for(int j = 0; j < oi->nJobs; ++j){
            ip->solMatrix[i][j] = oi->solMatrix[i][j];
        }
    }

	for (int i = 0; i < oi->nCarers; ++i){
        for(int j = 0; j < oi->nJobs; ++j){
            ip->timeMatrix[i][j] = oi->timeMatrix[i][j];
        }
    }

	for (int i = 0; i < oi->nCarers; ++i){
        for(int j = 0; j < oi->nJobs; ++j){
            ip->allCarerRoutes[i][j] = oi->allCarerRoutes[i][j];
        }
    }

    for (int i = 0; i < oi->nCarers; ++i){ // NB: NEW: 03/06/2021
        for(int j = 0; j < oi->nJobs; ++j){
            ip->carerWaitingMatrix[i][j] = oi->carerWaitingMatrix[i][j];
        }
    }

    for (int i = 0; i < oi->nCarers; ++i){ // NB: NEW: 03/06/2021
        for(int j = 0; j < oi->nJobs; ++j){
            ip->carerTravelMatrix[i][j] = oi->carerTravelMatrix[i][j];
        }
    }

	for (int i = 0; i < oi->nCarers; ++i) {
		ip->carerOrder[i] = oi->carerOrder[i];
		ip->carerWaitingTime[i] = oi->carerWaitingTime[i];
		ip->carerTravelTime[i] = oi->carerTravelTime[i];
	}

	for (int i = 0; i < oi->nJobs; ++i) {
		ip->carerRoute[i] = oi->carerRoute[i];
		ip->violatedTW[i] = oi->violatedTW[i];
		ip->violatedTWMK[i] = oi->violatedTWMK[i];
	}

} //END OF overwrite_instance function.

struct INSTANCE copy_instance(struct INSTANCE* oi) {
	int LOCAL_VERBOSE = oi->verbose;

	int** solMatrix;
	int nRows = oi->nCarers;
	int nCols = oi->nJobs;
	solMatrix = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++){
        solMatrix[i] = malloc(nCols*sizeof(int)); // Cols
    }
	for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrix[i][j] = oi->solMatrix[i][j];
        }
    }
	if (LOCAL_VERBOSE > 5){
        printf("Allocated solMatrix. (copying)\n");
    }


	double** timeMatrix;
	nRows = oi->nCarers;
	nCols = oi->nJobs;
	timeMatrix = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++){
        timeMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
	for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            timeMatrix[i][j] = oi->timeMatrix[i][j];
        }
    }
	if (LOCAL_VERBOSE > 5){
        printf("Allocated timeMatrix. (copying)\n");
    }

	int** allNurseRoutes;
	nRows = oi->nCarers;
	nCols = oi->nJobs;
	allNurseRoutes = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++){
        allNurseRoutes[i] = malloc(nCols*sizeof(int)); // Cols
    }
	for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allNurseRoutes[i][j] = oi->allCarerRoutes[i][j];
        }
    }
	if (LOCAL_VERBOSE > 5){
        printf("Allocated allCarerRoutes. (copying)\n");
    }

    double** nurseWaitingMatrix; //NB: NEW: 03/06/2021
    nRows = oi->nCarers;
    nCols = oi->nJobs;
    nurseWaitingMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseWaitingMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseWaitingMatrix[i][j] = oi->carerWaitingMatrix[i][j];
        }
    }
    if (LOCAL_VERBOSE > 5){
        printf("Allocated carerWaitingMatrix. (copying)\n");
    }

    double** nurseTravelMatrix; //NB: NEW: 03/06/2021
    nRows = oi->nCarers;
    nCols = oi->nJobs;
    nurseTravelMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        nurseTravelMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            nurseTravelMatrix[i][j] = oi->carerTravelMatrix[i][j];
        }
    }
    if (LOCAL_VERBOSE > 5){
        printf("Allocated carerTravelMatrix. (copying)\n");
    }

	// Allocate and populate vectors of size nCarers
	double* nurseWaitingTime = malloc(oi->nCarers * sizeof(double));
	double* nurseTravelTime = malloc(oi->nCarers * sizeof(double));
	int* nurseOrder = malloc(oi->nCarers * sizeof(int));
	for (int i = 0; i < oi->nCarers; ++i) {
		nurseOrder[i] = oi->carerOrder[i];
		nurseWaitingTime[i] = oi->carerWaitingTime[i];
		nurseTravelTime[i] = oi->carerTravelTime[i];
	}
	if (LOCAL_VERBOSE > 5){
        printf("Allocated vectors of size nCarers. (copying)\n");
    }

	// Allocate and populate vectors of size nJobs
	int* nurseRoute = malloc(oi->nJobs * sizeof(int));
	double* violatedTW = malloc(oi->nJobs * sizeof(double));
	double* violatedTWMK = malloc(oi->nJobs * sizeof(double));
	for (int i = 0; i < oi->nJobs; ++i) {
		nurseRoute[i] = oi->carerRoute[i];
		violatedTW[i] = oi->violatedTW[i];
		violatedTWMK[i] = oi->violatedTWMK[i];
	}
	if (LOCAL_VERBOSE > 5){
        printf("Allocated vectors of size nJobs. (copying)\n");
    }

	if (LOCAL_VERBOSE > 5){
        printf("Creating instance struct... (copying)\n");
    }

	struct INSTANCE inst = { .nJobs = oi->nJobs,
									.nCarers = oi->nCarers,
									.nSkills = oi->nSkills,
									.verbose = oi->verbose,
									.quality_measure = oi->quality_measure,
									.MAX_TIME_SECONDS = oi->MAX_TIME_SECONDS,
                                    .tw_interval = oi->tw_interval,
                                    .exclude_carer_travel = oi->exclude_carer_travel,
									.od = oi->od,
									.carer_travel_from_depot = oi->carer_travel_from_depot,
									.carer_travel_to_depot = oi->carer_travel_to_depot,
                                    .unavailMatrix = oi->unavailMatrix,
                                    .carerUnavail = oi->carerUnavail,
									.carerWorkingTimes = oi->carerWorkingTimes,
									.solMatrix = solMatrix,
									.timeMatrix = timeMatrix,
									.jobTimeInfo = oi->jobTimeInfo,
									.jobRequirements = oi->jobRequirements,
									.carerSkills = oi->carerSkills,
									.carerSkilled = oi->carerSkilled,
									.carerRoute = nurseRoute,
									.allCarerRoutes = allNurseRoutes,
									.carerOrder = nurseOrder,
									.carerWaitingTime = nurseWaitingTime,
									.carerTravelTime = nurseTravelTime,
									.doubleService = oi->doubleService,
									.dependsOn = oi->dependsOn,
									.violatedTW = violatedTW,
									.violatedTWMK = violatedTWMK,
									.isFeasible = 0,
									.MK_mind = oi->MK_mind,
									.MK_maxd = oi->MK_maxd,
									.capabilityOfDoubleServices = oi->capabilityOfDoubleServices,
									.prefScore = oi->prefScore,
									.algorithmOptions = oi->algorithmOptions,
									.carerWaitingMatrix = oi->carerWaitingMatrix, //NB: NEW: 03/06/2021
									.carerTravelMatrix = oi->carerTravelMatrix //NB: NEW: 03/06/2021
	};

	if (LOCAL_VERBOSE > 5){
        printf("Done. (copying)\n");
    }
	return inst;

}


// // Functions to read instances, and some hardcoded instances too
struct INSTANCE instance_from_python(int nJobs_data, int nCarers_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int tw_interval_data, bool exclude_carer_travel_data,
        double* od_data, double* carer_travel_from_depot_data, double* carer_travel_to_depot_data, int* unavail_matrix_data, int* carer_unavail_data,
        int* carerWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* carerSkills_data, int* doubleService_data, int* dependsOn_data,
        int* mk_mind_data, int* mk_maxd_data, int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data) {

    int printAllAllocations = 0;

    // quality_measure = 1 is Mk, 0 is Ait H.
    // Add very small number just to prevent accuracy errors
    int quality_measure = (int)(algorithmOptions_data[0] + 1e-6);

    if (printAllAllocations > 0){
        verbose_data = 6;
    }

    printAllAllocations = verbose_data;

    int ct = -1;
    int nRows, nCols;
    int nJobs = nJobs_data;
    int nCarers = nCarers_data;
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


    // 2. double** carer_travel_from_depot: nCarers x nJobs (using double* carer_travel_from_depot_data)
    nRows = nCarers;
    nCols = nJobs;
    ct = -1;
    double** carer_travel_from_depot;
    carer_travel_from_depot = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        carer_travel_from_depot[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            carer_travel_from_depot[i][j] = (double) carer_travel_from_depot_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated carer_travel_from_depot.\n");
    }

    // 3. double** nurse_travel_to_depot: nNurses x nJobs (using double* nurse_travel_to_depot_data)
    double** carer_travel_to_depot;
    ct = -1;
    carer_travel_to_depot = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        carer_travel_to_depot[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            carer_travel_to_depot[i][j] = (double) carer_travel_to_depot_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated carer_travel_to_depot.\n");
    }

    // 4. int** carerWorkingTimes: nCarers x 3 (using int* carerWorkingTimes_data)
    int** carerWorkingTimes;
    nRows = nCarers;
    nCols = 3;
    carerWorkingTimes = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        carerWorkingTimes[i] = malloc(nCols*sizeof(int)); // Cols
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            carerWorkingTimes[i][j] = carerWorkingTimes_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated carerWorkingTimes.\n");
    }

    // 5. int** nurseSkills: nNurses x nSkills (using int* nurseSkills_data)
    int** carerSkills;
    nRows = nCarers;
    nCols = nSkills;
    carerSkills = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        carerSkills[i] = malloc(nCols*sizeof(int)); // Cols
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            ct++;
            carerSkills[i][j] = carerSkills_data[ct];
        }
    }
    if (verbose_data > 5){
        printf("Allocated carerSkills.\n");
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
    nRows = nCarers;
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
    nRows = nCarers;
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
    int* carerRoute = malloc(nJobs * sizeof(int));

    // 11. int** allCarerRoutes: nCarers x nJobs (set to all -1)
    int** allCarerRoutes;
    nRows = nCarers;
    nCols = nJobs;
    allCarerRoutes = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        allCarerRoutes[i] = malloc(nCols*sizeof(int)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            allCarerRoutes[i][j] = -1;
        }
    }
    if (verbose_data > 5){
        printf("Allocated allCarerRoutes.\n");
    }

    // 12. double* carerWaitingTime: 1 x nCarers
    double* carerWaitingTime = malloc(nCarers * sizeof(double));

    // 13. double nurseTravelTime: 1 x nNurses
    double* carerTravelTime = malloc(nCarers * sizeof(double));

    // 14. - 17. int* doubleService, dependsOn, mk_mind, mk_maxd: 1 x nJobs (using _data)
    int* doubleService = malloc(nJobs * sizeof(int));
    int* dependsOn = malloc(nJobs * sizeof(int));
    int* mk_mind = malloc(nJobs * sizeof(int));
    int* mk_maxd = malloc(nJobs * sizeof(int));
    int nDoubleServices = 0;
    for (int i = 0; i < nJobs; ++i) {
        doubleService[i] = doubleService_data[i];
        dependsOn[i] = dependsOn_data[i];
        mk_mind[i] = mk_mind_data[i + 1];
        mk_maxd[i] = mk_maxd_data[i + 1];
        nDoubleServices += doubleService_data[i];
    }

    // 18. int* carerUnavail: 1 x nCarers (using int* carer_unavail_data) (24/05/2021)
    int* carerUnavail = malloc(nCarers * sizeof(int));
    for (int i = 0; i < nCarers; i++){
        carerUnavail[i] = carer_unavail_data[i];
    }

    // 19. double* violatedTW: 1 x nJobs
    double* violatedTW = malloc(nJobs * sizeof(double));

    // 20. double* violatedTWMK: 1 x nJobs
    double* violatedTWMK = malloc(nJobs * sizeof(double));

    // 21. int** carerSkilled: nCarers x nJobs (set using carerSkills, jobRequirements, doubleService)
    int** carerSkilled;
    nRows = nCarers;
    nCols = nJobs;
    carerSkilled = malloc(nRows * sizeof(int*)); // Rows
    for (int i = 0; i < nRows; i++){
        carerSkilled[i] = malloc(nCols*sizeof(int)); // Cols
    }
    carer_skilled_from_skills_and_requirements(carerSkills, jobRequirements, carerSkilled, doubleService, nJobs, nCarers, nSkills);
    if (verbose_data > 5){
        printf("Allocated carerSkilled.\n");
    }

    // 22. double** prefScore: nJobs x nCarers (using double* prefScore_data)
    double** prefScore; // Job x Nurse
    nRows = nJobs;
    nCols = nCarers;
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

    // 24. int*** capabilityOfDoubleServices: nCarers x nCarers x nDoubleServices (using int* capabilityOfDoubleServices_data)
    int*** capabilityOfDoubleServices;
    nRows = nCarers;
    nCols = nCarers;
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
    nRows = 50; /** NEED TO CHANGE THIS **/
    nCols = 4;
    // dim3 is nNurses
    unavailMatrix = malloc(nRows * sizeof(int**)); // Rows
    for (int i = 0; i < nRows; i++) {
        unavailMatrix[i] = malloc(nCols * sizeof(int*)); // Cols
        for (int j = 0; j < nCols; j++){
            unavailMatrix[i][j] = malloc(nCarers*sizeof(int));
        }
    }
    ct = -1;
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            for (int k = 0; k < nCarers; ++k) {
                ct++;
                unavailMatrix[i][j][k] = unavail_matrix_data[ct];
            }
        }
    }
    if (verbose_data > 5) {
        printf("Allocated unavailMatrix.\n");
    }

    // 26. double** carerWaitingMatrix: nCarers x nJobs (set to all 0)
    // NB: NEW: 03/06/2021
    double** carerWaitingMatrix;
    nRows = nCarers;
    nCols = nJobs;
    carerWaitingMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        carerWaitingMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            carerWaitingMatrix[i][j] = 0;
        }
    }
    if (verbose_data > 5){
        printf("Allocated carerWaitingMatrix.\n");
    }

    // 27. double** carerWaitingMatrix: nCarers x nJobs (set to all 0)
    // NB: NEW: 03/06/2021
    double** carerTravelMatrix;
    nRows = nCarers;
    nCols = nJobs;
    carerTravelMatrix = malloc(nRows * sizeof(double*)); // Rows
    for (int i = 0; i < nRows; i++){
        carerTravelMatrix[i] = malloc(nCols*sizeof(double)); // Cols
    }
    for (int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            carerTravelMatrix[i][j] = 0;
        }
    }
    if (verbose_data > 5){
        printf("Allocated carerTravelMatrix.\n");
    }

    // 28. int* carerOrder: 1 x nCarers (0... nCarers-1)
    int* carerOrder = malloc(nCarers * sizeof(int));
    for (int i = 0; i < nCarers; ++i){
        carerOrder[i] = i;
    }

    verbose_data = printAllAllocations;	// Return this to what it was

    struct INSTANCE inst = { .nJobs = nJobs,
            .nCarers = nCarers,
            .nSkills = nSkills,
            .verbose = verbose_data,
            .quality_measure = quality_measure,
            .MAX_TIME_SECONDS = MAX_TIME_SECONDS,
            .tw_interval = tw_interval_data,
            .exclude_carer_travel = exclude_carer_travel_data,
            .od = od,
            .carer_travel_from_depot = carer_travel_from_depot,
            .carer_travel_to_depot = carer_travel_to_depot,
            .unavailMatrix = unavailMatrix,
            .carerUnavail = carerUnavail,
            .carerWorkingTimes = carerWorkingTimes,
            .solMatrix = solMatrix,
            .timeMatrix = timeMatrix,
            .jobTimeInfo = jobTimeInfo,
            .jobRequirements = jobRequirements,
            .carerSkills = carerSkills,
            .carerSkilled = carerSkilled,
            .carerRoute = carerRoute,
            .allCarerRoutes = allCarerRoutes,
            .carerOrder = carerOrder,
            .carerWaitingTime = carerWaitingTime,
            .carerTravelTime = carerTravelTime,
            .doubleService = doubleService,
            .dependsOn = dependsOn,
            .violatedTW = violatedTW,
            .violatedTWMK = violatedTWMK,
            .isFeasible = 0,
            .MK_mind = mk_mind,
            .MK_maxd = mk_maxd,
            .capabilityOfDoubleServices = capabilityOfDoubleServices,
            .prefScore = prefScore,
            .algorithmOptions = algorithmOptions,
            .carerWaitingMatrix = carerWaitingMatrix, // NB: NEW: 03/06/2021
            .carerTravelMatrix = carerTravelMatrix // NB: NEW: 03/06/2021
    };

    return inst;

} // END OF instance_from_python

void free_instance_copy(struct INSTANCE* ip) {
	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->solMatrix[i]);
	free(ip->solMatrix);
	if (ip->verbose > 10)
		printf("Freed solMatrix. (copy)\n");

	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->timeMatrix[i]);
	free(ip->timeMatrix);
	if (ip->verbose > 10)
		printf("Freed timeMatrix. (copy)\n");

	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->allCarerRoutes[i]);
	free(ip->allCarerRoutes);
	if (ip->verbose > 10)
		printf("Freed allCarerRoutes. (copy)\n");

	free(ip->violatedTW);
	if (ip->verbose > 10)
		printf("freed ip->violatedTW (copy)\n");

	free(ip->violatedTWMK);
	if (ip->verbose > 10)
		printf("freed ip->violatedTWMK (copy)\n");

	free(ip->carerRoute);
	// ip->carerRoute = NULL;
	if (ip->verbose > 10)
		printf("freed ip->carerRoute (copy)\n");

	free(ip->carerOrder);
	if (ip->verbose > 10)
		printf("freed ip->carerOrder (copy)\n");

	free(ip->carerWaitingTime);
	if (ip->verbose > 10)
		printf("freed ip->carerWaitingTime (copy)\n");

	free(ip->carerTravelTime);
	if (ip->verbose > 10)
		printf("freed ip->carerTravelTime (copy)\n");

	if (ip->verbose > 10)
		printf("Finished freeing memory of instance copy.\n");
}

void free_instance_memory(struct INSTANCE* ip) {
	free(ip->carerRoute);
	ip->carerRoute = NULL; // Used as temp. array to avoid allocations
	if (ip->verbose > 10)
		printf("Set carerRoute to NULL.\n");

	// printf("\nWarning: There are more things to free!\n");
	/* deallocate the array */
	for (int i = 0; i < (ip->nJobs + 1); i++)
		free(ip->od[i]);
	free(ip->od);
	if (ip->verbose > 10)
		printf("Freed od.\n");

	for (int i = 0; i < (ip->nCarers); i++)
		free(ip->carer_travel_from_depot[i]);
	free(ip->carer_travel_from_depot);
	if (ip->verbose > 10)
		printf("Freed carer_travel_from_depot.\n");

	for (int i = 0; i < (ip->nCarers); i++)
		free(ip->carer_travel_to_depot[i]);
	free(ip->carer_travel_to_depot);
	if (ip->verbose > 10)
		printf("Freed carer_travel_to_depot.\n");

	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->solMatrix[i]);
	free(ip->solMatrix);
	if (ip->verbose > 10)
		printf("Freed solMatrix.\n");

	for (int i = 0; i < ip->nCarers; ++i)
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

	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->carerSkills[i]);
	free(ip->carerSkills);
	if (ip->verbose > 10)
		printf("Freed carerSkills.\n");


	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->carerSkilled[i]);
	free(ip->carerSkilled);
	if (ip->verbose > 10)
		printf("Freed carerSkilled.\n");

	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->carerWorkingTimes[i]);
	free(ip->carerWorkingTimes);
	if (ip->verbose > 10)
		printf("Freed carerWorkingTimes.\n\n");

	free(ip->violatedTWMK);
	if (ip->verbose > 10)
		printf("freed ip->violatedTWMK\n");

	free(ip->violatedTW);
	if (ip->verbose > 10)
		printf("freed ip->violatedTW\n");

	for (int i = 0; i < ip->nCarers; ++i)
		free(ip->allCarerRoutes[i]);
	free(ip->allCarerRoutes);
	if (ip->verbose > 10)
		printf("Freed allCarerRoutes.\n");


	// free(ip->carerRoute);
	// ip->carerRoute = NULL;
	// if(ip->verbose > 10)
	// 	printf("freed ip->carerRoute (set to NULL)\n");

	free(ip->carerOrder);
	ip->carerOrder = NULL;
	if (ip->verbose > 10)
		printf("freed ip->carerOrder\n");

	free(ip->doubleService);
	if (ip->verbose > 10)
		printf("freed ip->doubleService\n");

	free(ip->dependsOn);
	if (ip->verbose > 10)
		printf("freed ip->dependsOn\n");

	free(ip->carerWaitingTime);
	if (ip->verbose > 10)
		printf("freed ip->carerWaitingTime\n");

	free(ip->carerTravelTime);
	if (ip->verbose > 10)
		printf("freed ip->carerTravelTime\n");

	free(ip->algorithmOptions);
	if (ip->verbose > 10)
		printf("freed ip->algorithmOptions\n");

	free(ip->MK_mind);
	if (ip->verbose > 10)
		printf("freed ip->mk_mind\n");

	free(ip->MK_maxd);
	if (ip->verbose > 10)
		printf("freed ip->MK_maxd\n");

	if (ip->verbose > 10)
		printf("Finished freeing memory.\n");

}




struct INSTANCE generate_instance() {
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
		int nCarers = 3;
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
		int nCarers = 3;
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
			int nCarers = 2;
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
	// for (int i = 0; i < nCarers; ++i)
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
	// printf("Allocated carerSkilled.\n");



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
	// printf("Allocated carerWorkingTimes.\n");

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
	// printf("Allocated carerSkills.\n");

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

	// int * carerRoute = 	malloc(nJobs * sizeof(int));
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
	// struct INSTANCE inst = {.od = od, .startTime = startTime, .nJobs = nJobs, .nCarers = nCarers, .solMatrix = solMatrix, .jobTimeWindow = jobTimeWindow, .jobDuration = jobDuration, .carerSkilled = carerSkilled};

	// struct INSTANCE inst = {.nJobs = nJobs, 
	// 						.nCarers = nCarers,
	// 						.nSkills = nSkills, 
	// 						.verbose = 5, 
	// 						.MAX_TIME_SECONDS = 600,
	// 						.od = od, 
	// 						// .od_cost = od_cost, 
	// 						.carerWorkingTimes = carerWorkingTimes,
	// 						.solMatrix = solMatrix, 
	// 						.jobTimeInfo = jobTimeInfo, 
	// 						.jobRequirements = jobRequirements, 
	// 						.carerSkills = carerSkills,
	// 						.carerSkilled = carerSkilled,
	// 						.carerRoute = carerRoute};
	// New version:
	float MAX_TIME_SECONDS = 600;
	struct INSTANCE inst = { .nJobs = nJobs,
									.nCarers = nNurses,
									.nSkills = nSkills,
									.verbose = verbose_data,
									.MAX_TIME_SECONDS = MAX_TIME_SECONDS,
									.od = od,
		// .od_cost = od_cost, 
		.carerWorkingTimes = nurseWorkingTimes,
		.solMatrix = solMatrix,
		.timeMatrix = timeMatrix,
		.jobTimeInfo = jobTimeInfo,
		.jobRequirements = jobRequirements,
		.carerSkills = nurseSkills,
		.carerSkilled = nurseSkilled,
		.carerRoute = nurseRoute,
		.carerWaitingTime = nurseWaitingTime,
		.doubleService = doubleService,
		.violatedTW = violatedTW,
		.isFeasible = 0 };


	// ip = &inst;
	return inst;
}

void carer_skilled_from_skills_and_requirements(int** carerSkills, int** jobRequirements, int** carerSkilled, int* doubleService, int nJobs, int nCarers, int nSkills) {
	int canDoIt = 1;
	for (int i = 0; i < nCarers; ++i) {
		for (int j = 0; j < nJobs; ++j) {
			canDoIt = 1;
			for (int k = 0; k < nSkills; ++k) {
				if (carerSkills[i][k] < jobRequirements[j][k]) {
					// if (j == 9)
					// {
					// 	printf("a[%d][%d] = %d, r[%d][%d] = %d\n", i,k,a[i][k], j, k, r[j][k]);
					// }
					canDoIt = 0;
					break;
				}
			}
            carerSkilled[i][j] = canDoIt;
		}
	}


	int nursesThatCanDoIt = 0;
	for (int j = 0; j < nJobs; ++j) {
		nursesThatCanDoIt = 0;
		for (int i = 0; i < nCarers; ++i) {
			if (carerSkilled[i][j] > 0)
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
	// for (int i = 0; i < nCarers; ++i)
	// {
	// 	for (int j = 0; j < nJobs; ++j)
	// 	{
	// 		printf("%d\t", carerSkilled[i][j]);
	// 	}
	// 	printf("\n");
	// }


}