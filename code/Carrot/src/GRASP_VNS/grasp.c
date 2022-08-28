/*--------------/
GRASP_VNS
grasp.c
UoS
10/07/2021
/--------------*/


#include "float.h"
#include "math.h"
#include "constructive.h"
#include "grasp.h"

int GRASP(struct INSTANCE* ip) {

	clock_t start = clock();
	clock_t end = clock();
	double eps = 1e-6;
	double elapsedTime = 0.0;
	double average_gen_quality = 0.0;
	long double sum_of_qualities = 0.0;
	int given_quality = ip->qualityMeasure;
	int solutions_in_pool = (int)(ip->algorithmOptions[8] + eps);
	int rcl_strategy = (int)(ip->algorithmOptions[10] + eps);

	double* pool_quality = malloc(solutions_in_pool * sizeof(double));
	struct INSTANCE** pool = (struct INSTANCE**)malloc(solutions_in_pool * sizeof(struct INSTANCE*));
	for (int i = 0; i < solutions_in_pool; i++) {
		pool[i] = NULL;
		pool_quality[i] = -DBL_MAX;
	}

	int PR_STRATEGY = (int)(ip->algorithmOptions[9] + eps);
	int PR_DIRECTION_PARAMETER = (int)(ip->algorithmOptions[11] + eps);

	if (ip->verbose > 5) {
		printf("Started GRASP procedure...\n");
	}

	int doLS = 1;
	int doLSAfter = 0;
	int randomGraspParam = 0;
	double GRASP_param = 0.0;
	double delta_low = ip->algorithmOptions[4];
	double delta_range = ip->algorithmOptions[5];
	double currentQuality = -1; 
	double bestQuality = -1 * bigM;
	unsigned int LS_ITERS = bigM;
	unsigned int MAX_ITERATIONS = bigM;
	int LS_ITERS_AFTER = MAX_ITERATIONS; // = bigM
	float LS_TIME = ip->MAX_TIME_SECONDS;
	float LS_TIME_AFTER = ip->MAX_TIME_SECONDS;
	struct INSTANCE bestSol, emptySol;
	bestSol = (*ip); 

	int* bestNurseOrder = malloc(ip->nNurses * sizeof(int));
	for (int i = 0; i < ip->nNurses; ++i) {
		ip->nurseOrder[i] = i;
		bestNurseOrder[i] = i;
	}

	printf("\nSTARTING Nurse order: (");
	for (int i = 0; i < ip->nNurses; ++i) {
		printf("%d,", ip->nurseOrder[i]);
	}
	printf(")\n");

	int** bestSolMatrix = malloc(ip->nNurses * sizeof(int*));
	for (int i = 0; i < ip->nNurses; i++) {
		bestSolMatrix[i] = malloc(ip->nJobs * sizeof(int));
		for (int j = 0; j < ip->nJobs; ++j) {
			bestSolMatrix[i][j] = ip->solMatrix[i][j];
		}
	}

	clock_t startIT = clock();
	clock_t endIT = clock();
	int iter = 0;
	int exch1 = 0;
	int exch2 = 0;
	int no_imp_iter = 0; 
	int max_no_imp_iter = 25;
	int sols_generated = 0;
	int solutions_relinked = 0;
	int local_searches_performed = 0;
	int path_relinkings_performed = 0;
	float elapsedTimeIT = 0;
	double bestCR = -1 * bigM; 
	double before_exch = -1 * bigM;
	double average_constructive_sol = 0.0; 
	int changeNurseOrder = (int)(ip->algorithmOptions[6] + eps);
	emptySol = (*ip); 

	while (iter < MAX_ITERATIONS) {
		startIT = clock();
		currentQuality = SolutionQuality(ip, -30);

		struct INSTANCE newEmpty = emptySol;
		*ip = newEmpty;
        CleanSolutionFromStruct(ip);


		if (iter == 0) {
			before_exch = currentQuality;
		}
		else if (changeNurseOrder > 0) {
			if (currentQuality > before_exch) {
				before_exch = currentQuality;
                RandomTwoExchange(ip->nurseOrder, ip->nNurses, &exch1, &exch2);
				no_imp_iter = 0;
			}
			else {
				no_imp_iter++;
                TwoExchange(ip->nurseOrder, exch1, exch2);
			}
            Shuffle(ip->nurseOrder, (size_t) ip->nNurses);
		}

		double rnd_dbl = (float)rand() / (float)RAND_MAX;
		GRASP_param = delta_low + rnd_dbl * delta_range;


        RandomisedConstructive(ip, 1, GRASP_param, rcl_strategy);
		sols_generated++;

		endIT = clock();
		elapsedTimeIT = (float)(endIT - startIT) / CLOCKS_PER_SEC;

		double constrCurrentQuality = SolutionQuality(ip, -10000);
		if (sols_generated > 1) {
			average_constructive_sol = (average_constructive_sol * (sols_generated - 1) + constrCurrentQuality) / sols_generated;
		}
		else {
			average_constructive_sol = constrCurrentQuality;
		}

		if (constrCurrentQuality > bestCR) {
			bestCR = constrCurrentQuality;
		}

		int constructive_passes_threshold = 99;
		double constructive_filter_param = 1;
		double constr_tr_value = average_constructive_sol + (constructive_filter_param - 1) * abs(average_constructive_sol);
		float rnum = ((float)rand() / (float)RAND_MAX > 0.5);
		if (constrCurrentQuality > constr_tr_value) {
			if (rnum > 0.1) {
				constructive_passes_threshold = 10;
			}
		}
		else if (rnum > 0.8) {
			constructive_passes_threshold = 10;
		}

		end = clock();
		elapsedTime = ((double)(end - start)) / CLOCKS_PER_SEC;

		double ls_quality_test = -1;
		LS_TIME = ip->MAX_TIME_SECONDS - elapsedTime;
		if (LS_ITERS > 0 && doLS > 0 && constructive_passes_threshold > 1 && LS_TIME > 0) {
			local_searches_performed++;
			if (ip->verbose > 3) {
				printf("Running standard LS, passed threshold with quality: %.2f (average CR: %.2f, threshold value: %.2f )\n", constrCurrentQuality, average_constructive_sol, constr_tr_value);
				printf("PASS = %d\n", constructive_passes_threshold);
			}
			ls_quality_test = StandardLocalSearchTest(ip, LS_ITERS, LS_TIME, 100);
		}
		else {
			printf("SKIP LS, quality: %.2f (average CR: %.2f, threshold value: %.2f )\n", constrCurrentQuality, average_constructive_sol, constr_tr_value);
		}


		double ca_time = elapsedTimeIT;
		endIT = clock();
		elapsedTimeIT = (float)(endIT - startIT) / CLOCKS_PER_SEC;

		currentQuality = SolutionQuality(ip, -11);
		double ls_total_q = currentQuality;

		double TOL = 1e-6;
		
		int PR_DIRECTION = 0;
		if (PR_DIRECTION_PARAMETER == 1) {
			PR_DIRECTION = 1;
		}
		else if (PR_DIRECTION_PARAMETER == 2) {
			PR_DIRECTION = 2;
		}
		else if (PR_DIRECTION_PARAMETER == 3) {
			PR_DIRECTION = PickInteger(3);
		}

		if (PR_STRATEGY == 1) {
			solutions_relinked++;
			printf("Start PR round for sol with quality %.2f\n", SolutionQuality(ip, -123));
			for (int trial = 0; trial < solutions_in_pool * 2; trial++){
				int ps = PickInteger(solutions_in_pool);
				if (NULL != pool[ps]) {
					printf("\tAbout to PR incumbent with POOL-%d <-> ", ps);
					path_relinkings_performed++;
					struct INSTANCE relinkedSolution = CopyInstance(ip);
					double rel_quality = -1;
					rel_quality = DirectedPR(ip, pool[ps], currentQuality, pool_quality[ps], &relinkedSolution, PR_DIRECTION);
					if ((rel_quality > currentQuality + TOL) && (fabs(rel_quality - pool_quality[ps]) > TOL)) {
                        OverwriteInstance(ip, &relinkedSolution);
						printf("\tDone. Achieved quality: %.2f\n", SolutionQuality(ip, -123));
						currentQuality = rel_quality;
					}
					else {
						printf("\tDone. PR didn't work, achieved quality: %.2f\n", SolutionQuality(&relinkedSolution, -123));
					}
                    FreeInstanceCopy(&relinkedSolution);
					break;
				}
			}
		}

		else if (PR_STRATEGY == 2) {
			double pr_pi = 1;
			double pr_threshold = -bigM;
			if (currentQuality > pr_threshold) {
				solutions_relinked++;
				struct INSTANCE relinkedSolution = CopyInstance(ip);
				struct INSTANCE bestRelinkedSolution = CopyInstance(ip);
				double best_quality_of_round = currentQuality;
				for (int ps = 0; ps < solutions_in_pool; ps++) {
					if (NULL != pool[ps]) {
						double rel_quality = DirectedPR(ip, pool[ps], currentQuality, pool_quality[ps], &relinkedSolution, PR_DIRECTION);
						if ((rel_quality > best_quality_of_round + TOL) && (fabs(rel_quality - pool_quality[ps]) > TOL)) {
                            OverwriteInstance(&bestRelinkedSolution, &relinkedSolution);
							best_quality_of_round = rel_quality;
						}
					}
				}
				if (best_quality_of_round - TOL > currentQuality) {
                    OverwriteInstance(ip, &bestRelinkedSolution);
				}
				currentQuality = SolutionQuality(ip, -11);

				if (fabs(currentQuality - best_quality_of_round) > TOL) {
					printf("ERROR: PR option 2 did not work as expected!\n We thought we'd get: %.2f but we got %.2f\n", best_quality_of_round, currentQuality);
					exit(-34339);
				}

                FreeInstanceCopy(&relinkedSolution);
                FreeInstanceCopy(&bestRelinkedSolution);
			}
		}

		if (constructive_passes_threshold > 1) {
			if (ip->verbose > 10) {
                PrintRecalculatePoolContents(pool, pool_quality, solutions_in_pool);
                CalculatePrintDissimilarityMatrix(pool, solutions_in_pool);
			}
		}


		int insertHere = -1;
		int dissimilarity_threshold = 5;
		double lowest_dissimilarity = bigM;
		for (int ps = 0; ps < solutions_in_pool; ps++) {
			if (NULL == pool[ps]) {
				pool[ps] = malloc(sizeof(struct INSTANCE));
				insertHere = ps;
				break;
			}
			double dissimilarity_with_ps = SolutionDissimilarity(ip, pool[ps]);

			if (pool_quality[ps] > currentQuality) {
				if (dissimilarity_with_ps < dissimilarity_threshold) {
					insertHere = -1;
					break;
				}
				else {
					continue;
				}
			}


			if (dissimilarity_with_ps > lowest_dissimilarity) {
				continue;
			}
			else {
				lowest_dissimilarity = dissimilarity_with_ps;
				insertHere = ps;
			}
		}


		if (insertHere >= 0) {
			(*pool[insertHere]) = CopyInstance(ip);
			pool_quality[insertHere] = currentQuality; 
		}
		end = clock();
		elapsedTime = ((double)(end - start)) / CLOCKS_PER_SEC;

		if (currentQuality > bestQuality) {
			if (ip->verbose > 2) {
				printf("It %d ", iter);
				printf("CA+LS %.2f ", currentQuality);
				printf("CA %.2f ", constrCurrentQuality);
				printf("tott %.2f ", ip->objTime);
				printf("tard %.2f ", ip->objTardiness);
				printf("ovti %.2f ", ip->objOvertime);
				printf("pref %.2f ", ip->totalPref);
				printf("l_day %.2f ", ip->objLongestDay);
				// printf("\n");
				printf("GRASP_P %.4f ", GRASP_param);
				printf("et %.1fs (max %.1f)", elapsedTime, ip->MAX_TIME_SECONDS);
				printf("\n > Nurse order: (");
				for (int i = 0; i < ip->nNurses; ++i) {
					printf("%d,", ip->nurseOrder[i]);
				}
				printf(")\n");

				printf("Added a copy of the incumbent solution (q = %.2f) to the pool in position %d.\n", currentQuality, insertHere);
			}
			else if (ip->verbose >= 0) {
				printf("It %6d\t", iter);
				printf("Q %6.2f\tt %6.1f s\n", currentQuality, elapsedTime);
				printf(" > Nurse order: (");
				for (int i = 0; i < ip->nNurses; ++i) {
					printf("%d,", ip->nurseOrder[i]);
				}
				printf(")\n");
                SolutionQuality(ip, -1);
			}

			bestSol = CopyInstance(ip);
			bestQuality = currentQuality;
		}
		else if (ip->verbose > 5) {
			printf("It: %d\t", iter);
			printf("CA+LS: %.2f", currentQuality);
			printf(" (best: %.2f) ", bestQuality);
			printf("CA: %.2f\t", constrCurrentQuality);
			printf("\n > Nurse order: (");
			for (int i = 0; i < ip->nNurses; ++i) {
				printf("%d,", ip->nurseOrder[i]);
			}
			printf(")\n");
			printf("Elapsed time: %.2f s (MAX: %.2f)", elapsedTime, ip->MAX_TIME_SECONDS);
			printf("\n");
		}

		if (elapsedTime > ip->MAX_TIME_SECONDS) {
			if (ip->verbose > 0) {
				printf("\nTime finished! (%.2f seconds)\n", elapsedTime);
			}
			break;
		}
		iter++;

	}

	if (ip->verbose > 0) {
		printf("GRASP: Performed %d iterations.\n", iter);
	}

	double thisSol = SolutionQuality(ip, -3423);
	double cquality = 0.0;
	int pickthis = -1;
	if (ip->verbose > 5) {
		printf("Pool contents:\n");
	}

	for (int ps2 = 0; ps2 < solutions_in_pool; ps2++) {
		if (pool[ps2] != NULL) {
			cquality = SolutionQuality(pool[ps2], -3234);
			if (ip->verbose > 5) {
				printf("\tPOS %d - Q = %.2f\n", ps2, cquality);
			}
		}
		if (thisSol < cquality) {
			if (ip->verbose > 5) {
				printf("Pick solution %d with quality %.2f\n", ps2, cquality);
			}
			pickthis = ps2;
			thisSol = cquality;
		}
	}
	if (pickthis > -1) {
		bestSol = (*pool[pickthis]);
		*ip = *pool[pickthis];
		pool[pickthis] = NULL;
		if (ip->verbose > 5) {
			printf("Sol quality after copying: %.2f\n", SolutionQuality(ip, -235223));
		}
	}

	if (ip->verbose > 5) {
		printf("Saved best solution\n");
	}

	double quality = SolutionQuality(ip, -121212);

	
	int relinking_all_pairs = 1;

	if (relinking_all_pairs > 0) {
		double TOL = 1e-6;
		printf("Relinking all pairs of solutions in the pool.\n");
		struct INSTANCE output = CopyInstance(ip);
		struct INSTANCE bestRelinkedSolution = CopyInstance(ip);
		double best_final_quality = quality;
		for (int ps = 0; ps < solutions_in_pool; ps++) {
			for (int ps2 = 0; ps2 < solutions_in_pool; ps2++) {
				if ((ps2 != ps) && (NULL != pool[ps]) && (NULL != pool[ps2])) {
                    OverwriteInstance(&output, pool[ps]);
                    PathRelinking(&output, pool[ps2]);
					double relinking_quality = SolutionQuality(&output, -123);
					if ((relinking_quality > best_final_quality + TOL)) {
                        OverwriteInstance(&bestRelinkedSolution, &output);
						printf("\tPair %d - %d: *** achieved quality: %.2f ***\n", ps, ps2, SolutionQuality(&bestRelinkedSolution, -123));
						printf("\tbest_final_quality: %.2f, relinking_quality: %.2f\n", best_final_quality, relinking_quality);
						best_final_quality = relinking_quality;
					}
				}
			}
		}

		if (quality < best_final_quality - TOL) {
		    printf("quality: %.2f, best_final_quality: %.2f\n", quality, best_final_quality);
		    printf("bestRelinkedSolution quality: %.2f\n", SolutionQuality(&bestRelinkedSolution, -123));
            OverwriteInstance(ip, &bestRelinkedSolution);
			double new_quality = SolutionQuality(ip, -123);
			printf("new_quality: %.2f \n", new_quality);
		}

		int solcallnum = ip->verbose - 1;
		solcallnum = 12345;
		quality = SolutionQuality(ip, solcallnum);


		if (fabs(quality - best_final_quality) > TOL) {
			printf("ERROR: PR all pairs did not work as expected!\n We thought we'd get: (bfq) %.2f but we got (q) %.2f\n", best_final_quality, quality);
			printf("Sol quality type: %d\n", ip->qualityMeasure);
			printf("Exiting program.");
			exit(-34339);
		}
		printf("\tPR all pairs final quality: %.2f\n", quality);

		//Deallocate memory:
        FreeInstanceCopy(&output);
        FreeInstanceCopy(&bestRelinkedSolution);
	}

	if (ip->verbose > 3) {
		printf("Pool contents (cleaning):\n");
	}

	for (int ps2 = 0; ps2 < solutions_in_pool; ps2++) {
		if (pool[ps2] != NULL) {
			if (ip->verbose > 3) {
				printf("\tPOS %d - Q = %.2f\n", ps2, SolutionQuality(pool[ps2], -3234));
			}
            FreeInstanceCopy(pool[ps2]);
			pool[ps2] = NULL;
		}
	}
	free(pool);

	printf("Iterations performed: %d\n", iter);

	if (ip->verbose > 0) {
		printf("GRASP Finished.\n");
		printf("Final quality: %.2f\n", quality);
		printf("Theoretical best quality: %.2f\n", bestQuality);
		printf("Iterations performed: %d\n", iter);
		printf("sols_generated = %d\n", sols_generated);
		printf("local_searches_performed = %d\n", local_searches_performed);
		printf("solutions_relinked = %d\n", solutions_relinked);
		printf("total path_relinkings_performed = %d\n", path_relinkings_performed);
		end = clock();
		elapsedTime = ((double)(end - start)) / CLOCKS_PER_SEC;
		printf("Elapsed time: %.2f s (MAX: %.2f)\n", elapsedTime, ip->MAX_TIME_SECONDS);
	}


	for (int i = 0; i < ip->nNurses; ++i) {
		free(bestSolMatrix[i]);
	}
	free(bestSolMatrix);
	free(bestNurseOrder);

	return 0;

}

