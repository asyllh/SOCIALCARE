/*--------------/
EA_ALG
fns.cpp
UoS
27/01/2022
/--------------*/

#include "fns.h"

int MainWithOutput(struct Instance* ip, double* odmat_pointer, int* solMatrixPointer, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
        double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer){

    // Call the GRASP algorithm
    GRASP(ip);

    double finalQuality = SolutionQuality(ip, -1);
    if(ip->verbose > 0){
        std::cout << "Final solution quality is: " << finalQuality << std::endl;
    }

    // printf("Saving final quality in OD...\n");
    if(odmat_pointer != NULL){
        odmat_pointer[0] = finalQuality;
    }
    // printf("Done.\n");

    /* printf("\n------\nQuality indicators:\n");
     printf("Travel time   (min)\t%.2f\n", ip->objTravel);
     printf("Total time    (min)\t%.2f\n", ip->objTime);
     printf("Total tard.   (min)\t%.2f\n", ip->objTardiness);
     printf("Longest shift (min)\t%.2f\n", ip->objLongestDay);
     printf("Total Overtime(min)\t%.2f\n", ip->objOvertime);
     printf("\n------\n");*/

    //SolnToPythomFormat(ip, solMatrixPointer, timeMatrixPointer, nurseWaitingTimePointer, nurseTravelTimePointer, violatedTWPointer, nurseWaitingMatrixPointer, nurseTravelMatrixPointer,totalsArrayPointer);

    //exclude_nurse_travel_data = false;

    // solMatrixPointer = ip->solMatrix;
    if(ip->verbose > 5){
        std::cout << "End of program.\nFreeing memory..." << std::endl;
    }

    FreeInstanceMemory(ip);
    if(ip->verbose > 5){
        std::cout << "Done." << std::endl;
    }

    return 0;
}

void GRASP(struct Instance* ip){

    int numIterations = 0;
    int doLS = 1;
    int exch1 = 0;
    int exch2 = 0;
    int no_imp_iter = 0;
    int max_no_imp_iter = 25;
    int solnsGenerated = 0;
    int solnsRelinked = 0;
    int numLSPerformed = 0;
    int numPRPerformed = 0;
    double eps = 1e-6;
    double elapsedTime = 0.0;
    double GRASPParam = 0.0; // DELTA IN PAPER. Has effect only if randomGraspParam = 0. Later on in the function: double rnd_dbl = (float)rand() / (float)RAND_MAX; GRASP_param = delta_low + rnd_dbl * delta_range;
    double deltaLow = ip->algorithmOptions[4]; // 0.05;
    double deltaRange = ip->algorithmOptions[5]; // 0.25;
    double currentQuality = -1;
    double bestQuality = -1*bigM;
    float elapsedTimeIT = 0;
    double bestCR = -1*bigM;
    double beforeExchange = -1*bigM; //quality of solution before exchange
    double avgConstructiveSoln = 0.0;
    unsigned int LS_ITERS = bigM;
    unsigned int MAX_ITERATIONS = bigM;
    int LS_ITERS_AFTER = MAX_ITERATIONS; // = bigM
    float LS_TIME = ip->MAX_TIME_SECONDS;
    float LS_TIME_AFTER = ip->MAX_TIME_SECONDS;
    struct Instance bestSol, emptySol;
    clock_t start = clock();
    clock_t end = clock();
    clock_t startIT = clock();
    clock_t endIT = clock();

    int numSolnsInPool = (int) (ip->algorithmOptions[8] + eps); // Allocate space for a pool of solutions (aO[8] = 10)
    int rclStrategy = (int) (ip->algorithmOptions[10] + eps); //Type of strategy for cut-off value in RCL (used in RandomisedConstructive). (aO[10] = 1 (strategy 1 C=B-delta*(B-W)))
    int changeNurseOrder = (int) (ip->algorithmOptions[6] + eps); //what does this value do? (aO[6] = 1, so nurse order change active in GRASP)
    emptySol = (*ip);
    bestSol = (*ip);

    double* poolQuality = malloc(numSolnsInPool*sizeof(double));
    struct INSTANCE** pool = (struct INSTANCE**) malloc(numSolnsInPool*sizeof(struct INSTANCE*));
    for(int i = 0; i < numSolnsInPool; i++){
        pool[i] = NULL;
        poolQuality[i] = -DBL_MAX;
    }

    //int PR_STRATEGY = (int)(ip->algorithmOptions[9] + eps); // 1, 2, 3 or 4, as above (aO[9] = 1, so PR with one random solution in pool)
    //int PR_DIRECTION_PARAMETER = (int)(ip->algorithmOptions[11] + eps); // 0 forward, 1 backward, 2 forward & backward, 3 randomly chosen (including f&b) (aO[11] = 0, so forward)
    // int performPathRelinking = (int) ip->algorithmOptions[7] + eps;


    // Initial nurse order:
    std::vector<int> bestNurseOrder(ip->nNurses);
    //int* bestNurseOrder = malloc(ip->nNurses * sizeof(int)); // Rows
    for(int i = 0; i < ip->nNurses; ++i){ //Initially, nurseOrder is 0,1,...,nNurses-1, and so bestNurseOrder array is taken to be the same.
        ip->nurseOrder[i] = i;
        bestNurseOrder[i] = i;
    }

    std::cout << "\nSTARTING Nurse order: (";
    for(int i = 0; i < ip->nNurses; ++i){
        std::cout << ip->nurseOrder[i] << " ";
    }
    std::cout << ")" << std::endl;

    //Initial bestSolMatrix:
    std::vector<std::vector<int> > bestSolMatrix(ip->nNurses, std::vector<int>(ip->nJobs));
    //int** bestSolMatrix = malloc(ip->nNurses * sizeof(int*)); // Rows
    for(int i = 0; i < ip->nNurses; i++){
        //bestSolMatrix[i] = malloc(ip->nJobs * sizeof(int)); // Cols
        for(int j = 0; j < ip->nJobs; ++j){
            bestSolMatrix[i][j] = ip->solMatrix[i][j]; //Initially, bestSolMatrix is taken to be the same as solMatrix, so all -1's.
        }
    }

    while(numIterations < MAX_ITERATIONS){
        startIT = clock();
        currentQuality = SolutionQuality(ip, -30);

        struct Instance newEmpty = emptySol;
        *ip = newEmpty; //what do these two lines do?
        CleanSolutionFromStruct(ip); // Set all elements in solMatrix 2D array to -1.

        //Running constructive.
        if(numIterations == 0){ //First iteration only, no solution currently exists.
            beforeExchange = currentQuality;
        }
        else if(changeNurseOrder > 0){
            if(currentQuality > beforeExchange){ //If solution quality is better than the quality of the solution before the exchange, update before_exchange
                beforeExchange = currentQuality;
                RandomTwoExchange(ip->nurseOrder, ip->nNurses, &exch1, &exch2);
                no_imp_iter = 0;
            }
            else{ // Quality of solution before exchange is better than or equal to the quality of the current solution (after exchange), so reverse the previous exchange to return back to better quality solution.
                no_imp_iter++;
                TwoExchange(ip->nurseOrder, exch1, exch2);
            }
            Shuffle(ip->nurseOrder, (size_t) ip->nNurses); //Shuffle the contents of nurseOrder.
        }

        double randDbl = (float) rand()/(float) RAND_MAX;
        GRASPParam = deltaLow + randDbl*deltaRange; //delta value in paper, used for cut-off value strategies.

        //Assign jobs to nurses, create a solution.
        RandomisedConstructive(ip, 1, GRASPParam, rclStrategy);
        solnsGenerated++;

        endIT = clock();

        double constrCurrentQuality = SolutionQuality(ip, -10000); //quality of the current solution.
        if(solnsGenerated > 1){
            avgConstructiveSoln = (avgConstructiveSoln*(solnsGenerated - 1) + constrCurrentQuality)/solnsGenerated;
        }
        else{
            avgConstructiveSoln = constrCurrentQuality;
        }

        if(constrCurrentQuality > bestCR){
            bestCR = constrCurrentQuality;
        }

        // DEBUG : change to 0, higher than 0 means deactivated
        int constructivePassesThreshold = 99;
        double constructiveFilterParam = 1;
        double constrTrValue = avgConstructiveSoln + (constructiveFilterParam - 1)*abs(avgConstructiveSoln);
        float rnum = ((float) rand()/(float) RAND_MAX > 0.5);
        if(constrCurrentQuality > constrTrValue){
            if(rnum > 0.1){
                constructivePassesThreshold = 10;
            }
        }
        else if(rnum > 0.8){
            constructivePassesThreshold = 10;
        }

        end = clock();
        elapsedTime = ((double) (end - start))/CLOCKS_PER_SEC;

        double lsQualityTest = -1;
        LS_TIME = ip->MAX_TIME_SECONDS - elapsedTime;
        if(LS_ITERS > 0 && doLS > 0 && constructivePassesThreshold > 1 && LS_TIME > 0){
            numLSPerformed++;
            if(ip->verbose > 3){
                std::cout << "Running standard LS, passed threshold with quality: " << constrCurrentQuality << " (average CR: " << avgConstructiveSoln << ", threshold value: " << constrTrValue << ")"
                          << std::endl;
                std::cout << "PASS = " << constructivePassesThreshold << std::endl;
            }
            lsQualityTest = StandardLocalSearchTest(ip, LS_ITERS, LS_TIME, 100); //Run LS first-improvement VNS, returns the solution quality of the best solution created
        }
        else{
            std::cout << "SKIP LS, quality: " << constrCurrentQuality << "(average CR: " << avgConstructiveSoln << ", threshold value: " << constrTrValue << ")" << std::endl;
        }

        currentQuality = SolutionQuality(ip, -11);
        double lsTotalQ = currentQuality;

        // Check the pool:
        int insertHere = -1;
        int dissimThreshold = 5;
        double lowestDissim = bigM;
        //Check if solution should enter the pool
        for(int ps = 0; ps < numSolnsInPool; ps++){ //For each ps=0,...,#solnsinpool
            if(NULL == pool[ps]){ // If there are still NULL pointers, allocate them and assign here
                pool[ps] = malloc(sizeof(struct INSTANCE));
                insertHere = ps;
                break;
            }
            double dissimWithps = SolutionDissimilarity(ip, pool[ps]); //Returns the solution dissimilarity d(S_1, S_2) where S_1 = ip, S_2 = pool[ps].

            if(poolQuality[ps] > currentQuality){ //If quality of pool[ps] solution better than quality of ip
                if(dissimWithps
                        < dissimThreshold){ //If dissimilarity of solutions ip and pool[ps] is below the threshold, then the solution is too similar to solutions already in pool, so do not add soln to pool.
                    insertHere = -1; //Rejected introducing this solution in pool, too similar to pool[ps]
                    break; //Exit for ps loop
                }
                else{ // printf("\t-> Not better than POOL %d (and dissimilarity value %.2f)\n", ps, dissimilarity_with_ps);
                    continue; //Go to next ps (++ps)
                }
            }
            // We are good to insert, it's got better quality than this solution and it is not too similar to a better solution

            if(dissimWithps > lowestDissim){ //Dissimilarity is too high?
                // printf("\t-> Will NOT substitute %d (dissimilarity is %.2f, but lowest is %.2f)\n", ps, dissimilarity_with_ps, lowest_dissimilarity);
                continue;
            }
            else{
                lowestDissim = dissimWithps;
                insertHere = ps;
                // printf("\t-> Proposing to insert substituting %d (lowest dissimilarity so far, %.2f)\n", ps, lowest_dissimilarity);
            }
        } //End for (int ps = 0; ps < solutions_in_pool; ps++) loop
        //Finished checking pool, printf("Finished checking pool, insertHere = %d\n", insertHere);

        // Insert solution in the pool:
        if(insertHere >= 0){
            (*pool[insertHere]) = CopyInstance(ip); //pool[ps] is replaced with ip!
            poolQuality[insertHere] = currentQuality;
        }
        end = clock();
        elapsedTime = ((double) (end - start))/CLOCKS_PER_SEC;

        if(currentQuality > bestQuality){
            if(ip->verbose > 2){
                std::cout << "It: " << numIterations << "\nCA+LS: " << currentQuality << "\nCA: " << constrCurrentQuality << "\ntott: " << ip->objTime << "\ntard: " << ip->objTardiness << std::endl;
                std::cout << "ovti: " << ip->objOvertime << "\npref: " << ip->totalPref << "\nl_day: " << ip->objLongestDay << "\nGRASP_P: " << GRASPParam << std::endl;
                std::cout << "et: " << elapsedTime << "(max " << ip->MAX_TIME_SECONDS << ")" << std::endl;
                std::cout << "\n > Nurse order: (";
                for(int i = 0; i < ip->nNurses; ++i){
                    std::cout << ip->nurseOrder[i] << " " << std::endl;
                }
                std::cout << ")" << std::endl;
                std::cout << "Added a copy of the incumbent solution (q = " << currentQuality << ") to the pool in position " << insertHere << std::endl;
            }
            else if(ip->verbose >= 0){
                std::cout << "It: " << numIterations << "\tQ: " << currentQuality << "\tt: " << elapsedTime << "s" << std::endl;
                std::cout << " > Nurse order: (";
                for(int i = 0; i < ip->nNurses; ++i){
                    std::cout << ip->nurseOrder[i] << " ";
                }
                std::cout << ")" << std::endl;
                SolutionQuality(ip, -1);
            }

            bestSol = CopyInstance(ip); //bestSol = ip, store best solution so far
            bestQuality = currentQuality; //bestQuality = quality of ip, store quality of best solution so far
        }
        else if(ip->verbose > 5){ //currentQuality <= bestQuality
            std::cout << "It: " << numIterations << "\tCA+LS: " << currentQuality << " (best: " << bestQuality << ") CA: " << constrCurrentQuality << std::endl;
            std::cout << "\n > Nurse order: (";
            for(int i = 0; i < ip->nNurses; ++i){
                std::cout << ip->nurseOrder[i] << " ";
            }
            std::cout << ")" << std::endl;
            std::cout << "Elapsed time: " << elapsedTime << "s (MAX: " << ip->MAX_TIME_SECONDS << ")\n" << std::endl;
        }

        if(elapsedTime > ip->MAX_TIME_SECONDS){ //Exceeded time limit
            if(ip->verbose > 0){
                std::cout << "\nTime finished! (" << elapsedTime << " seconds)\n" << std::endl;
            }
            break; //exit while loop
        }
        numIterations++;

    } // ******************************** End of while (iter < MAX_ITERATIONS) loop*********************************** //

    if(ip->verbose > 0){
        std::cout << "GRASP: Performed " << numIterations << " iterations." << std::endl;
    }

    double thisSol = SolutionQuality(ip, -3423); //Quality of current ip solution
    double cquality = 0.0;
    int pickthis = -1;
    if(ip->verbose > 5){
        std::cout << "Pool contents:\n";
    }

    for(int ps2 = 0; ps2 < numSolnsInPool; ps2++){ //For each 'space' (position) in pool, ps2=0,...,solnsinpool
        if(pool[ps2] != NULL){ //If there is a solution in that position in the pool
            cquality = SolutionQuality(pool[ps2], -3234); //quality of the solution in that space in the pool
            if(ip->verbose > 5){
                std::cout << "\tPOS " << ps2 << " - Q = " << cquality << std::endl;
            }
        }
        if(thisSol
                < cquality){ //If there is no solution in that position in the pool (pool[ps2] == NULL) OR if the quality of the current solution ip is WORSE than the quality of the solution in pool[ps2]
            if(ip->verbose > 5){
                std::cout << "Pick solution " << ps2 << " with quality " << cquality << std::endl;
            }
            pickthis = ps2; //Select the solution in pool[ps2]
            thisSol = cquality; //update solution quality to be the quality of pool[ps2]
        }
    }
    if(pickthis > -1){ //If solution picked from pool
        bestSol = (*pool[pickthis]); //update best solution found
        *ip = *pool[pickthis]; // Get this pointer, ip = best solution found in pool
        pool[pickthis] = NULL; // Prevent this sol from being cleaned, remove solution from pool (remove pointer, ip still points to the solution though)
        if(ip->verbose > 5){
            std::cout << "Sol quality after copying: " << SolutionQuality(ip, -235223) << std::endl;
        }
    }

    if(ip->verbose > 5){
        std::cout << "Saved best solution" << std::endl;
    }

    double quality = SolutionQuality(ip, -121212); //quality of solution ip


    if(ip->verbose > 3){
        std::cout << "Pool contents (cleaning):\n";
    }
    //Deallocating memory:
    for(int ps2 = 0; ps2 < numSolnsInPool; ps2++){ //For each 'space' (position) in the pool
        if(pool[ps2] != NULL){ //If there's a solution in that position, remove it from the pool
            if(ip->verbose > 3){
                std::cout << "\tPOS " << ps2 << " - Q = " << SolutionQuality(pool[ps2], -3234) << std::endl;
            }
            FreeInstanceCopy(pool[ps2]);
            pool[ps2] = NULL;
        }
    }
    free(pool);

    std::cout << "Iterations performed: " << numIterations << std::endl;

    if(ip->verbose > 0){
        std::cout << "GRASP Finished.\n";
        std::cout << "Final quality: " << quality << std::endl;
        std::cout << "Theoretical best quality: " << bestQuality << std::endl;
        std::cout << "Iterations performed: " << numIterations << std::endl;;
        std::cout << "sols_generated = " << solnsGenerated << std::endl;
        std::cout << "local_searches_performed = " << numLSPerformed << std::endl;
        std::cout << "solutions_relinked = " << solnsRelinked << std::endl;
        std::cout << "total path_relinkings_performed = " << numPRPerformed << std::endl;
        end = clock();
        elapsedTime = ((double) (end - start))/CLOCKS_PER_SEC;
        std::cout << "Elapsed time: " << elapsedTime << "s (MAX: " << ip->MAX_TIME_SECONDS << ")" << std::endl;
    }

    // Deallocate memory
    //for(int i = 0; i < ip->nNurses; ++i){
    //    free(bestSolMatrix[i]);
    //}
    //free(bestSolMatrix);
    //free(bestNurseOrder);

    //return 0;

} //END OF GRASP FUNCTION.

