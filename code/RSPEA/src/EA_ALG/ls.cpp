/*--------------/
EA_ALG
ls.cpp
UoS
01/02/2022
/--------------*/

#include "ls.h"

double StandardLocalSearchTest(struct Instance* ip, int MAX_ITERATIONS, double MAX_TIME, int TEST_ITERATIONS){

    // This function is called after each iteration of RCA (Randomized Constructive Algorithm) and applies a local search procedure to the resulting solution of RCA to improve its quality.
    // The LS is a first-improvement VNS framework - useful because it can incorporate multiple different local search techniques/movements.
    // The LS operates by going through the list of neighbourhoods (types of LS techniques) one by one, and trying to improve the quality of the solution.
    // If the quality of the solution has not improved using that neighbourhood, then the procedure moves on to the next neighbourhood to try and improve the solution.
    // Within any neighbourhood, if the quality of the solution has improved, then the LS procedure goes back to the first neighbourhood in the list and starts the process again.
    // This is repeated until all neighbourhoods (LS techniques) have been applied to a solution and the solution has not improved.
    // The neighbourhoods used in this LS, in order, are as follows:
    // NE01 - Moving a job (BestSwitch): Move a single job from one nurse's route and insert the job into a better position in another nurse's (or the same nurse's) route.
    // NE02 - Exchanging two services (RouteTwoExchange): Swap two jobs between two nurses (that is, one job from one nurse's route is swapped with another job from another nurse's route).
    // NE03 - Nurse order change (NurseTwoExchange): Swaps the order of two nurses in the nurseOrder array.
    // NE04 - Remove and reinsert linked services (best_sync_double_service): Removes a double service job from two nurses and inserts the job into better positions in two other nurse's routes OR
    // removes dependent jobs from two nurses and inserts the jobs into better positions in two other nurse's routes (could be the same nurses in both cases but better positions).
    // NE05 - 2-OPT (TwoOptMove): Reverse the order of jobs between and including selected positions in a single nurse's route.
    // Function returns the quality of the final solution (retVale) (Note that if the while loop is still running but the number of iterations of the loop is ==TEST_ITERATIONS,
    // then the function returns the quality of the solution in the TEST_ITERATIONS'th iteration, it does not continue to find a better solution).

    // Parameter MAX_ITERATIONS is never used.

    double eps = 1e-6; // Add this small number to prevent numerical errors when reading params
    double retValue = -11;
    int test_assigned = 0;
    int useTwoOpt = (int) (ip->algorithmOptions[1] + eps); //aO[1] = 1, so NE05 2-OPT active.
    int useTwoExchange = (int) (ip->algorithmOptions[2] + eps); //aO[2] = 1, so NE02 2-exchange active.
    int useNurseOrderChange = (int) (ip->algorithmOptions[3] + eps); //aO[3] = 0, NE03 nurse order change not active. (why?)
    double quality = SolutionQuality(ip, -15); //Initial solution quality of ip.
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

    while(1 > 0){ //while true loop
        lsIters++;

        // Perform a swap: pick two nurses at random and a job at random.
        //int n1 = rand() % ip->nNurses; //not used
        //int n2 = rand() % ip->nNurses; //not used
        //int j1 = rand() % ip->nJobs; // not used
        infOnly = 0;
        twoOptDidSomething = 1;
        double b_baseQuality = SolutionQuality(ip, -16);

        if(lsIters == TEST_ITERATIONS){ //If the number of iterations is equal to TEST_ITERATIONS (which is 100), the return value is set as the quality of the current solution ip
            retValue = b_baseQuality;
            test_assigned = 1;
        }
        quality = b_baseQuality; //quality = solution quality of ip

        if(ip->verbose > 5){
            printf("Best switch (start quality: %.2f)\n", b_baseQuality);
        }

        end = clock();
        elapsedTime = (float) (end - start)/CLOCKS_PER_SEC;
        if(elapsedTime > MAX_TIME){ //Exceeded time limit, exit while loop
            break;
        }

        /// NE01: BestSwitch
        int bswitchValue = BestSwitch(ip, infOnly, MAX_TIME - elapsedTime); //BestSwitch attempts to move a single job from one nurse's route to another, =0 if successful, =-1 otherwise.
        if(bswitchValue > -1){ //If switch successful and solution quality has improved
            performedSwaps++;
            n1_improvements++;
            propQ = SolutionQuality(ip, -17); //quality of new solution
            n1_imp_amount = quality - propQ; //improvement amount
            quality = propQ; //quality is updated to the quality of the new solution.
            if(ip->verbose > 5){
                printf("BestSwitch improved to > %.2f\n", quality);
            }
            continue; //Start while loop again - start from first neighbourhood (this one!)
        }

        /// NE02 (2-exchange): RouteTwoExchange
        if(useTwoExchange > 0){
            end = clock();
            elapsedTime = (float) (end - start)/CLOCKS_PER_SEC;
            if(elapsedTime > MAX_TIME){ //Exceeded time limit, exit while loop
                break;
            }
            if(ip->verbose > 5){
                printf("Two exchange (start quality: %.2f)\n", quality);
            }
            int twoExchangeValue = RouteTwoExchange(ip, 1); //Swaps two jobs between two nurses (one job from one nurse is swapped with one job from another nurse), =0 if successful, =-1 otherwise.
            if(twoExchangeValue > -1){ //If swap successful and solution quality has improved
                propQ = SolutionQuality(ip, -1711);
                quality = propQ;
                if(ip->verbose > 5){
                    printf("TwoExchange improved to > %.2f\n", quality);
                }
                continue; //Start while loop again - start from first neighbourhood BestSwitch
            }
        }

        /// NE03 (Nurse order change): NurseTwoExchange
        if(useNurseOrderChange > 0){
            end = clock();
            elapsedTime = (float) (end - start)/CLOCKS_PER_SEC;
            if(elapsedTime > MAX_TIME){ //Exceeed time limit, exit while loop
                break;
            }
            int didItWork = NurseTwoExchange(ip); //Swaps two nurses in nurseOrder array, =1 if nurses swapped and solution improved, =-1 otherwise.
            if(didItWork > -1){ //If two nurses have been swapped and the solution quality has improved
                quality = SolutionQuality(ip, -1710);
                continue; //Start while loop again - start from the first neighbourhood BestSwitch
            }
        }

        /// NE04 (Best sync double switch): BestSyncDoubleSwitch
        if(ip->verbose > 5){
            printf("\tBest_sync_double_switch (start quality: %.2f)\n", SolutionQuality(ip, -18));
        }
        if(BestSyncDoubleSwitch(ip) > 0){ //If double service/dependent jobs switched successfully
            end = clock();
            elapsedTime = (float) (end - start)/CLOCKS_PER_SEC;
            if(elapsedTime > MAX_TIME){ //Exceeded time limit, exit while loop
                break;
            }
            if(ip->verbose > 5){
                printf("\tbest_sync_double_switch improved to > %.2f\n", SolutionQuality(ip, -19));
            }
            propQ = SolutionQuality(ip, -20);
            n2_improvements++;
            n2_imp_amount = quality - propQ;
            quality = propQ;
            continue; //Start while loop again - start from the first neighbourhood BestSwitch
        }

        /// NE05 (2-OPT): TwoOptMove
        if(ip->verbose > 5){
            printf("\t\t2opt (start quality: %.2f)\n", SolutionQuality(ip, -21));
        }
        twoOptDidSomething = 0;
        if(useTwoOpt > 0.5){
            double TOL = 0.1;
            double startoffwith = quality; //Initial quality of solution before changes are made

            for(int nurseidx = 0; nurseidx < ip->nNurses; nurseidx++){ //For each nurseidx=0,...,nNurses
                int nurse = ip->nurseOrder[nurseidx];
                int foundImprovement = 10;
                int nurseJobCount = GetNurseJobCount(ip, nurse); //Number of jobs in nurse's route.
                int countImprovements = 0;
                double nurseInitQ = SolutionQuality(ip, -22);
                int realOptIterations = 0;
                while(foundImprovement > 1){
                    foundImprovement = -1;
                    for(int posi = 0; posi < ip->nJobs; posi++){ //For each POSITION posi = 0,...,nJobs
                        for(int posj = posi + 1; posj < ip->nJobs; posj++){ //For each POSITION posj = posi+1,...,nJobs
                            double initq = SolutionQuality(ip, -23);
                            int res = TwoOptMove(ip, nurse, posi, posj); //Reverse order of jobs between and including positions posi and posj in nurse's route, =0 if successful, =-1 otherwise.
                            if(res < 0){ //Could not reverse order of jobs, go to next posj (++posj)
                                continue;
                            }
                            double endq = SolutionQuality(ip, -24); //quality of new solution with order of jobs reversed
                            realOptIterations++;
                            if(endq < initq + TOL){ //if solution quality has NOT improved
                                res = TwoOptMove(ip, nurse, posi, posj); //Undo TwoOptMove (return solution back to before)
                                if(res < 0){ //Job order could not be reversed
                                    printf("ERROR: Cannot undo 2opt move\n");
                                    exit(-1);
                                }
                            }
                            else if(endq + TOL > initq){ //If solution quality improved
                                if(ip->verbose > 5){
                                    printf("\t\t2opt improved to > %.2f\n", SolutionQuality(ip, -25));
                                }
                                foundImprovement = 10;
                                countImprovements++;
                                break; //Exit for loop posj, and then exit for loop posj (so back into while(foundImprovement >1) loop)
                            }
                        } //End for (int posj = posi + 1; posj < ip->nJobs; posj++) loop
                        if(foundImprovement > -1){
                            break;
                        }
                    } //End for (int posi = 0; posi < ip->nJobs; posi++) loop
                } //End while (foundImprovement > 1) loop
            } //End for (int nurseidx = 0; nurseidx < ip->nNurses; nurseidx++) loop

            // printf("2 opt went from: %.2f to %.2f (BF: %d)", startoffwith, SolutionQuality(ip, 0), backandforce);
            // END NEW 2 OPT IMPLEMENTATION
            double finishoffwith = SolutionQuality(ip, -27);
            if(finishoffwith > startoffwith + TOL){
                twoOptDidSomething = 1;
                n3_improvements++;
                n3_imp_amount = startoffwith - finishoffwith;
                quality = finishoffwith;
            }
        }

        if(ip->verbose > 5){
            printf("Two opt did something? %d\n", twoOptDidSomething);
        }
        if(twoOptDidSomething < 1){
            break;
        }
        end = clock();
        elapsedTime = (float) (end - start)/CLOCKS_PER_SEC;
        if(elapsedTime > MAX_TIME){ //Exceeded time limit
            break;
        }

    } // End of while true (while(1>0) loop.

    double final_quality = SolutionQuality(ip, -29);

    if(test_assigned < 1){
        retValue = final_quality;
    }
    //Otherwise, if test_assigned = 1, then this means that (lsIters == TEST_ITERATIONS), and so retValue is the quality of the solution in that iteration (b_baseQuality) (should not take solutions from further iterations).
    //Surely the function should just return retValue in that if statement rather than continue to go through the while true loop until the elapsedTime has exceeded the MAX_TIME?

    if(ip->verbose > 5){
        printf("Final quality: %.4f\n", final_quality);
        printf("\n\tBy neighbourhood:\n");
        printf("Neighbourhood\tMovements   \tImprovement\tAvg. Improvement\n");
        printf("1\t%d\t%.4f\t%.4f\n", n1_improvements, n1_imp_amount, n1_imp_amount/n1_improvements);
        printf("2\t%d\t%.4f\t%.4f\n", n2_improvements, n2_imp_amount, n2_imp_amount/n2_improvements);
        printf("3\t%d\t%.4f\t%.4f\n", n3_improvements, n3_imp_amount, n3_imp_amount/n3_improvements);
        printf("---- Performed %d LS iterations, %.2f seconds.----\n", lsIters, elapsedTime);
    }

    return retValue;

} //END OF StandardLocalSearchTest function.