void RandomisedConstructive(struct INSTANCE* ip, int randomness, double delta, int rclStrategy) {


	int DEBUG_PRINT = -1;
	if (DEBUG_PRINT > 0) {
		printf("\nSTARTING RANDOMISED CONSTRUCTIVE --------------------------\n");
		printf("Start solmatrix:\n");
        PrintSolMatrix(ip);
		printf("Constructive allocations...\n");
	}

	int* nurseSeed = malloc(ip->nNurses * sizeof(int));
	for (int j = 0; j < ip->nNurses; ++j) {
		nurseSeed[j] = -1;
	}

	double* allocatedJobs = malloc(ip->nJobs * sizeof(double));
	for (int i = 0; i < ip->nJobs; ++i) {
		allocatedJobs[i] = 0;
	}

	int b_indices_size = ip->nNurses * ip->nJobs;
	int** bestIndices = malloc(b_indices_size * sizeof(int*));
	for (int i = 0; i < b_indices_size; ++i) {
		bestIndices[i] = malloc(2 * sizeof(int));
	}

	double* rankValues = malloc(ip->nJobs * sizeof(double));
	int* RCL_seeds = malloc(ip->nJobs * sizeof(int));

	if (DEBUG_PRINT > 0) {
		printf("Constructive allocations.Done.\n");
	}


	int rcl_size = -1;
	int check_count = -1;
	double bestRank = -DBL_MAX;
	double worstRank = DBL_MAX;

	for (int nurseIdx = 0; nurseIdx < ip->nNurses; ++nurseIdx) {
		bestRank = -DBL_MAX;
		worstRank = DBL_MAX;
		int alreadyAllocated = 0;
		int unskilledFor = 0;
		int dismissedJobs = 0;
		int consideredJobs = 0;
		int nurse = ip->nurseOrder[nurseIdx];

		for (int job = 0; job < ip->nJobs; ++job) {
			check_count++;

			if (allocatedJobs[job] > 0.6) {
				alreadyAllocated++;
				rankValues[job] = -2 * bigM; 
				continue;
			}
			if (CheckSkills(ip, job, nurse) < 1) {
				unskilledFor++;
				rankValues[job] = -bigM;
				continue;
			}
			if (DEBUG_PRINT > 0) {
				printf("\n---Insert in nurse %d the job %d\n", nurse, job);
			}
			if (BestJobInsertion(ip, job, nurse) < 0){
				dismissedJobs++;
				rankValues[job] = -bigM;
				continue;
			}
			else {
				consideredJobs++;
			}
			if (DEBUG_PRINT > 0) {
				printf("Done.\nRanking job %d...\n\tValue: ", job);
			}

			int sol_q_num = -1000;


			rankValues[job] = SolutionQuality(ip, sol_q_num);

			if (rankValues[job] > bestRank) {
				bestRank = rankValues[job];
			}
			if (rankValues[job] < worstRank) {
				worstRank = rankValues[job];
			}
			if (DEBUG_PRINT > 0) {
				printf("%.2f\nDone.\nRemoving job %d\n", rankValues[job], job);
			}
            RemoveJob(ip, job, nurse);

			bestIndices[job][0] = nurse;
			bestIndices[job][1] = job;

		}

		if (consideredJobs < 1) {
			continue;
		}

        GenerateRCL(delta, RCL_seeds, &rcl_size, rankValues, ip->nJobs, bestRank, worstRank, rclStrategy);

		int cNurse = -1;
		int cJob = -1;

        RCLPick(bestIndices, RCL_seeds, rcl_size, &cNurse, &cJob);

		if (DEBUG_PRINT > 0) {
			printf("Decided on (nurse %d, job %d)\n", cNurse, cJob);
			printf("Solmatrix:\n");
            PrintSolMatrix(ip);
			printf("Nurse routes:\n");
            PrintIntMatrix(ip->allNurseRoutes, ip->nNurses, ip->nJobs);
			if (cJob < 0) {
				printf("ERROR: The best pick was a -1!!!!\n");
				exit(-1);
			}
		}

		if (BestJobInsertion(ip, cJob, cNurse) > -1) {
			if (ip->doubleService[cJob]) {
				allocatedJobs[cJob] += 0.5;
			}
			else {
				allocatedJobs[cJob] = 1;
			}
			nurseSeed[nurse] = cJob;
		}
		else {
			printf("\nERROR! Solmatrix:\n");
            PrintSolMatrix(ip);
			printf("\nERROR: FAILED to assign job %d as seed for nurse %d\n", cJob, cNurse);
			if (ip->doubleService[cJob]) {
				printf("(This job is a DS)\n");
			}
			exit(-1);
		}
	}

	free(RCL_seeds); 
	free(rankValues); 

	if (DEBUG_PRINT > 0) {
		printf("Finished with seeds\n");
	}

	int jobsRemaining = 0;
	for (int i = 0; i < ip->nJobs; ++i) {
		if (allocatedJobs[i] < 0.9) {
			jobsRemaining++;
		}
	}


	int ct;
	int ittt = 0;
	int assignmentIterations = 0;
	int MAX_ASSIGNMENT_ITS = jobsRemaining * 500;
	int* RCL = malloc(ip->nJobs * ip->nNurses * sizeof(int));
	double* rankAssignments = malloc(ip->nJobs * ip->nNurses * sizeof(double));

	while (jobsRemaining > 0) {
		ittt++;
		assignmentIterations++;


		bestRank = -DBL_MAX;
		worstRank = DBL_MAX;
		for (int i = 0; i < b_indices_size; ++i) {
			bestIndices[i][0] = -1;
			bestIndices[i][1] = -1;
		}

		double base_quality = SolutionQuality(ip, -3423462);
		ct = -1;
		int potentialAllocations = 0;
		for (int nurseidx = 0; nurseidx < ip->nNurses; ++nurseidx) {
			int nurse = ip->nurseOrder[nurseidx];
			for (int job = 0; job < ip->nJobs; ++job) {
				ct++;

				if (allocatedJobs[job] > 0.6 || ip->solMatrix[nurse][job] > -0.1) {
					rankAssignments[ct] = -DBL_MAX;
					continue;
				}

				int skillCheck = 0;
				if (!ip->doubleService[job]) {
					skillCheck = CheckSkills(ip, job, nurse);
				}
				else {
					if (allocatedJobs[job] < 0.4) {
						skillCheck = CheckSkillsDSFirst(ip, job, nurse);
					}
					else {
						for (int nurseb = 0; nurseb < ip->nNurses; nurseb++) {
							if (ip->solMatrix[nurseb][job] > -0.5) {
								skillCheck = CheckSkillsDS(ip, job, nurse, nurseb);
								break;
							}
						}
					}
				}

				if (allocatedJobs[job] > 0.6 || skillCheck < 1 || ip->solMatrix[nurse][job] > -0.1) {
					rankAssignments[ct] = -DBL_MAX;
					continue;
				}
				else { 
					potentialAllocations++;
				}

				if (BestJobInsertion(ip, job, nurse) < 0){
					printf("WARNING: Constructive could not insert job %d in nurse %d, but previous checks deemed it possible!!!\n", job, nurse);
					continue;
				}

				rankAssignments[ct] = SolutionQuality(ip, -1000);

				int difficult_job_priority = 0;
				if ((float)rand() / (float)RAND_MAX > 0.5)
					difficult_job_priority = 1;

				if (difficult_job_priority > 0) {
					rankAssignments[ct] = rankAssignments[ct] - base_quality;
					double chunk = 2 * bigM;
					if (ip->isFeasible < 1)
						rankAssignments[ct] -= bigM;
					if (ip->doubleService[job])
						rankAssignments[ct] = rankAssignments[ct] + chunk;

					double job_tw_duration = ip->jobTimeInfo[job][1] - ip->jobTimeInfo[job][0];
					double nurse_zero_journey_duration = ip->nurseWorkingTimes[0][1] - ip->nurseWorkingTimes[0][0];
					double job_tw_to_nurse_zero_ratio = job_tw_duration / nurse_zero_journey_duration;
					if (job_tw_to_nurse_zero_ratio < 0.5) {
						rankAssignments[ct] = rankAssignments[ct] + chunk * (0.7 - job_tw_to_nurse_zero_ratio);
					}
				}

				if (rankAssignments[ct] > bestRank) {
					bestRank = rankAssignments[ct];
				}
				if (rankAssignments[ct] < worstRank) {
					worstRank = rankAssignments[ct];
				}

                RemoveJob(ip, job, nurse);

				bestIndices[ct][0] = nurse;
				bestIndices[ct][1] = job;

			}

		}

		if (potentialAllocations < jobsRemaining) {
			printf("ERROR! Not enough allocations.\n");
		}

        GenerateRCL(delta, RCL, &rcl_size, rankAssignments, ip->nJobs*ip->nNurses, bestRank, worstRank, rclStrategy);
	
		int best_nurse = -1;
		int best_job = -1;

        RCLPick(bestIndices, RCL, rcl_size, &best_nurse, &best_job);

		if ((best_nurse < 0 || best_job < 0) || assignmentIterations > MAX_ASSIGNMENT_ITS) {
			if (assignmentIterations > MAX_ASSIGNMENT_ITS) {
				printf("\n\n ERROR: Too many iterations (%d)!!!!!!\n\n", MAX_ASSIGNMENT_ITS);
				printf("Best job and nurse were: nurse: %d, job: %d\n", best_nurse, best_job);
				printf("GRASP parameter delta was %.4f\n", delta);
			}
			printf("The RCL was generated with %d candidates (Delta %.2f, best = %.2f, worst = %.2f)\n---\n", rcl_size, delta, bestRank, worstRank);
			printf("ERROR: GRASP did not find ANY valid assignment, with still %d jobs remaining.\n", jobsRemaining);
			printf("Restricted Candidate List (elements %d):\n", rcl_size);
			for (int rcl_el = 0; rcl_el < rcl_size; rcl_el++) {
				printf("[EL %d\tV %.5e]\n", RCL[rcl_el], rankAssignments[RCL[rcl_el]]);
			}

			printf("JOBS:\n");
			for (int jjj = 0; jjj < ip->nJobs; ++jjj) {
				if (allocatedJobs[jjj] < 1) {
					printf("Job %d - Skills: [", jjj);
					for (int sss = 0; sss < ip->nSkills; ++sss) {
						printf("%d, ", ip->jobRequirements[jjj][sss]);
					}
					printf("]\n");
				}
			}
			printf("STAFF:\n");
			for (int nnn = 0; nnn < ip->nNurses; ++nnn) {
				printf("Nurse %d - Skills: [", nnn);
				for (int sss = 0; sss < ip->nSkills; ++sss) {
					printf("%d, ", ip->nurseSkills[nnn][sss]);
				}
				printf("]\n");
			}
			printf("\n");
			exit(-1);
		}

		int cJob = best_job;
		int cNurse = best_nurse;
		if (BestJobInsertion(ip, cJob, cNurse) > -1){
			if (ip->doubleService[cJob]) {
				allocatedJobs[cJob] += 0.5;
				if (allocatedJobs[cJob] >= 0.6) {
					jobsRemaining--;
				}
			}
			else {
				allocatedJobs[cJob] = 1;
				jobsRemaining--;
			}
		}

	}

	free(rankAssignments);
	free(RCL);
	for (int i = 0; i < ip->nNurses * ip->nJobs; ++i) {
		free(bestIndices[i]);
	}
	free(bestIndices);
	free(nurseSeed);
	free(allocatedJobs);

	if (DEBUG_PRINT > 0) {
		printf("End solmatrix:\n");
        PrintSolMatrix(ip);
		printf("\n-------------- END OF RANDOMISED CONSTRUCTIVE -------------- \n\n");
	}

	return;

}