void RandomisedConstructive(struct Instance* ip, int randomness, double delta, int rclStrategy){

    // double delta = GRASPParam from void GRASP(ip) function.
    //Seed: each nurse is assigned a single job to ensure that all nurses will have an initial route, regardless of how far their starting location is, and helps construct more balanced solution.
    //Assign remaining jobs: the jobs are ranked and assigned to the best position in any of the nurse routes, until no more jobs are left to assign.

    int DEBUG_PRINT = -1;
    if(DEBUG_PRINT > 0){
        std::cout << "\nSTARTING RANDOMISED CONSTRUCTIVE --------------------------\nStart solmatrix:" << std::endl;
        PrintSolMatrix(ip);
        std::cout << "Constructive allocations..." << std::endl;
    }

    std::vector<int> nurseSeed(ip->nNurses);
    //int* nurseSeed = malloc(ip->nNurses * sizeof(int)); // nurseSeed: 1 x nNurses, initialised to -1.
    for(int j = 0; j < ip->nNurses; ++j){
        nurseSeed[j] = -1;
    }

    std::vector<double> allocatedJobs(ip->nJobs);
    //double* allocatedJobs = malloc(ip->nJobs * sizeof(double)); // allocatedjobs: 1 x nJobs, initialised to 0. =1 if job has been allocated, =0.5 if double service job has been allocated to only one nurse, = 0 if not allocated.
    for(int i = 0; i < ip->nJobs; ++i){
        allocatedJobs[i] = 0;
    }

    int bIndicesSize = ip->nNurses*ip->nJobs;
    std::vector<std::vector<int> > bestIndices(bIndicesSize, std::vector<int>(2));
    //int** bestIndices = malloc(bIndicesSize * sizeof(int*)); //bestIndices: (nNurses x nJobs) x 2, column 0 = nurse, column 1 = job.
    //for (int i = 0; i < bIndicesSize; ++i) {
    //    bestIndices[i] = malloc(2 * sizeof(int));
    //}

    std::vector<double> rankValues(ip->nJobs);
    std::vector<int> rclSeeds(ip->nJobs); // was RCL_seeds
    //double* rankValues = malloc(ip->nJobs * sizeof(double)); // rankValues: 1 x nJobs. Quality of solution for each job
    //int* RCL_seeds = malloc(ip->nJobs * sizeof(int)); // RCL_seeds: 1 x nJobs.

    if(DEBUG_PRINT > 0){
        std::cout << "Constructive allocations.Done." << std::endl;
    }

    /*** ------------------------------------------------------------------ ASSIGN SEEDS ------------------------------------------------------------------ ***/

    int rclSize = -1; // Number of elements in the RCL (RCL_seeds) (rclSeeds vector int)
    int checkCount = -1; // Count number of loop iterations
    double bestRank = -DBL_MAX;
    double worstRank = DBL_MAX;

    for(int nurseIdx = 0; nurseIdx < ip->nNurses; ++nurseIdx){
        bestRank = -DBL_MAX;
        worstRank = DBL_MAX;
        int alreadyAllocated = 0;
        int unskilledFor = 0;
        int dismissedJobs = 0;
        int consideredJobs = 0;
        int nurse = ip->nurseOrder[nurseIdx]; // nurse = number of the nurse in the nurseIdxth position in the nurseOrder array.

        for(int job = 0; job < ip->nJobs; ++job){ //For 'nurse', go through all jobs 0,...,nJobs, and check the quality of the solution that arises when 'job' is inserted into 'nurse's route
            //Update rankValues[job] for each job, and store/keep updated the highest/lowest quality solution (bestRank/worstRank) of all solutions for the current nurse.
            checkCount++;

            if(allocatedJobs[job] > 0.6){ //Job 'job' has already been allocated
                alreadyAllocated++;
                rankValues[job] = -2*bigM;
                continue;
            }
            if(CheckSkills(ip, job, nurse) < 1){ //If nurse is not skilled to do the job (i.e. nurseSkilled[nurse][job] = 0)
                unskilledFor++;
                rankValues[job] = -bigM;
                continue;
            }
            if(DEBUG_PRINT > 0){
                std::cout << "\n---Insert in nurse " << nurse << " the job " << job << std::endl;
            }
            if(BestJobInsertion(ip, job, nurse) < 0){ //If job cannot be inserted into nurse's route.
                dismissedJobs++;
                rankValues[job] = -bigM;
                continue;
            }
            else{ //Job can and has been inserted into nurse's route.
                consideredJobs++;
            }
            if(DEBUG_PRINT > 0){
                std::cout << "Done.\nRanking job " << job << "...\n\tValue: ";
            }

            int solQNum = -1000;

            // Check the rank of the job and save the best one overall
            rankValues[job] = SolutionQuality(ip, solQNum); //Calculate the solution quality of the job-nurse assignment (i.e. the new solution with 'job' inserted into nurse's route), and save it to rankValues[job].

            // Keep track of best and worst ranks
            if(rankValues[job] > bestRank){
                bestRank = rankValues[job];
            }
            if(rankValues[job] < worstRank){
                worstRank = rankValues[job];
            }
            if(DEBUG_PRINT > 0){
                std::cout << rankValues[job] << "\nDone.\nRemoving job " << job << std::endl;
            }
            RemoveJob(ip, job, nurse); //Remove job from nurse's route, i.e. return solution back to original.

            bestIndices[job][0] = nurse; // The best quality solution (so far) was created with 'job' in 'nurse's route, store best indices.
            bestIndices[job][1] = job;

        } //End for(int job = 0; job < ip->nJobs; ++job) loop

        if(consideredJobs < 1){ //None of the jobs could be inserted into nurse's route, so continue in for loop (move on to next nurse (++nurseIdx) in nurseOrder array).
            continue;
        }

        //Create cut-off value and add elements (jobs) to the RCL (RCL_seeds). rcl_size = number of elements in the RCL,
        GenerateRCL(delta, rclSeeds, rclSize, rankValues, ip->nJobs, bestRank, worstRank, rclStrategy);

        int cNurse = -1;
        int cJob = -1;

        //Pick an element at random from RCL_seeds, returns the indicies for the selected element, i.e. the job-nurse assignment. cNurse = the selected nurse and cJob = the selected job (they are selected together).
        RCLPick(bestIndices, rclSeeds, rclSize, cNurse, cJob);

        if(DEBUG_PRINT > 0){
            // printf("Picking one of the %d top seed values for nurse %d\n", GRASP_param, nurse);
            std::cout << "Decided on (nurse " << cNurse << ", job " << cJob << std::endl;
            std::cout << "Solmatrix:\n";
            PrintSolMatrix(ip);
            std::cout << "Nurse routes:\n";
            PrintIntMatrix(ip->allNurseRoutes, ip->nNurses, ip->nJobs);
            if(cJob < 0){
                std::cout << "ERROR: The best pick was a -1!!!!" << std::endl;
                exit(-1);
            }
        }

        if(BestJobInsertion(ip, cJob, cNurse) > -1){ //If cJob has been successfully inserted into cNurse's route
            // printf("Chosen job %d as seed for nurse %d\n", cJob, nurse);
            if(ip->doubleService[cJob]){ // If cJob is a double service
                allocatedJobs[cJob] += 0.5; // Record cJob as being half-done (only one nurse so far, need two)
            }
            else{
                allocatedJobs[cJob] = 1; // If cJob is a normal job (only one nurse required), then mark cJob as allocated.
            }
            nurseSeed[nurse] = cJob; //Mark/assign cJob to the current nurse (not cNurse). Should this be nurseSeed[cNurse]?
        }
        else{
            std::cout << "\nERROR! Solmatrix:" << std::endl;
            PrintSolMatrix(ip);
            std::cout << "\nERROR: FAILED to assign job " << cJob << " as seed for nurse " << cNurse << std::endl;
            if(ip->doubleService[cJob]){
                std::cout << "(This job is a DS)" << std::endl;
            }
            exit(-1);
        }
    } // End for (int nurseIdx = 0; nurseIdx < ip->nNurses; ++nurseIdx) loop

    //Deallocate memory
    //free(rclSeeds);
    //free(rankValues);

    /*** ------------------------------------------------------------------ FINISHED WITH SEEDS ------------------------------------------------------------------ ***/

    if(DEBUG_PRINT > 0){
        std::cout << "Finished with seeds" << std::endl;
    }

    int jobsRemaining = 0; // Number of jobs that still need to be assigned.
    for(int i = 0; i < ip->nJobs; ++i){
        if(allocatedJobs[i] < 0.9){ //If the job i has not been completely fulfilled (the job hasn't been assigned to a nurse/two nurses or the job is a double service and only one nurse has been assigned the job).
            jobsRemaining++;
        }
    }

    /*** ------------------------------------------------------------------ REMAINING JOBS ------------------------------------------------------------------ ***/
    int ct;
    int ittt = 0; //Is this needed? Only used at the beginning of the while loop, ittt++.
    int assignmentIterations = 0;
    int MAX_ASSIGNMENT_ITS = jobsRemaining*500;
    std::vector<int> RCL(bIndicesSize);
    std::vector<double> rankAssignments(bIndicesSize);
    //int* RCL = malloc(ip->nJobs * ip->nNurses * sizeof(int)); // 1D array, size = 1 x (nJobs*nNurses), Restricted Candidate List.
    //double* rankAssignments = malloc(ip->nJobs * ip->nNurses * sizeof(double)); // 1D array, size = 1 x (nJobs*nNurses)

    while(jobsRemaining > 0){
        ittt++;
        assignmentIterations++;

        // Reset variables that search for the RCL elements
        bestRank = -DBL_MAX;
        worstRank = DBL_MAX;
        for(int i = 0; i < bIndicesSize; ++i){
            bestIndices[i][0] = -1;
            bestIndices[i][1] = -1;
        }

        double baseQuality = SolutionQuality(ip, -3423462); //solution quality of current solution ip.
        ct = -1;
        int potentialAllocations = 0;
        for(int nurseidx = 0; nurseidx < ip->nNurses; ++nurseidx){ // For each nurse 0,...,nNurses
            int nurse = ip->nurseOrder[nurseidx];
            for(int job = 0; job < ip->nJobs; ++job){
                ct++;
                // printf("Test %2d: nurse %d - job %d - ", ct, nurse, job);

                if(allocatedJobs[job] > 0.6 || ip->solMatrix[nurse][job] > -0.1){ //if job has already been allocated OR job is a double service and is already positioned in nurse's route.
                    rankAssignments[ct] = -DBL_MAX;
                    // printf(" already allocated\n");
                    continue; // Move on to next job (++job) in the for loop.
                }

                // There is potential to allocate this job to this nurse, let's see if they have the skill
                int skillCheck = 0;
                if(!ip->doubleService[job]){ //If the job is not a double service (doubleService[job] == 0)
                    skillCheck = CheckSkills(ip, job, nurse); //skillCheck = ip->nurseSkilled[nurse][job]
                }
                else{ //If the job is a double service
                    if(allocatedJobs[job] < 0.4){ // If no nurses have been assigned this job (recall that allocatedJobs[job] = 0.5 means that one nurse has been assigned the job and another nurse is required).
                        skillCheck = CheckSkillsDSFirst(ip, job, nurse); //skillCheck = 1 if there exists another nurse that can do the double service job with 'nurse', and = 0 otherwise.
                    }
                    else{ //If only one nurse has been assigned to this double service 'job' (allocatedJobs[job] == 0.5)
                        // Who else is doing it?
                        for(int nurseb = 0; nurseb < ip->nNurses; nurseb++){
                            if(ip->solMatrix[nurseb][job] > -0.5){ //if 'job' is positioned in nurseb's route, i.e. nurseb has been assigned 'job'
                                skillCheck = CheckSkillsDS(ip, job, nurse, nurseb); //skillCheck = 1 if nurse and nurseb can do the job together, and = 0 otherwise.
                                break;
                            }
                        }
                    }
                }

                //If job has been allocated OR nurse is unskilled to do the job OR job is already positioned in nurse's route, the the job-nurse assignment is discarded.
                if(allocatedJobs[job] > 0.6 || skillCheck < 1 || ip->solMatrix[nurse][job] > -0.1){
                    rankAssignments[ct] = -DBL_MAX;
                    continue; //Move on to the next job (++job) in the for loop.
                }
                else{
                    potentialAllocations++;
                }

                // printf("Nearly inserting job %d in nurse %d\n", job, nurse);
                if(BestJobInsertion(ip, job, nurse) < 0){
                    std::cout << "WARNING: Constructive could not insert job " << job << " in nurse " << nurse << ", but previous checks deemed it possible!!!" << std::endl;
                    continue;
                }

                rankAssignments[ct] = SolutionQuality(ip, -1000);

                // Keep track of best and worst ranks
                if(rankAssignments[ct] > bestRank){
                    bestRank = rankAssignments[ct];
                }
                if(rankAssignments[ct] < worstRank){
                    worstRank = rankAssignments[ct];
                }

                RemoveJob(ip, job, nurse);

                bestIndices[ct][0] = nurse;
                bestIndices[ct][1] = job;

            }// End for (int job = 0; job < ip->nJobs; ++job) loop

        } //End for (int nurseidx = 0; nurseidx < ip->nNurses; ++nurseidx) loop

        // printf("There were %d potential allocations, and there are %d remaining jobs.\n", potentialAllocations, jobsRemaining);
        if(potentialAllocations < jobsRemaining){
            std::cout << "ERROR! Not enough allocations." << std::endl;
        }

        //Create cut-off value and add elements (jobs) to the RCL (RCL_seeds).
        GenerateRCL(delta, RCL, rclSize, rankAssignments, ip->nJobs*ip->nNurses, bestRank, worstRank, rclStrategy);

        int bestNurse = -1;
        int bestJob = -1;

        //Pick an element at random from RCL_seeds, returns the indicies for the selected element, i.e. the job-nurse assignment. best_nurse = the selected nurse and best_job = the selected job.
        RCLPick(bestIndices, RCL, rclSize, bestNurse, bestJob);

        if((bestNurse < 0 || bestJob < 0) || assignmentIterations > MAX_ASSIGNMENT_ITS){
            if(assignmentIterations > MAX_ASSIGNMENT_ITS){
                std::cout << "\n\n ERROR: Too many iterations (" << MAX_ASSIGNMENT_ITS << ")!!!!!!\n" << std::endl;
                std::cout << "Best job and nurse were: nurse: " << bestNurse << ", job: " << bestJob << std::endl;
                std::cout << "GRASP parameter delta was " << delta << std::endl;
            }
            std::cout << "The RCL was generated with " << rclSize << " candidates (Delta " << delta << ", best = " << bestRank << ", worst = " << worstRank << std::endl;
            std::cout << "ERROR: GRASP did not find ANY valid assignment, with still " << jobsRemaining << " jobs remaining.\n" << std::endl;
            std::cout << "Restricted Candidate List (elements " << rclSize << "):\n";
            for(int rcl_el = 0; rcl_el < rclSize; rcl_el++){
                std::cout << "[EL " << RCL[rcl_el] << "\tV " << rankAssignments[RCL[rcl_el]] << std::endl;
            }

            std::cout << "JOBS:\n";
            for(int jjj = 0; jjj < ip->nJobs; ++jjj){
                if(allocatedJobs[jjj] < 1){
                    std::cout << "Job " << jjj << " - Skills: [";
                    for(int sss = 0; sss < ip->nSkills; ++sss){
                        std::cout << ip->jobRequirements[jjj][sss] << ", ";
                    }
                    std::cout << "]" << std::endl;
                }
            }
            std::cout << "STAFF:\n";
            for(int nnn = 0; nnn < ip->nNurses; ++nnn){
                std::cout << "Nurse " << nnn << " - Skills: [";
                for(int sss = 0; sss < ip->nSkills; ++sss){
                    std::cout << ip->nurseSkills[nnn][sss] << ", ";
                }
                std::cout << "]" << std::endl;
            }
            std::cout << std::endl;
            exit(-1);
        }

        int cJob = bestJob;
        int cNurse = bestNurse;
        if(BestJobInsertion(ip, cJob, cNurse) > -1){ //If cJob has been successfully inserted into cNurse's route
            if(ip->doubleService[cJob]){ //If cJob is a double service (doubleService[cJob] == 1).
                allocatedJobs[cJob] += 0.5; //Mark cJob as being allocated to a nurse
                if(allocatedJobs[cJob] >= 0.6){ //If cJob has been allocated two nurses, then the job has been fulfilled!
                    jobsRemaining--;
                }
            }
            else{ //if cJob is not a double service, then mark cJob as being allocated to a single nurse, and the job has been fulfilled.
                allocatedJobs[cJob] = 1;
                jobsRemaining--;
            }
        }

    } //End of while(jobsRemaining > 0) loop

    // Deallocate memory
    //free(rankAssignments);
    //free(RCL);
    //for(int i = 0; i < ip->nNurses*ip->nJobs; ++i){
    //    free(bestIndices[i]);
    //}
    //free(bestIndices);
    //free(nurseSeed);
    //free(allocatedJobs);

    if(DEBUG_PRINT > 0){
        std::cout << "End solmatrix:\n";
        PrintSolMatrix(ip);
        std::cout << "\n-------------- END OF RANDOMISED CONSTRUCTIVE -------------- \n\n" << std::endl;
    }

    return;

} // END OF RandomisedConstructive function.