int BestSwitch(struct Instance* ip, int onlyInfeasible, double MAX_TIME){

    //This function goes through nurses ni and attempts to move a single job from ni's route to another nurse nj's route.
    //Returns 0 if successful (i.e. job is moved and solution quality has improved), and = -1 otherwise.
    /// Problem: it only switches one job from one nurse ni to another nurse nj, it doesn't go through all of them. Is this function supposed to be like this, or is it meant to go through all of the nurses ni and jobs nj?

    clock_t start = clock();
    clock_t end = clock();
    double elapsedTime = 0;
    int firstSwitch = 1;
    double TOL = 0.0001;
    int nRows = ip->nNurses;
    int nCols = ip->nJobs;

    std::vector<std::vector<int> > solMatrixInitial(ip->nNurses, std::vector<int>(ip->nJobs));
    //int** solMatrixInitial = malloc(nRows*sizeof(int*)); // Rows
    //for(int i = 0; i < nRows; i++){
    //    solMatrixInitial[i] = malloc(nCols*sizeof(int)); // Cols
    //}

    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrixInitial[i][j] = ip->solMatrix[i][j];
        }
    }

    CopyIntMatrix(ip->solMatrix, solMatrixInitial, nRows, nCols); //solMatrixInitial = solMatrix for all [i][j].
    //printf("BestSwitch baseQuality\n");
    double baseQuality = SolutionQuality(ip, -6);

    int besti = -1; //Records best nurse ni
    int bestj = -1; //Records best nurse nj
    int best_job = -1; //Records best job job_from_ni

    for(int ni = 0; ni < ip->nNurses; ++ni){
        end = clock();
        elapsedTime = (float) (end - start)/CLOCKS_PER_SEC;
        if(elapsedTime > MAX_TIME){ //Exceeded time limit, exit for loop
            break;
        }
        for(int job_from_ni = 0; job_from_ni < ip->nJobs; ++job_from_ni){
            if(ip->solMatrix[ni][job_from_ni] < 0){ //job_from_ni is not in nurse ni's route, go to next job_from_ni (++job_from_ni)
                continue;
            }
            if((onlyInfeasible > 0.5) && (ip->violatedTW[job_from_ni] < TOL)){
                continue;
            }
            for(int nj = 0; nj < ip->nNurses; ++nj){
                //Note: job can be inserted into a better position in the SAME nurse!
                if(SwitchNurse(ip, ni, nj, job_from_ni) > -1){ //switch_nurse attempts to remove job_from_ni from ni's route and insert it into nj's route. If successful, =0, else = -1.
                    //printf("BestSwitch propQuality ni: %d, nj: %d, job_from_ni: %d\n", ni, nj, job_from_ni);
                    double propQuality = SolutionQuality(ip, -7);
                    // printf("\tTested switching job %d from nurse %d to nurse %d\n", job_from_i, ni, nj);
                    // printf("\tOriginal position (n%d, p%d) new position (n%d, p%d)\n", ni, solMatrixInitial[ni][job_from_i], nj,  ip->solMatrix[nj][job_from_i]);
                    // printf("\tQuality went from %.2f ", baseQuality); // printf(" UP to %.2f\n", propQuality);
                    if(propQuality > baseQuality){ //If quality of this new solution (with job_from_ni moved from ni to nj) is better than previous solution
                        baseQuality = propQuality;
                        besti = ni;
                        bestj = nj;
                        best_job = job_from_ni;
                        if(firstSwitch > 0){
                            break;
                        }
                    }
                    else{ //If quality of this new solution is the same as or worse than previous solution, then undo the switch, and try next nurse nj (++nj)
                        for(int i = 0; i < ip->nJobs; ++i){
                            ip->solMatrix[ni][i] = solMatrixInitial[ni][i];
                            ip->solMatrix[nj][i] = solMatrixInitial[nj][i];
                        }
                    }
                }
            } //End of for nj loop
            if(firstSwitch > 0 && besti > -1){
                break;
            }
        } //End of for job_from_ni loop
        if(firstSwitch > 0 && besti > -1){
            break;
        }
    } //End of for ni loop


    //FreeMatrixInt(solMatrixInitial, ip->nNurses);
    // printf("Finished best switch, with quality %d\n", baseQuality);

    // Finished, perform the best switch:
    if(besti > -1){
        // printf("Switched job %d from nurse %d to nurse %d.\n-------\n\n", best_job, besti, bestj);
        if(firstSwitch < 1){ //This is never true!
            SwitchNurse(ip, besti, bestj, best_job);
        }
        return 0;
    }

    if(ip->verbose > 5){
        printf("Best switch could NOT find anything good\n");
        printf("besti = %d, bestj = %d,  best_job = %d, baseQuality=%.2f\n", besti, bestj, best_job, baseQuality);
        // exit(-698696969696);
    }

    return -1;

} //END OF BestSwitch function.