void GenerateRCL(double delta, int* rclSeeds, int* rclSize, double* rankValues, int rvSize, double bestRank, double worstRank, int strategy){


	double worstAllowed = 0;
	double tolerance = 1e-6;


	double hardCutOff = -bigM + 1;
	if (hardCutOff > bestRank) {
		hardCutOff = -DBL_MAX;
	}

	double really_bad_value = bestRank - bigM;
	if (hardCutOff < really_bad_value) {
		hardCutOff = really_bad_value;
	}

	if (strategy == 1) {
		worstAllowed = bestRank - delta * fabs(bestRank - worstRank);
	}
	else if (strategy == 2) {
		worstAllowed = bestRank - fabs(bestRank) * delta;
	}
	else {
		printf("ERROR: In GenerateRCL() the value of 'strategy' is %d, which is not understood\n", strategy);
		exit(-321);
	}

	if (worstAllowed > bestRank) {
		printf("Warning: Something went wrong, worstAllowed > bestRank");
		printf("Creating list with strategy = %d and delta = %.9f\n>>\tBest: %.2f, Worst: %.2f, Worst allowed: %.2f, hardCutOff: %.2f\n",
			strategy, delta, bestRank, worstRank, worstAllowed, hardCutOff);
		printf("delta*abs(bestRank - worstRank) = %.9f\n", delta * abs(bestRank - worstRank));
		printf("abs(bestRank - worstRank) = %d\n", abs(bestRank - worstRank));
		printf("fabs(bestRank - worstRank) = %.9f\n", fabs(bestRank - worstRank));
		printf("bestRank - worstRank = %.9f\n", bestRank - worstRank);
		printf("abs(bestRank)*delta = %.9f\n", abs(bestRank) * delta);
	}

	if (worstAllowed < hardCutOff) {
		worstAllowed = hardCutOff;
		if (bestRank < -DBL_MAX + 100) {
			printf("ERROR: The best candidate in the RCL is infeasible (!)(!)(!)\n");
			exit(-124);
		}
	}

	if (strategy == 1 || strategy == 2) {
		(*rclSize) = 0;
		for (int i = 0; i < rvSize; i++) {
			if (rankValues[i] > worstAllowed - tolerance) {
				(*rclSize)++;
				rclSeeds[(*rclSize) - 1] = i;
			}
		}
	}

	if ((*rclSize) < 1) {
		printf("WARNING! RCL in GRASP has no candidates!\n");
		printf("Creating list with strategy = %d and delta = %.9f\n>>\tBest: %.2f, Worst: %.2f, Worst allowed: %.2f, hardCutOff: %.2f\n", strategy, delta, bestRank, worstRank, worstAllowed, hardCutOff);
		printf("There were %d to choose from (-DBL_MAX omitted)\n", rvSize);
		for (int iii = 0; iii < rvSize; iii++) {
			if (rankValues[iii] > -DBL_MAX + 100)
				printf("\t%d\t%.2f\n", iii, rankValues[iii]);
		}
		exit(-1);
	}

}