void GenerateRCL(double delta, std::vector<int>& rclSeeds, int& rclSize, std::vector<double>& rankValues, int rvSize, double bestRank, double worstRank, int rclStrategy){

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
    double hardCutOff = -bigM
    +1; // To avoid introducing infeasibility (those get -bigM)
    if(hardCutOff > bestRank){
        hardCutOff = -DBL_MAX; // If the best is infeasible, little we can do!
    }

    double reallyBadValue = bestRank - bigM;
    if(hardCutOff < reallyBadValue){
        hardCutOff = reallyBadValue;
    }

    if(rclStrategy == 1){
        worstAllowed = bestRank - delta*fabs(bestRank - worstRank); // C = B - delta(B - W)
    }
    else if(rclStrategy == 2){
        worstAllowed = bestRank - fabs(bestRank)*delta; // C = B - delta*B
    }
    else{
        printf("ERROR: In GenerateRCL() the value of 'strategy' is %d, which is not understood\n", rclStrategy);
        exit(-321);
    }

    if(worstAllowed > bestRank){ //If this is true, then no solutions can be added to the RCL because no solutions are better than the cut off value.
        printf("Warning: Something went wrong, worstAllowed > bestRank");
        printf("Creating list with strategy = %d and delta = %.9f\n>>\tBest: %.2f, Worst: %.2f, Worst allowed: %.2f, hardCutOff: %.2f\n",
               rclStrategy, delta, bestRank, worstRank, worstAllowed, hardCutOff);
        printf("delta*abs(bestRank - worstRank) = %.9f\n", delta*abs(bestRank - worstRank));
        printf("abs(bestRank - worstRank) = %d\n", abs(bestRank - worstRank));
        printf("fabs(bestRank - worstRank) = %.9f\n", fabs(bestRank - worstRank));
        printf("bestRank - worstRank = %.9f\n", bestRank - worstRank);
        printf("abs(bestRank)*delta = %.9f\n", abs(bestRank)*delta);
    }

    // Handle a hard cut-off value to avoid too many infeasibilities when constructing solutions
    if(worstAllowed < hardCutOff){
        worstAllowed = hardCutOff;
        if(bestRank < -DBL_MAX + 100){
            printf("ERROR: The best candidate in the RCL is infeasible (!)(!)(!)\n");
            exit(-124);
        }
    }

    if(rclStrategy == 1 || rclStrategy == 2){
        // printf("Started creating list. Delta: %.2f, Best: %.2f, Worst: %.2f, Worst allowed: %.2f\n", delta, bestRank, worstRank, worstAllowed);
        //(*rclSize) = 0; // No elements in the RCL to begin with.
        rclSize = 0; // No elements in the RCL to begin with.
        for(int i = 0; i < rvSize; i++){ // For i = 0,...,nJobs
            if(rankValues[i] > worstAllowed - tolerance){ //If quality of solution using job i (inserting job i into nurse's route) is better than cut-off value.
                //(*rclSize)++; //Increase the size of the RCL by one, as we are going to add an element to the RCL.
                rclSize++; //Increase the size of the RCL by one, as we are going to add an element to the RCL.
                // printf("\tAdded element number %d to the list, value: %.2f\n", (*rcl_size), rankValues[i]);
                //Add job i to the RCL:
                //rclSeeds[(*rclSize) - 1] = i; // The (*rcl_size - 1)th element of RCL_seeds is set to job i (have to use -1 because array index starts from 0 but rcl_size =1 means first element in RCL).
                rclSeeds[rclSize - 1] = i; // The (*rcl_size - 1)th element of RCL_seeds is set to job i (have to use -1 because array index starts from 0 but rcl_size =1 means first element in RCL).
            }
        }
    }


    //if((*rclSize) < 1){ //If no elements have been added to the RCL
    if(rclSize < 1){ //If no elements have been added to the RCL
        printf("WARNING! RCL in GRASP has no candidates!\n");
        printf("Creating list with strategy = %d and delta = %.9f\n>>\tBest: %.2f, Worst: %.2f, Worst allowed: %.2f, hardCutOff: %.2f\n", rclStrategy, delta, bestRank, worstRank, worstAllowed, hardCutOff);
        printf("There were %d to choose from (-DBL_MAX omitted)\n", rvSize);
        for(int iii = 0; iii < rvSize; iii++){
            if(rankValues[iii] > -DBL_MAX + 100){
                printf("\t%d\t%.2f\n", iii, rankValues[iii]);
            }
        }
        exit(-1);
    }

} //END OF GenerateRCL function.