int RouteTwoExchange(struct Instance* ip, int firstImprovement){

    // This function finds jobs in nurse ni and nurse nj's routes and swaps the jobs between the nurses in the same position
    // I.e. job is removed from nurse ni's route and replaced with a job from nurse nj's route (so the job that used to be in positon x in ni's route is replaced with another job from nj) and vice versa.
    // Returns 0 if successful (i.e. exchange performed and solution quality improved) and -1 otherwise.
    // Returns improvement exchanging two jobs between nurses. If firstImprovement = 1 returns first found. If firstImprovement < 1 returns best

    int retVal = -1;
    int bestni = -1;
    int bestnj = -1;
    int bestji = -1;
    int bestjj = -1;
    int secondNurseI = -1;
    double TOL = 0.0001;
    //printf("RouteTwoExchange iniQuality\n");
    double iniQuality = SolutionQuality(ip, -12121);
    double currentQuality = iniQuality;

    for(int ni = 0; ni < ip->nNurses - 1; ++ni){ //For each nurse ni 0,...,nNurses-1
        for(int job_from_ni = 0; job_from_ni < ip->nJobs; ++job_from_ni){ //For each job_from_ni 0,...,nJobs
            if(ip->solMatrix[ni][job_from_ni] < 0){ //If job_from_ni is not in ni's route, go to next job_from_ni (++job_from_ni)
                continue;
            }
            secondNurseI = -1;
            if(ip->doubleService[job_from_ni]){ //If job_from_ni is a doubleService, find the other nurse (secondNurseI) that does the job with nurse ni.
                secondNurseI = FindSecondNurseDS(ip, job_from_ni, ni);
            }
            for(int nj = ni + 1; nj < ip->nNurses; ++nj){ //For each nurse nj = ni+1,..., nNurses
                if(nj==secondNurseI){ // nj is the same nurse as secondNurseI, so nurse nj is already doing the job, go to next nurse nj (++nj)
                    continue;
                }

                // Can nj do job_from_ni?
                if(ip->doubleService[job_from_ni]){ //If job_from_ni is a double service
                    if(CheckSkillsDS(ip, job_from_ni, secondNurseI, nj)
                            < 1){ // returns ip->capabilityOfDoubleServices[secondNurseI][nj][jobdsindex], <1 means nj is not skilled to do the job with secondNurseI
                        continue;
                    }
                }
                else if(CheckSkills(ip, job_from_ni, nj) < 1){ // If ip->nurseSkilled[nj][job_from_ni] <1, nurse nj is not skilled to do job_from_ni, go to next nurse nj (++nj)
                    continue;
                }

                // If it can, see who we swap for:
                for(int job_from_nj = 0; job_from_nj < ip->nJobs; ++job_from_nj){
                    if(ip->solMatrix[nj][job_from_nj] < 0){ //If job_from_nj is not in nj's route, go to next job_from_nj
                        continue;
                    }
                    if(job_from_ni==job_from_nj){ //If the jobs are the same, go to next job_from_nj
                        continue;
                    }

                    int secondNurseJ = -1;
                    if(ip->doubleService[job_from_nj]){ //If job_from_nj is a double service, find the other nurse (secondNurseJ) that does the job with nurse nj
                        secondNurseJ = FindSecondNurseDS(ip, job_from_nj, nj);
                    }

                    //NEW: FIXED BUG - need to check that secondNurseJ isn't actually the nurse ni that's already doing the job!
                    if(secondNurseJ==ni){
                        continue;
                    }

                    // Can ni do job_from_nj?
                    if(ip->doubleService[job_from_nj]){ //If job_from_nj is a double service
                        if(CheckSkillsDS(ip, job_from_nj, secondNurseJ, ni) < 1){ //If ni is not skilled to do job_from_nj with secondNurseJ, go to next job_from_nj
                            continue;
                        }
                    }
                    else if(CheckSkills(ip, job_from_nj, ni) < 1){ // If ip->nurseSkilled[ni][job_from_nj] < 1, nurse ni is not skilled to do job_from_nj, go to next job_from_nj
                        continue;
                    }

                    // If we are here, it's ok to swap job_from_i with job_from_j
                    ExchangeJobsInRoute(ip, ni, job_from_ni, nj, job_from_nj); //put job_from_ni in nj's route in job_from_nj's position, and vice versa.
                    //printf("RouteTwoExchange tentQuality\n");
                    double tentQuality = SolutionQuality(ip, -123121); // quality of new solution after exchanging jobs
                    if(tentQuality > currentQuality + TOL){ //If quality of new solution is better than previous solution
                        retVal = 0;
                        if(firstImprovement < 1){ //if < 1 then keep going through loops, if >=1 then return first solution found.
                            currentQuality = tentQuality;
                            bestni = ni;
                            bestnj = nj;
                            bestji = job_from_ni;
                            bestjj = job_from_nj;
                        }
                        else{
                            if(ip->verbose > 5){
                                printf("\tFound a change moves the quality from %.2f to %.2f! ***\n", currentQuality, tentQuality);
                            }
                            return retVal; //return 0
                        }
                    }
                    // Undo the switch!
                    ExchangeJobsInRoute(ip, ni, job_from_nj, nj, job_from_ni); // printf("\n\tExchange reversed. "); printf("Quality back to %.2f\n", SolutionQuality(ip, -123451));
                } //End for job_from_j loop
            } //End for nj loop
        } //End for job_from_i loop
    } //End for ni loop

    if(retVal > -1){
        ExchangeJobsInRoute(ip, bestni, bestji, bestnj, bestjj);
        if(ip->verbose > 5){
            printf("\tThe best two exchange was (n%d, j%d) by (n%d, j%d)\n", bestni, bestji, bestnj, bestjj);
        }
    }
    else if(ip->verbose > 5){
        printf("\tNo exchange was good enough!\n");
    }

    return retVal;

} //END OF RouteTwoExchange function.