void RCLPick(int** bestIndices, int* rclSeeds, int rclSize, int* cNurse, int* cJob) {

	 int rd_int = PickInteger(rclSize);

	int el = rclSeeds[rd_int];

	(*cNurse) = bestIndices[el][0];
	(*cJob) = bestIndices[el][1];

	return;

}

int PickInteger(int max_int) {
	//This function picks an integer number randomly between 0 and max_int - 1

	return (int)(rand() % (max_int));
}


void CleanSolutionFromStruct(struct INSTANCE* ip) {
	for (int nurse = 0; nurse < ip->nNurses; ++nurse) {
		for (int job = 0; job < ip->nJobs; ++job) {
			ip->solMatrix[nurse][job] = -1;
		}
	}
}

int IdentifyEarlyInsert(struct INSTANCE* ip, struct INSTANCE* guiding, int guidingNurse, int guidingNursePos) {

	int position_insert = guidingNursePos;

	for (int irt = 1; irt < guiding->nJobs - guidingNursePos; irt++) { //For each
		int nxtJob = guiding->allNurseRoutes[guidingNurse][guidingNursePos + irt];
		if (nxtJob < 0) {
			break;
		}

		if (ip->solMatrix[guidingNurse][nxtJob] >= 0) {
			if (ip->solMatrix[guidingNurse][nxtJob] < position_insert) {
				position_insert = ip->solMatrix[guidingNurse][nxtJob];
			}
		}

	}

	if (position_insert == guidingNursePos) {
		int jc = GetJobCount(ip, guidingNurse);
		if (position_insert > jc) {
			position_insert = jc;
		}
	}

	return position_insert;

}