void RCLPick(std::vector<std::vector<int> >& bestIndices,std::vector<int>& RCL, int rclSize, int& cNurse, int& cJob) {

    // This function randomly picks an element from the RCL, which returns to us the indicies for the job-nurse assignment (cNurse and cJob).
    // Picks one random element from bestIndices. The index of bestIndices comes from RCL_seeds. The elements considered of RCL_seeds are only the first rcl_size elements. Output is saved in cNurse and cJob.

    int rd_int = PickInteger(rclSize);

    int el = RCL[rd_int]; //el = element, (should be) selected at random from RCL_seeds.

    //Job-nurse assignment
    //(*cNurse) = bestIndices[el][0]; //cNurse = the nurse for job 'el'
    //(*cJob) = bestIndices[el][1]; //cJob = the job for job 'el' to go into cNurse's route.
    cNurse = bestIndices[el][0]; //cNurse = the nurse for job 'el'
    cJob = bestIndices[el][1]; //cJob = the job for job 'el' to go into cNurse's route.

    return;

} // END OF RCLPick function.


int PickInteger(int maxInt){
    //This function picks an integer number randomly between 0 and maxInt - 1
    return (int)(rand() % (maxInt));

} // End of PickInteger function


int BestJobInsertion(struct Instance* ip, int job, int ni){
    // This function starts at position 0 of nurse ni's route, and trials adding 'job' into each position 0,...,nJobs + 10, calculating the quality of each solution created using the job in the new position.
    // Note that a job is inserted into a position, the quality is calculated, and then the job is removed from that position (i.e. the solution is returned to the original) before the function attempts to
    // insert the job into the next position.
    // Therefore, this function repeatedly uses the function 'InsertJobAtPosition' to test out every position.
    // The position that 'job' is inserted into in nurse ni's route that produces the best quality solution (bestQuality) is stored (bestPosition), and this position is then used to produce a solution,
    // InsertJobAtPosition(ip, job, ni, bestPosition).
    // This function returns 0 if 'job' has been inserted successfully into nurse ni's route, otherwise it returns -1 (job cannot be inserted into any position in nurse ni's route).

    int bestPosition = -1*bigM; //New
    double bestQuality = -1*bigM;
    int firstSwitch = 0; // Is this needed?

    if(InsertJobAtPosition(ip, job, ni, 0) < 0){ // Position = 0, if job cannot be inserted into nurse ni's route in position 0.
        if(ip->solMatrix[ni][job] < 0){ //If the job cannot be inserted into position 0 and the job is NOT already assigned to nurse ni, then print debug statement
            printf("ERROR: Job %d cannot be inserted into nurse %d in position 0, but job is NOT in nurse's route!\n", job, ni);
        }
        return -1;
        //printf("WARNING / DEBUG / position 0 failed. Job: %d, Nurse: %d\n", job, ni);
        //printf("Job: %d, Nurse: %d\n", job, ni);
    }
    else{ // Job has been inserted into nurse ni's route in position 0
        bestPosition = 0;
        bestQuality = SolutionQualityLight(ip); // Solution quality of ip with new job insertion
        RemoveJob(ip, job, ni); // Reverse job insertion, go back to original solution
        //printf("Insertion into position 0 successful. Job: %d, Nurse: %d\n", job, ni);
    }

    //int bestPosition = 0; // OLD: Because currently we've only inserted a job in position 0 of nurse ni's route (above), so best position is 0.
    // NOTE: This is incorrect (16/05/2021), as above we've removed the 'return -1' if a job can't be inserted into position 0. So, the bestPosition is not necessarily 0!
    // Changing this so that bestPosition = -1*bigM at the start of the function, and is set to =0 if insertion into position =0 is successful
    double propQuality = -1;
    for(int j = 1; j < ip->nJobs; ++j){ // Why nJobs + 10? start from j=1 because we've already done position 0 (above).
        if(InsertJobAtPosition(ip, job, ni, j) < 0){ // If job cannot be inserted into nurse ni's route in position j, exit for loop.
            break;
        }
        else{
            propQuality = SolutionQualityLight(ip); // Job has been inserted into nurse ni's route into position j, so calculate quality of this new solution
            if(propQuality > bestQuality){ // If this solution has better quality than current best quality solution
                bestPosition = j; // The best position to insert job is updated to position j
                bestQuality = propQuality; // The quality of solution with best position of inserted job is updated.
                if(firstSwitch > 0){ // Is this needed?
                    return 0;
                }
            }
            RemoveJob(ip, job, ni); //Reverse job insertion, go back to original solution.
        }
    }

    //We have found the best position in nurse ni's route to put job, so now we insert job into that best position.
    //InsertJobAtPosition(ip, job, ni, bestPosition);
    //return 0;
    if(bestPosition >= 0){
        InsertJobAtPosition(ip, job, ni, bestPosition);
        return 0;
    }
    else{
        printf("ERROR: bestPosition < 0, code should not reach here!");
        return -1;
    }

} //END OF BestJobInsertion function.