int NurseTwoExchange(struct Instance* ip){

    //This function swaps positions of pairs of nurses in nurseOrder array and evaluates solution quality
    //Returns 1 if solution has improved by swapping two nurses, otherwise returns -1.

    //printf("NurseTwoExchange cq\n");
    double cq = SolutionQuality(ip, -66643);

    for(int i = 0; i < ip->nNurses - 1; ++i){ //For all nurses i=0,...,nNurses-1
        for(int j = i + 1; j < ip->nNurses; ++j){ //For all nurses j=i+1,...,nNurses (so never swapping a nurse with itself)
            TwoExchange(ip->nurseOrder, i, j); //Swap the order of nurses in indicies i and j in nurseOrder array: int t = array[j]; array[j]=array[i]; array[i] = t
            //printf("NurseTwoExchange tentQ\n");
            double tentQ = SolutionQuality(ip, -66644);
            if(tentQ > cq){ //If quality of solution with order of nurses i and j swapped is better than previous solution
                return 1; //successful, exit
            }
            else{ //If quality of solution is not better with order of nurses i and j swapped, then swap the nurses back.
                TwoExchange(ip->nurseOrder, j, i);
            }
        }
    }
    return -1;

} // End of NurseTwoExchange function

int BestSyncDoubleSwitch(struct Instance* ip){

    //This function removes a double service job from two nurse's routes and tries to add the job back into two other routes in the best possible positions such that the solution quality improves
    //OR removes two dependent jobs from two nurse's routes and tries to add the two jobs back into two other routes in the best possible positions such that the solution quality improves.
    //Returns 1 if successful and -1 otherwise.
    //Note that if double service, then jobA and jobB are the same job, otherwise if dependent job then jobA and jobB are different jobs.

    int DEBUG_PRINT = -1;
    if(DEBUG_PRINT > 0){
        printf("--- STARTING BestSyncDoubleSwitch ---");
        printf("Initial solmatrix:\n");
        PrintSolMatrix(ip);
        printf("---\n");
    }
    /*
		1 - Remove a double/sync service twice
		2 - Insert the 1st part of the job in every possible position
		3 - Insert 2nd part on the best position
		4 - Evaluate
	*/
    int firstSwitch = 1;
    double TOL = 0.0001;
    int nRows = ip->nNurses;
    int nCols = ip->nJobs;
    //printf("BestSyncDoubleSwitch baseQuality\n");
    double baseQuality = SolutionQuality(ip, DEBUG_PRINT - 30); //original solution quality of ip
    double overAllBestFitness = -1*bigM;
    int overalBestJob = -1;
    int overall_bestA = -1;
    int overall_bestB = -1;
    int overall_bestAPos = -1;
    int overall_bestANurse = -1;
    int overall_bestBPos = -1;
    int overall_bestBNurse = -1;
    int getOut = 0;

    for(int i = 0; i < ip->nJobs; ++i){ //For each job i=0,...,nJobs
        // Do we consider this job?
        int considerJob = -1;
        int nurseA = -1;
        int nurseB = -1;
        int jobA = i;
        int jobB = i;
        int posJobA = -1;
        int posJobB = -1;
        if(ip->doubleService[i] > 0){ //If job i is a doubleService
            considerJob = 1;
            for(int nu = 0; nu < ip->nNurses; ++nu){ //For each nurse nu=0,...,nNurses
                if(ip->solMatrix[nu][i] > -1){ //If job i is in nurse nu's route
                    if(nurseA < 0){ //If nurseA has not yet been set
                        nurseA = nu; //set nurseA
                        posJobA = ip->solMatrix[nu][i]; //posJobA = position of job i in nurseA's route.
                    }
                    else{ //If nurseA has already been set
                        nurseB = nu; //set nurseB
                        posJobB = ip->solMatrix[nu][i]; //posJobB = position of job i in nurseB's route.
                        break; //exit for nu loop.
                    }
                }
            }
        }
        else if(ip->dependsOn[i] >= i){ // If job i depends on another job, consider only one of these
            jobB = ip->dependsOn[i]; //jobB = the job that job i depends on
            considerJob = 1;
            for(int nu = 0; nu < ip->nNurses; ++nu){ //For each nurse nu=0,...,nNurses
                if(ip->solMatrix[nu][jobA] > -1){ //If jobA (job i) is in nurse nu's route
                    nurseA = nu; //set nurseA to nurse nu
                    posJobA = ip->solMatrix[nu][jobA]; //posJobA = position of jobA (job i) in nurseA's route.
                }
                if(ip->solMatrix[nu][jobB] > -1){ //If jobB (the job that jobA (job i) depends on) is in nurse nu's route (so it could be that both jobA and jobB are in the same nurse nu's route)
                    nurseB = nu; //set nurseB to nurse nu
                    posJobB = ip->solMatrix[nu][jobB]; //posJobB = position of jobB (job that jobA (job i) depends on) in nurseB's route.
                }
            }
        }
        else{ //Job i is not a double service nor does it depend on another job
            considerJob = -1;
        }
        if(considerJob < 1){
            continue; //go to next job i (++i)
        }

        if((nurseA < 0) || (nurseB < 0)){ //If one or both nurses have not been set, then there's an error - function should not have reached this line.
            if(DEBUG_PRINT > 0){
                printf("ERROR - CURRENT SOLMATRIX:\n");
                PrintSolMatrix(ip);
            }
            printf("ERROR: Double Service does not have correct allocations.\n");
            printf("\tIn theory: job %d from nurse %d depends on job %d from nurse %d\n", jobA, nurseA, jobB, nurseB);
            exit(-5);
        }

        if(DEBUG_PRINT > 0){
            printf("Chose to test job %d from nurse %d and its dependency, job %d from nurse %d\n", jobA, nurseA, jobB, nurseB);
            printf("About to remove %d from nurse %d\n", jobA, nurseA);
            printf("Currently at position (posJobA) %d\t", posJobA);
            printf("Currently at position %d\n", ip->solMatrix[nurseA][jobA]);
        }

        //Remove jobA from nurseA's route
        RemoveJob(ip, jobA, nurseA);

        if(DEBUG_PRINT > 0){
            printf("About to remove %d from nurse %d\n", jobB, nurseB);
            printf("Currently at position (posJobB) %d\t", posJobB);
            printf("Currently at position %d\n", ip->solMatrix[nurseB][jobB]);
        }

        //Remove jobB from nurseB's route
        RemoveJob(ip, jobB, nurseB);

        int jobABestNurse = -1;
        int jobBBestNurse = -1;
        int jobABestPos = -1;
        int jobBBestPos = -1;
        int allPositionsCovered = 0;
        double bestJobPosQuality = baseQuality; //original solution quality of ip

        if(allPositionsCovered > 0){ //go to next job (++i) in for loop
            continue;
        }

        for(int tn = 0; tn < ip->nNurses; ++tn){ //For each nurse tn=0,...,nNurses
            if(DEBUG_PRINT > 0){
                printf("Testing job A (%d) in nurse %d\n", jobA, tn);
            }
            if(ip->doubleService[jobA]){ //If jobA is a double service
                if(CheckSkillsDSFirst(ip, jobA, tn)
                        < 1){ //If no other nurse can do jobA with nurse tn. (capabilityOfDoubleService, if there's another nurse that can do the job with tn then =1, else = 0)
                    if(DEBUG_PRINT > 0){
                        printf("\t(!) Not possible, does not cover first part of DS.\n");
                    }
                    continue; //Go to next nurse tn (++tn)
                }
            }
            else{ //If jobA is not a double service
                if(CheckSkills(ip, jobA, tn) < 1){ //If nurse tn is not skilled to do jobA (nurseSkilled[tn][jobA] = 0)
                    if(DEBUG_PRINT > 0){
                        printf("\t(!) Not possible, unskilled.\n");
                    }
                    continue; //Go to next nurse tn (++tn)
                }
            }
            for(int tpos = 0; tpos < ip->nJobs; ++tpos){ //For each position tpos = 0,...,nJobs
                // printf("\tSolmatrix value A %d\n", ip->solMatrix[nurseA][jobA]);
                // printf("\tSolmatrix value B %d\n", ip->solMatrix[nurseB][jobB]);
                if(DEBUG_PRINT > 0){
                    printf("\t(-) Testing position %d ", tpos);
                }
                if((tn==nurseA) && (tpos==posJobA)){ //If tn is nurseA and tpos is position of jobA in nurseA (tn) route
                    if(DEBUG_PRINT > 0){
                        printf("- skipping, it was already here before.\n");
                    }
                    continue; //Go to next position tpos (++tpos)
                }

                // Insert jobA:
                int insertAValue = InsertJobAtPosition(ip, jobA, tn, tpos); //Insert jobA into nurse tn's route in position tpos, =0 if successful, =-1 otherwise.
                if(insertAValue < 0){ //If jobA could not be inserted into nurse tn's route in position tpos
                    if(DEBUG_PRINT > 0){
                        printf("- insertion failed (index too high?)\n");
                    }
                    allPositionsCovered = 1;
                    break; //Exit for loop tpos, try next nurse tn (++tn)
                }
                else if(DEBUG_PRINT > 0){ //jobA has been inserted into nurse tn's route in position tpos successfully
                    printf("- part A inserted in Nurse %d at position %d (Ret %d)\n", tn, tpos, insertAValue);
                }

                // Deal with jobB
                // printf("\tSolmatrix value A %d\n", ip->solMatrix[tn][jobA]);
                // printf("\tSolmatrix value B %d\n", ip->solMatrix[nurseB][jobB]);
                int finalTNB = -1; //nurse
                int finalPosB = -1; //position of jobB in nurse tnb's route
                int bPossible = 0;
                double bestTargetQuality = -1*bigM;
                double tQuality = bestTargetQuality;
                if(DEBUG_PRINT > 0){
                    printf("\tStart checks for Position B\n");
                }
                for(int tnb = 0; tnb < ip->nNurses; ++tnb){ //For each nurse tnb = 0,...,nNurses
                    if(DEBUG_PRINT > 0){
                        printf("\tTesting job B (%d) in nurse %d\n", jobB, tnb);
                    }
                    if(ip->doubleService[jobA] && CheckSkillsDS(ip, jobB, tn, tnb) < 1){ //If jobA is a double service AND nurse tn and nursetnb are NOT skilled to do jobB together
                        if(DEBUG_PRINT > 0){
                            printf("\t\t(!) Not possible, unskilled to do second part of DS.\n");
                        }
                        continue; //Go to next nurse tnb (++tnb)
                    }
                    else if((!ip->doubleService[jobA]) && CheckSkills(ip, jobB, tnb) < 1){ //If jobA is NOT a double service AND nurse tnb is NOT skilled to do jobB
                        if(DEBUG_PRINT > 0){
                            printf("\t\t(!) Not possible, unskilled.\n");
                        }
                        continue; //Go to next nurse tnb (++tnb)
                    }
                    if(DEBUG_PRINT > 0){
                        printf("\t\t(-) Possible, calling BestJobInsertion\n");
                    }

                    int insertValue = BestJobInsertion(ip, jobB, tnb); //Try to insert jobB into nurse tnb's route, =0 if successful, = -1 otherwise.
                    if(insertValue > -1){ //If jobB inserted into nurse tnb's route successfully
                        bPossible = 1;
                        //printf("BestSyncDoubleSwitch tQuality\n");
                        tQuality = SolutionQuality(ip, -5);
                        if(tQuality > bestTargetQuality){ //If solution quality has improved
                            bestTargetQuality = tQuality;
                            finalTNB = tnb; //nurse tnb
                            finalPosB = ip->solMatrix[tnb][jobB]; //position of jobB in nurse tnb's route
                        }
                        if(DEBUG_PRINT > 0){
                            printf("\t>>> Inserted job B in nurse %d (Ins val %d, quality %.2f) <<<\n", tnb, insertValue, tQuality);
                        }
                        RemoveJob(ip, jobB, tnb); //Return solution back to before
                    }
                    else if(DEBUG_PRINT > 0){
                        printf("\tInsert not possible (%d)\n", insertValue);
                        printf("\tjobB = %d\n", jobB);
                        printf("\ttnb = %d\n", tnb);
                        printf("\tip->solMatrix[tnb][jobB] = %d\n", ip->solMatrix[tnb][jobB]);
                    }
                } //End for (int tnb = 0; tnb < ip->nNurses; ++tnb)

                if(bPossible < 1){
                    printf("WARNING: It was not possible to insert the second part of the job anywhere. Strange...\n");
                    printf("Solmatrix value A %d\n", ip->solMatrix[nurseA][jobA]);
                    printf("Solmatrix value B %d\n", ip->solMatrix[nurseB][jobB]);
                    printf("\tconsiderJob = %d\n", considerJob);
                    printf("\tnurseA = %d\n", nurseA);
                    printf("\tnurseB = %d\n", nurseB);
                    printf("\tjobA = %d\n", jobA);
                    printf("\tposJobA = %d\n", posJobA);
                    printf("\tposJobB = %d\n", posJobB);
                    printf("\ttn = %d\n", tn);
                    printf("\ttpos = %d\n", tpos);
                    printf("\ttQuality = %.2f\n", tQuality);
                    printf("\tbestTargetQuality = %.2f\n", bestTargetQuality);
                    exit(-21423553);
                    continue;
                }

                // Check the quality of this job:
                if(bestTargetQuality > bestJobPosQuality){ //If solution has improved by removing and reinserting jobs
                    bestJobPosQuality = bestTargetQuality;
                    jobABestNurse = tn;
                    jobBBestNurse = finalTNB;
                    jobABestPos = tpos;
                    jobBBestPos = finalPosB;
                    if(DEBUG_PRINT > 0){
                        printf("\t\t * best target quality %.2f*\n", bestJobPosQuality);
                    }
                    if(firstSwitch){ //firstSwitch = 1
                        getOut = 100;
                    }
                }
                // Undo:
                if(DEBUG_PRINT > 0){
                    printf("\t >> Finished testing job A, nurse %d, position %d <<\n", tn, tpos);
                }
                RemoveJob(ip, jobA, tn); //Remove jobA from nurse tn's route
                if(getOut > 0){ //If solution found with better quality
                    break;
                }
            } //End for (int tpos = 0; tpos < ip->nJobs; ++tpos)
            if(getOut > 0){ //If solution found with better quality
                break;
            }
        } //End for (int tn = 0; tn < ip->nNurses; ++tn)

        if(bestJobPosQuality > overAllBestFitness + TOL){
            overAllBestFitness = bestJobPosQuality;
            overalBestJob = i;
            overall_bestA = jobA;
            overall_bestB = jobB;
            overall_bestAPos = jobABestPos;
            overall_bestANurse = jobABestNurse;
            overall_bestBPos = jobBBestPos;
            overall_bestBNurse = jobBBestNurse;
            if(DEBUG_PRINT > 0){
                printf("\n\n\t *** best overall quality %.2f ***\n\n", overAllBestFitness);
            }
        }
        // Insert these jobs back before anything else:
        if(posJobA < posJobB){
            if(InsertJobAtPosition(ip, jobA, nurseA, posJobA) < 0){
                printf("ERROR! Solmatrix:\n");
                PrintSolMatrix(ip);
                printf("ERROR: Cannot insert %d where it was!\n", jobA);
                printf("jobA = %d, nurseA = %d, posJobA = %d\n", jobA, nurseA, posJobA);
                printf("jobB = %d, nurseB = %d, posJobB = %d\n", jobB, nurseB, posJobB);
                exit(-34234523);
            }
        }

        if(InsertJobAtPosition(ip, jobB, nurseB, posJobB) < 0){
            printf("ERROR! Solmatrix:\n");
            PrintSolMatrix(ip);
            printf("ERROR: Cannot insert the %d where it was!\n", jobB);
            printf("jobA = %d, nurseA = %d, posJobA = %d\n", jobA, nurseA, posJobA);
            printf("jobB = %d, nurseB = %d, posJobB = %d\n", jobB, nurseB, posJobB);
            exit(-34234523);
        }

        // Repeat the previous code. This is to avoid the case where both are inserted in the same nurse,
        // and A is the last job, which would mess up the indices.
        if(!(posJobA < posJobB)){
            if(InsertJobAtPosition(ip, jobA, nurseA, posJobA) < 0){
                printf("ERROR! Solmatrix:\n");
                PrintSolMatrix(ip);
                printf("ERROR: Cannot insert %d where it was!\n", jobA);
                printf("!(posJobA < posJobB)\n");
                printf("jobA = %d, nurseA = %d, posJobA = %d\n", jobA, nurseA, posJobA);
                printf("jobB = %d, nurseB = %d, posJobB = %d\n", jobB, nurseB, posJobB);
                exit(-34234523);
            }
        }
        if(getOut > 0){
            break;
        }
    } // End for (int i = 0; i < ip->nJobs; ++i), end of scanning valid jobs

    // Do the move:


    // double fQual = SolutionQuality(ip, 0);
    if(overAllBestFitness > baseQuality + TOL){
        if(DEBUG_PRINT > 0){
            printf(">>>> Movement best_double_switch, went from %.2f to %.2f <<<<\n", baseQuality, overAllBestFitness);
            printf("overall_bestA = %d\n", overall_bestA);
            printf("overall_bestANurse = %d\n", overall_bestANurse);
            printf("overall_bestAPos = %d\n", overall_bestAPos);
            printf("overall_bestB = %d\n", overall_bestB);
            printf("overall_bestBNurse = %d\n", overall_bestBNurse);
            printf("overall_bestBPos = %d\n", overall_bestBPos);
        }

        if((overall_bestA < 0) || (overall_bestANurse < 0) || (overall_bestAPos < 0) || (overall_bestB < 0) || (overall_bestBNurse < 0) || (overall_bestBPos < 0)){
            if(DEBUG_PRINT > 0){
                printf("best_double_switch did not find anything but -1s\n");
                printf("--- -1 early FINISHED BestSyncDoubleSwitch ---");
            }
            return -1;
        }

        // First find them and remove them:
        for(int nn = 0; nn < ip->nNurses; ++nn){
            if(ip->solMatrix[nn][overall_bestA] > -1){
                RemoveJob(ip, overall_bestA, nn);
            }
            if(ip->solMatrix[nn][overall_bestB] > -1){
                RemoveJob(ip, overall_bestB, nn);
            }
        }
        // Insert them back in the new positions:
        if(InsertJobAtPosition(ip, overall_bestA, overall_bestANurse, overall_bestAPos) < 0){
            printf("ERROR: Cannot insert the move found on local search!!!\n");
            exit(-34234523);
        }
        if(InsertJobAtPosition(ip, overall_bestB, overall_bestBNurse, overall_bestBPos) < 0){
            printf("ERROR: Cannot insert the move found on local search!!!\n");
            exit(-34234523);
        }
        if(DEBUG_PRINT > 0){
            //printf("BestSyncDoubleSwitch dcheckQual1\n");
            double dcheckQual = SolutionQuality(ip, DEBUG_PRINT - 30);
            printf("DOUBLE CHECK: Real sol quality: %.2f, expected sol quality: %.2f\n", dcheckQual, overAllBestFitness);
        }
        if(DEBUG_PRINT > 0){
            printf("--- 1 FINISHED BestSyncDoubleSwitch ---");
        }
        return 1;
    }
    else{
        if(DEBUG_PRINT > 0){
            //printf("BestSyncDoubleSwitch dcheckQual2\n");
            double dcheckQual = SolutionQuality(ip, DEBUG_PRINT - 30);
            printf(">    Not doing best_double_switch, started with %.2f and best was only %.2f    <\n", baseQuality, overAllBestFitness);
        }
        if(DEBUG_PRINT > 0){
            printf("--- -1 FINISHED BestSyncDoubleSwitch ---");
        }
        return -1;
    }

} // END OF BestSyncDoubleSwitch function.