double SolutionDissimilarity(struct INSTANCE* input1, struct INSTANCE* input2) {

	double weight1 = 1;
	double weight2 = 1;
	double weight3 = 1;

	int nurse_order_value = 0;
	int assignment_value = 0;
	int route_value = 0;

	for (int i = 0; i < input1->nNurses; ++i) {
		if (input1->nurseOrder[i] != input2->nurseOrder[i])
			nurse_order_value++;
	}


	for (int i = 0; i < input1->nNurses; ++i) {
		for (int j = 0; j < input1->nJobs; ++j) {
			if (input1->solMatrix[i][j] < 0) {
				continue;
			}
			if (input2->solMatrix[i][j] < 0) {
				assignment_value++;
			}

			int i_des = FindArcDestination(i, j, input1);


			int arc_dissimilar = 1;
			for (int nu = 0; nu < input2->nNurses; ++nu) {
				if (input2->solMatrix[nu][j] > -1) {
					int i_des2 = FindArcDestination(nu, j, input2);
					if (((i_des2 != -1) && (i_des2 == i_des)) || ((i_des == -1) && (i_des2 == -1) && (nu == i))) {
						arc_dissimilar = 0;
						break;
					}
					else {
						if (input1->doubleService[j]) {
							continue;
						}
						else {
							break;
						}
					}
				}
			}

			route_value += arc_dissimilar;

			if ((input1->solMatrix[i][j] == 0) && (input2->solMatrix[i][j] != 0)) {
				route_value++;
			}

		}
	}

	double d_value = weight1 * nurse_order_value + weight2 * assignment_value + weight3 * route_value;

	return(d_value);

}

int FindArcDestination(int sourceNurse, int sourceJob, struct INSTANCE* ip) {

	int c_pos = ip->solMatrix[sourceNurse][sourceJob];
	int n_pos = c_pos+1;
	int d_pos = -1;
	for (int i = 0; i < ip->nJobs; ++i) {
		if (ip->solMatrix[sourceNurse][i] == n_pos) {
			d_pos = i;
			break;
		}
	}

	return(d_pos);

}

void PrintRecalculatePoolContents(struct INSTANCE** pool, double* poolQuality, int solutionsInPool) {
	printf("Pool contents:\n");
	for (int ps2 = 0; ps2 < solutionsInPool; ps2++) {
		if (pool[ps2] != NULL)
			printf("\tPOOL %d - Q = %.2f (rec %.2f)\n", ps2, SolutionQuality(pool[ps2], -3234), poolQuality[ps2]);
	}
}

