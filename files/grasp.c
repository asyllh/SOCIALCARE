#include "float.h"
#include "math.h"
#include "constructive.h"
#include "grasp.h"

int GRASP(struct INSTANCE* ip) {

	// PR01 Perform path relinking between every generated solution and a random solution from the pool.
	// PR02 Perform path relinking between some new solutions (above a threshold) and all solutions in the pool.
	// PR03 Perform path relinking at the end of the algorithm, between all pairs of solutions in the pool.
	// PR04 Do not perform path relinking at all.
	
	clock_t start = clock();
	clock_t end = clock();
	double eps = 1e-6;
	double elapsedTime = 0.0;
	double average_gen_quality = 0.0;
	long double sum_of_qualities = 0.0;
	int given_quality = ip->quality_measure;
	int solutions_in_pool = (int)(ip->algorithmOptions[8] + eps); // Allocate space for a pool of solutions (aO[8] = 10)
	int rcl_strategy = (int)(ip->algorithmOptions[10] + eps); //Type of strategy for cut-off value in RCL (used in randomised_constructive). (aO[10] = 1 (strategy 1 C=B-delta*(B-W)))

	double* pool_quality = malloc(solutions_in_pool * sizeof(double));
	struct INSTANCE** pool = (struct INSTANCE**)malloc(solutions_in_pool * sizeof(struct INSTANCE*));
	for (int i = 0; i < solutions_in_pool; i++) {
		pool[i] = NULL;
		pool_quality[i] = -DBL_MAX;
	}

	int PR_STRATEGY = (int)(ip->algorithmOptions[9] + eps); // 1, 2, 3 or 4, as above (aO[9] = 1, so PR with one random solution in pool)
	int PR_DIRECTION_PARAMETER = (int)(ip->algorithmOptions[11] + eps); // 0 forward, 1 backward, 2 forward & backward, 3 randomly chosen (including f&b) (aO[11] = 0, so forward)
	// int performPathRelinking = (int) ip->algorithmOptions[7] + eps;

	if (ip->verbose > 5) {
		printf("Started GRASP procedure...\n");
	}

	int doLS = 1;
	int doLSAfter = 0;
	int randomGraspParam = 0;
	double GRASP_param = 0.0; // DELTA IN PAPER. Has effect only if randomGraspParam = 0. Later on in the function: double rnd_dbl = (float)rand() / (float)RAND_MAX; GRASP_param = delta_low + rnd_dbl * delta_range;
	double delta_low = ip->algorithmOptions[4]; // 0.05;
	double delta_range = ip->algorithmOptions[5]; // 0.25;
	double currentQuality = -1; 
	double bestQuality = -1 * bigM;
	unsigned int LS_ITERS = bigM;
	unsigned int MAX_ITERATIONS = bigM;
	int LS_ITERS_AFTER = MAX_ITERATIONS; // = bigM
	float LS_TIME = ip->MAX_TIME_SECONDS;
	float LS_TIME_AFTER = ip->MAX_TIME_SECONDS;
	struct INSTANCE bestSol, emptySol;
	bestSol = (*ip); 

	// Initial nurse order: 
	int* bestNurseOrder = malloc(ip->nNurses * sizeof(int)); // Rows
	for (int i = 0; i < ip->nNurses; ++i) { //Initially, nurseOrder is 0,1,...,nNurses-1, and so bestNurseOrder array is taken to be the same.
		ip->nurseOrder[i] = i;
		bestNurseOrder[i] = i;
	}

	printf("\nSTARTING Nurse order: (");
	for (int i = 0; i < ip->nNurses; ++i) {
		printf("%d,", ip->nurseOrder[i]);
	}
	printf(")\n");

	//Initial bestSolMatrix:
	int** bestSolMatrix = malloc(ip->nNurses * sizeof(int*)); // Rows
	for (int i = 0; i < ip->nNurses; i++) {
		bestSolMatrix[i] = malloc(ip->nJobs * sizeof(int)); // Cols
		for (int j = 0; j < ip->nJobs; ++j) {
			bestSolMatrix[i][j] = ip->solMatrix[i][j]; //Initially, bestSolMatrix is taken to be the same as solMatrix, so all -1's.
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
	double before_exch = -1 * bigM; //quality of solution before exchange
	double average_constructive_sol = 0.0; 
	int changeNurseOrder = (int)(ip->algorithmOptions[6] + eps); //what does this value do? (aO[6] = 1, so nurse order change active in GRASP)
	emptySol = (*ip); 

	while (iter < MAX_ITERATIONS) {
		startIT = clock();
		currentQuality = sol_quality(ip, -30);

		struct INSTANCE newEmpty = emptySol;
		*ip = newEmpty; //what do these two lines do?
		clean_solution_from_struct(ip); // Set all elements in solMatrix 2D array to -1.

		//Running constructive.
		if (iter == 0) { //First iteration only, no solution currently exists.
			before_exch = currentQuality;
		}
		else if (changeNurseOrder > 0) {
			if (currentQuality > before_exch) { //If solution quality is better than the quality of the solution before the exchange, update before_exchange
				before_exch = currentQuality;
				random_two_exchange(ip->nurseOrder, ip->nNurses, &exch1, &exch2);
				no_imp_iter = 0;
			}
			else { // Quality of solution before exchange is better than or equal to the quality of the current solution (after exchange), so reverse the previous exchange to return back to better quality solution.			
				no_imp_iter++;
				two_exchange(ip->nurseOrder, exch1, exch2);
			}
			shuffle(ip->nurseOrder, (size_t)ip->nNurses); //Shuffle the contents of nurseOrder.
		}

		double rnd_dbl = (float)rand() / (float)RAND_MAX;
		GRASP_param = delta_low + rnd_dbl * delta_range; //delta value in paper, used for cut-off value strategies.

		//Assign jobs to nurses, create a solution.
		randomised_constructive(ip, 1, GRASP_param, rcl_strategy);
		sols_generated++;

		endIT = clock();
		elapsedTimeIT = (float)(endIT - startIT) / CLOCKS_PER_SEC;
		// Constructive finished.

		double constrCurrentQuality = sol_quality(ip, -10000); //quality of the current solution.
		if (sols_generated > 1) {
			average_constructive_sol = (average_constructive_sol * (sols_generated - 1) + constrCurrentQuality) / sols_generated;
		}
		else {
			average_constructive_sol = constrCurrentQuality;
		}

		if (constrCurrentQuality > bestCR) {
			bestCR = constrCurrentQuality;
		}

		// DEBUG : change to 0, higher than 0 means deactivated
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
			ls_quality_test = standard_local_search_test(ip, LS_ITERS, LS_TIME, 100); //Run LS first-improvement VNS, returns the solution quality of the best solution created
		}
		else {
			printf("SKIP LS, quality: %.2f (average CR: %.2f, threshold value: %.2f )\n", constrCurrentQuality, average_constructive_sol, constr_tr_value);
		}

		// Extra debug:
		double ca_time = elapsedTimeIT; //constructive algorithm time (seconds) (never used)
		endIT = clock();
		elapsedTimeIT = (float)(endIT - startIT) / CLOCKS_PER_SEC; //constructive algorithm time + local search time (seconds)
		// End extra debug

		currentQuality = sol_quality(ip, -11);
		double ls_total_q = currentQuality;

		/// ------------------------- START PATH RELINKING (PR) ------------------------- ///
		// Strategies:
		// PR01 Perform path relinking between every generated solution and a random solution from the pool.
		// PR02 Perform path relinking between some new solutions (above a threshold) and all solutions in the pool.
		// PR03 Perform path relinking at the end of the algorithm, between all pairs of solutions in the pool.
		// PR04 Do not perform path relinking at all.
		// PR_DIRECTION_PARAMETER: 0 forward, 1 backward, 2 forward & backward, 3 randomly chosen (including f&b)

		//Here, using the values from algorithmOptions, PR_DIRECTION_PARAMETER = 0, so PR_DIRECTION stays = 0 - the direction of PR is FORWARD.
		//Here, using the values from algorithmOptions, PR_STRATEGRY = 1, so the strategy used is PR01 - PR for every solution with one random solution in the pool.

		double TOL = 1e-6;
		
		int PR_DIRECTION = 0; //Forward
		if (PR_DIRECTION_PARAMETER == 1) {
			PR_DIRECTION = 1; //Backward
		}
		else if (PR_DIRECTION_PARAMETER == 2) {
			PR_DIRECTION = 2; //Forward and backward
		}
		else if (PR_DIRECTION_PARAMETER == 3) {
			PR_DIRECTION = pick_integer(3); //Randomly chosen (either forward, backwards, or forward and backward)
		}

		/// PR01: PR between every generated solution and a random solution from the pool
		if (PR_STRATEGY == 1) {
			solutions_relinked++;
			printf("Start PR round for sol with quality %.2f\n", sol_quality(ip, -123));
			for (int trial = 0; trial < solutions_in_pool * 2; trial++){ // Don't do too many trials here (solution_in_pool * 2 = 20)
				int ps = pick_integer(solutions_in_pool); //Pick random integer between 0 and solutions_in_pool-1
				if (NULL != pool[ps]) {
					printf("\tAbout to PR incumbent with POOL-%d <-> ", ps);
					path_relinkings_performed++;
					struct INSTANCE relinkedSolution = copy_instance(ip);
					double rel_quality = -1;
					// Do PR with ip and pool[ps], and return the quality of the best solution (relinkedSolution).
					rel_quality = directed_path_relinking(ip, pool[ps], currentQuality, pool_quality[ps], &relinkedSolution, PR_DIRECTION);
					if ((rel_quality > currentQuality + TOL) && (fabs(rel_quality - pool_quality[ps]) > TOL)) { //Quality of relinkedSoln > quality of ip AND quality of relinkedSoln and pool[ps] sol differ.
						// PR improved the current solution and it's different from the one in the pool
						overwrite_instance(ip, &relinkedSolution); //ip solution updated to the (better) relinkedSolution
						printf("\tDone. Achieved quality: %.2f\n", sol_quality(ip, -123));
						currentQuality = rel_quality; //Update current quality of ip
					}
					else {
						printf("\tDone. PR didn't work, achieved quality: %.2f\n", sol_quality(&relinkedSolution, -123));
					}
					free_instance_copy(&relinkedSolution); //Deallocate memory
					break;
				} //End if(NULL != pool[ps])
			} //End for(int trial = 0; trial < solutions_in_pool * 2; trial++)
		} //End if (PR_STRATEGY == 1)

		else if (PR_STRATEGY == 2) {
			double pr_pi = 1;
			double pr_threshold = -bigM;
			if (currentQuality > pr_threshold) { //Passed the threshold
				solutions_relinked++;
				struct INSTANCE relinkedSolution = copy_instance(ip);
				struct INSTANCE bestRelinkedSolution = copy_instance(ip);
				double best_quality_of_round = currentQuality;
				for (int ps = 0; ps < solutions_in_pool; ps++) {
					if (NULL != pool[ps]) {
						double rel_quality = directed_path_relinking(ip, pool[ps], currentQuality, pool_quality[ps], &relinkedSolution, PR_DIRECTION);
						if ((rel_quality > best_quality_of_round + TOL) && (fabs(rel_quality - pool_quality[ps]) > TOL)) { //Quality of relinkedSoln > quality of ip AND quality of relinkedSoln and pool[ps] sol differ.
							// PR improved the current solution and it's different from the one in the pool
							overwrite_instance(&bestRelinkedSolution, &relinkedSolution); //relinkedSolution is stored as bestRelinkedSolution
							best_quality_of_round = rel_quality;
						}
					}
				}
				if (best_quality_of_round - TOL > currentQuality) { //If bestRelinkedSolution has better quality than current quality of ip
					overwrite_instance(ip, &bestRelinkedSolution); //Update ip, ip is now bestRelinkedSolution
				}
				currentQuality = sol_quality(ip, -11); //currentQuality = quality of updated ip solution (final quality)

				if (fabs(currentQuality - best_quality_of_round) > TOL) { //If current quality of ip and quality of bestRS are not the same
					printf("ERROR: PR option 2 did not work as expected!\n We thought we'd get: %.2f but we got %.2f\n", best_quality_of_round, currentQuality);
					exit(-34339);
				}
				//Deallocate memory
				free_instance_copy(&relinkedSolution);
				free_instance_copy(&bestRelinkedSolution);
			}
		} //End else if (PR_STRATEGY == 2)

		if (constructive_passes_threshold > 1) {
			if (ip->verbose > 10) {
				print_and_recalculate_pool_contents(pool, pool_quality, solutions_in_pool);
				calculate_and_print_dissimilartiy_matrix(pool, solutions_in_pool);
			}
		}

		/// ------------------------- END PATH RELINKING (PR) ------------------------- ///
	
		// printf("\n-!- %.2f\t%.2f\t%.2f\t%.2f\n", constrCurrentQuality, ls_quality_test, ls_total_q, currentQuality);

		// Check the pool:
		int insertHere = -1;
		int dissimilarity_threshold = 5;
		double lowest_dissimilarity = bigM;
		//Check if solution should enter the pool
		for (int ps = 0; ps < solutions_in_pool; ps++) { //For each ps=0,...,#solnsinpool
			if (NULL == pool[ps]) { // If there are still NULL pointers, allocate them and assign here
				pool[ps] = malloc(sizeof(struct INSTANCE));
				insertHere = ps;
				break;
			}
			double dissimilarity_with_ps = solution_dissimilarity(ip, pool[ps]); //Returns the solution dissimilarity d(S_1, S_2) where S_1 = ip, S_2 = pool[ps].

			if (pool_quality[ps] > currentQuality) { //If quality of pool[ps] solution better than quality of ip
				if (dissimilarity_with_ps < dissimilarity_threshold) { //If dissimilarity of solutions ip and pool[ps] is below the threshold, then the solution is too similar to solutions already in pool, so do not add soln to pool.
					insertHere = -1; //Rejected introducing this solution in pool, too similar to pool[ps]
					break; //Exit for ps loop
				}
				else { // printf("\t-> Not better than POOL %d (and dissimilarity value %.2f)\n", ps, dissimilarity_with_ps);
					continue; //Go to next ps (++ps)
				}
			}
			// We are good to insert, it's got better quality than this solution and it is not too similar to a better solution

			if (dissimilarity_with_ps > lowest_dissimilarity) { //Dissimilarity is too high?
				// printf("\t-> Will NOT substitute %d (dissimilarity is %.2f, but lowest is %.2f)\n", ps, dissimilarity_with_ps, lowest_dissimilarity);
				continue;
			}
			else {
				lowest_dissimilarity = dissimilarity_with_ps;
				insertHere = ps;
				// printf("\t-> Proposing to insert substituting %d (lowest dissimilarity so far, %.2f)\n", ps, lowest_dissimilarity);
			}
		} //End for (int ps = 0; ps < solutions_in_pool; ps++) loop
		//Finished checking pool, printf("Finished checking pool, insertHere = %d\n", insertHere);

		// Insert solution in the pool:
		if (insertHere >= 0) {
			(*pool[insertHere]) = copy_instance(ip); //pool[ps] is replaced with ip!
			pool_quality[insertHere] = currentQuality; 
		}
		end = clock();
		elapsedTime = ((double)(end - start)) / CLOCKS_PER_SEC;

		if (currentQuality > bestQuality) {
			if (ip->verbose > 2) {
				printf("It %d ", iter);
				printf("CA+LS %.2f ", currentQuality);
				// printf("(improves %.2f) ", bestQuality);
				printf("CA %.2f ", constrCurrentQuality);
				printf("tott %.2f ", ip->objTime);
				printf("tard %.2f ", ip->objTardiness);
				printf("ovti %.2f ", ip->objOvertime);
				printf("pref %.2f ", ip->totPref);
				printf("l_day %.2f ", ip->objLongestDay);
				// printf("\n");
				printf("GRASP_P %.4f ", GRASP_param);
				printf("et %.1fs (max %.1f)", elapsedTime, ip->MAX_TIME_SECONDS);
				printf("\n > Nurse order: (");
				for (int i = 0; i < ip->nNurses; ++i) {
					printf("%d,", ip->nurseOrder[i]);
				}
				printf(")\n");

				// Show what is on the pool:
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
				sol_quality(ip, -1);
			}

			bestSol = copy_instance(ip); //bestSol = ip, store best solution so far
			bestQuality = currentQuality; //bestQuality = quality of ip, store quality of best solution so far
		}
		else if (ip->verbose > 5) { //currentQuality <= bestQuality
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

		if (elapsedTime > ip->MAX_TIME_SECONDS) { //Exceeded time limit
			if (ip->verbose > 0) {
				printf("\nTime finished! (%.2f seconds)\n", elapsedTime);
			}
			break; //exit while loop
		}
		iter++;

	} // End of while (iter < MAX_ITERATIONS) loop

	if (ip->verbose > 0) {
		printf("GRASP: Performed %d iterations.\n", iter);
	}

	double thisSol = sol_quality(ip, -3423); //Quality of current ip solution
	double cquality = 0.0;
	int pickthis = -1;
	if (ip->verbose > 5) {
		printf("Pool contents:\n");
	}

	for (int ps2 = 0; ps2 < solutions_in_pool; ps2++) { //For each 'space' (position) in pool, ps2=0,...,solnsinpool
		if (pool[ps2] != NULL) { //If there is a solution in that position in the pool
			cquality = sol_quality(pool[ps2], -3234); //quality of the solution in that space in the pool
			if (ip->verbose > 5) {
				printf("\tPOS %d - Q = %.2f\n", ps2, cquality);
			}
		}
		if (thisSol < cquality) { //If there is no solution in that position in the pool (pool[ps2] == NULL) OR if the quality of the current solution ip is WORSE than the quality of the solution in pool[ps2]
			if (ip->verbose > 5) {
				printf("Pick solution %d with quality %.2f\n", ps2, cquality);
			}
			pickthis = ps2; //Select the solution in pool[ps2]
			thisSol = cquality; //update solution quality to be the quality of pool[ps2]
		}
	}
	if (pickthis > -1) { //If solution picked from pool
		bestSol = (*pool[pickthis]); //update best solution found
		*ip = *pool[pickthis]; // Get this pointer, ip = best solution found in pool
		pool[pickthis] = NULL; // Prevent this sol from being cleaned, remove solution from pool (remove pointer, ip still points to the solution though)
		if (ip->verbose > 5) {
			printf("Sol quality after copying: %.2f\n", sol_quality(ip, -235223));
		}
	}

	if (ip->verbose > 5) {
		printf("Saved best solution\n");
	}

	double quality = sol_quality(ip, -121212); //quality of solution ip

	/// ------------------------- START PATH RELINKING (PR) BETWEEN ALL PAIRS OF SOLUTIONS ------------------------- ///
	// PR03 Perform path relinking at the end of the algorithm, between all pairs of solutions in the pool.
	
	int relinking_all_pairs = 1;

	if (relinking_all_pairs > 0) {
		double TOL = 1e-6;
		printf("Relinking all pairs of solutions in the pool.\n");
		struct INSTANCE output = copy_instance(ip);
		struct INSTANCE bestRelinkedSolution = copy_instance(ip);
		double best_final_quality = quality;
		for (int ps = 0; ps < solutions_in_pool; ps++) {
			for (int ps2 = 0; ps2 < solutions_in_pool; ps2++) {
				if ((ps2 != ps) && (NULL != pool[ps]) && (NULL != pool[ps2])) {
					overwrite_instance(&output, pool[ps]);
					path_relinking(&output, pool[ps2]);
					double relinking_quality = sol_quality(&output, -123);
					if ((relinking_quality > best_final_quality + TOL)) {
						// PR improved the current solution and it's different from the one in the pool
						overwrite_instance(&bestRelinkedSolution, &output);
						printf("\tPair %d - %d: *** achieved quality: %.2f ***\n", ps, ps2, sol_quality(&bestRelinkedSolution, -123));
						best_final_quality = relinking_quality;
					}
				}
			}
		}

		if (quality < best_final_quality - TOL) { //If bestRS has better quality than previous ip solution (before PR), then update ip to be bestRS
			overwrite_instance(ip, &bestRelinkedSolution);
		}

		int solcallnum = ip->verbose - 1;
		solcallnum = 12345;
		quality = sol_quality(ip, solcallnum); //quality of updated ip solution

		if (fabs(quality - best_final_quality) > TOL) { //If quality of ip is not the same as quality of bestRS, then we haven't updated ip correctly
			printf("ERROR: PR all pairs did not work as expected!\n We thought we'd get: %.2f but we got %.2f\n", best_final_quality, quality);
			printf("Sol quality type: %d\n", ip->quality_measure);
			exit(-34339);
		}
		printf("\tPR all pairs final quality: %.2f\n", quality);

		//Deallocate memory:
		free_instance_copy(&output);
		free_instance_copy(&bestRelinkedSolution);
	}
	
	/// ------------------------- END PATH RELINKING (PR) BETWEEN ALL PAIRS OF SOLUTIONS ------------------------- ///


	if (ip->verbose > 3) {
		printf("Pool contents (cleaning):\n");
	}
	//Deallocating memory:
	for (int ps2 = 0; ps2 < solutions_in_pool; ps2++) { //For each 'space' (position) in the pool
		if (pool[ps2] != NULL) { //If there's a solution in that position, remove it from the pool
			if (ip->verbose > 3) {
				printf("\tPOS %d - Q = %.2f\n", ps2, sol_quality(pool[ps2], -3234));
			}
			free_instance_copy(pool[ps2]);
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

	// Deallocate memory
	for (int i = 0; i < ip->nNurses; ++i) {
		free(bestSolMatrix[i]);
	}
	free(bestSolMatrix);
	free(bestNurseOrder);

	return 0;

} //END OF GRASP FUNCTION.

void randomised_constructive(struct INSTANCE* ip, int randomness, double delta, int rcl_strategy) {

	//Seed: each nurse is assigned a single job to ensure that all nurses will have an initial route, regardless of how far their starting location is, and helps construct more balanced solution.
	//Assign remaining jobs: the jobs are ranked and assigned to the best position in any of the nurse routes, until no more jobs are left to assign.

	int DEBUG_PRINT = -1;
	if (DEBUG_PRINT > 0) {
		printf("\nSTARTING RANDOMISED CONSTRUCTIVE --------------------------\n");
		printf("Start solmatrix:\n");
		print_solmatrix(ip);
		printf("Constructive allocations...\n");
	}

	int* nurseSeed = malloc(ip->nNurses * sizeof(int)); // nurseSeed: 1 x nNurses, initialised to -1.
	for (int j = 0; j < ip->nNurses; ++j) {
		nurseSeed[j] = -1;
	}

	double* allocatedJobs = malloc(ip->nJobs * sizeof(double)); // allocatedjobs: 1 x nJobs, initialised to 0. =1 if job has been allocated, =0.5 if double service job has been allocated to only one nurse, = 0 if not allocated.
	for (int i = 0; i < ip->nJobs; ++i) {
		allocatedJobs[i] = 0;
	}

	int b_indices_size = ip->nNurses * ip->nJobs;
	int** bestIndices = malloc(b_indices_size * sizeof(int*)); //bestIndices: (nNurses x nJobs) x 2, column 0 = nurse, column 1 = job.
	for (int i = 0; i < b_indices_size; ++i) {
		bestIndices[i] = malloc(2 * sizeof(int));
	}

	double* rankValues = malloc(ip->nJobs * sizeof(double)); // rankValues: 1 x nJobs. Quality of solution for each job
	int* RCL_seeds = malloc(ip->nJobs * sizeof(int)); // RCL_seeds: 1 x nJobs.

	if (DEBUG_PRINT > 0) {
		printf("Constructive allocations.Done.\n");
	}

	// Assign seeds:
	int rcl_size = -1; // Number of elements in the RCL (RCL_seeds)
	int check_count = -1; // Count number of loop iterations
	double bestRank = -DBL_MAX;
	double worstRank = DBL_MAX;

	for (int nurseIdx = 0; nurseIdx < ip->nNurses; ++nurseIdx) {
		bestRank = -DBL_MAX;
		worstRank = DBL_MAX;
		int alreadyAllocated = 0;
		int unskilledFor = 0;
		int dismissedJobs = 0;
		int consideredJobs = 0;
		int nurse = ip->nurseOrder[nurseIdx]; // nurse = number of the nurse in the nurseIdxth position in the nurseOrder array. 

		for (int job = 0; job < ip->nJobs; ++job) { //For 'nurse', go through all jobs 0,...,nJobs, and check the quality of the solution that arises when 'job' is inserted into 'nurse's route
			//Update rankValues[job] for each job, and store/keep updated the highest/lowest quality solution (bestRank/worstRank) of all solutions for the current nurse. 
			check_count++;

			if (allocatedJobs[job] > 0.6) { //Job 'job' has already been allocated
				alreadyAllocated++;
				rankValues[job] = -2 * bigM; 
				continue;
			}
			if (check_skills(ip, job, nurse) < 1) { //If nurse is not skilled to do the job (i.e. nurseSkilled[nurse][job] = 0)
				unskilledFor++;
				rankValues[job] = -bigM;
				continue;
			}
			if (DEBUG_PRINT > 0) {
				printf("\n---Insert in nurse %d the job %d\n", nurse, job);
			}
			if (best_job_insertion(ip, job, nurse) < 0){ //If job cannot be inserted into nurse's route.
				dismissedJobs++;
				rankValues[job] = -bigM;
				continue;
			}
			else { //Job can and has been inserted into nurse's route.
				consideredJobs++;
			}
			if (DEBUG_PRINT > 0) {
				printf("Done.\nRanking job %d...\n\tValue: ", job);
			}

			int sol_q_num = -1000;

			// Check the rank of the job and save the best one overall
			rankValues[job] = sol_quality(ip, sol_q_num); //Calculate the solution quality of the job-nurse assignment (i.e. the new solution with 'job' inserted into nurse's route), and save it to rankValues[job].

			// Keep track of best and worst ranks
			if (rankValues[job] > bestRank) {
				bestRank = rankValues[job];
			}
			if (rankValues[job] < worstRank) {
				worstRank = rankValues[job];
			}
			if (DEBUG_PRINT > 0) {
				printf("%.2f\nDone.\nRemoving job %d\n", rankValues[job], job);
			}
			remove_job(ip, job, nurse); //Remove job from nurse's route, i.e. return solution back to original.

			bestIndices[job][0] = nurse; // The best quality solution (so far) was created with 'job' in 'nurse's route, store best indices.
			bestIndices[job][1] = job;

		} //End for(int job = 0; job < ip->nJobs; ++job) loop

		if (consideredJobs < 1) { //None of the jobs could be inserted into nurse's route, so continue in for loop (move on to next nurse (++nurseIdx) in nurseOrder array).
			continue;
		}

		//Create cut-off value and add elements (jobs) to the RCL (RCL_seeds). rcl_size = number of elements in the RCL,
		generate_rcl(delta, &rcl_size, RCL_seeds, rankValues, ip->nJobs, bestRank, worstRank, rcl_strategy);

		int cNurse = -1;
		int cJob = -1;

		//Pick an element at random from RCL_seeds, returns the indicies for the selected element, i.e. the job-nurse assignment. cNurse = the selected nurse and cJob = the selected job (they are selected together).
		rcl_pick(bestIndices, RCL_seeds, rcl_size, &cNurse, &cJob);

		if (DEBUG_PRINT > 0) {
			// printf("Picking one of the %d top seed values for nurse %d\n", GRASP_param, nurse);
			printf("Decided on (nurse %d, job %d)\n", cNurse, cJob);
			printf("Solmatrix:\n");
			print_solmatrix(ip);
			printf("Nurse routes:\n");
			print_int_matrix(ip->allNurseRoutes, ip->nNurses, ip->nJobs);
			if (cJob < 0) {
				printf("ERROR: The best pick was a -1!!!!\n");
				exit(-1);
			}
		}

		if (best_job_insertion(ip, cJob, cNurse) > -1) { //If cJob has been successfully inserted into cNurse's route
			// printf("Chosen job %d as seed for nurse %d\n", cJob, nurse);
			if (ip->doubleService[cJob]) { // If cJob is a double service 
				allocatedJobs[cJob] += 0.5; // Record cJob as being half-done (only one nurse so far, need two)
			}
			else {
				allocatedJobs[cJob] = 1; // If cJob is a normal job (only one nurse required), then mark cJob as allocated.
			}
			nurseSeed[nurse] = cJob; //Mark/assign cJob to the current nurse (not cNurse). Should this be nurseSeed[cNurse]?
		}
		else {
			printf("\nERROR! Solmatrix:\n");
			print_solmatrix(ip);
			printf("\nERROR: FAILED to assign job %d as seed for nurse %d\n", cJob, cNurse);
			if (ip->doubleService[cJob]) {
				printf("(This job is a DS)\n");
			}
			exit(-1);
		}
	} // End for (int nurseIdx = 0; nurseIdx < ip->nNurses; ++nurseIdx) loop

	//Deallocate memory
	free(RCL_seeds); 
	free(rankValues); 

	/*--------Finished with seeds--------*/

	if (DEBUG_PRINT > 0) {
		printf("Finished with seeds\n");
	}

	int jobsRemaining = 0; // Number of jobs that still need to be assigned.
	for (int i = 0; i < ip->nJobs; ++i) {
		if (allocatedJobs[i] < 0.9) { //If the job i has not been completely fulfilled (the job hasn't been assigned to a nurse/two nurses or the job is a double service and only one nurse has been assigned the job).
			jobsRemaining++;
		}
	}

	// Assign remaining items
	int ct;
	int ittt = 0; //Is this needed? Only used at the beginning of the while loop, ittt++.
	int assignmentIterations = 0;
	int MAX_ASSIGNMENT_ITS = jobsRemaining * 500;
	int* RCL = malloc(ip->nJobs * ip->nNurses * sizeof(int)); // 1D array, size = 1 x (nJobs*nNurses), Restricted Candidate List.
	double* rankAssignments = malloc(ip->nJobs * ip->nNurses * sizeof(double)); // 1D array, size = 1 x (nJobs*nNurses)

	while (jobsRemaining > 0) {
		ittt++;
		assignmentIterations++;

		// Reset variables that search for the RCL elements
		bestRank = -DBL_MAX;
		worstRank = DBL_MAX;
		for (int i = 0; i < b_indices_size; ++i) {
			bestIndices[i][0] = -1;
			bestIndices[i][1] = -1;
		}

		double base_quality = sol_quality(ip, -3423462); //solution quality of current solution ip.
		ct = -1;
		int potentialAllocations = 0;
		for (int nurseidx = 0; nurseidx < ip->nNurses; ++nurseidx) { // For each nurse 0,...,nNurses
			int nurse = ip->nurseOrder[nurseidx]; 
			for (int job = 0; job < ip->nJobs; ++job) {
				ct++;
				// printf("Test %2d: nurse %d - job %d - ", ct, nurse, job);

				if (allocatedJobs[job] > 0.6 || ip->solMatrix[nurse][job] > -0.1) { //if job has already been allocated OR job is a double service and is already positioned in nurse's route.
					rankAssignments[ct] = -DBL_MAX;
					// printf(" already allocated\n");
					continue; // Move on to next job (++job) in the for loop.
				}

				// There is potential to allocate this job to this nurse, let's see if they have the skill
				int skillCheck = 0;
				if (!ip->doubleService[job]) { //If the job is not a double service (doubleService[job] == 0)
					skillCheck = check_skills(ip, job, nurse); //skillCheck = ip->nurseSkilled[nurse][job]
				}
				else { //If the job is a double service
					if (allocatedJobs[job] < 0.4) { // If no nurses have been assigned this job (recall that allocatedJobs[job] = 0.5 means that one nurse has been assigned the job and another nurse is required).
						skillCheck = check_skills_ds_first(ip, job, nurse); //skillCheck = 1 if there exists another nurse that can do the double service job with 'nurse', and = 0 otherwise.
					}
					else { //If only one nurse has been assigned to this double service 'job' (allocatedJobs[job] == 0.5)
						// Who else is doing it?
						for (int nurseb = 0; nurseb < ip->nNurses; nurseb++) {
							if (ip->solMatrix[nurseb][job] > -0.5) { //if 'job' is positioned in nurseb's route, i.e. nurseb has been assigned 'job'
								skillCheck = check_skills_ds(ip, job, nurse, nurseb); //skillCheck = 1 if nurse and nurseb can do the job together, and = 0 otherwise.
								break;
							}
						}
					}
				}

				//If job has been allocated OR nurse is unskilled to do the job OR job is already positioned in nurse's route, the the job-nurse assignment is discarded.
				if (allocatedJobs[job] > 0.6 || skillCheck < 1 || ip->solMatrix[nurse][job] > -0.1) {
					rankAssignments[ct] = -DBL_MAX;
					continue; //Move on to the next job (++job) in the for loop.
				}
				else { 
					potentialAllocations++;
				}

				// printf("Nearly inserting job %d in nurse %d\n", job, nurse);
				if (best_job_insertion(ip, job, nurse) < 0){
					printf("WARNING: Constructive could not insert job %d in nurse %d, but previous checks deemed it possible!!!\n", job, nurse);
					continue;
				}

				rankAssignments[ct] = sol_quality(ip, -1000);
				
				// debug : remove this! only for testing if priority to "difficult" jobs is good
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
				// end of debug part!

				// Keep track of best and worst ranks
				if (rankAssignments[ct] > bestRank) {
					bestRank = rankAssignments[ct];
				}
				if (rankAssignments[ct] < worstRank) {
					worstRank = rankAssignments[ct];
				}

				remove_job(ip, job, nurse);

				bestIndices[ct][0] = nurse;
				bestIndices[ct][1] = job;

			}// End for (int job = 0; job < ip->nJobs; ++job) loop

		} //End for (int nurseidx = 0; nurseidx < ip->nNurses; ++nurseidx) loop

		// printf("There were %d potential allocations, and there are %d remaining jobs.\n", potentialAllocations, jobsRemaining);
		if (potentialAllocations < jobsRemaining) {
			printf("ERROR! Not enough allocations.\n");
		}

		//Create cut-off value and add elements (jobs) to the RCL (RCL_seeds).
		generate_rcl(delta, &rcl_size, RCL, rankAssignments, ip->nJobs * ip->nNurses, bestRank, worstRank, rcl_strategy);
	
		int best_nurse = -1;
		int best_job = -1;

		//Pick an element at random from RCL_seeds, returns the indicies for the selected element, i.e. the job-nurse assignment. best_nurse = the selected nurse and best_job = the selected job.
		rcl_pick(bestIndices, RCL, rcl_size, &best_nurse, &best_job);

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
		if (best_job_insertion(ip, cJob, cNurse) > -1){ //If cJob has been successfully inserted into cNurse's route
			if (ip->doubleService[cJob]) { //If cJob is a double service (doubleService[cJob] == 1).
				allocatedJobs[cJob] += 0.5; //Mark cJob as being allocated to a nurse
				if (allocatedJobs[cJob] >= 0.6) { //If cJob has been allocated two nurses, then the job has been fulfilled!
					jobsRemaining--;
				}
			}
			else { //if cJob is not a double service, then mark cJob as being allocated to a single nurse, and the job has been fulfilled.
				allocatedJobs[cJob] = 1;
				jobsRemaining--;
			}
		}

	} //End of while(jobsRemaining > 0) loop

	// Deallocate memory
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
		print_solmatrix(ip);
		printf("\n-------------- END OF RANDOMISED CONSTRUCTIVE -------------- \n\n");
	}

	return;

} // END OF randomised_constructive function.

void grasp_ls(struct INSTANCE* ip, int LS_ITERS) {
	double quality = sol_quality(ip, 0);
	int maxSwitches = 3;
	int performedSwaps = 0;
	int performedSwitches = maxSwitches + 1000;
	int doSwitch = 1;
	int firstFeasible = 1;
	double lastQuality = quality;
	// int count = 0;
	// for (int i = 0; i < MS_ITERATIONS; ++i)
	float elapsedTime = 0;
	int infOnly = 1;
	// int MAX_LS_ITERS = 100000;
	for (int i = 0; i < LS_ITERS; ++i) {
		/* Perform a swap */
		int n1 = rand() % ip->nNurses;
		int n2 = rand() % ip->nNurses;
		int j1 = rand() % ip->nJobs;

		if (best_switch(ip, infOnly, ip->MAX_TIME_SECONDS) > -1) {
			performedSwaps++;
			double propQ = sol_quality(ip, 0);
			// if (propQ < quality) // Reverse
			// {
			// 	if(repair(ip, n1) < 0)
			// 	{
			// 		printf("COULD NOT UNDO REPAIR\n");				
			// 	}
			// }
			// else
			quality = propQ;
			if (infOnly < 0.1) {
				infOnly = 1;
				// if (ip->verbose > 2)
				// 	printf("Accepting only switches from infeasibilities!.\n");
			}
		}
		else {
			if (infOnly > 0.1) {
				infOnly = 0;
				if (ip->verbose > 2) {
					// printf("Accepting any switch now.\n");
					continue;
				}
			}
			else {
				if (performedSwitches < maxSwitches) {
					doSwitch = 1;
				}
				else
					break;
			}
		}

		if (doSwitch > 0) {

			performedSwitches++;
			int zzzz = 0;
			for (int switchTrials = 0; switchTrials < 1000; ++switchTrials) {
				n1 = rand() % ip->nNurses;
				n2 = rand() % ip->nNurses;
				j1 = rand() % ip->nJobs;
				if (switch_nurse(ip, n1, n2, j1) > -1) {
					zzzz = 1;
					break;
				}
			}
			if (zzzz > 0) {
				doSwitch = 0;
			}
			// else
			  // printf("WARNING: 1000 iterations expired and no switch could be performed!\n");

		}

		if (quality > lastQuality) {
			lastQuality = quality;
			// if (ip->verbose > 1)
			// {
			// 	// printf("\nSTART----------------------------------------------------\n");
			// 	// sol_quality(ip, 1);
			// 	// printf("Quality improved to: %.2f (%.2f) \n", quality, alternative_quality(ip, 0));
			// 	// printf("Quality improved to: %.2f (%.2f) \n", quality, alternative_quality(ip, 0));
			// 	// printf("\n----------------------------------------------------END\n");
			// }
			  // printf("Quality improved to: %.2f\n", quality);
		}

		// if (quality > -1 && firstFeasible > 0)
		// {
		// 	firstFeasible = -1;
		// 	printf("Potential problem! Quality: %d\n", quality);
		// 	printf("Switch %d, %d, %d\n", n1, n2, j1);
		// 	print_solmatrix(ip);
		// 	report_solution(ip);
		// 	break;

		// }

		// end = clock();
		// elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;

	}




	return;
}

void generate_rcl(double delta, int* rcl_size, int* RCL_seeds, double* rankValues, int rv_size, double bestRank, double worstRank, int strategy) {

	//This function creates the cut-off value C (worstAllowed) for the RCL, and then goes through all rankValues[i] for i=0,...,nJobs.
	//If rankValues[i] > cut off point (i.e quality of solution with job i inserted has quality that is higher than the cut-off point), then job i is added to the RCL (RCL_seeds).
	/*
	  Generate a Restricted Candidate List based on
	  the value of delta \in [0, 1].

	  Can use different strategies:
		- S1: Value based, top delta% candidates of the range
		- S2: Value based, all candidates within delta% of best
		- S3: Rank based, delta% best candidates

	  Outputs:
		- Size of list is saved in rcl_size
		- indices of list elements are saved on the
		  first rcl_size elements of RCL_Seeds
	*/

	// S1 and S2 - Just need to set a threshold value
	double worstAllowed = 0; // Cut-off value C
	double tolerance = 1e-6; // Use a tolerance to try and be consistent between runs

	// double hardCutOff = -DBL_MAX;
	double hardCutOff = -bigM + 1; // To avoid introducing infeasibility (those get -bigM)
	if (hardCutOff > bestRank) {
		hardCutOff = -DBL_MAX; // If the best is infeasible, little we can do!
	}

	double really_bad_value = bestRank - bigM;
	if (hardCutOff < really_bad_value) {
		hardCutOff = really_bad_value;
	}

	if (strategy == 1) {
		worstAllowed = bestRank - delta * fabs(bestRank - worstRank); // C = B - delta(B - W)
	}
	else if (strategy == 2) {
		worstAllowed = bestRank - fabs(bestRank) * delta; // C = B - delta*B
	}
	else {
		printf("ERROR: In generate_rcl() the value of 'strategy' is %d, which is not understood\n", strategy);
		exit(-321);
	}

	if (worstAllowed > bestRank) { //If this is true, then no solutions can be added to the RCL because no solutions are better than the cut off value.
		printf("Warning: Something went wrong, worstAllowed > bestRank");
		printf("Creating list with strategy = %d and delta = %.9f\n>>\tBest: %.2f, Worst: %.2f, Worst allowed: %.2f, hardCutOff: %.2f\n",
			strategy, delta, bestRank, worstRank, worstAllowed, hardCutOff);
		printf("delta*abs(bestRank - worstRank) = %.9f\n", delta * abs(bestRank - worstRank));
		printf("abs(bestRank - worstRank) = %d\n", abs(bestRank - worstRank));
		printf("fabs(bestRank - worstRank) = %.9f\n", fabs(bestRank - worstRank));
		printf("bestRank - worstRank = %.9f\n", bestRank - worstRank);
		printf("abs(bestRank)*delta = %.9f\n", abs(bestRank) * delta);
	}

	// Handle a hard cut-off value to avoid too many infeasibilities when constructing solutions
	if (worstAllowed < hardCutOff) {
		worstAllowed = hardCutOff;
		if (bestRank < -DBL_MAX + 100) {
			printf("ERROR: The best candidate in the RCL is infeasible (!)(!)(!)\n");
			exit(-124);
		}
	}

	if (strategy == 1 || strategy == 2) {
		// printf("Started creating list. Delta: %.2f, Best: %.2f, Worst: %.2f, Worst allowed: %.2f\n", delta, bestRank, worstRank, worstAllowed);
		(*rcl_size) = 0; // No elements in the RCL to begin with.
		for (int i = 0; i < rv_size; i++) { // For i = 0,...,nJobs
			if (rankValues[i] > worstAllowed - tolerance) { //If quality of solution using job i (inserting job i into nurse's route) is better than cut-off value.
				(*rcl_size)++; //Increase the size of the RCL by one, as we are going to add an element to the RCL.
				// printf("\tAdded element number %d to the list, value: %.2f\n", (*rcl_size), rankValues[i]);
				//Add job i to the RCL:
				RCL_seeds[(*rcl_size) - 1] = i; // The (*rcl_size - 1)th element of RCL_seeds is set to job i (have to use -1 because array index starts from 0 but rcl_size =1 means first element in RCL).
			}
		}
	}

	if (strategy == 3) { //debug : feature. Not implemented yet. Can use update_top_values for this.
		printf("STRATEGY 3 For creating RCL in GRASP - Not implemented yet!");
		exit(-1);
	}

	if ((*rcl_size) < 1) { //If no elements have been added to the RCL
		printf("WARNING! RCL in GRASP has no candidates!\n");
		printf("Creating list with strategy = %d and delta = %.9f\n>>\tBest: %.2f, Worst: %.2f, Worst allowed: %.2f, hardCutOff: %.2f\n", strategy, delta, bestRank, worstRank, worstAllowed, hardCutOff);
		printf("There were %d to choose from (-DBL_MAX omitted)\n", rv_size);
		for (int iii = 0; iii < rv_size; iii++) {
			if (rankValues[iii] > -DBL_MAX + 100)
				printf("\t%d\t%.2f\n", iii, rankValues[iii]);
		}
		exit(-1);
	}

} //END OF generate_rcl function.

void rcl_pick(int** bestIndices, int* RCL_seeds, int rcl_size, int* cNurse, int* cJob) {

	// This function randomly picks an element from the RCL, which returns to us the indicies for the job-nurse assignment (cNurse and cJob).
	// Picks one random element from bestIndices. The index of bestIndices comes from RCL_seeds. The elements considered of RCL_seeds are only the first rcl_size elements. Output is saved in cNurse and cJob.

	 int rd_int = pick_integer(rcl_size);
//	int rd_int = 0; //ISSUE, THIS SHOULD BE SELECTING A RANDOM INTEGER BETWEEN 0 AND rcl_size, SHOULD NOT ALWAYS BE 0 AS THIS MEANS THAT THE ELEMENT PICKED WILL ALWAYS BE THE 0TH ELEMENT OF RCL_seeds!

	int el = RCL_seeds[rd_int]; //el = element, (should be) selected at random from RCL_seeds.

	//Job-nurse assignment
	(*cNurse) = bestIndices[el][0]; //cNurse = the nurse for job 'el'
	(*cJob) = bestIndices[el][1]; //cJob = the job for job 'el' to go into cNurse's route.

	return;

} // END OF rcl_pick function.

// Deprecated with new RCL functions above (May 19)
void best_index_pick(int** bestIndices, int GRASP_param, int* cNurse, int* cJob) {
	// Identify how many VALID indices
	int ct = 0;

	for (int i = 0; i < GRASP_param; i++) {
		if ((bestIndices[i][0] >= 0) && (bestIndices[i][1] >= 0))
			ct++;
	}

	if (ct < 1) {
		printf("ERROR: In GRASP, there are no valid options to pick from.");
		printf("Because the best indices were: \n");
		printf("bestI\n");
		for (int iii = 0; iii < GRASP_param; ++iii)
			printf("nurse %d, job %d\n", bestIndices[iii][0], bestIndices[iii][1]);
		// printf("\tbestI\t\tbestVal\n");
		// for (int iii = 0; iii < GRASP_param; ++iii)
		// 	printf("\t%d\n", bestIndices[iii]);

		  // printf("\t%d\t\t%.2f\n", bestIndices[iii], bestValues[iii]);
		(*cNurse) = -1;
		(*cJob) = -1;
		return;
	}

	int choice = pick_integer(ct);
	int chosenIndex = -1;
	int ct2 = -1;
	for (int i = 0; i < GRASP_param; i++) {
		if (bestIndices[i][0] >= 0 && bestIndices[i][1] >= 0) // Valid choice, advance counter
			ct2++;

		if (ct2 >= choice) // Found the choice number we wanted
		{
			chosenIndex = i;
			break;
		}
	}
	if ((bestIndices[chosenIndex][0] < 0) || (bestIndices[chosenIndex][1] < 0))
		// if (1 > 0)
	{
		printf("ERROR: Still returning a -1!\n");
		printf("Pick was: %d\n", chosenIndex);
		printf("Choice was: %d\n", choice);
		printf("ct was %d\n", ct);
		printf("ct2 was %d\n", ct2);
		for (int iii = 0; iii < GRASP_param; ++iii)
			printf("\tNURSE %d, JOB %d\n", bestIndices[iii][0], bestIndices[iii][1]);
		exit(-1);
	}
	else
		(*cNurse) = bestIndices[chosenIndex][0];
	(*cJob) = bestIndices[chosenIndex][1];
	// return chosenIndex;
}

int pick_integer(int max_int) {
	//This function picks an integer number randomly between 0 and max_int - 1

	return (int)(rand() % (max_int));
}

void update_top_values(int GRASP_param, int** bestIndices, double* bestValues, double newValue, int bestNurse, int bestJob) {
	// printf("Proposed idx %d, proposed val %.2f\n", i, newValue);
	// printf("bestI\t\tbestVal\n");
	// for (int iii = 0; iii < GRASP_param; ++iii)
	// 	printf("%d\t\t%.2f\n", bestIndices[iii], bestValues[iii]);

	// Check if in TOP values and replace if necessary:
	for (int k = GRASP_param - 1; k >= 0; --k) {
		if ((bestIndices[k][0] < 0 && bestIndices[k][1] < 0) || (newValue > bestValues[k])) {
			for (int kk = 0; kk < k; ++kk) {
				bestValues[kk] = bestValues[kk + 1];
				bestIndices[kk][0] = bestIndices[kk + 1][0];
				bestIndices[kk][1] = bestIndices[kk + 1][1];
			}
			bestValues[k] = newValue;
			bestIndices[k][0] = bestNurse;
			bestIndices[k][1] = bestJob;
			break;
		}
	}
}

void clean_solution_from_struct(struct INSTANCE* ip) {
	for (int nurse = 0; nurse < ip->nNurses; ++nurse) {
		for (int job = 0; job < ip->nJobs; ++job) {
			ip->solMatrix[nurse][job] = -1;
		}
	}
}

int identify_early_insert(struct INSTANCE* ip, struct INSTANCE* guiding, int guiding_nurse, int guiding_nurse_pos) {
	
	//Identify if a job needs to be inserted earlier than proposed. This prevents jobs being inserted after others that are present in the same nurse and are seen to appear later in the guiding solution.
	
	int position_insert = guiding_nurse_pos;

	for (int irt = 1; irt < guiding->nJobs - guiding_nurse_pos; irt++) { //For each 
		int nxtJob = guiding->allNurseRoutes[guiding_nurse][guiding_nurse_pos + irt];
		// Check if the guiding nurse is doing any more jobs after this one
		if (nxtJob < 0) {
			break;
		}
		// Check if this nurse is doing that job
		if (ip->solMatrix[guiding_nurse][nxtJob] >= 0) {
			if (ip->solMatrix[guiding_nurse][nxtJob] < position_insert) {
				// printf("\t> Solved a problem! Job was due to be inserted at pos: %d, \n", position_insert);
				// printf("\t> but we insert it in position %d instead, to prevent overtaking job %d\n", ip->solMatrix[guiding_nurse][nxtJob], nxtJob);
				position_insert = ip->solMatrix[guiding_nurse][nxtJob];
			}
		}

	}

	if (position_insert == guiding_nurse_pos) {
		int jc = get_job_count(ip, guiding_nurse); //jc = number of jobs in guiding_nurse's route in ip solution
		if (position_insert > jc) { //If position of mvjob in guiding_nurse's route in guiding solution is greater than number of jobs in guiding_nurse's route in ip solution
			//That is, the number of jobs = number of possible positions to add a job, so if position_insert is greater than jc it means that position_insert is not in the guiding_nurse's route
			//E.g. nurse only does 5 jobs (in positions 0,1,2,3,4), but position_insert is 8, so we're trying to insert a job in position 8 when nurse doesn't even have jobs in positions 5,6, or 7.
			//Therefore, we make position_insert equal to jc, which makes position_insert be a new added position at the end of nurse's route (e.g. position 5 in this example.
			position_insert = jc;
		}
	}

	return position_insert;

} //END OF identify_early_insert function.

void find_best_worst_pool_qualities(double* pool_best, double* pool_worst, struct INSTANCE** pool, double* pool_quality, int pool_size) {
	/* Finds the best and worst qualities currently in the solution pool
	   using only the solutions that have been initialised (not NULL ptrs)
	   This code maximises, so best = max, worst = min
	*/
	*pool_best = -bigM;
	*pool_worst = bigM;
	for (int i = 0; i < pool_size; ++i) {
		if (NULL != pool[i]) {
			if (pool_quality[i] < *pool_worst)
				*pool_worst = pool_quality[i];

			if (pool_quality[i] > * pool_best)
				*pool_best = pool_quality[i];
		}
	}
}

double solution_dissimilarity(struct INSTANCE* input1, struct INSTANCE* input2) {
	/*
	Calculate a measure of dissimilarity between two solutions.	It is a combination of three parts:
	  1) Dissimilarity in the nurse order
	  2) Dissimilarity in the nurse-patient assignment
	  3) Dissimilarity in the routes (looking at arcs present in the solution, regardless of the assignment)
	This computes dissimilarity between input1 and input2, meaning it penalises the elements from input1 that are not present in input2, but ignoring elements from input2 not present in input1
	*/
	//This function calculates the dissimilarity between solutions input1 and input2, and returns the value of the dissimilarity.
	//input1 = ip, input2 = pool[ps]
	// In paper: d(S_1, S_2) = d_o(O_1,O_2) + d_a(R_1, R_2) + d_r(R_1, R_2).

	// The elements above are weighted with the weights below
	double weight1 = 1;
	double weight2 = 1;
	double weight3 = 1;

	// These are the dissimilarity values:
	int nurse_order_value = 0;
	int assignment_value = 0;
	int route_value = 0;

	// Calculate nurse dissimilarity (d_o(O_1, O_2) in paper) - Hamming distance between the ordering vectors (nurseOrder), which is the number of different digits.
	for (int i = 0; i < input1->nNurses; ++i) { //For i = 0,...,nNurses
		if (input1->nurseOrder[i] != input2->nurseOrder[i]) //If the nurse in nurseOrder[i] for input 1 is NOT the same nurse in nurseOrder[i] for input 2 (i.e. nurses are in different order)
			nurse_order_value++;
	}

	// Calculate assignment dissimilarity and arc dissimilarity
	for (int i = 0; i < input1->nNurses; ++i) { //For i = 0,...,nNurses
		for (int j = 0; j < input1->nJobs; ++j) { //For j - 0,...,nJobs
			if (input1->solMatrix[i][j] < 0) { //If in ip solution, job j is not in nurse i's route, go to next job (++j)
				continue;
			}
			if (input2->solMatrix[i][j] < 0) { //If job j IS in nurse i's route in ip soln but is NOT in nurse i's rute in pool[ps] solution, then there's a difference in the two solutions (ip and pool[ps])
				assignment_value++;
			}

			int i_des = find_arc_destination(i, j, input1); //See where this arc goes: i_des = the job# after job j in nurse i's route in ip solution, if j is the last job in nurse i's route then i_des = -1 (nurse goes back to depot).
			// So we have an arc j -> i_des, find out where this job goes to on input2

			int arc_dissimilar = 1; // Assume the arc is not there (unless we find it)
			for (int nu = 0; nu < input2->nNurses; ++nu) { //For each nurse nu = 0,...,nNurses (for pool[ps])
				if (input2->solMatrix[nu][j] > -1) { //If job j is in nurse nu's route in pool[ps] solution
					int i_des2 = find_arc_destination(nu, j, input2); // i_des2 = the job# after job j in nurse nu's route in pool[ps] soln, if j is the last job in nu's route then i_des2 = -1 (nu returns back to depot)

					//If there's a job after j in nu's route AND the job after j in nu's route is the same job as the job after j in nurse i's route (i.e. both nurse i and nurse nu have the same job j
					// in their routes AND the job AFTER job j in both of their routes is the SAME job - so both nurse's have job j going to job k in their routes)
					// OR if there is NO job after job j in nurse i's route AND there's NO job after job j in nurse nu's route AND nurse i and nurse nu are the SAME nurses
					// (i.e. the job j in both nurse i and nurse nu's route are the last job in their route, and the nurse's are actually the same nurses!)
					if (((i_des2 != -1) && (i_des2 == i_des)) || ((i_des == -1) && (i_des2 == -1) && (nu == i))) { //Then there's no dissimilarity!
						arc_dissimilar = 0;
						break;
					}
					else { //There's a dissimilarity
						if (input1->doubleService[j]) { //If job j is a double service, continue (go to next nurse nu, (++nu))
							continue;
						}
						else { //If j is not a double service, break out of the for nu loop.
							break;
						}
					}
				}
			} // End for (int nu = 0; nu < input2->nNurses; ++nu) - looking for arcs in input2

			route_value += arc_dissimilar; //Number of arc dissimilarities

			// If this was the nurses's first visit (value 0), we also check (nurse depot -> point) similarity
			if ((input1->solMatrix[i][j] == 0) && (input2->solMatrix[i][j] != 0)) { //If job j is nurse i's first job in ip solution AND job j is not nurse i's first job in pool[ps] solution
				route_value++; //Another arc dissimilarity has been found!
			}

		}  // End for (int j = 0; j < input1->nJobs; ++j)      
	} // End for (int i = 0; i < input1->nNurses; ++i)

	// d(S_1, S_2) = d_o(O_1,O_2) + d_a(R_1, R_2) + d_r(R_1, R_2)
	double d_value = weight1 * nurse_order_value + weight2 * assignment_value + weight3 * route_value;

	return(d_value);

} //END OF solution_dissimilarity function.

int find_arc_destination(int source_nurse, int source_job, struct INSTANCE* ip) {
	/*
	  Given a nurse/job combination, find out where the nurse goes next to (the arc)
	  If there is no "next job" we assume it goes back to depot, noted by "-1"
	*/
	// This function find the job in the next position of source_nurse's route after the job source_job.
	// If there is a job after source_job in source_nurse's route, then this function returns that job, otherwise it returns -1 (which means that there are no other jobs in source_nurse's route,
	// source_job is the last job in source_nurse's route and so the nurse ends up returning to the depot).

	int c_pos = ip->solMatrix[source_nurse][source_job]; //c_pos position of job in nurse's route, 'current position'
	/*int n_pos = c_pos++;*/ /// 'next position'. Increment is AFTER, so n_pos = c_pos, and THEN c_pos is increased by one, should this be ++c_pos or just c_pos + 1 instead as we're trying to find the NEXT position in the route?
	int n_pos = c_pos+1; // Changed from c_pos++ to cpos+1, 26/12/2020.
	int d_pos = -1; // If we don't find the following point, assume it goes back to depot
	for (int i = 0; i < ip->nJobs; ++i) { //For each job i = 0,...,nJobs
		if (ip->solMatrix[source_nurse][i] == n_pos) { //If the position of job i in nurse's route is in the NEXT position of nurse's route after position c_pos
			d_pos = i; //Store the job i as d_pos
			break;
		}
	}

	return(d_pos);

} //END OF find_arc_destination function.

void print_and_recalculate_pool_contents(struct INSTANCE** pool, double* pool_quality, int solutions_in_pool) {
	printf("Pool contents:\n");
	for (int ps2 = 0; ps2 < solutions_in_pool; ps2++) {
		if (pool[ps2] != NULL)
			printf("\tPOOL %d - Q = %.2f (rec %.2f)\n", ps2, sol_quality(pool[ps2], -3234), pool_quality[ps2]);
	}
}

void calculate_and_print_dissimilartiy_matrix(struct INSTANCE** pool, int solutions_in_pool) {
	printf("Pool dissimilarity matrix:\n\t");
	for (int ps3 = 0; ps3 < solutions_in_pool; ps3++)
		printf("POOL%d\t", ps3);

	for (int ps3 = 0; ps3 < solutions_in_pool; ps3++) {
		printf("\nPOOL %d", ps3);
		for (int ps4 = 0; ps4 < solutions_in_pool; ps4++) {
			if ((pool[ps3] != NULL) && (pool[ps4] != NULL))
				printf("\t%.1f", solution_dissimilarity(pool[ps3], pool[ps4]));
			else
				printf("\t----");
		}
	}
}

double forward_path_relinking(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output) {
	return directed_path_relinking(input1, input2, q1, q2, output, 0);
}

double forward_and_backward_path_relinking(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output1) {
	/*
	  Performs both, forward and backward path relinking and
	  returns the relinked solution with the highest quality
	  from the two
	*/

	struct INSTANCE output2 = copy_instance(input1);
	double rq_forward = forward_path_relinking(input1, input2, q1, q2, output1);
	double rq_backward = backward_path_relinking(input1, input2, q1, q2, &output2);
	double best_relinking_quality = rq_forward;
	if (rq_backward > rq_forward) {
		overwrite_instance(output1, &output2);
		best_relinking_quality = rq_backward;
	}

	free_instance_copy(&output2);

	return best_relinking_quality;
}

double backward_path_relinking(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output) {
	// Performs backwards path relinking
	// Starts from the best solution and goes towards the worst
	// Result is saved on "output" (this is always equal or better than the best of q1 or q2)
	return directed_path_relinking(input1, input2, q1, q2, output, 1);
}

double directed_path_relinking(struct INSTANCE* input1, struct INSTANCE* input2, double q1, double q2, struct INSTANCE* output, int direction) {

	// This function performs PR in the requested direction, and returns the quality of the best solution created.
	/*
	  Performs path relinking forward or backward, depending on the parameter "direction"
	  direction = 0 -> forward
	  direction = 1 -> backward
	  direction = 2 -> forward and backward
	*/
	//input1 = ip, input2 = pool[ps], q1 = currentQuality, q2 = quality of pool[ps] (pool_quality[ps]), output = &relinkedSolution, direction = PR_DIRECTION.

	double relinking_quality = 0;

	if (direction < 2) { //If direction is forward (0) or backward (1)
		int source_sol_1 = 0;
		if ((direction == 0 && q1 > q2) || (direction == 1 && q1 < q2)) { //(forward direction AND current soln quality better than ps soln quality) OR (backward direction AND current quality worse than ps quality)
			source_sol_1 = -1;
		}
		if (source_sol_1 > -1) { /// Should this be >-1? This will never occur, as source_sol_1 is only either 0 or -1, so can never be > 0. Changed from 0 to -1, 26/12/2020.
			overwrite_instance(output, input1); //relinkedSolution = ip solution
			relinking_quality = path_relinking(output, input2); //PR with relinkedSolution as starting solution and pool[ps] solution as guiding solution.
		}
		else {
			overwrite_instance(output, input2); //relinkedSolution = pool[ps] solution
			relinking_quality = path_relinking(output, input1); //PR with relinkedSolution as starting solution as ip solution as guiding solution.
		}
	}
	else if (direction == 2) {
		relinking_quality = forward_and_backward_path_relinking(input1, input2, q1, q2, output); //Does both forward and backward PR and returns the quality of the best solution found.
	}
	else {
		printf("ERROR: Unknown PR direction %d in directed_path_relinking(...)\n", direction);
		exit(-23483);
	}

	return relinking_quality;

} //END OF directed_path_relinking function.

double path_relinking(struct INSTANCE* ip, struct INSTANCE* guiding) {
	
	// Modify ip iteratively so that it ends being identical to "guiding". Save the best solution found during the process, apply local search to it and return it in "ip" again.
	// debug: missing: need to modify ip right at the start to prevent moving jobs between nurses that are "interchangeable" (same skills, times and start locations), 
	// this will result in much less work for the relinking

	// This function takes a solution ip and uses small movements to modify ip iteratively so that eventually, ip is equivalent to the guiding solution.
	// Movements: change nurse order (swap order of two nurses in nurseOrder array so that a nurse's position in ip matches the nurse's position in guiding solution,
	// or remove a job from a nurse's route in ip and reinsert the job in another nurse's route in ip to match the position of that job in the guiding soltuion.
	// The quality of ip is calculated after every improvement, and the improvements continue to be made until ip = guiding solution.
	// Then, the best intermediate solution ip found in the path is taken as the best solution, and ip becomes this best solution.
	// The aim is to (hopefully) find an intermediate solution within the path from ip to the guiding solution that has better quality solution than both the original solution ip AND the guiding solution.
	// If the best intermediate solution ip found is not better than both the original solution AND the guiding solution, then ip stays the same as it was originally (starting_solution).
	// Otherwise, the best intermediate solution ip found in the path is taken to be the best solution, and ip = best intermediate solution
	// This function returns 'ach' - which is the quality of the ip solution created using this function (either quality of new, better ip solution or the quality of the original ip solution).
	
	//ip = relinkedSolution (either ip or pool[ps]), guiding = either pool[ps] or ip
	//so either modifying ip toward pool[ps], or modifying pool[ps] toward ip.

	struct INSTANCE bestSolution = copy_instance(ip); //Best intermediate solution
	struct INSTANCE starting_solution = copy_instance(ip); //Keep a copy of the original ip solution before any modifications, as we might have to revert back to this solution if no improvements made.
	double starting_quality = sol_quality(ip, -1476879); // Quality of original ip solution (starting_solution)

	int nmoves = guiding->nNurses + guiding->nJobs; //n+m in paper, max number of moves
	int* performed = malloc(nmoves * sizeof(int)); //size = 1 x nMoves (movements that have been performed), 0-1
	double* all_moves_quality = malloc(nmoves * sizeof(double)); // size = 1 x nMoves (quality of movements that have been performed)

	int pr_failed = 1; // = -1 if PR successful, = 1 if failed
	int pr_rounds = 0; //Number of iterations
	int PR_ROUNDS_MAX = 10; // Max number of iterations - to avoid infinite loop if there is an error below (a couple of rounds should suffice)
	double tolerance = 1e-6;
	double gqual = -DBL_MAX; //Quality of guiding solution
	double finish_sol_q = -DBL_MAX; //Quality of ip during relinking
	double best_inst_quality = -DBL_MAX;

	while (pr_failed > 0 && pr_rounds < PR_ROUNDS_MAX) { //while PR has not failed and max number of iterations has not been exceeded
		pr_failed = -1;
		for (int i = 0; i < nmoves; i++) { //Reset, no movements performed
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
			for (int mv = 0; mv < nmoves; mv++) { //For mv=0,...,nmoves
				if (performed[mv]) { //If move has been performed, continue onto next mv (++mv)
					continue;
				}
				if (mv < guiding->nNurses) { //Then we can do nurseOrder swap
					if (ip->nurseOrder[mv] == guiding->nurseOrder[mv]) { //same nurse in same position in both solutions, move not necessary (nothing to move!)
						performed[mv] = 1;
						moves_performed++;
						continue; //Go to next mv (++mv)
					}
					//Otherwise, if nurses are not in the same position in nurseOrder array in both solutions, need to swap two elements in nurseOrder for ip so that ip->nurseOrder[mv] == guiding->nurseOrder[mv]
					int A = ip->nurseOrder[mv]; //nurse in position mv of nurseOrder for ip
					int A_pos = mv; //position in nurseOrder for ip
					int B = guiding->nurseOrder[mv]; //nurse in position mv of nurseOrder for guiding
					int B_pos = -1; //position in nurseOrder for guiding, set to -1
					int new_pos_mv = guiding->nurseOrder[mv]; //nurse in position mv of nurseOrder for guiding
					for (int el = 0; el < guiding->nNurses; el++) { //For each position el = 0,...,nNurses
						if (el == mv) { //If position el is the same as position mv, go to next position el (++el)
							continue;
						}
						if (ip->nurseOrder[el] == B) { //If nurse in position el is the same as the nurse in position mv of nurseOrder for guiding, save the position el
							B_pos = el;
							break;
						}
					}
					// Now, switch the position of the nurse in position B_pos (el) of nurseOrderip with the position of the nurse in position A_pos (mv) of nurseOrderip
					// Then, ip->nurseOrder[mv] == guiding->nurseOrder[mv]!
					ip->nurseOrder[B_pos] = A;
					ip->nurseOrder[A_pos] = B;

					double move_quality = sol_quality(ip, -3900090);
					if (move_quality > best_move_quality) { //If quality has improved, update best_move_quality and save the best position in nurseOrder to switch
						best_move_to_perform = mv; 
						best_move_quality = move_quality;
					}
					//Return nurseOrder in ip back to before
					ip->nurseOrder[B_pos] = B;
					ip->nurseOrder[A_pos] = A;

				} //End if(mv < guiding->nNurses)

				else { // All other moves (can't do nurses because mv >= nNurses, so that number of nurses doesn't exist in nurseOrder).
					int mvjob = mv - guiding->nNurses; //0,...,nJobs
					if (ip->doubleService[mvjob] > 0) { //If mvjob is a double service 
						int guiding_nurse1 = -1; // First nurse that does job 'mvjob'
						int guiding_nurse_pos1 = -1; //Position of 'mvjob' in first nurse's route
						int guiding_nurse2 = -1; //Second nurse that does job 'mvjob'
						int guiding_nurse_pos2 = -1; //Position of 'mvjob' in second nurse's route
						//This function returns, for the guiding solution, the two nurses that do the job 'mvjob', and the position of 'mvjob' in each nurse's route
						get_nurse_and_position_of_job_ds(guiding, mvjob, &guiding_nurse1, &guiding_nurse_pos1, &guiding_nurse2, &guiding_nurse_pos2);

						int current_nurse1 = -1; 
						int current_nurse_pos1 = -1;
						int current_nurse2 = -1;
						int current_nurse_pos2 = -1;
						//This function returns, for the ip solution, the two nurses that do the job 'mvjob', and the position of 'mvjob' in each nurse's route
						get_nurse_and_position_of_job_ds(ip, mvjob, &current_nurse1, &current_nurse_pos1, &current_nurse2, &current_nurse_pos2);

						// Check for coincidence, this can only happen if nurse1 is nurse1 in the other sol, as they are searched in increasing order, so no need to check if guiding_nurse1 = current_nurse2 
						// because in that case it will happen that guiding_nurse2 > current_nurse1 by def.
						if (((guiding_nurse1 == current_nurse1) && (current_nurse_pos1 == guiding_nurse_pos1)) && ((guiding_nurse2 == current_nurse2) && (current_nurse_pos2 == guiding_nurse_pos2))) {
							// They're the same, so don't need to move anything!
							performed[mv] = 1;
							moves_performed++;
							continue;
						}

						// Test the move:
						remove_job(ip, mvjob, current_nurse1); //Remove mvjob from both nurse's routes in ip solution
						remove_job(ip, mvjob, current_nurse2);
						int position_insert1 = identify_early_insert(ip, guiding, guiding_nurse1, guiding_nurse_pos1); //Returns updated position guiding_nurse_pos1
						int position_insert2 = identify_early_insert(ip, guiding, guiding_nurse2, guiding_nurse_pos2); //Returns updated position guiding_nurse_pos2
						//If cannot insert mvjob into position 'position_insert1' in guiding_nurse1's route in ip OR cannot insert mvjob into position_insert2 in guiding_nurse2's route in ip:
						if ((insert_job_at_position(ip, mvjob, guiding_nurse1, position_insert1) < 0) || (insert_job_at_position(ip, mvjob, guiding_nurse2, position_insert2) < 0)) {
							all_moves_quality[mv] = -DBL_MAX;
							printf("ERROR: This shouldn't happen!!!!!! PR cannot match guiding solution. DS\n");
							printf("Trying to get job %d into nurse %d position %d\n", mvjob, guiding_nurse1, position_insert1);
							printf("AND: job %d into nurse %d position %d\n", mvjob, guiding_nurse2, position_insert2);
							printf("This is SOLMATRIX after the trials:\n");
							print_solmatrix(ip);
							printf("Inserting back for debugging...\n");
							insert_job_at_position(ip, mvjob, current_nurse2, current_nurse_pos2);
							insert_job_at_position(ip, mvjob, current_nurse1, current_nurse_pos1);
							print_solmatrix(ip);
							exit(-23535);
							continue;
						}

						// Successfully inserted mvjob into positions in guiding_nurse1 and guiding_nurse2's routes.
						double move_quality = sol_quality(ip, -3900090);
						if (move_quality > best_move_quality) { //If quality has improved, update best_move_quality and save the best mv
							best_move_to_perform = mv;
							best_move_quality = move_quality;
						}

						// Return ip solution back to before, first by removing mvjob from guiding_nurse's routes and then adding mvjob back into current_nurse's routes.
						remove_job(ip, mvjob, guiding_nurse1);
						remove_job(ip, mvjob, guiding_nurse2);
						if ((insert_job_at_position(ip, mvjob, current_nurse1, current_nurse_pos1) < 0) || (insert_job_at_position(ip, mvjob, current_nurse2, current_nurse_pos2) < 0)) {
							printf("ERROR: We have messed it up!\nCannot insert job %d back into nurse %d or %d\n", mvjob, current_nurse1, current_nurse2);
							exit(-32543543);
						}
					} //End if (ip->doubleService[mvjob] > 0)
					else { // Single service
						int guiding_nurse = -1; //Nurse in guiding solution that does mvjob
						int guiding_nurse_pos = -1; //Position of mvjob in guiding_nurse's route in guiding solution
						//This function returns the nurse in guiding solution that does mvjob, and the positon of mvjob in that nurse's route
						get_nurse_and_position_of_job(guiding, mvjob, &guiding_nurse, &guiding_nurse_pos);

						int current_nurse = -1; //Nurse in ip solution that does mvjob
						int current_nurse_pos = -1; //Position of mvjob in current_nurse's route in ip solution
						//This function returns the nurse in ip solution that does mvjob, and the position of mvjob in that nurse's route
						get_nurse_and_position_of_job(ip, mvjob, &current_nurse, &current_nurse_pos);

						double move_quality = -DBL_MAX;
						if ((guiding_nurse == current_nurse) && (current_nurse_pos == guiding_nurse_pos)) { //If same nurse is doing the same job in the same position of the route in both solutions, don't need to do anything!
							performed[mv] = 1;
							moves_performed++;
							continue;
						}
						else {
							remove_job(ip, mvjob, current_nurse); //Remove mvjob from current_nurse's route in ip solution
							int position_insert = identify_early_insert(ip, guiding, guiding_nurse, guiding_nurse_pos); //Return updated position
							if (insert_job_at_position(ip, mvjob, guiding_nurse, position_insert) < 0) { //If mvjob cannot be inserted into guiding_nurse's route in position_insert of ip solution
								all_moves_quality[mv] = -DBL_MAX;
								insert_job_at_position(ip, mvjob, current_nurse, current_nurse_pos); //Revert ip solution back to before.
								// May not be an error if guiding_nurse_pos is too large (which might happen, we just need to wait in that case)
								// printf("ERROR: This shouldn't happen!!!!!! PR cannot match guiding solution. Single Serv.\n");
								continue;
							}
							move_quality = sol_quality(ip, -3900090); //Calculate quality of ip solution
						}
						if (move_quality > best_move_quality) { //If quality has improved, update best_move_quality and save the best mv
							best_move_to_perform = mv;
							best_move_quality = move_quality;
						}
						remove_job(ip, mvjob, guiding_nurse); //Revert ip back to before
						if (insert_job_at_position(ip, mvjob, current_nurse, current_nurse_pos) < 0) { //Insert mvjob back into current_nurse, shouldn't this be < 0? Changed 26/12/2020, added <0 to statement.
							printf("ERROR: We have messed it up!\nCannot insert job %d back into nurse %d\n", mvjob, current_nurse);
							exit(-32543543);
						}
					} //End else single service
				} //End else all other moves
			} // End for (int mv = 0; mv < nmoves; mv++) loop

			// Perform the best move for real:
			if (best_move_to_perform >= 0) {
				if (best_move_to_perform < guiding->nNurses) { //If the move to be performed is nurseOrder swap
					int A = ip->nurseOrder[best_move_to_perform];
					int A_pos = best_move_to_perform;
					int B = guiding->nurseOrder[best_move_to_perform];
					int B_pos = -1;
					int new_pos_best_move_to_perform = guiding->nurseOrder[best_move_to_perform];
					for (int el = 0; el < guiding->nNurses; el++) {
						if (el == best_move_to_perform) {
							continue;
						}
						if (ip->nurseOrder[el] == B) {
							B_pos = el;
							break;
						}
					}
					// Perform nurseOrder swap in ip solution
					ip->nurseOrder[B_pos] = A;
					ip->nurseOrder[A_pos] = B;
				}
				else { //The move to be performed is a job removal/reinsertion
					int mvjob = best_move_to_perform - guiding->nNurses;
					if (ip->doubleService[mvjob] > 0) { //If mvjob is a double service
						int guiding_nurse1 = -1;
						int guiding_nurse_pos1 = -1;
						int guiding_nurse2 = -1;
						int guiding_nurse_pos2 = -1;
						//Get the two nurses in guiding solution that are assigned mvjob, and get the position of mvjob in each of two nurses' routes.
						get_nurse_and_position_of_job_ds(guiding, mvjob, &guiding_nurse1, &guiding_nurse_pos1, &guiding_nurse2, &guiding_nurse_pos2);

						int current_nurse1 = -1;
						int current_nurse_pos1 = -1;
						int current_nurse2 = -1;
						int current_nurse_pos2 = -1;
						//Get the two nurses in ip solution that are assigned mvjob, and get the position of mvjob in each of the two nurses' routes.
						get_nurse_and_position_of_job_ds(ip, mvjob, &current_nurse1, &current_nurse_pos1, &current_nurse2, &current_nurse_pos2);

						//Remove mvjob from the two nurses' routes in ip solution
						remove_job(ip, mvjob, current_nurse1);
						remove_job(ip, mvjob, current_nurse2);

						// Check positions
						int position_insert1 = identify_early_insert(ip, guiding, guiding_nurse1, guiding_nurse_pos1);
						int position_insert2 = identify_early_insert(ip, guiding, guiding_nurse2, guiding_nurse_pos2);

						//If could not insert mvjob into guiding_nurse1's route in ip solution OR could not insert mvjob into guiding_nurse2's route in ip solution:
						if ((insert_job_at_position(ip, mvjob, guiding_nurse1, position_insert1) < 0) || (insert_job_at_position(ip, mvjob, guiding_nurse2, position_insert2) < 0)) {
							insert_job_at_position(ip, mvjob, current_nurse1, current_nurse_pos1); //Reinsert mvjob into the original positions of the original nurses (revert solution back to before)
							insert_job_at_position(ip, mvjob, current_nurse2, current_nurse_pos2);
							printf("ERROR: This shouldn't happen!!!!!! PR cannot match guiding solution. DS\n");
							exit(-23535);
							continue;
						}
						//Successfully inserted mvjob into positions in guiding_nurse1 and guiding_nurse2's routes in ip solution
					}
					else { //mvjob is a single service
						int guiding_nurse = -1;
						int guiding_nurse_pos = -1;
						get_nurse_and_position_of_job(guiding, mvjob, &guiding_nurse, &guiding_nurse_pos); //Get nurse that does mvjob in guiding solution and the position of mvjob in that nurse's route

						int current_nurse = -1;
						int current_nurse_pos = -1;
						get_nurse_and_position_of_job(ip, mvjob, &current_nurse, &current_nurse_pos); //Get nurse that does mvjob in ip solution and the position of mvjob in that nurse's route

						remove_job(ip, mvjob, current_nurse); //Remove mvjob from current_nurse's route in ip solution

						int position_insert = identify_early_insert(ip, guiding, guiding_nurse, guiding_nurse_pos); //Returns updates position
						if (insert_job_at_position(ip, mvjob, guiding_nurse, position_insert) < 0) { //If cannot insert mvjob into guiding_nurse's route in position_insert in ip solution
							printf("ERROR: We have messed it up!\nCannot perform the move we thought was best! (job %d to nurse %d)\n", mvjob, current_nurse);
							exit(-32003);
						}
						//Successfully inserted mvjob into position in guiding_nurse's route in ip solution.
					} //End else mvjob is single service
				} //End else the move to be performed is a job removal/reinsertion

				// Record the move and assess performance
				performed[best_move_to_perform] = 1;
				moves_performed++;
				double fq = sol_quality(ip, -54911); //Quality of solution after actual move performed.
				if (fq > best_inst_quality) { //If true, then this is the best solution of the path so far!
					overwrite_instance(&bestSolution, ip); //bestSolution = ip
					best_inst_quality = best_move_quality; //best_move_quality should be the same as fq.
				}

			} // End if (best_move_to_perform >= 0)
		} // End of while(moves_performed < nmoves)

		finish_sol_q = sol_quality(ip, -142381); //Quality of ip solution
		gqual = sol_quality(guiding, -14923); //Quality of guiding solution
		/// Should also add a check to make sure that the quality of the guiding solution is not changing.
		if (abs(finish_sol_q - gqual) > tolerance) { //If there is a difference between the quality of the ip and guiding solutions, then ip still hasn't reached the guiding solution yet, need another loop of PR
			pr_failed = 1; 
			pr_rounds++;
			if (pr_rounds < PR_ROUNDS_MAX) { //If not yet reached the maximum number of PR iterations, do another iteration of the while (pr_failed > 0 && pr_rounds < PR_ROUNDS_MAX) loop.
				continue;
			}
			else { //Reached the max number of PR iterations, cannot continue.
				printf("Path relinking did not work as expected.\n");
				printf("Start %.2f Target %.2f (Finished %.2f)\n", starting_quality, gqual, finish_sol_q);
				printf("Finished stuff:\nNurse Order:\n");
				print_int_matrix_one(ip->nurseOrder, 1, ip->nNurses);
				printf("Sol Matrix:\n");
				print_solmatrix(ip);
				printf("Guiding stuff:\nNurse Order:\n");
				print_int_matrix_one(guiding->nurseOrder, 1, guiding->nNurses);
				printf("Sol Matrix:\n");
				print_solmatrix(guiding);
				exit(-126835);
			}
		}

	} //End of while (pr_failed > 0 && pr_rounds < PR_ROUNDS_MAX) loop


	// Going back to the best solution we found:
	overwrite_instance(ip, &bestSolution); //ip = bestSolution (make ip be the best solution found)
	double best_along_q = sol_quality(ip, -143053); //Quality of ip

	// Perform local search on ip
	standard_local_search(ip, 10000, 60); //standard_local_search_test but only ONE iteration

	double ach = sol_quality(ip, -14357921); //Quality of ip after LS

	//If quality of ip now is better than initial quality of ip (at beginning of function before any changes were made) AND quality of ip now is better than guiding solution
	if ((ach > starting_quality + tolerance) && (ach > gqual + tolerance)) { 
		// PR WORKED!
		// printf("> PR worked: Start %.2f Target %.2f -> Achieved %.2f\t\t(Finished %.2f, Best along %.2f)\n", starting_quality, gqual, ach, finish_sol_q, best_along_q);
		// printf("Quality of finishing solution: %.2f\n", finish_sol_q);
		// printf("Quality of best along the search: %.2f\n", best_along_q);
		// printf("Done.\nPath relinking finished.\n");
		// printf("Starting quality: %.2f\n", starting_quality);
		// printf("Target: %.2f\n", gqual);
		// printf("Achieved: %.2f\n", ach);
	}
	else { //Quality of ip solution has not improved
		// printf("> PR did NOT work: Start %.2f Target %.2f -> Achieved %.2f\t\t(Finished %.2f, Best along %.2f)\n", starting_quality, gqual, ach, finish_sol_q, best_along_q);    
		overwrite_instance(ip, &starting_solution); //Restore ip back to the solution it was at the beginning of the function (before any changes were made to ip)
		ach = starting_quality; //Set ach = quality of original ip solution (before any changes were made to ip)
	}
	
	//Deallocate memory:
	free_instance_copy(&bestSolution);
	free_instance_copy(&starting_solution);
	free(performed);
	free(all_moves_quality);

	return ach;

} //END OF path_relinking function.

void get_nurse_and_position_of_job(struct INSTANCE* ip, int job, int* nurse, int* position) {

	// This function finds the nurse that does a given 'job', and returns the nurse that does the 'job' and the position in the nurse's route that the job is in.

	for (int nurse_g = 0; nurse_g < ip->nNurses; nurse_g++) {
		if (ip->solMatrix[nurse_g][job] >= 0) {
			(*nurse) = nurse_g;
			(*position) = ip->solMatrix[nurse_g][job];
			break;
		}
	}

} //END OF get_nurse_and_position_of_job function.

void get_nurse_and_position_of_job_ds(struct INSTANCE* ip, int job, int* nurse1, int* position1, int* nurse2, int* position2) {

	// For a given double service 'job', this function finds the two nurses that have been assigned the job and returns the nurse numbers (nurse1 and nurse2)
	// and the position of the job in each nurse's route (position1 and position 2)

	int onFirst = 1;
	for (int nurse_g = 0; nurse_g < ip->nNurses; nurse_g++) { //For each nurse_g=0,...,nNurses
		if (ip->solMatrix[nurse_g][job] >= 0) { //If job in in nurse_g's route
			if (onFirst > 0) { 
				(*nurse1) = nurse_g; //nurse_g does the job, store it as nurse1
				(*position1) = ip->solMatrix[nurse_g][job]; //position1 = position of job in nurse_g's (nurse1's) route.
				onFirst = -1000;
			}
			else {
				(*nurse2) = nurse_g; //other nurse_g does the job with nurse1, store is as nurse2
				(*position2) = ip->solMatrix[nurse_g][job]; //position2 = position of job in nurse2's route.
				break;
			}
		}
	}

} //END OF get_nurse_and_position_of_job_ds function.

void standard_local_search(struct INSTANCE* ip, int MAX_ITERATIONS, double MAX_TIME) {
	//This function runs standard_local_search_test but with only ONE iteration.
	//Note that the parameter MAX_ITERATIONS is not used in the standard_local_search_test function.
	standard_local_search_test(ip, MAX_ITERATIONS, MAX_TIME, 1);
}

double standard_local_search_test(struct INSTANCE* ip, int MAX_ITERATIONS, double MAX_TIME, int TEST_ITERATIONS) {

	// This function is called after each iteration of RCA (Randomized Constructive Algorithm) and applies a local search procedure to the resulting solution of RCA to improve its quality.
	// The LS is a first-improvement VNS framework - useful because it can incorporate multiple different local search techniques/movements.
	// The LS operates by going through the list of neighbourhoods (types of LS techniques) one by one, and trying to improve the quality of the solution.
	// If the quality of the solution has not improved using that neighbourhood, then the procedure moves on to the next neighbourhood to try and improve the solution.
	// Within any neighbourhood, if the quality of the solution has improved, then the LS procedure goes back to the first neighbourhood in the list and starts the process again.
	// This is repeated until all neighbourhoods (LS techniques) have been applied to a solution and the solution has not improved.
	// The neighbourhoods used in this LS, in order, are as follows:
	// NE01 - Moving a job (best_switch): Move a single job from one nurse's route and insert the job into a better position in another nurse's (or the same nurse's) route.
	// NE02 - Exchanging two services (route_two_exchange): Swap two jobs between two nurses (that is, one job from one nurse's route is swapped with another job from another nurse's route).
	// NE03 - Nurse order change (nurse_two_exchange): Swaps the order of two nurses in the nurseOrder array.
	// NE04 - Remove and reinsert linked services (best_sync_double_service): Removes a double service job from two nurses and inserts the job into better positions in two other nurse's routes OR
	// removes dependent jobs from two nurses and inserts the jobs into better positions in two other nurse's routes (could be the same nurses in both cases but better positions).
	// NE05 - 2-OPT (two_opt_move): Reverse the order of jobs between and including selected positions in a single nurse's route.
	// Function returns the quality of the final solution (retVale) (Note that if the while loop is still running but the number of iterations of the loop is ==TEST_ITERATIONS, 
	// then the function returns the quality of the solution in the TEST_ITERATIONS'th iteration, it does not continue to find a better solution).

	// Parameter MAX_ITERATIONS is never used.
	
	double eps = 1e-6; // Add this small number to prevent numerical errors when reading params
	double retValue = -11;
	int test_assigned = 0;
	int useTwoOpt = (int)(ip->algorithmOptions[1] + eps); //aO[1] = 1, so NE05 2-OPT active.
	int useTwoExchange = (int)(ip->algorithmOptions[2] + eps); //aO[2] = 1, so NE02 2-exchange active.
	int useNurseOrderChange = (int)(ip->algorithmOptions[3] + eps); //aO[3] = 0, NE03 nurse order change not active. (why?)
	double quality = sol_quality(ip, -15); //Initial solution quality of ip.	
	int performedSwaps = 0;
	int performedSwitches = 1000;
	int twoOptDidSomething = 0;
	float elapsedTime = 0; // Not used at the moment!!!!
	int infOnly = 1;
	int lsIters = -1; // Number of local search iterations.
	int backandforceTrials = bigM; //never used

	// Test performance
	double propQ = 0.0;
	int n1_improvements = 0;
	int n2_improvements = 0;
	int n3_improvements = 0;
	double n1_imp_amount = 0;
	double n2_imp_amount = 0;
	double n3_imp_amount = 0;

	clock_t start = clock();
	clock_t end = clock();

	while (1 > 0) { //while true loop
		lsIters++;

		// Perform a swap: pick two nurses at random and a job at random.
		//int n1 = rand() % ip->nNurses; //not used
		//int n2 = rand() % ip->nNurses; //not used
		//int j1 = rand() % ip->nJobs; // not used
		infOnly = 0;
		twoOptDidSomething = 1;
		double b_baseQuality = sol_quality(ip, -16);

		if (lsIters == TEST_ITERATIONS) { //If the number of iterations is equal to TEST_ITERATIONS (which is 100), the return value is set as the quality of the current solution ip
			retValue = b_baseQuality; 
			test_assigned = 1;
		}
		quality = b_baseQuality; //quality = solution quality of ip

		if (ip->verbose > 5) {
			printf("Best switch (start quality: %.2f)\n", b_baseQuality);
		}

		end = clock();
		elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
		if (elapsedTime > MAX_TIME) { //Exceeded time limit, exit while loop
			break;
		}

		/// NE01: best_switch
		int bswitchValue = best_switch(ip, infOnly, MAX_TIME - elapsedTime); //best_switch attempts to move a single job from one nurse's route to another, =0 if successful, =-1 otherwise.
		if (bswitchValue > -1) { //If switch successful and solution quality has improved
			performedSwaps++;
			n1_improvements++;
			propQ = sol_quality(ip, -17); //quality of new solution
			n1_imp_amount = quality - propQ; //improvement amount
			quality = propQ; //quality is updated to the quality of the new solution.
			if (ip->verbose > 5) {
				printf("best_switch improved to > %.2f\n", quality);
			}
			continue; //Start while loop again - start from first neighbourhood (this one!)
		}

		/// NE02 (2-exchange): route_two_exchange
		if (useTwoExchange > 0) {
			end = clock();
			elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
			if (elapsedTime > MAX_TIME) { //Exceeded time limit, exit while loop
				break;
			}
			if (ip->verbose > 5) {
				printf("Two exchange (start quality: %.2f)\n", quality);
			}
			int twoExchangeValue = route_two_exchange(ip, 1); //Swaps two jobs between two nurses (one job from one nurse is swapped with one job from another nurse), =0 if successful, =-1 otherwise.
			if (twoExchangeValue > -1) { //If swap successful and solution quality has improved
				propQ = sol_quality(ip, -1711);
				quality = propQ;
				if (ip->verbose > 5) {
					printf("two_exchange improved to > %.2f\n", quality);
				}
				continue; //Start while loop again - start from first neighbourhood best_switch
			}
		}

		/// NE03 (Nurse order change): nurse_two_exchange
		if (useNurseOrderChange > 0) {
			end = clock();
			elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
			if (elapsedTime > MAX_TIME) { //Exceeed time limit, exit while loop
				break;
			}
			int didItWork = nurse_two_exchange(ip); //Swaps two nurses in nurseOrder array, =1 if nurses swapped and solution improved, =-1 otherwise.
			if (didItWork > -1) { //If two nurses have been swapped and the solution quality has improved
				quality = sol_quality(ip, -1710);
				continue; //Start while loop again - start from the first neighbourhood best_switch
			}
		}

		/// NE04 (Best sync double switch): best_sync_double_switch
		if (ip->verbose > 5) {
			printf("\tBest_sync_double_switch (start quality: %.2f)\n", sol_quality(ip, -18));
		}	
		if (best_sync_double_switch(ip) > 0) { //If double service/dependent jobs switched successfully
			end = clock();
			elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
			if (elapsedTime > MAX_TIME) { //Exceeded time limit, exit while loop
				break;
			}
			if (ip->verbose > 5) {
				printf("\tbest_sync_double_switch improved to > %.2f\n", sol_quality(ip, -19));
			}
			propQ = sol_quality(ip, -20);
			n2_improvements++;
			n2_imp_amount = quality - propQ;
			quality = propQ;
			continue; //Start while loop again - start from the first neighbourhood best_switch
		}

		/// NE05 (2-OPT): two_opt_move
		if (ip->verbose > 5) {
			printf("\t\t2opt (start quality: %.2f)\n", sol_quality(ip, -21));
		}
		twoOptDidSomething = 0;
		if (useTwoOpt > 0.5) {
			double TOL = 0.1;
			double startoffwith = quality; //Initial quality of solution before changes are made

			for (int nurseidx = 0; nurseidx < ip->nNurses; nurseidx++) { //For each nurseidx=0,...,nNurses
				int nurse = ip->nurseOrder[nurseidx]; 
				int foundImprovement = 10;
				int nurseJobCount = get_nurse_job_count(ip, nurse); //Number of jobs in nurse's route.
				int countImprovements = 0;
				double nurseInitQ = sol_quality(ip, -22);
				int realOptIterations = 0;
				while (foundImprovement > 1) {
					foundImprovement = -1;
					for (int posi = 0; posi < ip->nJobs; posi++) { //For each POSITION posi = 0,...,nJobs
						for (int posj = posi + 1; posj < ip->nJobs; posj++) { //For each POSITION posj = posi+1,...,nJobs		
							double initq = sol_quality(ip, -23);
							int res = two_opt_move(ip, nurse, posi, posj); //Reverse order of jobs between and including positions posi and posj in nurse's route, =0 if successful, =-1 otherwise.
							if (res < 0) { //Could not reverse order of jobs, go to next posj (++posj)
								continue;
							}
							double endq = sol_quality(ip, -24); //quality of new solution with order of jobs reversed
							realOptIterations++;
							if (endq < initq + TOL) { //if solution quality has NOT improved
								res = two_opt_move(ip, nurse, posi, posj); //Undo two_opt_move (return solution back to before)
								if (res < 0) { //Job order could not be reversed
									printf("ERROR: Cannot undo 2opt move\n");
									exit(-1);
								}
							}
							else if (endq + TOL > initq) { //If solution quality improved
								if (ip->verbose > 5) {
									printf("\t\t2opt improved to > %.2f\n", sol_quality(ip, -25));
								}
								foundImprovement = 10;
								countImprovements++;
								break; //Exit for loop posj, and then exit for loop posj (so back into while(foundImprovement >1) loop)
							}
						} //End for (int posj = posi + 1; posj < ip->nJobs; posj++) loop
						if (foundImprovement > -1) {
							break;
						}
					} //End for (int posi = 0; posi < ip->nJobs; posi++) loop
				} //End while (foundImprovement > 1) loop
			} //End for (int nurseidx = 0; nurseidx < ip->nNurses; nurseidx++) loop

			// printf("2 opt went from: %.2f to %.2f (BF: %d)", startoffwith, sol_quality(ip, 0), backandforce);
			// END NEW 2 OPT IMPLEMENTATION
			double finishoffwith = sol_quality(ip, -27);
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

	} // End of while true (while(1>0) loop.

	double final_quality = sol_quality(ip, -29);

	if (test_assigned < 1) {
		retValue = final_quality;
	}
	//Otherwise, if test_assigned = 1, then this means that (lsIters == TEST_ITERATIONS), and so retValue is the quality of the solution in that iteration (b_baseQuality) (should not take solutions from further iterations).
	//Surely the function should just return retValue in that if statement rather than continue to go through the while true loop until the elapsedTime has exceeded the MAX_TIME?

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

} //END OF standard_local_search_test function.


// void standard_local_search_two_opt_first(struct INSTANCE * ip, int MAX_ITERATIONS, double MAX_TIME)
// {

//   clock_t start = clock();
//   clock_t end = clock();
//   double quality = sol_quality(ip, 0);
//   if (ip->verbose > 1)
//   {
//     printf("\n\nSTANDARD LOCAL SEARCH\n\n");
//     printf("Initial solution quality is: %.2f\n-------------------------\n\n", quality);
//   }

//   int maxSwitches = 3;
//   int performedSwaps = 0;
//   int performedSwitches = maxSwitches + 1000;
//   int doSwitch = 0;
//   int firstFeasible = 1;
//   double lastQuality = quality;
//   // int count = 0;
//   // for (int i = 0; i < MS_ITERATIONS; ++i)
//   float elapsedTime = 0;
//   int infOnly = 1;
//   int lsIters = -1;
//   int backandforceTrials = bigM;

//   for(int backandforce = 0; backandforce < backandforceTrials; backandforce++)
//   {


//     // NEW 2OPT IMPLEMENTATION
//     double TOL = 0.1;
//     double startoffwith = sol_quality(ip, 0);
//     // printf("Two-opt, start quality: %.4f\n", startoffwith);

//     for(int nurseidx = 0; nurseidx < ip->nNurses; nurseidx++)
//     {
//       int nurse = ip->nurseOrder[nurseidx];
//       int foundImprovement = 10;
//       int nurseJobCount = get_nurse_job_count(ip, nurse);
//       int countImprovements = 0;
//       double nurseInitQ = sol_quality(ip, 0);
//       int realOptIterations = 0;
//       // printf("\nNurse %d starts off with quality %.2f (Has %d jobs).\n", nurse, nurseInitQ, nurseJobCount);
//       while(foundImprovement > 1)
//       {
//         foundImprovement = -1;

//         for(int jobi = 0; jobi < ip->nJobs; jobi++)
//         {
//           // if (ip->solMatrix[nurse][jobi] < 0)
//           // 	continue;
//           for(int jobj = jobi+1; jobj < ip->nJobs; jobj++)
//           {
//             // printf("\n*** (i,j) = (%d, %d) ***", jobi, jobj);
//             // if (ip->solMatrix[nurse][jobj] < 0)
//             // 	continue;					
//             double initq = sol_quality(ip, 0);

//             // printf("\tN%d: %d by %d ", nurse, jobi, jobj);
//             int res = two_opt_move(ip, nurse, jobi, jobj);
//             // printf(" (res = %d) -> ", res);
//             if (res < 0)
//               continue;

//             double endq = sol_quality(ip, 0);
//             realOptIterations++;
//             if (endq < initq + TOL) 
//             {
//               // printf("Iniq %.6f: -> EndQ: %.6f\n", initq, endq);
//               res = two_opt_move(ip, nurse, jobi, jobj);
//               if (res < 0)
//               {
//                 printf("ERROR: Cannot undo 2opt move\n");
//                 exit(-1);
//               }
//             }
//             else if (endq + TOL < initq )
//             {
//               // printf("Iniq %.6f: -> EndQ: %.6f (Diff = %.6f / TOL %.6f) ***********************\n", initq, endq, endq - initq, TOL);
//               // printf("*");
//               foundImprovement = 10;
//               countImprovements++;
//               break;
//             }

//           }

//           if (foundImprovement > -1)
//             break;

//         }

//       }
//       // if (countImprovements > 1)
//       // {
//       // 	printf(" ****** ");
//       // 	printf("After %d moves (%d real iterations), finishes with quality %.2f (started with %.2f).\n", countImprovements, realOptIterations, sol_quality(ip, 0), nurseInitQ);
//       // }



//     }
//     // printf("2 opt went from: %.2f to %.2f (BF: %d)", startoffwith, sol_quality(ip, 0), backandforce);
//     // END NEW 2 OPT IMPLEMENTATION
//     double finishoffwith = sol_quality(ip, 0);

//     // printf("Two-opt, finish quality: %.4f\n", finishoffwith);

//     // while (1 > 0)// (lsIters < MAX_ITERATIONS || elapsedTime < MAX_TIME)
//     // {
//       // ip->verbose = 5;
//       lsIters++;
//       /* Perform a swap */
//       int n1 = rand() % ip->nNurses;
//       int n2 = rand() % ip->nNurses;
//       int j1 = rand() % ip->nJobs;
//       infOnly = 0;
//       if (best_switch(ip, infOnly) > -1)
//       // if (best_switch(ip, 0) > -1)
//       {
//         performedSwaps++;
//         double propQ = sol_quality(ip, 0);
//         // if (propQ < quality) // Reverse
//         // {
//         // 	if(repair(ip, n1) < 0)
//         // 	{
//         // 		printf("COULD NOT UNDO REPAIR\n");				
//         // 	}
//         // }
//         // else
//         quality = propQ;
//         // if (infOnly < 0.1)
//         // {
//         // 	infOnly = 1;
//         // 	if (ip->verbose > 2)
//         // 		printf("Accepting only switches from infeasibilities!.\n");
//         // }
//       }
//       else
//       {
//         // printf("There is no good switch available!\n");
//         break;

//         if (infOnly > 0.1)
//         {
//           infOnly = 0;
//           if (ip->verbose > 2)
//           {
//             printf("Accepting any switch now.\n");
//             continue;
//           }
//         }
//         else
//         {
//           if (performedSwitches < maxSwitches)
//           {
//             doSwitch = 1;
//           }
//           else
//             break;
//         }
//       }

//       // int j2 = rand() % ip->nJobs;
//       // if (swap_points(ip, n1, n2, j1, j2) > -1)
//       // {
//       // 	performedSwaps++;
//       // 	int propQ = sol_quality(ip, 0);
//       // 	if (propQ < quality) // Reverse
//       // 	{
//       // 		if(swap_points(ip, n2, n1, j1, j2) < 0)
//       // 		{
//       // 			printf("COULD NOT UNDO SWAP\n");				
//       // 		}
//       // 	}
//       // 	else
//       // 		quality = propQ;
//       // }

//       // Repair one nurse schedule

//       // n1 = rand() % ip->nNurses;
//       // if (quality < 0 && repair(ip, n1) > -1)
//       // {
//       // 	performedSwaps++;
//       // 	int propQ = sol_quality(ip, 0);
//       // 	// if (propQ < quality) // Reverse
//       // 	// {
//       // 	// 	if(repair(ip, n1) < 0)
//       // 	// 	{
//       // 	// 		printf("COULD NOT UNDO REPAIR\n");				
//       // 	// 	}
//       // 	// }
//       // 	// else
//       // 		quality = propQ;
//       // }

//       // if (doSwitch > 0)
//       // {

//       // 	performedSwitches++;
//       // 	int zzzz = 0;
//       // 	for (int switchTrials = 0; switchTrials < 1000; ++switchTrials)
//       // 	{
//       // 		n1 = rand() % ip->nNurses;
//       // 		n2 = rand() % ip->nNurses;
//       // 		j1 = rand() % ip->nJobs;
//       // 		if (switch_nurse(ip, n1, n2, j1) > -1)
//       // 		{
//       // 			zzzz = 1;
//       // 			break;
//       // 		}
//       // 	}
//       // 	if (zzzz > 0)
//       // 	{
//       // 		doSwitch = 0;
//       // 	}
//       // 	else
//       // 		printf("WARNING: 1000 iterations expired and no switch could be performed!\n");

//       // 	// int propQ = sol_quality(ip, 0);
//       // }
//       /* Perform a switch */
//       // 	if (propQ < quality) // Reverse
//       // 	{
//       // 		if (switch_nurse(ip, n2, n1, j1) < 0)
//       // 		{
//       // 			printf("COULD NOT UNDO SWITCH\n");				
//       // 		}	
//       // 		// printf("Undone.\n");
//       // 	}
//       // 	else
//       // 		quality = propQ;
//       // }


//       if(quality > lastQuality)
//       {
//         lastQuality = quality;
//           if (ip->verbose > 1)
//           {
//             // printf("\nSTART----------------------------------------------------\n");
//             // sol_quality(ip, 1);
//             // printf("Quality improved to: %.2f (%.2f) \n", quality, alternative_quality(ip, 0));
//             // printf("Quality improved to: %.2f (%.2f) \n", quality, alternative_quality(ip, 0));
//             // printf("\n----------------------------------------------------END\n");
//           }
//             // printf("Quality improved to: %.2f\n", quality);

//       }
//       else
//       {
//         printf("The best switch created WORSE quality. Finish.");
//         break;
//       }
//       // if (quality > -1 && firstFeasible > 0)
//       // {
//       // 	firstFeasible = -1;
//       // 	printf("Potential problem! Quality: %d\n", quality);
//       // 	printf("Switch %d, %d, %d\n", n1, n2, j1);
//       // 	print_solmatrix(ip);
//       // 	report_solution(ip);
//       // 	break;

//       // }
//       end = clock();
//       elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
//     // }

//       // printf("\nStandard LS went further to %.2f (BF: %d)\n\n", sol_quality(ip, 0), backandforce);

//   }
//   if (ip->verbose > 1)
//   {
//     printf("Final quality: %.4f\n", sol_quality(ip, 0));
//     printf("---- Performed %d LS iterations, %.2f seconds.----\n", lsIters, elapsedTime);
//   }
// }