int InsertJobAtPosition(struct Instance* ip, int job, int ni, int posi){
    // This function attempts to push forward all of the positions of jobs in nurse ni's route and add a new job in a particular position, 'posi' in nurse ni's route.
    // This function returns retVal which is either 0 (job has been inserted into position 'posi' successfully) or -1 (job cannot be inserted into position 'posi').

    // Check the job is not there
    if(ip->solMatrix[ni][job] > -1){ //The job is already assigned to that nurse.
        //printf(" <> Job there. job: %d, nurse: %d, solMatrix position: %d, posi: %d <>\n", job, ni, ip->solMatrix[ni][job], posi);
        return -1;
    }

    // Check nurse can do the job
    if(CheckSkills(ip, job, ni) < 1 && ip->doubleService[job] < 0.5){ //why 0.5? doubleService is an int array.
        printf(" <> Unskilled <>\n"); //Nurse is not skilled to do the job (nurseSkilled[ni][job] = 0) and the job is not a double service.
        return -1;
    }

    int retVal = -1;
    int avJobs = 0; //available jobs? what does this mean?
    //This for loop checks which jobs are already in nurse ni's route, and push each of these jobs forward one position IF they are positioned later in the nurse's route than position 'posi', so that the 'job' can
    //be inserted into the position 'posi'.
    for(int i = 0; i < ip->nJobs; ++i){ //Go through all jobs i = 0,...,nJobs-1
        if(ip->solMatrix[ni][i] > -1){ //If nurse ni has been assigned job i, i.e. job i is in nurse ni's route
            avJobs++; //Is this right to check >-1 when this has been done at the beginning of the function and return -1?
        }
        else{ //Otherwise, if the nurse has not been assigned job i, skip to the next job
            continue;
        }
        //solMatrix[ni][i] contains the position of the i'th job in nurse ni's route.
        if(ip->solMatrix[ni][i] >= posi){ //If the position of job i in nurse ni's route is >= posi, i.e. job i is positioned later in the route than posi
            retVal = 0;
            ip->solMatrix[ni][i]++; //Increase the position of job i in nurse ni's route by one, so push forward job i in the route.
        }
    }

    if((retVal==0) || (avJobs==posi)){
        ip->solMatrix[ni][job] = posi; //Put job in position 'posi' in nurse ni's route.
        SetNurseRoute(ip, ni); //Update allNurseRoutes for nurse ni.
        SetTimesFrom(ip, ni);
        return 0;
    }
    SetNurseRoute(ip, ni); // Update allNurseRoutes for nurse ni.

    return retVal;

} // End of InsertJobAtPosition function


int RemoveJob(struct Instance* ip, int job, int ni){
    if(ip->solMatrix[ni][job] < 0){
        return -1; // Job isn't here
    }

    for(int i = 0; i < ip->nJobs; ++i){
        if(job==i){
            continue;
        }
        if(ip->solMatrix[ni][i] > ip->solMatrix[ni][job]){
            --ip->solMatrix[ni][i];
        }
    }
    ip->solMatrix[ni][job] = -1;

    // printf("Removing job %d from nurse %d\n", job, ni);
    SetNurseRoute(ip, ni);
    SetTimesFrom(ip, ni);

    return 0;

} // End of RemoveJob function


double SolutionDissimilarity(struct Instance* input1, struct Instance* input2){

    /* Calculate a measure of dissimilarity between two solutions.	It is a combination of three parts:
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
    for(int i = 0; i < input1->nNurses; ++i){ //For i = 0,...,nNurses
        if(input1->nurseOrder[i] != input2->nurseOrder[i]) //If the nurse in nurseOrder[i] for input 1 is NOT the same nurse in nurseOrder[i] for input 2 (i.e. nurses are in different order)
            nurse_order_value++;
    }

    // Calculate assignment dissimilarity and arc dissimilarity
    for(int i = 0; i < input1->nNurses; ++i){ //For i = 0,...,nNurses
        for(int j = 0; j < input1->nJobs; ++j){ //For j - 0,...,nJobs
            if(input1->solMatrix[i][j] < 0){ //If in ip solution, job j is not in nurse i's route, go to next job (++j)
                continue;
            }
            if(input2->solMatrix[i][j] < 0){ //If job j IS in nurse i's route in ip soln but is NOT in nurse i's rute in pool[ps] solution, then there's a difference in the two solutions (ip and pool[ps])
                assignment_value++;
            }

            int i_des = FindArcDestination(input1, i, j); //See where this arc goes: i_des = the job# after job j in nurse i's route in ip solution, if j is the last job in nurse i's route then i_des = -1 (nurse goes back to depot).
            // So we have an arc j -> i_des, find out where this job goes to on input2

            int arc_dissimilar = 1; // Assume the arc is not there (unless we find it)
            for(int nu = 0; nu < input2->nNurses; ++nu){ //For each nurse nu = 0,...,nNurses (for pool[ps])
                if(input2->solMatrix[nu][j] > -1){ //If job j is in nurse nu's route in pool[ps] solution
                    int i_des2 = FindArcDestination(input2, nu, j); // i_des2 = the job# after job j in nurse nu's route in pool[ps] soln, if j is the last job in nu's route then i_des2 = -1 (nu returns back to depot)

                    //If there's a job after j in nu's route AND the job after j in nu's route is the same job as the job after j in nurse i's route (i.e. both nurse i and nurse nu have the same job j
                    // in their routes AND the job AFTER job j in both of their routes is the SAME job - so both nurse's have job j going to job k in their routes)
                    // OR if there is NO job after job j in nurse i's route AND there's NO job after job j in nurse nu's route AND nurse i and nurse nu are the SAME nurses
                    // (i.e. the job j in both nurse i and nurse nu's route are the last job in their route, and the nurse's are actually the same nurses!)
                    if(((i_des2 != -1) && (i_des2 == i_des)) || ((i_des == -1) && (i_des2 == -1) && (nu == i))){ //Then there's no dissimilarity!
                        arc_dissimilar = 0;
                        break;
                    }
                    else{ //There's a dissimilarity
                        if(input1->doubleService[j]){ //If job j is a double service, continue (go to next nurse nu, (++nu))
                            continue;
                        }
                        else{ //If j is not a double service, break out of the for nu loop.
                            break;
                        }
                    }
                }
            } // End for (int nu = 0; nu < input2->nNurses; ++nu) - looking for arcs in input2

            route_value += arc_dissimilar; //Number of arc dissimilarities

            // If this was the nurses's first visit (value 0), we also check (nurse depot -> point) similarity
            if((input1->solMatrix[i][j] == 0) && (input2->solMatrix[i][j] != 0)){ //If job j is nurse i's first job in ip solution AND job j is not nurse i's first job in pool[ps] solution
                route_value++; //Another arc dissimilarity has been found!
            }

        }  // End for (int j = 0; j < input1->nJobs; ++j)
    } // End for (int i = 0; i < input1->nNurses; ++i)

    // d(S_1, S_2) = d_o(O_1,O_2) + d_a(R_1, R_2) + d_r(R_1, R_2)
    double d_value = weight1*nurse_order_value + weight2*assignment_value + weight3*route_value;

    return (d_value);

} //END OF SolutionDissimilarity function.

int FindArcDestination(struct Instance* ip, int sourceNurse, int sourceJob){

    /* Given a nurse/job combination, find out where the nurse goes next to (the arc)
      If there is no "next job" we assume it goes back to depot, noted by "-1"
    */
    // This function find the job in the next position of source_nurse's route after the job source_job.
    // If there is a job after source_job in source_nurse's route, then this function returns that job, otherwise it returns -1 (which means that there are no other jobs in source_nurse's route,
    // source_job is the last job in source_nurse's route and so the nurse ends up returning to the depot).

    int c_pos = ip->solMatrix[sourceNurse][sourceJob]; //c_pos position of job in nurse's route, 'current position'
    /*int n_pos = c_pos++;*/ /// 'next position'. Increment is AFTER, so n_pos = c_pos, and THEN c_pos is increased by one, should this be ++c_pos or just c_pos + 1 instead as we're trying to find the NEXT position in the route?
    int n_pos = c_pos+1; // Changed from c_pos++ to cpos+1, 26/12/2020.
    int d_pos = -1; // If we don't find the following point, assume it goes back to depot
    for (int i = 0; i < ip->nJobs; ++i) { //For each job i = 0,...,nJobs
        if (ip->solMatrix[sourceNurse][i] == n_pos) { //If the position of job i in nurse's route is in the NEXT position of nurse's route after position c_pos
            d_pos = i; //Store the job i as d_pos
            break;
        }
    }

    return(d_pos);

} //END OF FindArcDestination function.

double SolutionQuality(struct Instance* ip, int report){
    if(report > 0){
        printf("-------- SolutionQuality(ip, %d) --------\n\n", report);
    }

    // printf("\n\n********\nStarted SolutionQuality():\nSetting times...\n");

    if(report == -98765){
        printf("Initial allNurseRoutes:\n");
        PrintAllNurseRoutes(ip);
    }

    // printf("Solmatrix when starting SolutionQuality\n");
    // print_solmatrix(ip);

    // Set all nurse routes, as these get used multiple times in this function
    SetAllNurseRoutes(ip);
    if(report == -98765){
        printf("After SetAllNurseRoutes function:\n");
        PrintAllNurseRoutes(ip);
    }
    if(report == -98765){
        printf("\nNurse Order:\n");
        for(int i = 0; i < ip->nNurses; ++i){
            int nurse = ip->nurseOrder[i];
            printf("%d ", nurse);
        }
        printf("\n");
    }

    // printf("All routes set.\n");
    SetTimesFull(ip);
    /*if (report > 0)
        printf("Done.\nSynchronising jobs...\n");
        SynchroniseJobs(ip);
    if (report > 0)
        printf("Done.\nReporting quality...\n"); */

    //double quality = Objective(ip, report);
    double quality = ObjectiveFunction(ip, report); // for testing, 05/11/2021

    if(report > 0){
        printf("Nurse order: ( ");
        for(int i = 0; i < ip->nNurses; ++i){
            printf("%d,  ", ip->nurseOrder[i]);
        }
        printf(" )\n");
        printf("-------- finished. SolutionQuality(ip, %d) --------\n\n", report);
    }
    // exit(-32423452359238749);
    // printf("\nsol_quality() = %.2f", quality);

    return quality;

}// End of SolutionQuality function


double SolutionQualityLight(struct Instance* ip){
    /* Same as SolutionQuality, but assumes that nurse routes
    are already correct, and are not recalculated */

    SetTimesFull(ip);

    double lightQuality = ObjectiveFunction(ip, -1); // For testing, 05/11/2021

    return (lightQuality);

}// End of SolutionQualityLight function