void CalculatePrintDissimilarityMatrix(struct INSTANCE** pool, int solutionsInPool) {
	printf("Pool dissimilarity matrix:\n\t");
	for (int ps3 = 0; ps3 < solutionsInPool; ps3++)
		printf("POOL%d\t", ps3);

	for (int ps3 = 0; ps3 < solutionsInPool; ps3++) {
		printf("\nPOOL %d", ps3);
		for (int ps4 = 0; ps4 < solutionsInPool; ps4++) {
			if ((pool[ps3] != NULL) && (pool[ps4] != NULL))
				printf("\t%.1f", SolutionDissimilarity(pool[ps3], pool[ps4]));
			else
				printf("\t----");
		}
	}
}

double ForwardPR(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output) {
	return DirectedPR(input1, input2, q1, q2, output, 0);
}

double ForwardBackwardPR(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output1) {

	struct INSTANCE output2 = CopyInstance(input1);
	double rq_forward = ForwardPR(input1, input2, q1, q2, output1);
	double rq_backward = BackwardPR(input1, input2, q1, q2, &output2);
	double best_relinking_quality = rq_forward;
	if (rq_backward > rq_forward) {
        OverwriteInstance(output1, &output2);
		best_relinking_quality = rq_backward;
	}

    FreeInstanceCopy(&output2);

	return best_relinking_quality;
}

double BackwardPR(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output) {
	return DirectedPR(input1, input2, q1, q2, output, 1);
}

double DirectedPR(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output, int direction) {

	double relinking_quality = 0;

	if (direction < 2) {
		int source_sol_1 = 0;
		if ((direction == 0 && q1 > q2) || (direction == 1 && q1 < q2)) {
			source_sol_1 = -1;
		}
		if (source_sol_1 > -1) { /// Should this be >-1? This will never occur, as source_sol_1 is only either 0 or -1, so can never be > 0. Changed from 0 to -1, 26/12/2020.
            OverwriteInstance(output, input1);
			relinking_quality = PathRelinking(output, input2);
		}
		else {
            OverwriteInstance(output, input2);
			relinking_quality = PathRelinking(output, input1);
		}
	}
	else if (direction == 2) {
		relinking_quality = ForwardBackwardPR(input1, input2, q1, q2, output);
	}
	else {
		printf("ERROR: Unknown PR direction %d in DirectedPR(...)\n", direction);
		exit(-23483);
	}

	return relinking_quality;

}