int TwoOptMove(struct Instance* ip, int ni, int pos1, int pos2){

    //This function takes two positions pos1 and pos2 in a nurse ni's route and reverses the order of the jobs between and including those positions.
    //Returns 0 if successful (i.e. managed to reverse order of jobs) and -1 otherwise.

    // Error checks
    if(pos1==pos2){
        printf("\nERROR: 2-opt got equal positions %d and %d\n", pos1, pos2);
        return -1;
    }
    else if(pos1 > pos2){
        int aux = pos1;
        pos1 = pos2;
        pos2 = aux;
    }
    if(pos2 >= ip->nJobs){
        printf("\nERROR: 2-opt got job pos2 = %d, but there are only %d (pos1 = %d)\n", pos2, ip->nJobs, pos1);
        return -1;
    }

    //int* nurseRoute = malloc(ip->nJobs*sizeof(int)); //size = 1 x nJobs (positions)
    std::vector<int> nurseRoute(ip->nJobs);
    GetNurseRoute(ip, ni, nurseRoute); //Fills array nurseRoute[position] = job for nurse ni's route

    if(nurseRoute[pos2] < 0){ // There's no job in position 2, so clearly not enough jobs in the route for this move
        //free(nurseRoute);
        return -1;
    }

    // Start move:
    std::vector<int> nurseRouteAux(ip->nJobs);
    //int* nurseRouteAux = malloc(ip->nJobs*sizeof(int));

    for(int i = 0; i < ip->nJobs; i++){ //nurseRouteAux = nurseRoute
        nurseRouteAux[i] = nurseRoute[i];
    }

    nurseRouteAux[pos2] = nurseRoute[pos1]; //Job that was in pos1 in nurseRoute is now in pos2 in nurseRouteAux
    nurseRouteAux[pos1] = nurseRoute[pos2]; //Job that was in pos2 in nurseRoute is now in pos1 in nurseRouteAux

    int revCounter = 0;
    for(int i = pos1; i < pos2; i++){ //For all positions between pos1 and pos2 (this loop also includes pos1 again), reverse the order of the jobs in the route (reverse the positions of the jobs)
        nurseRouteAux[pos1 + revCounter] = nurseRoute[pos2 - revCounter];
        revCounter++;
    }

    for(int j = 0; j < ip->nJobs; j++){ //For all positions j = 0,...,nJobs
        // printf("%d\t", nurseRouteAux[j]);
        if(nurseRouteAux[j] < 0){ //If no job in position j of route, exit, no more jobs in route (end of positions)
            break;
        }
        ip->solMatrix[ni][nurseRouteAux[j]] = j; //solMatrix[ni][job in position j of nurseRouteAux] = position j.

    }
    //free(nurseRoute);
    //free(nurseRouteAux);

    return 0;

} //END OF TwoOptMove function.