double ObjectiveFunction(struct Instance* ip, int report){

    if(ip->verbose < 0){
        report = -1;
    }
    //report = -5959;

    int feasible = 1;
    int job = -1;
    int lastPosition = -1;
    double totalWaitingTime = 0;
    double totalTardiness = 0;
    double totalMKTardiness = 0;
    double maxTardiness = 0;
    double totalTime = 0;
    double tardiness = 0;
    double arriveAt = 0;
    double totalTravelTime = 0;
    double totalOvertime = 0;
    double maxSpareTime = -bigM;
    double minSpareTime = bigM;
    double dayWork = 0;
    double longestDay = 0;
    double shortestDay = bigM;
    double sparetime = 0;
    double overtime = 0;
    double finishTime = 0;
    double tTime = 0;
    double maxOvertime = 0;
    double avgSpare = 0;
    //double totalServiceTime = 0;
    ip->totalPref = 0;

    //Update totalService time
    /*for (int j = 0; j < ip->nJobs; ++j){
        totalServiceTime += ip->jobTimeInfo[j][2];
    }*/

    for(int i = 0; i < ip->nNurses; ++i){ // For each index i in nurseOrder
        int ni = ip->nurseOrder[i]; //ni - nurse[i] in nurseOrder
        totalTravelTime += ip->nurseTravelTime[ni]; //Add (total) travel time for nurse ni to total travel time (for all nurses).
        totalWaitingTime += ip->nurseWaitingTime[ni]; // Add (total) waiting time for nurse ni to total waiting time (for all nurses)
        if(report > 0){
            int* timeNWTStart;
            timeNWTStart = MinsToTime((double) (ip->nurseWorkingTimes[ni][0]));
            // printf("Nurse %d. Start time: %.2f\t", ni, (double)(ip->nurseWorkingTimes[ni][0]));
            printf("Carer %d. Start time: %d:%d:%d, ", ni, *timeNWTStart, *(timeNWTStart + 1), *(timeNWTStart + 2));
            int* timeNWTEnd;
            timeNWTEnd = MinsToTime((double) (ip->nurseWorkingTimes[ni][1]));
            printf("End time: %d:%d:%d, ", *timeNWTEnd, *(timeNWTEnd + 1), *(timeNWTEnd + 2));
            int* timeNWTShift;
            timeNWTShift = MinsToTime((double) (ip->nurseWorkingTimes[ni][2]));
            printf("Length of shift: %d:%d:%d\n", *timeNWTShift, *(timeNWTShift + 1), *(timeNWTShift + 2));
        }

        // If the nurse route is empty:
        if(ip->allNurseRoutes[ni][0] < -0.5){ //If there is no job in position 0 (first position) of nurse ni's route (therefore [ni][0] = -1)
            // Spare time is whole day:
            sparetime = (double) ip->nurseWorkingTimes[ni][1] - (double) ip->nurseWorkingTimes[ni][0]; // = finish time of nurse ni - start time of nurse ni
            avgSpare += sparetime;
            finishTime = (double) ip->nurseWorkingTimes[ni][0]; // = start time of nurse ni, since ni doen't have any jobs so finish time is the same as start time
            /*if (shortestDay > finishTime) {
				shortestDay = finishTime;
			}*/
            shortestDay = 0.0; // If there is a nurse whose route is empty, then that nurse is not being used, and so the shortest shift time is 0.
            dayWork = 0;
            if(sparetime > maxSpareTime){
                maxSpareTime = sparetime;
            }
            if(sparetime < minSpareTime){
                minSpareTime = sparetime;
            }
            if(report > 0){
                printf("\tEmpty route for Carer %d, setting spare time to: %.2f\n\n", ni, sparetime);
            }
            continue;
        }

        for(int p = 0; p < ip->nJobs + 1; ++p){ //For each POSITION 0,...,nJobs + 1
            if(p >= ip->nJobs){ // If position is >= number of jobs, then end of route, can't have more positions than jobs in route
                job = -1; // We always need to identify a "-1" to understand that's the end of the route
            }
            else{
                job = ip->allNurseRoutes[ni][p]; // job = job # at position p in ni's route
            }
            if(job < -0.1){ // if job = -1 then end of route
                if(p > 0){ //If p > 0 then not first position in route
                    lastPosition = ip->allNurseRoutes[ni][p-1]; //last position = job at the last position of ni's route (p-1 since job = -1 so p is out of bounds, need to go to previous position for last job)
                }
                else{ //position = 0, so there is no job before in the route.
                    lastPosition = -1;
                }
                // printf("\t>> That was the last job for nurse %d (lastPosition = %d).\n", ni, lastPosition);
                break;
            }

            ip->totalPref += ip->prefScore[job][ni];
            arriveAt = ip->timeMatrix[ni][job];
            totalMKTardiness += ip->violatedTWMK[job];

            if(ip->doubleService[job] < 0.5){ // If job is not a DS
                totalTardiness += ip->violatedTW[job];
                if(ip->violatedTW[job] > maxTardiness){
                    maxTardiness = ip->violatedTW[job];
                }
            }
            else{
                // Add only half of the time if DS (EACH JOB CONTAINS ALREADY TWICE THE TARDINESS), so in total we have the tardiness for both of them (as in Mankowska)
                double realDSTardiness = ip->violatedTW[job]/2;
                totalTardiness += realDSTardiness;
                if(realDSTardiness > maxTardiness){
                    maxTardiness = realDSTardiness;
                }
            }

            if(report > 0){
                if(ip->doubleService[job] > 0){ // If job is DS
                    int* timeArriveDS;
                    timeArriveDS = MinsToTime(arriveAt);
                    // printf("\tJob %d (DS) arrives at %.2f, ", job, arriveAt);
                    printf("\tJob %d (DS) arrives at %d:%d:%d, ", job, *timeArriveDS, *(timeArriveDS + 1), *(timeArriveDS + 2));
                }
                else{
                    int* timeArrive;
                    timeArrive = MinsToTime(arriveAt);
                    // printf("\tJob %d arrives at %.2f, ", job, arriveAt);
                    printf("\tJob %d arrives at %d:%d:%d, ", job, *timeArrive, *(timeArrive + 1), *(timeArrive + 2));
                }
                int* timeTWStart;
                timeTWStart = MinsToTime((double) (ip->jobTimeInfo[job][0]));
                // printf("prefScore: %.2f, [TW %d - %d], ", ip->prefScore[job][ni], ip->jobTimeInfo[job][0], ip->jobTimeInfo[job][1]);
                printf("prefScore: %.2f, [TW %d:%d:%d", ip->prefScore[job][ni], *timeTWStart, *(timeTWStart + 1), *(timeTWStart + 2));
                int* timeTWEnd;
                timeTWEnd = MinsToTime((double) (ip->jobTimeInfo[job][1]));
                printf(" - %d:%d:%d], ", *timeTWEnd, *(timeTWEnd + 1), *(timeTWEnd + 2));

                if(ip->dependsOn[job] > -1){
                    printf("(Dep %d [%d - %d]) ", ip->dependsOn[job], ip->mkMinD[job], ip->mkMaxD[job]);
                }

                double readyToNext = arriveAt + (double) ip->jobTimeInfo[job][2];
                int* timeDuration;
                timeDuration = MinsToTime((double) (ip->jobTimeInfo[job][2]));
                // printf("duration(ST): %.d ", ip->jobTimeInfo[job][2]);
                printf("duration(ST): %d:%d:%d", *timeDuration, *(timeDuration + 1), *(timeDuration + 2));
                if((p < ip->nJobs - 1) && (ip->allNurseRoutes[ni][p + 1] > -1)){
                    int* timeTR;
                    double travelTime = GetTravelTime(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]);
                    timeTR = MinsToMinSecs(travelTime);
                    // printf("travel(TR): %.2f ", get_travel_time(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]));
                    printf(", travel(TR): %d:%d ", *timeTR, *(timeTR + 1));
                    readyToNext += GetTravelTime(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]);
                }
                int* timeRTN;
                timeRTN = MinsToTime(readyToNext);
                // printf("[ready at: %.2f]", readyToNext);
                printf("[ready at: %d:%d:%d]", *timeRTN, *(timeRTN + 1), *(timeRTN + 2));

                //printf("\n");
                if(ip->violatedTW[job] > 0.1){
                    int* timeViolatedTW;
                    timeViolatedTW = MinsToMinSecs(ip->violatedTW[job]);
                    // printf(" *** Misses TW by %.2f ***", ip->violatedTW[job]);
                    printf(" *** Misses TW by %d:%d ***", *timeViolatedTW, *(timeViolatedTW + 1));
                }

                if(ip->violatedTWMK[job] > 0.1){
                    //printf("\t *** Misses MK TW by %.2f (Mind %d, Maxd %d)***\n", ip->violatedTWMK[job], ip->mkMinD[job], ip->mkMaxD[job]);
                    printf(" *** Misses MK TW by %.2f (Mind %d, Maxd %d)***", ip->violatedTWMK[job], ip->mkMinD[job], ip->mkMaxD[job]);
                }
                printf("\n");
            }
        } // End for (int p = 0; p < ip->nJobs + 1; ++p) (positions), end analysing route of nurse ni

        if(lastPosition > -0.5){ // If the nurse went somewhere...
            tTime = (double) TravelTimeToDepot(ip, ni, lastPosition);
            tTime += (double) ip->jobTimeInfo[lastPosition][2];
            finishTime = (double) (ip->timeMatrix[ni][lastPosition] + tTime);
            dayWork = (double) finishTime - (double) ip->nurseWorkingTimes[ni][0];
            if(shortestDay > dayWork){
                shortestDay = dayWork;
            }
        }

        overtime = finishTime - (double) ip->nurseWorkingTimes[ni][1];
        sparetime = std::max(0.0, -1*overtime);
        //totalTime += dayWork;

        if(sparetime > maxSpareTime){
            maxSpareTime = sparetime;
        }
        if(sparetime < minSpareTime){
            minSpareTime = sparetime;
        }
        if(overtime < 0){
            sparetime = -1*overtime;
        }
        else{
            totalOvertime += overtime;
            if(overtime > maxOvertime){
                maxOvertime = overtime;
            }
            sparetime = 0;
        }
        if(longestDay < dayWork){
            longestDay = dayWork;
        }

        if(report > 0){
            int* timeFinish;
            timeFinish = MinsToTime(finishTime);
            // printf("Finishes: %.2f Overtime: %.2f Waiting time: %.2f\n", finishTime, overtime, ip->nurseWaitingTime[ni]);
            printf("Finish time: %d:%d:%d, ", *timeFinish, *(timeFinish + 1), *(timeFinish + 2));
            if(overtime < 0){
                double overtimePositive = overtime*-1;
                int* timeOTPositive;
                timeOTPositive = MinsToTime(overtimePositive);
                printf("Overtime: -%d:%d:%d, ", *timeOTPositive, *(timeOTPositive + 1), *(timeOTPositive + 2)); // Negative overtime, put '-' in front of time.
            }
            else if(overtime > 0){ // Postive overtime.
                int* timeOT;
                timeOT = MinsToTime(overtime);
                printf("Overtime: %d:%d:%d, ", *timeOT, *(timeOT + 1), *(timeOT + 2));
            }
            else{
                printf("Overtime: %.2f, ", overtime);
            }
            int* timeWaiting;
            timeWaiting = MinsToTime(ip->nurseWaitingTime[ni]);
            printf("Waiting time: %d:%d:%d\n\n", *timeWaiting, *(timeWaiting + 1), *(timeWaiting + 2));
        }

    } // End for (int i = 0; i < ip->nNurses; ++i) main loop

    /*** NEW WORKLOAD BALANCE CALCULATION - 03/11/2021 ***/
    /* Calculate the total service time for each nurse, and divide by the total shift length (working time) of each nurse.
     * NEED TO MAKE SURE SHIFT LENGTH FOR NURSE IS NOT JUST FROM BEGINNING TO END OF THE DAY, MUST ONLY BE WORKING AVAILABLE SHIFT TIME, NOT UNAVAILABLE TIME.
     * Then, calculate average and subtract proportion of each nurse from average.
     * Select the maximum value.
     */

    int maxDiffWorkloadNurseIndex = -1;
    int maxTravelTimeNurseIndex = -1;
    int maxWaitingTimeNurseIndex = -1;
    double maxDiffWorkload = 0.0;
    double maxTravelTime = 0.0;
    double maxWaitingTime = 0.0;

    std::vector<int> jobsAssigned(ip->nJobs);
    std::vector<int> nurseServiceTime(ip->nNurses);
    std::vector<double> nurseWorkload(ip->nNurses);
    std::vector<double> diffWorkloads(ip->nNurses);
    //int* jobsAssigned = malloc(ip->nJobs * sizeof(int));
    //int* nurseServiceTime = malloc(ip->nNurses * sizeof(int));
    //double* nurseWorkload = malloc(ip->nNurses * sizeof(double));
    //double* diffWorkloads = malloc(ip->nNurses * sizeof(double));

    //Calculate the number of jobs that have actually been assigned.
    for(int i = 0; i < ip->nJobs; ++i){
        jobsAssigned[i] = 0;
    }

    // Go through allNurseRoutes, if a job has been assigned then mark it as checked in jobsAssigned array. (This helps us ensure that we don't count double service jobs as two separate jobs if we were to just count the number of >=0 numbers in allNurseRoutes)
    for(int i = 0; i < ip->nNurses; ++i){
        for(int p = 0; p < ip->nJobs; ++p){
            if(ip->allNurseRoutes[i][p] < 0){
                continue;
            }
            else{
                //jobsAssigned[ip->allNurseRoutes[i][p]] = 1;
                jobsAssigned[ip->allNurseRoutes[i][p]] += 1;
            }
        }
    }

    //Count number of jobs that have been assigned.
    int numJobsAssigned = 0;
    for(int i = 0; i < ip->nJobs; ++i){
        numJobsAssigned += jobsAssigned[i];
    }


    if(numJobsAssigned == ip->nJobsIncDS){ // If all jobs have been assigned.
        // 1. Calculate total service time for each nurse.
        for(int i = 0; i < ip->nNurses; ++i){
            int ni = ip->nurseOrder[i];
            nurseServiceTime[ni] = 0;
            for(int p = 0; p < ip->nJobs; ++p){
                //printf("allNurseRoutes[%d][%d] = %d \n", ni, p, ip->allNurseRoutes[ni][p]);
                if(ip->allNurseRoutes[ni][p] < 0){ // Then there is no job in position p, value of allNurseRoutes[ni][p] = -1, so it's the end of the route.
                    break;
                }
                else{
                    //printf("jobTimeInfo[allNurseRoutes[%d][%d]][2] = %d \n", ni, p, ip->jobTimeInfo[ip->allNurseRoutes[ni][p]][2]);
                    nurseServiceTime[ni] += ip->jobTimeInfo[ip->allNurseRoutes[ni][p]][2]; // add length of job j (allNurseRoutes[ni][p]) to nurseServiceTime
                }
            }
        }

        //3. Calculate nurseWorkload - proportion for each nurse: total service time/total shift length.
        for(int i = 0; i < ip->nNurses; ++i){
            int ni = ip->nurseOrder[i];
            nurseWorkload[ni] = 0;
            if(nurseServiceTime[ni] == 0){ // no service time for that nurse, so skip (error due to dividing by 0 otherwise.
                continue;
            }
            else{
                nurseWorkload[ni] = (double)nurseServiceTime[ni] / (double)ip->nurseWorkingTimes[ni][4]; // nurseWorkingTimes[i][4] is the duration of working shift times for nurse i (not [i][2], which is the total day length for nurse i)
            }
        }

        //4. Calculate totalNurseWorkload = sum of all nurseWorkload[i]!
        double totalNurseWorkload = 0;
        for(int i = 0; i < ip->nNurses; ++i){
            totalNurseWorkload += nurseWorkload[i];
        }

        //5. Calculate average nurseWorkload proportions = sum of all nurseWorkloads divided by number of nurses.
        double avgNurseWorkload = totalNurseWorkload / (double)ip->nNurses;

        //6. Calculate difference between individual workloads and avgNurseWorkload for each nurse.
        int bigNum = -99999;

        for(int i = 0; i < ip->nNurses; ++i){
            int ni = ip->nurseOrder[i];
            diffWorkloads[ni] = -bigNum;
            if(nurseWorkload[ni] == 0){ // Nurse has no work/jobs, skip.
                continue;
            }
            else{
                diffWorkloads[ni] = nurseWorkload[ni] - avgNurseWorkload;
            }
        }

        //7. Find maximum value in diffWorkloads.
        for(int i = 0; i < ip->nNurses; ++i){
            int ni = ip->nurseOrder[i];
            if(diffWorkloads[ni] == -bigNum){
                continue;
            }
            else if(diffWorkloads[ni] > maxDiffWorkload){
                maxDiffWorkload = diffWorkloads[ni];
                maxDiffWorkloadNurseIndex = ni;
            }
        }

        //printf("maxDiffWorkload = %.2f, maxNurseDiffWorkload = %d\n", maxDiffWorkload, maxDiffWorkloadNurseIndex);


        /*** NEW: MAX TRAVEL TIME - 05/11/2021 ***/
        // Find the maximum travel time of all nurses
        // Need to have new alpha value in algorithmOptions! alpha_8
        // What do we do here if not all jobs have been assigned yet?
        for(int i = 0; i < ip->nNurses; ++i){
            if(ip->nurseTravelTime[i] > maxTravelTime){
                maxTravelTime = ip->nurseTravelTime[i];
                maxTravelTimeNurseIndex = i;
            }
        }

        //printf("maxTravelTime = %.2f, maxTravelTimeNurseIndex = %d\n", maxTravelTime, maxTravelTimeNurseIndex);

        /*** NEW: MAX WAITING TIME - 05/11/2021 ***/
        // Find the maximum waiting time of all nurses
        // Need to have new alpha value in algorithmOptions! alpha_9
        // What do we do here if not all jobs have been assigned yet?
        for(int i = 0; i < ip->nNurses; ++i){
            if(ip->nurseWaitingTime[i] > maxWaitingTime){
                maxWaitingTime = ip->nurseWaitingTime[i];
                maxWaitingTimeNurseIndex = i;
            }
        }

        //printf("maxWaitingTime = %.2f, maxWaitingTimeNurseIndex = %d\n", maxWaitingTime, maxWaitingTimeNurseIndex);

        //report = -5959;
        //exit(-1);

        // End else (calculate workload balance, max travel time and max waiting time)
    } // End else if(numJobsAssigned == ip->nJobs)


    /* Mankowska objective: Z = A*D + B*T + C*Tmax
     * A = B = C = 1/3 (User set weights)
     * D = Total distance travelled by caregivers (OD-matrix)
     * T = Total tardiness
     * Tmax = Max tardiness
     * double quality = (totalTime + totalTardiness + maxTardiness)/3;
     */

    // double quality = -1*totalTravelTime -10000000000000*totalTardiness; // TSP-TW
    double TOL = 1e-4;
    double mkAllowedTardiness = totalOvertime + totalTardiness;
    // double mkAllowedTardiness = totalTardiness;
    double mkMaxTardiness = maxTardiness;
    if(maxOvertime > mkMaxTardiness){
        mkMaxTardiness = maxOvertime;
    }

    totalTime = totalWaitingTime + totalTravelTime + ip->totalServiceTime;

    double quality = 0.0;
    double aitQuality = 0.0;
    double mkQuality = 0.0;
    double wbQuality = 0.0;
    double paperQuality = 0.0;
    // int qualityType = ip->qualityMeasure;
    double infeasibilityM1 = ip->nNurses*12*60; // Multiplier
    double infeasibilityM2 = ip->nNurses*12*60; // Chunk sum

    //Ait H.
    aitQuality = -1*(0.3*totalTravelTime + ip->totalPref); // Real obj
    // Avoid infeasibility:
    aitQuality += -10000*(totalMKTardiness + mkAllowedTardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mkAllowedTardiness > TOL) || (totalOvertime > TOL)){
        aitQuality += -10000;
    }

    // Mankowska
    mkQuality = -1*(totalTravelTime + mkAllowedTardiness + mkMaxTardiness)/3; // Mankowska
    if(totalMKTardiness > TOL){
        mkQuality += -1000 - 10000*(totalMKTardiness);
    }

    // Workload Balance
    double dayDiff = (longestDay - shortestDay);
    wbQuality = -1*(0.3*totalTravelTime + ip->totalPref) - 0.1*dayDiff + 0.1*minSpareTime; // Real obj
    // Avoid infeasibility:
    wbQuality += -10000*(totalMKTardiness + mkAllowedTardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mkAllowedTardiness > TOL) || (totalOvertime > TOL)){
        wbQuality += -10000;
    }

    // Paper
    /*paperQuality = ip->algorithmOptions[51]*totalTravelTime +
            ip->algorithmOptions[52]*totalWaitingTime +
            ip->algorithmOptions[53]*mkAllowedTardiness +
            ip->algorithmOptions[54]*totalOvertime +
            ip->algorithmOptions[55]*minSpareTime +
            ip->algorithmOptions[56]*ip->totalPref +
            ip->algorithmOptions[57]*mkMaxTardiness;
    if(ip->algorithmOptions[50] > 0.5){
        // No tardiness of any type is allowed (including overtime)
        if((totalMKTardiness > TOL) || (mkAllowedTardiness > TOL) || (totalOvertime > TOL)){
            paperQuality += -infeasibilityM1*(totalMKTardiness + mkAllowedTardiness + totalOvertime);
            paperQuality += -infeasibilityM2;
        }
    }
    else{
        // Tardiness and overtime are allowed, but infeasibleTardiness is not (double ups, gaps)
        if(totalMKTardiness > TOL){
            paperQuality += -infeasibilityM1*totalMKTardiness;
            paperQuality += -infeasibilityM2;
        }
    }*/

    // New Paper Quality: 06/11/2021
    paperQuality = ip->algorithmOptions[51]*totalTravelTime
            + ip->algorithmOptions[52]*totalWaitingTime
            + ip->algorithmOptions[53]*mkAllowedTardiness
            + ip->algorithmOptions[54]*totalOvertime
                    //+ ip->algorithmOptions[55]*minSpareTime
            + ip->algorithmOptions[56]*ip->totalPref
            + ip->algorithmOptions[57]*mkMaxTardiness
            + ip->algorithmOptions[58]*maxTravelTime
            + ip->algorithmOptions[59]*maxWaitingTime
            + ip->algorithmOptions[60]*maxDiffWorkload;
    if(ip->algorithmOptions[50] > 0.5){
        // No tardiness of any type is allowed (including overtime)
        if((totalMKTardiness > TOL) || (mkAllowedTardiness > TOL) || (totalOvertime > TOL)){
            paperQuality += -infeasibilityM1*(totalMKTardiness + mkAllowedTardiness + totalOvertime);
            paperQuality += -infeasibilityM2;
        }
    }
    else{
        // Tardiness and overtime are allowed, but infeasibleTardiness is not (double ups, gaps)
        if(totalMKTardiness > TOL){
            paperQuality += -infeasibilityM1*totalMKTardiness;
            paperQuality += -infeasibilityM2;
        }
    }

    if(ip->qualityMeasure == 0){
        quality = aitQuality;
    }
    else if(ip->qualityMeasure == 1){
        quality = mkQuality;
    }
    else if(ip->qualityMeasure == 5){
        quality = wbQuality;
    }
    else if(ip->qualityMeasure == 6){
        quality = paperQuality;
    }


    if(report==12345 || report == -5959){
        //double infeasibility_M1 = ip->nNurses*12*60; // Multiplier
        //double infeasibility_M2 = ip->nNurses*12*60; // Chunk sum
        //double dayDiff = longestDay - shortestDay;

        if(ip->qualityMeasure == 0){
            printf("QUALITY MEASURE: Ait. H (%d)\n", ip->qualityMeasure);
        }
        else if(ip->qualityMeasure == 1){
            printf("QUALITY MEASURE: Mankowska (%d)\n", ip->qualityMeasure);
        }
        else if(ip->qualityMeasure == 5){
            printf("QUALITY MEASURE: Workload Balance (%d)\n", ip->qualityMeasure);
        }
        else if(ip->qualityMeasure == 6){
            printf("QUALITY MEASURE: Paper (%d)\n", ip->qualityMeasure);
        }
        else{
            printf("QUALITY MEASURE:  No specific type given (%d)\n", ip->qualityMeasure);
        }
        printf("Number of jobs assigned. numJobsAssigned: %d, nJobsIncDS: %d \n", numJobsAssigned, ip->nJobsIncDS);
        printf("---------------------------------\n");
        printf("ALG OPTIONS:\n");
        printf("algOp[50] = %.2f (if 1, tardiness/overtime are infeasible)\n", ip->algorithmOptions[50]);
        printf("alpha1 [51] (totalTravelTime) = %.2f\n", ip->algorithmOptions[51]);
        printf("alpha2 [52] (totalWaitingTime) = %.2f\n", ip->algorithmOptions[52]);
        printf("alpha3 [53] (mkAllowedTardiness) = %.2f\n", ip->algorithmOptions[53]);
        printf("alpha4 [54] (totalOvertime) = %.2f\n", ip->algorithmOptions[54]);
        printf("alpha5 [55] (minSpareTime) = %.2f\n", ip->algorithmOptions[55]);
        printf("alpha6 [56] (totalPref) = %.2f\n", ip->algorithmOptions[56]);
        printf("alpha7 [57] (mkMaxTardiness) = %.2f\n", ip->algorithmOptions[57]);
        printf("alpha7 [58] (maxTravelTime) = %.2f\n", ip->algorithmOptions[58]);
        printf("alpha7 [59] (maxWaitingTime) = %.2f\n", ip->algorithmOptions[59]);
        printf("alpha7 [60] (maxDiffWorkload) = %.2f\n", ip->algorithmOptions[60]);

        printf("\n-------------------\n");
        printf("\nOUTPUT VARIABLES:\n");
        printf("Total Time (totalTime): %.2f\n", totalTime);
        printf("Total Travel Time (totalTravelTime) = %.2f\n", totalTravelTime);
        printf("Total Waiting Time (totalWaitingTime): %.2f\n", totalWaitingTime);
        printf("Total Service Time (totalServiceTime): %.2f\n", ip->totalServiceTime);
        printf("Total Service Time Inc DS (totalServiceTimeIncDS): %.2f\n", ip->totalServiceTimeIncDS);
        printf("Total Overtime (totalOvertime) = %.2f\n", totalOvertime);
        printf("Total Violated TW (totalTardiness) = %.2f\n", totalTardiness);
        printf("Total Violated TWMK (totalMKTardiness) = %.2f\n", totalMKTardiness);
        printf("Max Tardiness (maxTardiness) = %.2f\n", maxTardiness);
        printf("Max Tardiness MK (mkMaxTardiness) = %.2f\n", mkMaxTardiness);
        printf("Total Tardiness MK (mkAllowedTardiness) = %.2f (totalOvertime + totalTardiness)\n", mkAllowedTardiness);
        printf("Max Spare Time (maxSpareTime): %.2f\n", maxSpareTime);
        printf("Min Spare Time (minSpareTime): %.2f\n", minSpareTime);
        printf("Shortest Shift (shortestDay): %.2f\n", shortestDay);
        printf("Longest Shift (longestDay): %.2f\n", longestDay);
        printf("Shift Difference (dayDiff): %.2f\n", dayDiff);
        printf("Total Preference Score (ip->totalPref) = %.2f\n", ip->totalPref);
        printf("Max Travel Time (maxTravelTime) = %.2f\n", maxTravelTime);
        printf("Max Waiting Time (maxWaitingTime) = %.2f\n", maxWaitingTime);
        printf("Max Diff Workload (Balance) (maxDiffWorkload) = %.2f\n", maxDiffWorkload);
        printf("Quality = %.2f\n", quality);

        printf("\n------------------------------\n");
        printf("ALL QUALITIES:\n");
        printf("Ait H: %.2f\n", aitQuality);
        printf("Mankowska: %.2f\n", mkQuality);
        printf("Workload Balance: %.2f\n", wbQuality);
        printf("Paper: %.2f\n", paperQuality);
        //exit(-1);

    }

    // if (totalTardiness > 0)
    // 	ip->isFeasible = 0;
    // else
    // 	ip->isFeasible = 1;
    ip->isFeasible = feasible;

    // NB NEW: FREE NEW ADDED VARIABLES FOR PAPER QUALITY! 18/11/2021
    //free(jobsAssigned);
    //free(nurseServiceTime);
    //free(nurseWorkload);
    //free(diffWorkloads);


    // Save the details:
    // NB: updated, added new variables: 04/06/2021
    ip->objTime = totalTime;
    ip->objWaiting = totalWaitingTime;
    ip->objTravel = totalTravelTime;
    ip->objService = ip->totalServiceTime;
    ip->objTardiness = totalTardiness;
    ip->objMaxTardiness = maxTardiness;
    ip->objMKTardiness = totalMKTardiness;
    ip->objMKAllowedTardiness = mkAllowedTardiness;
    ip->objOvertime = totalOvertime;
    ip->objMaxOvertime = maxOvertime;
    ip->objMinSpareTime = minSpareTime;
    ip->objMaxSpareTime = maxSpareTime;
    ip->objShortestDay = shortestDay;
    ip->objLongestDay = longestDay;
    //ip->objAitHQuality = aitQuality;
    //ip->objMKQuality = mkQuality;
    //ip->objWBQuality = wbQuality;
    //ip->objPaperQuality = paperQuality;
    ip->objQuality = quality;
    ip->objMaxTravelTime = maxTravelTime;
    ip->objMaxWaitingTime = maxWaitingTime;
    ip->objMaxDiffWorkload = maxDiffWorkload;
    return quality;

} // End of ObjectiveNew function.