double PathRelinking(struct INSTANCE* ip, struct INSTANCE* guiding) {

	struct INSTANCE bestSolution = CopyInstance(ip);
	struct INSTANCE starting_solution = CopyInstance(ip);
	double starting_quality = SolutionQuality(ip, -1476879);

	int nmoves = guiding->nNurses + guiding->nJobs;
	int* performed = malloc(nmoves * sizeof(int));
	double* all_moves_quality = malloc(nmoves * sizeof(double));

	int pr_failed = 1;
	int pr_rounds = 0;
	int PR_ROUNDS_MAX = 10;
	double tolerance = 1e-6;
	double gqual = -DBL_MAX;
	double finish_sol_q = -DBL_MAX;
	double best_inst_quality = -DBL_MAX;

	while (pr_failed > 0 && pr_rounds < PR_ROUNDS_MAX) {
		pr_failed = -1;
		for (int i = 0; i < nmoves; i++) {
			performed[i] = 0;
			all_moves_quality[i] = -DBL_MAX;
		}
		int iters = 0;
		int moves_performed = 0;
		int real_moves_performed = 0;
		while (moves_performed < nmoves) {
			iters++;
			if (iters > nmoves * 10) {
				printf("ERROR: It seems that we are going into an infinite loop (Relinking)\n");
				printf("nmoves: %d\n", nmoves);
				printf("moves_performed: %d\n", moves_performed);
				printf("iters: %d\n", iters);
				exit(-34235);
			}
			double best_move_quality = -DBL_MAX;
			int best_move_to_perform = -1;
			for (int mv = 0; mv < nmoves; mv++) {
				if (performed[mv]) {
					continue;
				}
				if (mv < guiding->nNurses) {
					if (ip->nurseOrder[mv] == guiding->nurseOrder[mv]) {
						performed[mv] = 1;
						moves_performed++;
						continue;
					}

					int A = ip->nurseOrder[mv];
					int A_pos = mv;
					int B = guiding->nurseOrder[mv];
					int B_pos = -1;
					for (int el = 0; el < guiding->nNurses; el++) {
						if (el == mv) {
							continue;
						}
						if (ip->nurseOrder[el] == B) {
							B_pos = el;
							break;
						}
					}
					ip->nurseOrder[B_pos] = A;
					ip->nurseOrder[A_pos] = B;

					double move_quality = SolutionQuality(ip, -3900090);
					if (move_quality > best_move_quality) {
						best_move_to_perform = mv; 
						best_move_quality = move_quality;
					}
					ip->nurseOrder[B_pos] = B;
					ip->nurseOrder[A_pos] = A;

				}

				else {
					int mvjob = mv - guiding->nNurses;
					if (ip->doubleService[mvjob] > 0) {
						int guiding_nurse1 = -1;
						int guiding_nurse_pos1 = -1;
						int guiding_nurse2 = -1;
						int guiding_nurse_pos2 = -1;
                        NurseAndJobPositionDS(guiding, mvjob, &guiding_nurse1, &guiding_nurse_pos1, &guiding_nurse2, &guiding_nurse_pos2);

						int current_nurse1 = -1; 
						int current_nurse_pos1 = -1;
						int current_nurse2 = -1;
						int current_nurse_pos2 = -1;
                        NurseAndJobPositionDS(ip, mvjob, &current_nurse1, &current_nurse_pos1, &current_nurse2, &current_nurse_pos2);

						if (((guiding_nurse1 == current_nurse1) && (current_nurse_pos1 == guiding_nurse_pos1)) && ((guiding_nurse2 == current_nurse2) && (current_nurse_pos2 == guiding_nurse_pos2))) {
							performed[mv] = 1;
							moves_performed++;
							continue;
						}

                        RemoveJob(ip, mvjob, current_nurse1);
                        RemoveJob(ip, mvjob, current_nurse2);
						int position_insert1 = IdentifyEarlyInsert(ip, guiding, guiding_nurse1, guiding_nurse_pos1);
						int position_insert2 = IdentifyEarlyInsert(ip, guiding, guiding_nurse2, guiding_nurse_pos2);
						if ((InsertJobAtPosition(ip, mvjob, guiding_nurse1, position_insert1) < 0) || (InsertJobAtPosition(ip, mvjob, guiding_nurse2, position_insert2) < 0)) {
							all_moves_quality[mv] = -DBL_MAX;
							printf("ERROR: This shouldn't happen!!!!!! PR cannot match guiding solution. DS\n");
							printf("Trying to get job %d into nurse %d position %d\n", mvjob, guiding_nurse1, position_insert1);
							printf("AND: job %d into nurse %d position %d\n", mvjob, guiding_nurse2, position_insert2);
							printf("This is SOLMATRIX after the trials:\n");
                            PrintSolMatrix(ip);
							printf("Inserting back for debugging...\n");
                            InsertJobAtPosition(ip, mvjob, current_nurse2, current_nurse_pos2);
                            InsertJobAtPosition(ip, mvjob, current_nurse1, current_nurse_pos1);
                            PrintSolMatrix(ip);
							exit(-23535);
						}

						double move_quality = SolutionQuality(ip, -3900090);
						if (move_quality > best_move_quality) {
							best_move_to_perform = mv;
							best_move_quality = move_quality;
						}

                        RemoveJob(ip, mvjob, guiding_nurse1);
                        RemoveJob(ip, mvjob, guiding_nurse2);
						if ((InsertJobAtPosition(ip, mvjob, current_nurse1, current_nurse_pos1) < 0) || (InsertJobAtPosition(ip, mvjob, current_nurse2, current_nurse_pos2) < 0)) {
							printf("ERROR: We have messed it up!\nCannot insert job %d back into nurse %d or %d\n", mvjob, current_nurse1, current_nurse2);
							exit(-32543543);
						}
					}
					else {
						int guiding_nurse = -1;
						int guiding_nurse_pos = -1;
                        NurseAndJobPosition(guiding, mvjob, &guiding_nurse, &guiding_nurse_pos);

						int current_nurse = -1;
						int current_nurse_pos = -1;
                        NurseAndJobPosition(ip, mvjob, &current_nurse, &current_nurse_pos);

						double move_quality = -DBL_MAX;
						if ((guiding_nurse == current_nurse) && (current_nurse_pos == guiding_nurse_pos)) {
							performed[mv] = 1;
							moves_performed++;
							continue;
						}
						else {
                            RemoveJob(ip, mvjob, current_nurse);
							int position_insert = IdentifyEarlyInsert(ip, guiding, guiding_nurse, guiding_nurse_pos);
							if (InsertJobAtPosition(ip, mvjob, guiding_nurse, position_insert) < 0) {
								all_moves_quality[mv] = -DBL_MAX;
                                InsertJobAtPosition(ip, mvjob, current_nurse, current_nurse_pos);
								continue;
							}
							move_quality = SolutionQuality(ip, -3900090);
						}
						if (move_quality > best_move_quality) {
							best_move_to_perform = mv;
							best_move_quality = move_quality;
						}
                        RemoveJob(ip, mvjob, guiding_nurse);
						if (InsertJobAtPosition(ip, mvjob, current_nurse, current_nurse_pos) < 0) {
							printf("ERROR: We have messed it up!\nCannot insert job %d back into nurse %d\n", mvjob, current_nurse);
							exit(-32543543);
						}
					}
				}
			}

			if (best_move_to_perform >= 0) {
				if (best_move_to_perform < guiding->nNurses) {
					int A = ip->nurseOrder[best_move_to_perform];
					int A_pos = best_move_to_perform;
					int B = guiding->nurseOrder[best_move_to_perform];
					int B_pos = -1;
					for (int el = 0; el < guiding->nNurses; el++) {
						if (el == best_move_to_perform) {
							continue;
						}
						if (ip->nurseOrder[el] == B) {
							B_pos = el;
							break;
						}
					}
					ip->nurseOrder[B_pos] = A;
					ip->nurseOrder[A_pos] = B;
				}
				else {
					int mvjob = best_move_to_perform - guiding->nNurses;
					if (ip->doubleService[mvjob] > 0) {
						int guiding_nurse1 = -1;
						int guiding_nurse_pos1 = -1;
						int guiding_nurse2 = -1;
						int guiding_nurse_pos2 = -1;
                        NurseAndJobPositionDS(guiding, mvjob, &guiding_nurse1, &guiding_nurse_pos1, &guiding_nurse2, &guiding_nurse_pos2);

						int current_nurse1 = -1;
						int current_nurse_pos1 = -1;
						int current_nurse2 = -1;
						int current_nurse_pos2 = -1;
                        NurseAndJobPositionDS(ip, mvjob, &current_nurse1, &current_nurse_pos1, &current_nurse2, &current_nurse_pos2);

                        RemoveJob(ip, mvjob, current_nurse1);
                        RemoveJob(ip, mvjob, current_nurse2);

						int position_insert1 = IdentifyEarlyInsert(ip, guiding, guiding_nurse1, guiding_nurse_pos1);
						int position_insert2 = IdentifyEarlyInsert(ip, guiding, guiding_nurse2, guiding_nurse_pos2);

						if ((InsertJobAtPosition(ip, mvjob, guiding_nurse1, position_insert1) < 0) || (InsertJobAtPosition(ip, mvjob, guiding_nurse2, position_insert2) < 0)) {
                            InsertJobAtPosition(ip, mvjob, current_nurse1, current_nurse_pos1);
                            InsertJobAtPosition(ip, mvjob, current_nurse2, current_nurse_pos2);
							printf("ERROR: This shouldn't happen!!!!!! PR cannot match guiding solution. DS\n");
							exit(-23535);
						}
					}
					else {
						int guiding_nurse = -1;
						int guiding_nurse_pos = -1;
                        NurseAndJobPosition(guiding, mvjob, &guiding_nurse, &guiding_nurse_pos);

						int current_nurse = -1;
						int current_nurse_pos = -1;
                        NurseAndJobPosition(ip, mvjob, &current_nurse, &current_nurse_pos);

                        RemoveJob(ip, mvjob, current_nurse);

						int position_insert = IdentifyEarlyInsert(ip, guiding, guiding_nurse, guiding_nurse_pos);
						if (InsertJobAtPosition(ip, mvjob, guiding_nurse, position_insert) < 0) {
							printf("ERROR: We have messed it up!\nCannot perform the move we thought was best! (job %d to nurse %d)\n", mvjob, current_nurse);
							exit(-32003);
						}

					}
				}

				performed[best_move_to_perform] = 1;
				moves_performed++;
				double fq = SolutionQuality(ip, -54911);
				if (fq > best_inst_quality) {
                    OverwriteInstance(&bestSolution, ip);
					best_inst_quality = best_move_quality;
				}

			}
		}

		finish_sol_q = SolutionQuality(ip, -142381);
		gqual = SolutionQuality(guiding, -14923);
		if (abs(finish_sol_q - gqual) > tolerance) {
			pr_failed = 1; 
			pr_rounds++;
			if (pr_rounds < PR_ROUNDS_MAX) {
				continue;
			}
			else {
				printf("Path relinking did not work as expected.\n");
				printf("Start %.2f Target %.2f (Finished %.2f)\n", starting_quality, gqual, finish_sol_q);
				printf("Finished stuff:\nNurse Order:\n");
                PrintIntMatrixOne(ip->nurseOrder, 1, ip->nNurses);
				printf("Sol Matrix:\n");
                PrintSolMatrix(ip);
				printf("Guiding stuff:\nNurse Order:\n");
                PrintIntMatrixOne(guiding->nurseOrder, 1, guiding->nNurses);
				printf("Sol Matrix:\n");
                PrintSolMatrix(guiding);
				exit(-126835);
			}
		}

	}

    OverwriteInstance(ip, &bestSolution);

    StandardLocalSearch(ip, 10000, 60);

	double ach = SolutionQuality(ip, -14357921);

	if ((ach > starting_quality + tolerance) && (ach > gqual + tolerance)) { 

	}
	else {
        OverwriteInstance(ip, &starting_solution);
		ach = starting_quality;
	}

    FreeInstanceCopy(&bestSolution);
    FreeInstanceCopy(&starting_solution);
	free(performed);
	free(all_moves_quality);

	return ach;

}

void NurseAndJobPosition(struct INSTANCE* ip, int job, int* nurse, int* position) {

	for (int nurse_g = 0; nurse_g < ip->nNurses; nurse_g++) {
		if (ip->solMatrix[nurse_g][job] >= 0) {
			(*nurse) = nurse_g;
			(*position) = ip->solMatrix[nurse_g][job];
			break;
		}
	}

}