int SwitchNurse(struct Instance* ip, int ni, int nj, int pi){

    //This function takes a job 'pi' out of nurse ni's route and attempts to put the job 'pi' into nurse nj's route.
    //If it's not possible to put 'pi' into nurse nj's route, then 'pi' is just put back into nurse ni's route in its original position.
    //Function returns -1 if no switch has occurred, and returns 0 if job 'pi' has been taken out of ni's route and put into nj's route successfully.

    /// Surely need to check that nurse ni and nurse nj are not the same nurse? Perhaps do this in BestSwitch function before this switch_nurse function is called?

    if(ip->solMatrix[ni][pi] < 0){ //Job pi is not assigned to nurse ni
        return -1; // Nothing to switch
    }

    if(ip->nurseSkilled[nj][pi] < 1 && ip->doubleService[pi] < 1){ //Nurse ni is not skilled to do job pi, and job pi is not a double service
        return -1; // Nothing to switch
    }

    if(ip->doubleService[pi] > 0){ // If job pi is a double service
        int secondNurse = FindSecondNurseDS(ip, pi, ni); //secondNurse = the other nurse that is assigned to job 'pi' along with nurse 'ni'.

        if(secondNurse==-1){ //If there is no other nurse that has been assigned to do job 'pi' with nurse 'ni'
            printf("ERROR: When attempting to switch job %d from nurse %d to nurse %d\n", pi, ni, nj);
            printf("-> Double service was not found in any other nurse.\n");
            exit(-1);
        }
        // If secondNurse and nurse 'nj' are not capable of doing the double service 'pi' together
        if(CheckSkillsDS(ip, pi, secondNurse, nj) < 1){ //returns ip->capabilityOfDoubleServices[secondNurse][nj][jobdsindex];
            return -1;
        }

    }
    int oldPos = ip->solMatrix[ni][pi]; //oldPos = position of job pi in nurse ni's route (in current solution before it gets modified below)

    RemoveJob(ip, pi, ni); //Remove job pi from nurse ni's route.

    if(BestJobInsertion(ip, pi, nj) < 0){ //If job pi has NOT been inserted in nurse nj's route.
        InsertJobAtPosition(ip, pi, ni, oldPos); //Then insert job back into nurse ni's route in it's previous position.
        return -1;
    }

    //Otherwise, BestJobInsertion(ip,pi,nj) >= 0 which means that job pi has been inserted into nurse nj's route successfully.

    return 0;

}// End of SwitchNurse function

