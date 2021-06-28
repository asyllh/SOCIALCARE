#include "constructive.h"
void overwrite_instance(struct INSTANCE* ip, struct INSTANCE* oi) {
	/*
	Copies the contents of the solution of "oi" into "ip"
	without creating any new allocations.
	Only copies the solution, no algorithm options or data
	*/

	for (int i = 0; i < oi->nNurses; ++i)
		for (int j = 0; j < oi->nJobs; ++j)
			ip->solMatrix[i][j] = oi->solMatrix[i][j];

	for (int i = 0; i < oi->nNurses; ++i)
		for (int j = 0; j < oi->nJobs; ++j)
			ip->timeMatrix[i][j] = oi->timeMatrix[i][j];


	for (int i = 0; i < oi->nNurses; ++i)
		for (int j = 0; j < oi->nJobs; ++j)
			ip->allNurseRoutes[i][j] = oi->allNurseRoutes[i][j];

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

} //END OF overwrite_instance function.

struct INSTANCE copy_instance(struct INSTANCE* oi) {
	int LOCAL_VERBOSE = oi->verbose;
	int** solMatrix;
	int nRows = oi->nNurses;
	int nCols = oi->nJobs;

	solMatrix = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		solMatrix[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			solMatrix[i][j] = oi->solMatrix[i][j];
	//end populating
	if (LOCAL_VERBOSE > 5)
		printf("Allocated solMatrix. (copying)\n");


	double** timeMatrix;
	nRows = oi->nNurses;
	nCols = oi->nJobs;
	timeMatrix = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++)
		timeMatrix[i] = malloc(nCols * sizeof(double)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			timeMatrix[i][j] = oi->timeMatrix[i][j];
	//end populating
	if (LOCAL_VERBOSE > 5)
		printf("Allocated timeMatrix. (copying)\n");

	int** allNurseRoutes;
	nRows = oi->nNurses;
	nCols = oi->nJobs;
	allNurseRoutes = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		allNurseRoutes[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			allNurseRoutes[i][j] = oi->allNurseRoutes[i][j];
	//end populating
	if (LOCAL_VERBOSE > 5)
		printf("Allocated allNurseRoutes. (copying)\n");


	// Allocate and populate vectors of size nNurses
	double* nurseWaitingTime = malloc(oi->nNurses * sizeof(double));
	double* nurseTravelTime = malloc(oi->nNurses * sizeof(double));
	int* nurseOrder = malloc(oi->nNurses * sizeof(int));
	for (int i = 0; i < oi->nNurses; ++i) {
		nurseOrder[i] = oi->nurseOrder[i];
		nurseWaitingTime[i] = oi->nurseWaitingTime[i];
		nurseTravelTime[i] = oi->nurseTravelTime[i];
	}
	if (LOCAL_VERBOSE > 5)
		printf("Allocated vectors of size nNurses. (copying)\n");

	// Allocate and populate vectors of size nJobs
	int* nurseRoute = malloc(oi->nJobs * sizeof(int));
	double* violatedTW = malloc(oi->nJobs * sizeof(double));
	double* violatedTWMK = malloc(oi->nJobs * sizeof(double));
	for (int i = 0; i < oi->nJobs; ++i) {
		nurseRoute[i] = oi->nurseRoute[i];
		violatedTW[i] = oi->violatedTW[i];
		violatedTWMK[i] = oi->violatedTWMK[i];
	}
	if (LOCAL_VERBOSE > 5)
		printf("Allocated vectors of size nJobs. (copying)\n");

	if (LOCAL_VERBOSE > 5)
		printf("Creating instance struct... (copying)\n");
	struct INSTANCE inst = { .nJobs = oi->nJobs,
									.nNurses = oi->nNurses,
									.nSkills = oi->nSkills,
									.verbose = oi->verbose,
									.quality_measure = oi->quality_measure,
									.MAX_TIME_SECONDS = oi->MAX_TIME_SECONDS,
                                    .tw_interval = oi->tw_interval,
                                    .exclude_nurse_travel = oi->exclude_nurse_travel,
									.od = oi->od,
									.nurse_travel_from_depot = oi->nurse_travel_from_depot,
									.nurse_travel_to_depot = oi->nurse_travel_to_depot,
                                    .unavailMatrix = oi->unavailMatrix,
                                    .nurseUnavail = oi->nurseUnavail,
									.nurseWorkingTimes = oi->nurseWorkingTimes,
									.solMatrix = solMatrix,
									.timeMatrix = timeMatrix,
									.jobTimeInfo = oi->jobTimeInfo,
									.jobRequirements = oi->jobRequirements,
									.nurseSkills = oi->nurseSkills,
									.nurseSkilled = oi->nurseSkilled,
									.nurseRoute = nurseRoute,
									.allNurseRoutes = allNurseRoutes,
									.nurseOrder = nurseOrder,
									.nurseWaitingTime = nurseWaitingTime,
									.nurseTravelTime = nurseTravelTime,
									.doubleService = oi->doubleService,
									.dependsOn = oi->dependsOn,
									.violatedTW = violatedTW,
									.violatedTWMK = violatedTWMK,
									.isFeasible = 0,
									.MK_mind = oi->MK_mind,
									.MK_maxd = oi->MK_maxd,
									.capabilityOfDoubleServices = oi->capabilityOfDoubleServices,
									.prefScore = oi->prefScore,
									.algorithmOptions = oi->algorithmOptions
	};
	if (LOCAL_VERBOSE > 5)
		printf("Done. (copying)\n");
	return inst;

}


// // Functions to read instances, and some hardcoded instances too
struct INSTANCE instance_from_python(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int tw_interval_data, bool exclude_nurse_travel_data,
	                                double* od_data, double* nurse_travel_from_depot_data, double* nurse_travel_to_depot_data, int* unavail_matrix_data, int* nurse_unavail_data,
	                                int* nurseWorkingTimes_data, int* jobTimeInfo_data,
	                                int* jobRequirements_data, int* nurseSkills_data, int* doubleService_data, int* dependsOn_data, int* mk_mind_data, int* mk_maxd_data,
	                                int* capabilityOfDoubleServices_data, double* prefScore_data, double* algorithmOptions_data) {
	int printAllAllocations = 0;

	// quality_measure = 1 is Mk, 0 is Ait H.
	// Add very small number just to prevent accuracy errors
	int quality_measure = (int)(algorithmOptions_data[0] + 1e-6);

	if (printAllAllocations > 0)
		verbose_data = 6;

	printAllAllocations = verbose_data;
	// Argument list:
	// double * od
	// int startTime
	// int nJobs
	// int nNurses
	// int * solMatrix
	// int * jobTimeWindow
	// int * jobDuration
	// int * nurseSkilled

	// Real parameter types:
	// int **od;
	// int ** solMatrix;
	// int ** jobTimeWindow;
	// int **nurseSkilled;
	// int startTime;
	// int nJobs;
	// int nNurses;
	// int * jobDuration;

	int ct = -1;
	int nRows, nCols;
	int nJobs = nJobs_data;
	int nNurses = nNurses_data;
	int nSkills = nSkills_data;
	if (verbose_data > 5){
        printf("Starting allocation of memory\n");
    }
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
		for (int j = 0; j < nCols; ++j) {
			ct++;
			od[i][j] = (double)od_data[ct];
		}

	if (verbose_data > 5){
        printf("Allocated od.\n");
    }


	nRows = nNurses;
	nCols = nJobs;
	ct = -1;
	double** nurse_travel_from_depot;
	// double ** nurse_travel_from_depot_cost = NULL; // Not used at the moment
	nurse_travel_from_depot = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurse_travel_from_depot[i] = malloc(nCols * sizeof(double)); // Cols
		// nurse_travel_from_depot_cost[i] = malloc(nCols * sizeof(int)); // No need to allocate this at the moment;

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j) {
			ct++;
			nurse_travel_from_depot[i][j] = (double)nurse_travel_from_depot_data[ct];
			// printf("\tnurse_travel_from_depot[%d][%d] = %.2f\n", i, j, nurse_travel_from_depot[i][j]);
		}

	if (verbose_data > 5){
        printf("Allocated nurse_travel_from_depot.\n");
    }


	double** nurse_travel_to_depot;
	ct = -1;
	// double ** nurse_travel_to_depot_cost = NULL; // Not used at the moment
	nurse_travel_to_depot = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurse_travel_to_depot[i] = malloc(nCols * sizeof(double)); // Cols
		// nurse_travel_to_depot_cost[i] = malloc(nCols * sizeof(int)); // No need to allocate this at the moment;

	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j) {
			ct++;
			nurse_travel_to_depot[i][j] = (double)nurse_travel_to_depot_data[ct];
			// printf("\tnurse_travel_to_depot[%d][%d] = %.2f\n", i, j, nurse_travel_to_depot[i][j]);
		}