void RandomTwoExchange(std::vector<int>& array, size_t n, int* i, int* j){ //In GRASP: RandomTwoExchange(ip->nurseOrder, ip->nNurses, &exch1, &exch2)

    if(n < 2){ //If nNurses < 2, then &exch1 and &exch2 = 0, and exit.
        i = 0;
        j = 0;
        return;
    }

    //Otherwise, if nNurses >=2, then use RandomInteger function to set *i and *j to random ints between 0 and n-1
    (*i) = RandomInteger(0, n - 1);
    (*j) = RandomInteger(0, n - 1);

    //If the random_integer function coincidentally set *i and *j to be the same, use RandomInteger to choose another value for *j until *i and *j are different.
    while(*i == *j){
        (*j) = RandomInteger(0, n - 1);
    }

    // Then, perform TwoExchange - swap the elements in array[i] and array[j].
    TwoExchange(array, (*i), (*j));
}

void TwoExchange(std::vector<int>& array, int i, int j){ //In GRASP: two_exchange(ip->nurseOrder, exch1, exch2), and RandomTwoExchange: TwoExchange(array, (*i), (*j))

    //This function swaps the elements in array[i] and array[j] (element that was in array[i] is now in array [j] and vice vera)

    int t = array[j];
    array[j] = array[i];
    array[i] = t;

} //END OF TwoExchange.