int ExchangeJobsInRoute(struct Instance* ip, int ni, int jobFromNi, int nj, int jobFromNj){

    //This function removes job_from_nj from nj's route and inserts job_from_ni into nj's route in the same position that job_from_nj was in, and
    //removes job_from_ni from ni's route and inserts job_from_nj into ni's route in the same position that job_from_ni was in.
    /// Maybe we should check ip->solMatrix[nj][job_from_ni] and ip->solMatrix[ni][job_from_nj] at the same time at the beginning of the function instead?

    //1. Put job_from_ni in nurse nj's route in the same position that job_from_nj was in, and remove job_from_nj from nj's route.
    int aux = ip->solMatrix[nj][jobFromNi]; //Position of job_from_ni in nj's route, should be =-1 as job_from_ni shouldn't be in nj's route!

    if(aux > -1){ // If job_from_ni is already in nj's route, then there's an error - we shouldn't do the swap.
        printf("\n---\n---\nWARNING: Exchanging a job maybe we should not????\n");
        printf("ip->solMatrix[%d][%d] = %d\n", nj, jobFromNi, aux);
        printf("\tTrying to exchange (n%d, j%d) with (n%d, j%d).\nSolmatrix:\n", ni, jobFromNi, nj, jobFromNj);
        PrintSolMatrix(ip);
        exit(-123424);
    }

    //Set position of job_from_ni in nj's route to be the same position as job_from_nj in nurse nj's route
    ip->solMatrix[nj][jobFromNi] = ip->solMatrix[nj][jobFromNj]; //e.g if job_from_nj is in position 3 in nj's route, now job_from_ni is set as position 3 in nurse nj's route.
    ip->solMatrix[nj][jobFromNj] = aux; //Then, set position of job_from_nj in nurse nj's route to be aux (which should be -1), so essentially removing job_from_nj from nj's route.

    //2. Put job_from_ni in nurse nj's route in the same position that job_from_nj was in, and remove job_from_nj from nj's route.
    aux = ip->solMatrix[ni][jobFromNj]; //Position of job_from_nj in nurse ni's route, should be =-1 as job_from_nj shouldn't be in ni's route!

    if(aux > -1){ //If job_from_nj is already in ni's route, then there's an error - we shouldn't do the swap.
        printf("\n---\n---\nWARNING: Exchanging a job maybe we should not????\n");
        printf("ip->solMatrix[%d][%d] = %d\n", ni, jobFromNj, aux);
        printf("\tTrying to exchange (n%d, j%d) with (n%d, j%d).\nSolmatrix:\n", ni, jobFromNi, nj, jobFromNj);
        PrintSolMatrix(ip);
        exit(-123424);
    }

    //Set position of job_from_nj in ni's route to be the same position as job_from_ni in nurse ni's route.
    ip->solMatrix[ni][jobFromNj] = ip->solMatrix[ni][jobFromNi]; //e.g. if job_from_ni is in position 5 in nurse ni's route, now job_from_nj is set as position 5 in nurse ni's route.
    ip->solMatrix[ni][jobFromNi] = aux; //Then, set position of job_from_ni in nurse ni's route to be aux (which should be -1), removing job_from_ni from ni's route.

    return 0;

} //END OF ExchangeJobsInRoute function.