	if (verbose_data > 5){
        printf("Allocated nurse_travel_to_depot.\n");
    }


	// Should include eventually working times here as well

	// nCols = nJobs;
	// int * jobDuration = malloc(nCols * sizeof(int));
	// //start single vector pop
	// for (int i = 0; i < nCols; ++i)
	// 	jobDuration[i] = jobTimeWindow_data[i][2];
	//end populating

	// if (verbose_data > 5)
	// 	printf("Allocated jobDuration.\n");


    // nurseWorkingTimes_data
	int** nurseWorkingTimes;
	nRows = nNurses;
	nCols = 3;
	nurseWorkingTimes = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurseWorkingTimes[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	ct = -1;
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j) {
			ct++;
			nurseWorkingTimes[i][j] = nurseWorkingTimes_data[ct];
		}
	//end populating
	if (verbose_data > 5){
        printf("Allocated nurseWorkingTimes.\n");
    }

	// nurseSkills_data
	int** nurseSkills;
	nRows = nNurses;
	nCols = nSkills;
	nurseSkills = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurseSkills[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	ct = -1;
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j) {
			ct++;
			nurseSkills[i][j] = nurseSkills_data[ct];
		}
	//end populating
	if (verbose_data > 5){
        printf("Allocated nurseSkills.\n");
    }