void Shuffle(std::vector<int>& array, size_t n){ //In GRASP: Shuffle(ip->nurseOrder, (size_t)ip->nNurses)

    // Based on the idea from: https://benpfaff.org/writings/clc/shuffle.html
    // Or from: https://stackoverflow.com/questions/6127503/shuffle-array-in-c
    // (Ben Pfaff's Writings)

    //Randomly Shuffle elements of the array (nurseOrder)
    if(n > 1){
        size_t i;
        for(i = 0; i < n - 1; i++){
            size_t j = i + rand()/(RAND_MAX/(n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

int RandomInteger(int minVal, int maxVal){
    //return random value between min_val and max_val
    return minVal + (rand()%(maxVal + 1));
}

void CleanSolutionFromStruct(struct Instance* ip){
    for(int nurse = 0; nurse < ip->nNurses; ++nurse){
        for(int job = 0; job < ip->nJobs; ++job){
            ip->solMatrix[nurse][job] = -1;
        }
    }
}

int* MinsToTime(double time){

    // e.g. time = 590.82
    static int s_time[3];
    s_time[0] = 0;
    s_time[1] = 0;
    s_time[2] = 0;
    int minsRound = floor(time); // First part of time, e.g. 590
    double decimalMins = time - minsRound; // End part of time, e.g. 0.82
    int hours = minsRound/60; // Convert mins to hours, e.g. 590 / 60 = 9 hours
    int minutes = minsRound%60; // Get remainder of mins, e.g. 590 % 60 = 50 mins
    int seconds = round(decimalMins*60); //Convert the decimal minutes to seconds, e.g. 0.82 mins * 60 = 49 seconds
    s_time[0] = hours;
    s_time[1] = minutes;
    s_time[2] = seconds;

    return s_time;

}// End MinsToTime function

int* MinsToMinSecs(double time){
    //e.g. time = 7.52 (minutes)
    static int s_minSecs[2];
    s_minSecs[0] = 0;
    s_minSecs[1] = 0;
    int minutes = floor(time); // First part of time, e.g. 7 mins
    double decimalMins = time - minutes; // Second part of time, e.g. 0.52 mins
    int seconds = round(decimalMins*60);
    s_minSecs[0] = minutes;
    s_minSecs[1] = seconds;

    return s_minSecs;

} // End MinsToMinSecs function