void NurseAndJobPositionDS(struct INSTANCE* ip, int job, int* nurse1, int* position1, int* nurse2, int* position2) {

	int onFirst = 1;
	for (int nurse_g = 0; nurse_g < ip->nNurses; nurse_g++) {
		if (ip->solMatrix[nurse_g][job] >= 0) {
			if (onFirst > 0) { 
				(*nurse1) = nurse_g;
				(*position1) = ip->solMatrix[nurse_g][job];
				onFirst = -1000;
			}
			else {
				(*nurse2) = nurse_g;
				(*position2) = ip->solMatrix[nurse_g][job];
				break;
			}
		}
	}

}

void StandardLocalSearch(struct INSTANCE* ip, int MAX_ITERATIONS, double MAX_TIME) {
    StandardLocalSearchTest(ip, MAX_ITERATIONS, MAX_TIME, 1);
}

double StandardLocalSearchTest(struct INSTANCE* ip, int MAX_ITERATIONS, double MAX_TIME, int TEST_ITERATIONS) {

	double eps = 1e-6;
	double retValue = -11;
	int test_assigned = 0;
	int useTwoOpt = (int)(ip->algorithmOptions[1] + eps);
	int useTwoExchange = (int)(ip->algorithmOptions[2] + eps);
	int useNurseOrderChange = (int)(ip->algorithmOptions[3] + eps);
	double quality = SolutionQuality(ip, -15);
	int performedSwaps = 0;
	int twoOptDidSomething = 0;
	float elapsedTime = 0;
	int infOnly = 1;
	int lsIters = -1;

	double propQ = 0.0;
	int n1_improvements = 0;
	int n2_improvements = 0;
	int n3_improvements = 0;
	double n1_imp_amount = 0;
	double n2_imp_amount = 0;
	double n3_imp_amount = 0;

	clock_t start = clock();
	clock_t end = clock();

	while (1 > 0) {
		lsIters++;

		infOnly = 0;
		twoOptDidSomething = 1;
		double b_baseQuality = SolutionQuality(ip, -16);

		if (lsIters == TEST_ITERATIONS) {
			retValue = b_baseQuality; 
			test_assigned = 1;
		}
		quality = b_baseQuality;

		if (ip->verbose > 5) {
			printf("Best switch (start quality: %.2f)\n", b_baseQuality);
		}

		end = clock();
		elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
		if (elapsedTime > MAX_TIME) {
			break;
		}

		int bswitchValue = BestSwitch(ip, infOnly, MAX_TIME - elapsedTime);
		if (bswitchValue > -1) {
			performedSwaps++;
			n1_improvements++;
			propQ = SolutionQuality(ip, -17);
			n1_imp_amount = quality - propQ;
			quality = propQ;
			if (ip->verbose > 5) {
				printf("BestSwitch improved to > %.2f\n", quality);
			}
			continue;
		}


		if (useTwoExchange > 0) {
			end = clock();
			elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
			if (elapsedTime > MAX_TIME) {
				break;
			}
			if (ip->verbose > 5) {
				printf("Two exchange (start quality: %.2f)\n", quality);
			}
			int twoExchangeValue = RouteTwoExchange(ip, 1);
			if (twoExchangeValue > -1) {
				propQ = SolutionQuality(ip, -1711);
				quality = propQ;
				if (ip->verbose > 5) {
					printf("TwoExchange improved to > %.2f\n", quality);
				}
				continue;
			}
		}

		if (useNurseOrderChange > 0) {
			end = clock();
			elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
			if (elapsedTime > MAX_TIME) {
				break;
			}
			int didItWork = NurseTwoExchange(ip);
			if (didItWork > -1) {
				quality = SolutionQuality(ip, -1710);
				continue;
			}
		}


		if (ip->verbose > 5) {
			printf("\tBest_sync_double_switch (start quality: %.2f)\n", SolutionQuality(ip, -18));
		}	
		if (BestSyncDoubleSwitch(ip) > 0) {
			end = clock();
			elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
			if (elapsedTime > MAX_TIME) {
				break;
			}
			if (ip->verbose > 5) {
				printf("\tbest_sync_double_switch improved to > %.2f\n", SolutionQuality(ip, -19));
			}
			propQ = SolutionQuality(ip, -20);
			n2_improvements++;
			n2_imp_amount = quality - propQ;
			quality = propQ;
			continue;
		}

		if (ip->verbose > 5) {
			printf("\t\t2opt (start quality: %.2f)\n", SolutionQuality(ip, -21));
		}
		twoOptDidSomething = 0;
		if (useTwoOpt > 0.5) {
			double TOL = 0.1;
			double startoffwith = quality;

			for (int nurseidx = 0; nurseidx < ip->nNurses; nurseidx++) {
				int nurse = ip->nurseOrder[nurseidx];
				int foundImprovement = 10;
				int nurseJobCount = GetNurseJobCount(ip, nurse);
				int countImprovements = 0;
				double nurseInitQ = SolutionQuality(ip, -22);
				int realOptIterations = 0;
				while (foundImprovement > 1) {
					foundImprovement = -1;
					for (int posi = 0; posi < ip->nJobs; posi++) {
						for (int posj = posi + 1; posj < ip->nJobs; posj++) {
							double initq = SolutionQuality(ip, -23);
							int res = TwoOptMove(ip, nurse, posi, posj);
							if (res < 0) {
								continue;
							}
							double endq = SolutionQuality(ip, -24);
							realOptIterations++;
							if (endq < initq + TOL) {
								res = TwoOptMove(ip, nurse, posi, posj);
								if (res < 0) {
									printf("ERROR: Cannot undo 2opt move\n");
									exit(-1);
								}
							}
							else if (endq + TOL > initq) {
								if (ip->verbose > 5) {
									printf("\t\t2opt improved to > %.2f\n", SolutionQuality(ip, -25));
								}
								foundImprovement = 10;
								countImprovements++;
								break;
							}
						}
						if (foundImprovement > -1) {
							break;
						}
					}
				}
			}

			double finishoffwith = SolutionQuality(ip, -27);
			if (finishoffwith > startoffwith + TOL) {
				twoOptDidSomething = 1;
				n3_improvements++;
				n3_imp_amount = startoffwith - finishoffwith;
				quality = finishoffwith;
			}
		}

		if (ip->verbose > 5) {
			printf("Two opt did something? %d\n", twoOptDidSomething);
		}
		if (twoOptDidSomething < 1) {
			break;
		}
		end = clock();
		elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
		if (elapsedTime > MAX_TIME) { //Exceeded time limit
			break;
		}

	}

	double final_quality = SolutionQuality(ip, -29);

	if (test_assigned < 1) {
		retValue = final_quality;
	}

	if (ip->verbose > 5) {
		printf("Final quality: %.4f\n", final_quality);
		printf("\n\tBy neighbourhood:\n");
		printf("Neighbourhood\tMovements   \tImprovement\tAvg. Improvement\n");
		printf("1\t%d\t%.4f\t%.4f\n", n1_improvements, n1_imp_amount, n1_imp_amount / n1_improvements);
		printf("2\t%d\t%.4f\t%.4f\n", n2_improvements, n2_imp_amount, n2_imp_amount / n2_improvements);
		printf("3\t%d\t%.4f\t%.4f\n", n3_improvements, n3_imp_amount, n3_imp_amount / n3_improvements);
		printf("---- Performed %d LS iterations, %.2f seconds.----\n", lsIters, elapsedTime);
	}

	return retValue;

}