	// jobRequirements_data
	int** jobRequirements;
	nRows = nJobs;
	nCols = nSkills;
	jobRequirements = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		jobRequirements[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	ct = -1;
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j) {
			ct++;
			jobRequirements[i][j] = jobRequirements_data[ct];
		}
	//end populating
	if (verbose_data > 5){
        printf("Allocated jobRequirements.\n");
    }


	int** jobTimeInfo;
	nRows = nJobs;
	nCols = 3;
	jobTimeInfo = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		jobTimeInfo[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	ct = -1;
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j) {
			ct++;
			jobTimeInfo[i][j] = jobTimeInfo_data[ct];
		}
	//end populating

	if (verbose_data > 5){
        printf("Allocated jobTimeInfo.\n");
    }


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
	if (verbose_data > 5){
        printf("Allocated solMatrix.\n");
    }


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
	if (verbose_data > 5){
        printf("Allocated timeMatrix.\n");
    }


	int* nurseRoute = malloc(nJobs * sizeof(int));

	int** allNurseRoutes;
	nRows = nNurses;
	nCols = nJobs;
	allNurseRoutes = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		allNurseRoutes[i] = malloc(nCols * sizeof(int)); // Cols
	//start populating
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j)
			allNurseRoutes[i][j] = -1;
	//end populating
	if (verbose_data > 5){
        printf("Allocated allNurseRoutes.\n");
    }

	double* nurseWaitingTime = malloc(nNurses * sizeof(double));
	double* nurseTravelTime = malloc(nNurses * sizeof(double));

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
		// printf("DS %d = %d\n", i, doubleService_data[i]);
	}

	// nurseUnavail from nurse_unavail_data (24/05/2021)
    int* nurseUnavail = malloc(nNurses * sizeof(int));
	for (int i = 0; i < nNurses; i++){
        nurseUnavail[i] = nurse_unavail_data[i];
	}


	double* violatedTW = malloc(nJobs * sizeof(double));
	double* violatedTWMK = malloc(nJobs * sizeof(double));
	int** nurseSkilled;
	nRows = nNurses;
	nCols = nJobs;
	nurseSkilled = malloc(nRows * sizeof(int*)); // Rows
	for (int i = 0; i < nRows; i++)
		nurseSkilled[i] = malloc(nCols * sizeof(int)); // Cols
	nurse_skilled_from_skills_and_requirements(nurseSkills, jobRequirements, nurseSkilled, doubleService, nJobs, nNurses, nSkills);
	if (verbose_data > 5){
        printf("Allocated nurseSkilled.\n");
    }



	double** prefScore; // Job x Nurse
	nRows = nJobs;
	nCols = nNurses;
	prefScore = malloc(nRows * sizeof(double*)); // Rows
	for (int i = 0; i < nRows; i++)
		prefScore[i] = malloc(nCols * sizeof(double)); // Cols
	//start populating
	ct = -1;
	for (int i = 0; i < nRows; ++i)
		for (int j = 0; j < nCols; ++j) {
			ct++;
			prefScore[i][j] = prefScore_data[ct];
		}
	//end populating
	if (verbose_data > 5)
		printf("Allocated prefScore.\n");

	int nOptions = 100;
	double* algorithmOptions = malloc(nOptions * sizeof(double));
	for (int i = 0; i < nOptions; ++i) {
		algorithmOptions[i] = algorithmOptions_data[i];
	}
	if (verbose_data > 5)
		printf("Allocated algorithmOptions.\n");

	// capabilityOfDoubleServices_data
	int*** capabilityOfDoubleServices;
	nRows = nNurses;
	nCols = nNurses;
	// dim3 is nDoubleServices
	capabilityOfDoubleServices = malloc(nRows * sizeof(int**)); // Rows
	for (int i = 0; i < nRows; i++) {
		capabilityOfDoubleServices[i] = malloc(nCols * sizeof(int*)); // Cols
		for (int j = 0; j < nCols; j++)
			capabilityOfDoubleServices[i][j] = malloc(nDoubleServices * sizeof(int)); // Cols
	}
	//start populating
	// printf("Size should be %d x %d x %d\n", nRows, nCols, nDoubleServices);
	ct = -1;
	for (int i = 0; i < nRows; ++i) {
		for (int j = 0; j < nCols; ++j) {
			for (int k = 0; k < nDoubleServices; ++k) {
				ct++;
				// printf("E %d (%d, %d, %d), ", ct, i, j, k);
				// printf("value %d", capabilityOfDoubleServices_data[ct]);
				capabilityOfDoubleServices[i][j][k] = capabilityOfDoubleServices_data[ct];
				// printf(", allocated to %d\t", capabilityOfDoubleServices[i][j][k]);
			}

		}
		// printf("\n");
	}

    // unavailMatrix from unavail_matrix_data (24/05/2021)
    int*** unavailMatrix;
    nRows = 10; /** NEED TO CHANGE THIS **/
    nCols = 4;
    // dim3 is nDoubleServices
    unavailMatrix = malloc(nRows * sizeof(int**)); // Rows
    for (int i = 0; i < nRows; i++) {
        unavailMatrix[i] = malloc(nCols * sizeof(int*)); // Cols
        for (int j = 0; j < nCols; j++)
            unavailMatrix[i][j] = malloc(nNurses * sizeof(int));
    }

    // printf("Size should be %d x %d x %d\n", nRows, nCols, nNurses);
    ct = -1;
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            for (int k = 0; k < nNurses; ++k) {
                ct++;
                unavailMatrix[i][j][k] = unavail_matrix_data[ct];
            }

        }
    }

	if (verbose_data > 5) {
		printf("Allocated capabilityOfDoubleServices.\n");
		printf("Data allocated, leaving instance_from_python()...\n");
	}

	verbose_data = printAllAllocations;	// Return this to what it was
	int* nurseOrder = malloc(nNurses * sizeof(int));
	for (int i = 0; i < nNurses; ++i){
        nurseOrder[i] = i;
    }

	struct INSTANCE inst = { .nJobs = nJobs,
	        .nNurses = nNurses,
			.nSkills = nSkills,
            .verbose = verbose_data,
            .quality_measure = quality_measure,
            .MAX_TIME_SECONDS = MAX_TIME_SECONDS,
            .tw_interval = tw_interval_data,
            .exclude_nurse_travel = exclude_nurse_travel_data,
            .od = od,
            .nurse_travel_from_depot = nurse_travel_from_depot,
            .nurse_travel_to_depot = nurse_travel_to_depot,
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
            .MK_mind = mk_mind,
            .MK_maxd = mk_maxd,
            .capabilityOfDoubleServices = capabilityOfDoubleServices,
            .prefScore = prefScore,
            .algorithmOptions = algorithmOptions
	};

	return inst;

} // END OF instance_from_python

void free_instance_copy(struct INSTANCE* ip) {
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
	// ip->nurseRoute = NULL;
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

void free_instance_memory(struct INSTANCE* ip) {
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
		free(ip->nurse_travel_from_depot[i]);
	free(ip->nurse_travel_from_depot);
	if (ip->verbose > 10)
		printf("Freed nurse_travel_from_depot.\n");

	for (int i = 0; i < (ip->nNurses); i++)
		free(ip->nurse_travel_to_depot[i]);
	free(ip->nurse_travel_to_depot);
	if (ip->verbose > 10)
		printf("Freed nurse_travel_to_depot.\n");

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

void nurse_skilled_from_skills_and_requirements(int** nurseSkills, int** jobRequirements, int** nurseSkilled, int* doubleService, int nJobs, int nNurses, int nSkills) {
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