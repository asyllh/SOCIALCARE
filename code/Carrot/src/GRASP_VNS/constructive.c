/*--------------/
GRASP_VNS
constructive.c
UoS
10/07/2021
/--------------*/

/** NEED TO CHECK THAT VIOLATEDTW AND VIOLATEDTWMK ARE SET CORRECTLY IN OBJECTIVE FUNCTION 17/02/2022**/

/** MAKE SURE THAT BREAK TIMES ARE REMOVED FROM WAITING TIMES! **/


#include <stdlib.h> // To have min in linux
#include "Python.h"
#include "constructive.h"
#include "grasp.h"

// #ifdef _WIN32
// #  ifdef MODULE_API_EXPORTS
// #    define MODULE_API __declspec(dllexport)
// #  else
// #    define MODULE_API __declspec(dllimport)
// #  endif
// #else
// #  define MODULE_API
// #endif

int main(int argc, char const* argv[]){
    struct INSTANCE inst = GenerateInstance();
    struct INSTANCE* ip = &inst;
    ip->verbose = 4;
    return MainWithOutput(ip, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

MODULE_API int python_entry(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int twInterval_data, bool excludeNurseTravel_data,
                            double* od_data, double* nurseTravelFromDepot_data, double* nurseTravelToDepot_data, int* unavailMatrix_data, int* nurseUnavail_data,
                            int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* solMatrixPointer,
                            int* doubleService_data, int* dependsOn_data, int* mkMinD_data, int* mkMaxD_data, int* capabilityOfDoubleServices_data, double* prefScore_data,
                            double* algorithmOptions_data, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                            double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer, int randomSeed){

    // printf("Inside C function. Random seed = %d.\n", randomSeed);

    double eps = 1e-6;
    int printInputData = (int)algorithmOptions_data[99] + eps;

    int qualityMeasureModAPI = (int)(algorithmOptions_data[0] + eps); // printf("Quality measure	= %d\n\n", qualityMeasureModAPI);
    int doGapNotPrecedence = (int)(algorithmOptions_data[12] + eps); // printf("Use gaps for dependent jobs (rather than precedence)	= %d\n", doGapNotPrecedence);

    if(qualityMeasureModAPI == 0 && doGapNotPrecedence == 0){
        printf("WARNING: Solving with AIT H quality measure but with PRECEDENCE rather than GAP.\n");
    }

    // printf("Quality measure	= %d\n\n", (int) (algorithmOptions_data[0] + eps));
    // printf("do_twopt 		= useTwoOpt = (int) ip->algorithmOptions[1] + eps = %d\n", (int) (algorithmOptions_data[1] + eps));
    // printf("no_change_ls	=useNurseOrderChange = (int) ip->algorithmOptions[3] + eps = %d\n\n", (int) (algorithmOptions_data[3] + eps));
    // printf("no_change_grasp = int changeNurseOrder = (int) ip->algorithmOptions[6] + eps; = %d\n", (int) (algorithmOptions_data[6] + eps));
    // printf("pr_strategy = int PR_STRATEGY = (int) ip->algorithmOptions[9] + eps; = %d\n", (int) (algorithmOptions_data[9] + eps));
    // printf("pr_direction = int PR_DIRECTION_PARAMETER = (int) ip->algorithmOptions[11] + eps; = %d\n", (int) (algorithmOptions_data[11] + eps));
    // printf("sols_in_pool = int solutions_in_pool = (int) ip->algorithmOptions[8] + eps; = %d\n", (int) (algorithmOptions_data[8] + eps));
    // printf("grasp_dl = double delta_low = ip->algorithmOptions[4]  + eps;// 0.05; = %.2f\n", algorithmOptions_data[4]  + eps);
    // printf("grasp_dr = double delta_range = ip->algorithmOptions[5]  + eps; // 0.25; = %.2f\n", algorithmOptions_data[5]  + eps);
    // printf("rcl_strategy = int rcl_strategy = (int) ip->algorithmOptions[10] + eps; = %d\n", (int) (algorithmOptions_data[10] + eps));
    // printf("\n\nuseTwoExchange = (int) ip->algorithmOptions[2] + eps = %d\n", (int) (algorithmOptions_data[2] + eps));

    // do_twopt		"-do_twopt="		o 	(0,1)
    // no_change_ls	"-no_change_ls="	o	(0,1)
    // no_change_grasp	"-no_change_grasp="	o	(0,1)
    // pr_strategy		"-pr_strategy="		o	(1,2,3)
    // pr_direction	"-pr_direction="	o	(1,2,3,4)
    // sols_in_pool	"-sols_in_pool="	i	(1,100)
    // grasp_dl		"-grasp_dl="		r	(0,0.5)
    // grasp_dr		"-grasp_dr="		r	(0,1)
    // rcl_strategy	"-rcl_strategy="	o	(1,2)

    if(printInputData > 0){
        printf("nJobs_data = %d\n", nJobs_data);
        printf("nNurses_data = %d\n", nNurses_data);
        printf("nSkills_data = %d\n", nSkills_data);
        printf("verbose_data = %d\n", verbose_data);
        printf("MAX_TIME_SECONDS = %.2f\n", MAX_TIME_SECONDS);
        printf("randomSeed = %d\n", randomSeed);;
        printf("od_data = \n");
        PrintDoubleMatrixOne(od_data, nJobs_data + 1, nJobs_data + 1);
        printf("nurseTravelFromDepot = \n");
        PrintDoubleMatrixOne(nurseTravelFromDepot_data, nNurses_data, nJobs_data);
        printf("nurseTravelToDepot = \n");
        PrintDoubleMatrixOne(nurseTravelToDepot_data, nNurses_data, nJobs_data);
        printf("nurseWorkingTimes_data = \n");
        PrintIntMatrixOne(nurseWorkingTimes_data, nNurses_data, 3);
        printf("jobTimeInfo_data = \n");
        PrintIntMatrixOne(jobTimeInfo_data, nJobs_data, 3);
        printf("jobRequirements_data = \n");
        PrintIntMatrixOne(jobRequirements_data, nJobs_data, nSkills_data);
        printf("nurseSkills_data = \n");
        PrintIntMatrixOne(nurseSkills_data, nNurses_data, nSkills_data);
        printf("solMatrixPointer = \n");
        PrintIntMatrixOne(solMatrixPointer, nNurses_data, nJobs_data);
        printf("mkMinD_data = \n");
        PrintIntMatrixOne(mkMinD_data, 1, nJobs_data);
        printf("mkMaxD_data = \n");
        PrintIntMatrixOne(mkMaxD_data, 1, nJobs_data);
        printf("doubleService_data = \n");
        PrintIntMatrixOne(doubleService_data, 1, nJobs_data);
        printf("dependsOn_data = \n");
        PrintIntMatrixOne(dependsOn_data, 1, nJobs_data);
        int nDS = 0;
        for(int i = 0; i < nJobs_data; ++i){
            nDS += doubleService_data[i];
        }
        printf("capabilityOfDoubleServices_data (only first nurses*nurses, jobs)= \n");
        PrintIntMatrixOne(capabilityOfDoubleServices_data, nDS, nNurses_data*nNurses_data);
        printf("prefScore_data = \n");
        PrintDoubleMatrixOne(prefScore_data, nJobs_data, nNurses_data);
        printf("algorithmOptions_data = \n");
        PrintDoubleMatrixOne(algorithmOptions_data, 1, 100);
    }

    // printf("In C.\nReading input...\n");
    struct INSTANCE inst = InstanceFromPython(nJobs_data, nNurses_data, nSkills_data, verbose_data, MAX_TIME_SECONDS, twInterval_data, excludeNurseTravel_data, od_data, nurseTravelFromDepot_data,
                                              nurseTravelToDepot_data, unavailMatrix_data, nurseUnavail_data, nurseWorkingTimes_data, jobTimeInfo_data, jobRequirements_data, nurseSkills_data,
                                              doubleService_data, dependsOn_data, mkMinD_data, mkMaxD_data, capabilityOfDoubleServices_data, prefScore_data, algorithmOptions_data);

    // printf("Done.\nBack in python_entry()\n");
    if(printInputData > 0){
        printf("nurseSkilled = \n");
        PrintIntMatrix(inst.nurseSkilled, nNurses_data, nJobs_data);
        printf("\n---------------  end of input data in C  ---------------\n\n");
    }

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
    printf("THE MIN PREF OF THIS INSTANCE IS:\n");
    printf("<-< minPref = %.2f >->\n", minPref);

    struct INSTANCE* ip = &inst;
    if(randomSeed >= 0){
        // printf("Setting a fixed random seed: %d.\n", randomSeed);
        srand(randomSeed);
    }
    else{
        // printf("Setting a true random seed.\n");
        srand(time(NULL));
    }

    int thenum = rand();
    // printf("Sample random number: %d\n", thenum);
    // return(-32435);
    // printf("Random seed set, main_with_output()\n");
    // ip->verbose = 10000;

    // Select the type of solver used:
    int retvalue = 0;
    double int_tol = 0.0001;
    if(ip->algorithmOptions[98] >= -int_tol && ip->algorithmOptions[98] <= int_tol){
        // Solve with GRASP
        retvalue = MainWithOutput(ip, od_data, solMatrixPointer, timeMatrixPointer, nurseWaitingTimePointer, nurseTravelTimePointer, violatedTWPointer, nurseWaitingMatrixPointer, nurseTravelMatrixPointer, totalsArrayPointer);
    }
    else if(ip->algorithmOptions[98] >= 1 - int_tol && ip->algorithmOptions[98] <= 1 + int_tol){
        // Evaluate solMatrix only
        evaluate_given_solution(ip, solMatrixPointer, od_data);
    }
    else if(ip->algorithmOptions[98] >= 2 - int_tol && ip->algorithmOptions[98] <= 2 + int_tol){
        // Use Branch and Cut
        // branch_and_cut_solver(ip, solMatrixPointer, od_data);
        printf("Solving with option '%d' is not implemented.", (int) ip->algorithmOptions[98]);
        retvalue = -1;
    }
    else{
        // Not implemented
        printf("Solving with option '%d' is not implemented.", (int) ip->algorithmOptions[98]);
        retvalue = -1;
    }


    // printf("Finished in C.\n");

    if(qualityMeasureModAPI==0 && doGapNotPrecedence==0){
        printf("[WARNING]: Solving with AIT H quality measure but with PRECEDENCE rather than GAP.\n");
    }

    return retvalue;

} // END OF MODULE_API int python_entry function

int evaluate_given_solution(struct INSTANCE* ip, int* solMatrixPointer, double* odmat_pointer){
    // There is no solving here, only evaluating a given solMatrix
    // The order is 0, 1, 2, ...
    // A different ordering cannot be specified here, but might be tested
    // by rearranging solMatrix in the caller function
    // It might be useful, as it can return the quality of a given matrix

    //start populating
    int ct = 0;
    for(int i = 0; i < ip->nNurses; ++i){
        ip->nurseOrder[i] = i;
        for(int j = 0; j < ip->nJobs; ++j){
            ip->solMatrix[i][j] = solMatrixPointer[ct];
            ct++;
        }
    }

    printf("\n-- Testing this matrix ---\n");
    PrintSolMatrix(ip);
    printf("-\n");

    printf("SolMatrix Quality\t%.4f\n", SolutionQuality(ip, 11));

    printf("-- TESTING A SOLUTION, NOT SOLVING -- \n");
    FreeInstanceMemory(ip);

    return 0;
}

int MainWithOutput(struct INSTANCE* ip, double* odmat_pointer, int* solMatrixPointer, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                   double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer){

    // Call the GRASP algorithm
    GRASP(ip);


    // printf("Swiches %d\n", count);
    double finalQuality = SolutionQuality(ip, -1);
    if(ip->verbose > 0){
        // ReportSolution(ip);
        printf("Final solution quality is: %.2f\n", finalQuality);

        // printf("After %d swaps and %d switches, solution quality is: %.2f\n", performedSwaps, performedSwitches, finalQuality);
    }

    // printf("Saving final quality in OD...\n");
    if(odmat_pointer!=NULL){
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

    // if (finalQuality > 31)
    // 	print_solmatrix(ip);
    if(ip->verbose > 10)
        printf("Finishing stuff...\n");

    //SolMatrixToPythonFormat(ip, solMatrixPointer);
    SolnToPythomFormat(ip, solMatrixPointer, timeMatrixPointer, nurseWaitingTimePointer, nurseTravelTimePointer, violatedTWPointer, nurseWaitingMatrixPointer, nurseTravelMatrixPointer,
                       totalsArrayPointer);

    //exclude_nurse_travel_data = false;

    // solMatrixPointer = ip->solMatrix;
    if(ip->verbose > 5)
        printf("End of program.\nFreeing memory...\n");
    FreeInstanceMemory(ip);
    if(ip->verbose > 5)
        printf("Done.");

    return 0;
}

void SolMatrixToPythonFormat(struct INSTANCE* ip, int* solMatrixPointer){
    int ct = 0;
    if(solMatrixPointer!=NULL){
        if(ip->verbose > 10){
            printf("Allocating stuff back to python\n");
        }

        for(int i = 0; i < ip->nNurses; ++i){
            for(int j = 0; j < ip->nJobs; ++j){
                solMatrixPointer[ct] = ip->solMatrix[i][j];
                ct++;
            }
            if(ip->verbose > 10){
                printf("\n");
            }
        }
    }
}

void SolnToPythomFormat(struct INSTANCE* ip, int* solMatrixPointer, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                        double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer){

    //solMatrix (nNursesxnJobs)
    int ct = 0;
    if(solMatrixPointer != NULL){
        if(ip->verbose > 10){
            printf("Allocating solMatrix back to python\n");
        }
        for(int i = 0; i < ip->nNurses; ++i){
            for(int j = 0; j < ip->nJobs; ++j){
                solMatrixPointer[ct] = ip->solMatrix[i][j];
                ct++;
            }
            if(ip->verbose > 10){
                printf("\n");
            }
        }
    }

    //timeMatrix (nNursesxnJobs)
    ct = 0;
    if(timeMatrixPointer != NULL){
        if(ip->verbose > 10){
            printf("Allocating timeMatrix back to python\n");
        }
        for(int i = 0; i < ip->nNurses; ++i){
            for(int j = 0; j < ip->nJobs; ++j){
                timeMatrixPointer[ct] = ip->timeMatrix[i][j];
                ct++;
            }
            if(ip->verbose > 10){
                printf("\n");
            }
        }
    }

    //nurseWaitingTime (1xnNurses)
    ct = 0;
    if(nurseWaitingTimePointer != NULL){
        if(ip->verbose > 10){
            printf("Allocating nurseWaitingTime back to python");
        }
        for(int i = 0; i < ip->nNurses; ++i){
            nurseWaitingTimePointer[ct] = ip->nurseWaitingTime[i];
            ct++;
        }
        if(ip->verbose > 10){
            printf("\n");
        }
    }

    //nurseTravelTime (1xnNurses)
    ct = 0;
    if(nurseTravelTimePointer != NULL){
        if(ip->verbose > 10){
            printf("Allocating nurseTravelTime back to python");
        }
        for(int i = 0; i < ip->nNurses; ++i){
            nurseTravelTimePointer[ct] = ip->nurseTravelTime[i];
            ct++;
        }
        if(ip->verbose > 10){
            printf("\n");
        }
    }

    //violatedTW (1xnJobs)
    ct = 0;
    if(violatedTWPointer != NULL){
        if(ip->verbose > 10){
            printf("Allocating violatedTW back to python");
        }
        for(int i = 0; i < ip->nJobs; ++i){
            violatedTWPointer[ct] = ip->violatedTW[i];
            ct++;
        }
        if(ip->verbose > 10){
            printf("\n");
        }
    }

    //nurseWaitingMatrix (nNursesxnJobs)
    ct = 0;
    if(nurseWaitingMatrixPointer != NULL){
        if(ip->verbose > 10){
            printf("Allocating nurseWaitingMatrix back to python\n");
        }
        for(int i = 0; i < ip->nNurses; ++i){
            for(int j = 0; j < ip->nJobs; ++j){
                nurseWaitingMatrixPointer[ct] = ip->nurseWaitingMatrix[i][j];
                ct++;
            }
            if(ip->verbose > 10){
                printf("\n");
            }
        }
    }

    //nurseTravelMatrix (nNursesxnJobs)
    ct = 0;
    if(nurseTravelMatrixPointer != NULL){
        if(ip->verbose > 10){
            printf("Allocating nurseTravelMatrix back to python\n");
        }
        for(int i = 0; i < ip->nNurses; ++i){
            for(int j = 0; j < ip->nJobs; ++j){
                nurseTravelMatrixPointer[ct] = ip->nurseTravelMatrix[i][j];
                ct++;
            }
            if(ip->verbose > 10){
                printf("\n");
            }
        }
    }

    if(totalsArrayPointer != NULL){ // NB: NEW: 04/06/2021
        totalsArrayPointer[0] = ip->objTime;
        totalsArrayPointer[1] = ip->objWaiting;
        totalsArrayPointer[2] = ip->objTravel;
        totalsArrayPointer[3] = ip->objService;
        totalsArrayPointer[4] = ip->objTardiness;
        totalsArrayPointer[5] = ip->objMaxTardiness;
        totalsArrayPointer[6] = ip->objMKTardiness;
        totalsArrayPointer[7] = ip->objMKAllowedTardiness;
        totalsArrayPointer[8] = ip->objOvertime;
        totalsArrayPointer[9] = ip->objMaxOvertime;
        totalsArrayPointer[10] = ip->objMinSpareTime;
        totalsArrayPointer[11] = ip->objMaxSpareTime;
        totalsArrayPointer[12] = ip->objShortestDay;
        totalsArrayPointer[13] = ip->objLongestDay;
        totalsArrayPointer[14] = ip->objAitHQuality;
        totalsArrayPointer[15] = ip->objMKQuality;
        totalsArrayPointer[16] = ip->objWBQuality;
        totalsArrayPointer[17] = ip->objPaperQuality;
        totalsArrayPointer[18] = ip->objQuality;
        totalsArrayPointer[19] = ip->totalPref;
        totalsArrayPointer[20] = ip->objMaxTravelTime;
        totalsArrayPointer[21] = ip->objMaxWaitingTime;
        totalsArrayPointer[22] = ip->objMaxDiffWorkload;
    }

}


void ConstructiveBasic(struct INSTANCE* ip){

    int nRows, nCols;

    if(ip->verbose > 1){
        printf("Generated instance!\n");
        printf("Read data for %d nurses and %d jobs. \n", ip->nNurses, ip->nJobs);
    }

    int oldAssignment = 0;
    if(oldAssignment > 0){
        nCols = ip->nNurses;

        int* jobsCount = malloc(nCols*sizeof(int));
        for(int i = 0; i < nCols; ++i)
            jobsCount[i] = 0;
        if(ip->verbose > 2){
            printf("Assigned vector for jobsCount.\n");
        }


        // First fit
        int jobsAssigned = 0;
        int chosenNurse = 0;
        float balancedTarget = (float) ip->nJobs/(float) ip->nNurses;
        int* jobPermutation = malloc(ip->nJobs*sizeof(int));
        for(int i = 0; i < ip->nJobs; ++i){
            jobPermutation[i] = i;
            if(ip->doubleService[i] <= 0.5){
                ip->doubleService[i] = -1;
            }
        }
        ///////////// TESTING! /////////////////
        // ip->doubleService[9] = 1;
        // ip->doubleService[5] = 1;
        Shuffle(jobPermutation, (size_t) ip->nJobs);
        if(ip->verbose > 2){
            printf("Generated a permutation of jobs:\n");
            PrintVector(jobPermutation, (size_t) ip->nJobs);
        }

        // Single assingments
        for(int ii = 0; ii < ip->nJobs; ++ii){
            int i = jobPermutation[ii];
            // int i = ii;
            for(int nCount = 0; nCount < ip->nNurses; ++nCount){
                int nursej = (chosenNurse + nCount)%(ip->nNurses);
                // printf("Chosen nurse: %d", nursej);
                // Check if nurse nursej can do job i:
                if(ip->nurseSkilled[nursej][i] > 0 || ip->doubleService[i] > 0){
                    // ip->solMatrix[nursej][i] = jobsCount[nursej];
                    BestJobInsertion(ip, i, nursej);
                    // printf("Job %d assigned to nurse %d in position %d\n", i, j, jobsCount[nursej]);
                    jobsCount[nursej] = jobsCount[nursej] + 1;
                    jobsAssigned = jobsAssigned + 1;
                    chosenNurse++;
                    // if ((float) jobsCount[j] > balancedTarget)
                    // 	chosenNurse = (chosenNurse + 1) % ip->nNurses;
                    // ip->doubleService[i] = -1;
                    // printf("About to check DS...\n");
                    if(ip->doubleService[i] > 0){
                        BestJobInsertion(ip, i, ((nursej + 1)%ip->nNurses));
                        jobsCount[((nursej + 1)%ip->nNurses)] = jobsCount[((nursej + 1)%ip->nNurses)] + 1;
                        if(ip->verbose > 2)
                            printf("Double service %d assigned to nurse %d\n", i, ((nursej + 1)%ip->nNurses));

                    }
                    // printf("Done.\n");
                    break;
                }

                // Check timewindow

            }
        }
        free(jobsCount);
        free(jobPermutation);

        if(jobsAssigned!=ip->nJobs){
            printf("WARNING: Not all jobs could be assigned, only %d out of %d\n", jobsAssigned, ip->nJobs);

            printf("Double services: ");

            for(int i = 0; i < ip->nJobs; ++i)
                if(ip->doubleService[i] > 0)
                    printf("%d, ", i);
            printf("\n");

            printf("nurseSkilled: \n");
            PrintIntMatrix(ip->nurseSkilled, ip->nNurses, ip->nJobs);

            printf("jobRequirements: \n");
            PrintIntMatrix(ip->jobRequirements, ip->nJobs, ip->nSkills);

            printf("nurseSkills: \n");
            PrintIntMatrix(ip->nurseSkills, ip->nNurses, ip->nSkills);

        }
        else if(ip->verbose > 2){
            printf("Assigned all jobs correctly.\n");
            ReportSolution(ip);
        }
    }
    else{
        InitialJobAssignment(ip);
    }
    // StandardLocalSearch(ip, ip->MAX_TIME_SECONDS);

}

int CheckSkills(struct INSTANCE* ip, int job, int nurse){
    //This function returns 1 if carer is skilled to do the job, and returns 0 otherwise.

    return ip->nurseSkilled[nurse][job];
}

int CheckSkillsDSFirst(struct INSTANCE* ip, int job, int nursei){

    // This function checks to see if there is another nurse that can do the double service with nurse i.
    // Function returns 1 if there is another nurse, and 0 if there is not.

    // This for loop is to count up the number of double service jobs up to our current one, 'job'.
    // This is because the capabilityOfDoubleServices 3d matrix is nNurses x nNurses x nDoubleServices! So we need to find out what number of double service our 'job' is.
    int jobdsindex = 0;
    for(int i = 0; i < job; ++i){ // Only count up to (but not including) our 'job', this ensures that jobdsindex is correct and not out of bounds.
        jobdsindex += ip->doubleService[i];
    }

    //Now go through all nurses and try to find another nurse, i, that can do our double service job, jobdsindex, together with our current nurse, careri.
    for(int i = 0; i < ip->nNurses; ++i){
        if(i==nursei){
            continue;
        }
        // Is there any other nurse that can do it?
        if(ip->capabilityOfDoubleServices[nursei][i][jobdsindex] > 0){ // careri and i are capable of performing the job together!
            return 1;
        }
    }

    return 0;

} // END OF CheckSkillsDSFirst function.

int CheckSkillsDS(struct INSTANCE* ip, int job, int nursei, int nursej){

    // This function checks whether careri and carerj are capable of doing double service 'job' together.
    // Returns 1 if yes, and 0 if no (capabilityOfDoubleService 3D matrix is a 0-1 matrix).

    // This for loop is to count up the number of double service jobs up to our current one, 'job'.
    // This is because the capabilityOfDoubleServices 3d matrix is nNurses x nNurses x nDoubleServices! So we need to find out what number of double service our 'job' is.
    int jobdsindex = 0;
    for(int i = 0; i < job; ++i){
        jobdsindex += ip->doubleService[i];
    }

    return ip->capabilityOfDoubleServices[nursei][nursej][jobdsindex];
}

void InitialJobAssignment(struct INSTANCE* ip){
    if(ip->verbose > 10){
        printf("Started initial job assignment...\n");
    }

    int* allocatedJobs = malloc(ip->nJobs*sizeof(int));
    for(int i = 0; i < ip->nJobs; ++i)
        allocatedJobs[i] = 0;
    int* nurseSeed = malloc(ip->nNurses*sizeof(int));
    for(int j = 0; j < ip->nNurses; ++j)
        nurseSeed[j] = -1;


    // Find seed for first nurse:


    int ajc = 0; // counter for allocated jobs
    int cNurse = -1;
    int cJob = -1;
    int its = 0;
    while(ajc < ip->nJobs){
        its++;
        // Doing this in parallel
        cNurse = (cNurse + 1)%ip->nNurses;

        if(nurseSeed[cNurse] < 0){
            nurseSeed[cNurse] = FurthestUnallocatedPointNurse(ip, allocatedJobs, cNurse);
            if(nurseSeed[cNurse] > -1 && BestJobInsertion(ip, nurseSeed[cNurse], cNurse) > -1){
                allocatedJobs[nurseSeed[cNurse]] = 1;
                ajc++;
                // printf("Allocated seed for nurse %d, job %d (total of %d out of %d)\n", cNurse, nurseSeed[cNurse], ajc, ip->nJobs);

            }
            else{
                printf("WARNING: Could not find any job for nurse %d at InitialJobAssignment()\n", cNurse);
                printf("Seed was: %d\n\n", nurseSeed[cNurse]);
                // exit(-1);
            }
        }
        else{
            int nextPoint = ClosestUnallocatedPointNurse(ip, allocatedJobs, nurseSeed[cNurse], cNurse);
            int wentWrong = 0;
            if(nextPoint < 0){
                wentWrong = 1;
                printf("WARNING: Got as results: %d\n", nextPoint);
            }
            else{
                wentWrong = BestJobInsertion(ip, nextPoint, cNurse);
                if(wentWrong < 0){
                    printf("BestJobInsertion returned %d\n", wentWrong);
                    wentWrong = 1;
                    if(ip->doubleService[nextPoint]){
                        printf("Job is a DS\n");
                    }
                    printf("ip->nurseSkills:\n");
                    PrintIntMatrix(ip->nurseSkills, ip->nNurses, ip->nSkills);
                    printf("ip->jobRequirements:\n");
                    PrintIntMatrix(ip->jobRequirements, ip->nJobs, ip->nSkills);

                }

            }

            if(wentWrong < 1){
                // printf("Allocated job for nurse %d, job %d (total of %d out of %d)\n", cNurse, nextPoint, ajc, ip->nJobs);
                allocatedJobs[nextPoint] = 1;
                ajc++;
            }
            else{
                printf("WARNING: Could not allocate anything for nurse %d (Iteration %d)\n", cNurse, its);
            }
        }

        if(its > 5000*ip->nJobs){
            printf("WARNING: After %d iterations could only allocate %d jobs out of %d.\n", its, ajc, ip->nJobs);
            break;
        }


    }

    // Assign double services now:
    for(int i = 0; i < ip->nJobs; ++i){
        if(ip->doubleService[i] < 1)
            continue;
        double bestTravel = bigM;
        int toThisNurse = -1;
        for(int j = 0; j < ip->nNurses; ++j){
            double intTravel = GetTravelTime(ip, nurseSeed[j], i);

            if(intTravel < GetTravelTime(ip, i, nurseSeed[j]))
                intTravel = GetTravelTime(ip, i, nurseSeed[j]);

            if(intTravel < bestTravel && ip->solMatrix[j][i] < 0){
                toThisNurse = j;
                bestTravel = intTravel;
            }
        }
        if(toThisNurse < 0 || BestJobInsertion(ip, i, toThisNurse) < 0){
            printf("\tWARNING: Could not allocate double service %d (toThisNurse = %d)!\n", i, toThisNurse);
            // FreeInstanceMemory(ip);
            // free(allocatedJobs);
            // free(nurseSeed);
            // exit(-1);
        }
        else
            continue;
        printf("ERROR: Could not allocate double service %d AT ALL!(toThisNurse = %d)!\n", i, toThisNurse);
    }

    free(allocatedJobs);
    free(nurseSeed);
    if(ip->verbose > 10){
        printf("Finished initial job assignment.\n");
    }

}

int ClosestUnallocatedPointNurse(struct INSTANCE* ip, int* allocatedJobs, int job, int nurse){
    int closestPoint = -1;
    double bestDistance = bigM;
    double cdist;
    for(int i = 0; i < ip->nJobs; ++i){
        if(allocatedJobs[i] > 0){
            continue;
        }
        if((ip->doubleService[i] < 1 && CheckSkills(ip, i, nurse) < 1)){
            continue;
        }

        // debug : speed // these could be two separate loops, so the if-statement is only made once
        if(job < -0.5)
            cdist = TravelTimeFromDepot(ip, nurse, i);
        else{
            cdist = GetTravelTime(ip, job, i);
            if(cdist < GetTravelTime(ip, i, job))
                cdist = GetTravelTime(ip, i, job);
        }

        if(cdist < bestDistance){
            closestPoint = i;
            bestDistance = cdist;
        }
        // printf("\tProperly considered job %d, with a travel dist to %d of %.2f (best is %.2f)\n", i, job, cdist, bestDistance);
    }
    // printf("\tReturning %d\n", closestPoint);

    return closestPoint;
}

int FurthestUnallocatedPointNurse(struct INSTANCE* ip, int* allocatedJobs, int nurse){
    // Furthest from all unallocated + DEPOT
    int furthestPoint = -1;
    double bestDistance = -1;
    double cdist;
    for(int job = -1; job < ip->nJobs; ++job){
        if(job > -1 && allocatedJobs[job] < 1){
            continue;
            // printf("Skipping: %d because allocatedJobs[job] = %d\n", job, allocatedJobs[job]);

        }
        // printf("Got to examine other jobs, coming here with: %d\n", job);

        for(int i = 0; i < ip->nJobs; ++i){
            if(allocatedJobs[i] > 0)
                continue;
            if(ip->doubleService[i] < 1 && CheckSkills(ip, i, nurse) < 0){
                continue;
                printf("\tSkipping job %d, because nurse %d is unskilled!\n", i, nurse);
            }


            // debug : speed // these could be two separate loops, so the if-statement is only made once
            if(job < -0.5)
                cdist = TravelTimeFromDepot(ip, nurse, i);
            else{
                cdist = GetTravelTime(ip, job, i);
                if(cdist > GetTravelTime(ip, i, job))
                    cdist = GetTravelTime(ip, i, job);
            }

            // printf("Going from %d to %d or viceversa takes a time of %.2f\n", i, job, cdist);
            if(cdist > bestDistance){
                furthestPoint = i;
                bestDistance = cdist;
            }
        }
    }
    // exit(0);
    return furthestPoint;
}

void PrintIntMatrix(int** matrix, int nRows, int nCols){
    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
}

void PrintIntMatrixOne(int* matrix, int nRows, int nCols){
    int count = 0;
    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            printf("%d\t", matrix[count]);
            count++;
        }
        printf("\n");
    }
}

void PrintDoubleMatrixOne(double* matrix, int nRows, int nCols){
    int count = 0;
    for(int i = 0; i < nRows; ++i){
        printf("|");
        for(int j = 0; j < nCols; ++j){
            printf("%.4f\t", matrix[count]);
            count++;
        }
        printf("|\n");
    }
}

void PrintDoubleMatrix(double** matrix, int nRows, int nCols){
    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            printf("%.4f\t", matrix[i][j]);
        }
        printf("\n");
    }
}

void PrintSolMatrix(struct INSTANCE* ip){
    for(int i = 0; i < ip->nNurses; ++i){
        for(int j = 0; j < ip->nJobs; ++j){
            if(ip->solMatrix[i][j] > -1)
                printf("%2d\t", ip->solMatrix[i][j]);
            else
                printf(" .\t", ip->solMatrix[i][j]);

        }
        printf("\t\\\\\n");
    }
}

void PrintAllNurseRoutes(struct INSTANCE* ip){
    for(int i = 0; i < ip->nNurses; ++i){
        for(int j = 0; j < ip->nJobs; ++j){
            if(ip->allNurseRoutes[i][j] > -1)
                printf("%2d\t", ip->allNurseRoutes[i][j]);
            else
                printf(" .\t", ip->allNurseRoutes[i][j]);

        }
        printf("\t\\\\\n");
    }
}

void PrintTimeMatrix(struct INSTANCE* ip){
    for(int i = 0; i < ip->nNurses; ++i){
        for(int j = 0; j < ip->nJobs; ++j){
            printf("%.2f\t", ip->timeMatrix[i][j]);
        }
        printf("\n");
    }
}

double GetTravelTime(struct INSTANCE* ip, int i, int j){
    // DEBUG : SPEED we can remove the error check if the -1 call is never made anymore
    if(i < 0 || j < 0){
        printf("\nERROR: Calling travel time with a -1 in get_travel_time(struct INSTANCE * ip, int jobi, int jobj).\n");
        printf("This is not valid since we allow for nurses to have different starting locations.\n");
        exit(-4323452);
    }

    return ip->od[i + 1][j + 1];
}

double TravelTimeFromDepot(struct INSTANCE* ip, int nurse, int job){
    if(ip->nurseTravelFromDepot[nurse][job] > 100000000000)
        printf("\nError! when calling with FROM nurse %d job %d, travel reported is %.3f", nurse, job, ip->nurseTravelFromDepot[nurse][job]);
    return ip->nurseTravelFromDepot[nurse][job];
}

double TravelTimeToDepot(struct INSTANCE* ip, int nurse, int job){
    if(ip->nurseTravelToDepot[nurse][job] > 100000000000)
        printf("\nError! when calling with TO nurse %d job %d, travel reported is %.3f", nurse, job, ip->nurseTravelToDepot[nurse][job]);
    return ip->nurseTravelToDepot[nurse][job];
}

double GetTravelTimeBNC(struct INSTANCE* ip, int nodei, int nodej){
    // The difference here is that the first ip->nNurses indices
    // correspond to nurse depots, the rest are jobs
    if(nodei < ip->nNurses && nodej < ip->nNurses){
        printf("\nWARNING: Trying to access distance between two nurse depots in 'GetTravelTimeBNC(...)', which is not defined.\n");
        return -1;  //	This is not defined
    }

    if(nodei < ip->nNurses)
        return TravelTimeFromDepot(ip, nodei, nodej - ip->nNurses);
    else if(nodej < ip->nNurses)
        return TravelTimeFromDepot(ip, nodej, nodei - ip->nNurses);
    else
        return GetTravelTime(ip, nodei - ip->nNurses, nodej - ip->nNurses);
}

int ReportSolution(struct INSTANCE* ip){

    //printf("ReportSolution\n");
    SolutionQuality(ip, 1);
    return 0;
}

int SwapPoints(struct INSTANCE* ip, int ni, int nj, int pi, int pj){
    // Swap the point pi from nurse ni by the point pj from nurse nj
    if(ip->solMatrix[ni][pi] < 0 || ip->solMatrix[nj][pj] < 0){
        printf("Jobs not assigned\n");
        return -1; // Nothing to swap
    }

    if(ip->nurseSkilled[ni][pj] < 1 || ip->nurseSkilled[nj][pi] < 1){
        printf("Not skilled\n");
        return -1; // Nothing to swap
    }

    int posi = ip->solMatrix[ni][pi];
    int posj = ip->solMatrix[nj][pj];
    ip->solMatrix[ni][pi] = -1;
    ip->solMatrix[nj][pj] = -1;
    ip->solMatrix[ni][pj] = posi;
    ip->solMatrix[nj][pi] = posj;
    return 0;
}

int RemoveJob(struct INSTANCE* ip, int job, int ni){
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
}

int JobInsertionLast(struct INSTANCE* ip, int job, int ni){
    double baseQuality = -1*bigM;
    // int nurse_job_count = get_nurse_job_count(ip, ni);

    if(InsertJobAtPosition(ip, job, ni, 0) < 0){
        return -1;
    }
    else{
        baseQuality = SolutionQuality(ip, -2000);
        return 0;
    }

}

int BestJobInsertion(struct INSTANCE* ip, int job, int ni){
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
        /*for(int i = 0; i < 10; ++i){
            printf("%2d\t", ip->solMatrix[ni][i]);
        }
        printf("\n");*/
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

int InsertJobAtPosition(struct INSTANCE* ip, int job, int ni, int posi){
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
}

int BestSyncDoubleSwitch(struct INSTANCE* ip){

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

int RouteTwoExchange(struct INSTANCE* ip, int firstImprovement){

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

int ExchangeJobsInRoute(struct INSTANCE* ip, int ni, int job_from_ni, int nj, int job_from_nj){

    //This function removes job_from_nj from nj's route and inserts job_from_ni into nj's route in the same position that job_from_nj was in, and
    //removes job_from_ni from ni's route and inserts job_from_nj into ni's route in the same position that job_from_ni was in.
    /// Maybe we should check ip->solMatrix[nj][job_from_ni] and ip->solMatrix[ni][job_from_nj] at the same time at the beginning of the function instead?

    //1. Put job_from_ni in nurse nj's route in the same position that job_from_nj was in, and remove job_from_nj from nj's route.
    int aux = ip->solMatrix[nj][job_from_ni]; //Position of job_from_ni in nj's route, should be =-1 as job_from_ni shouldn't be in nj's route!

    if(aux > -1){ // If job_from_ni is already in nj's route, then there's an error - we shouldn't do the swap.
        printf("\n---\n---\nWARNING: Exchanging a job maybe we should not????\n");
        printf("ip->solMatrix[%d][%d] = %d\n", nj, job_from_ni, aux);
        printf("\tTrying to exchange (n%d, j%d) with (n%d, j%d).\nSolmatrix:\n", ni, job_from_ni, nj, job_from_nj);
        PrintSolMatrix(ip);
        exit(-123424);
    }

    //Set position of job_from_ni in nj's route to be the same position as job_from_nj in nurse nj's route
    ip->solMatrix[nj][job_from_ni] = ip->solMatrix[nj][job_from_nj]; //e.g if job_from_nj is in position 3 in nj's route, now job_from_ni is set as position 3 in nurse nj's route.
    ip->solMatrix[nj][job_from_nj] = aux; //Then, set position of job_from_nj in nurse nj's route to be aux (which should be -1), so essentially removing job_from_nj from nj's route.

    //2. Put job_from_ni in nurse nj's route in the same position that job_from_nj was in, and remove job_from_nj from nj's route.
    aux = ip->solMatrix[ni][job_from_nj]; //Position of job_from_nj in nurse ni's route, should be =-1 as job_from_nj shouldn't be in ni's route!

    if(aux > -1){ //If job_from_nj is already in ni's route, then there's an error - we shouldn't do the swap.
        printf("\n---\n---\nWARNING: Exchanging a job maybe we should not????\n");
        printf("ip->solMatrix[%d][%d] = %d\n", ni, job_from_nj, aux);
        printf("\tTrying to exchange (n%d, j%d) with (n%d, j%d).\nSolmatrix:\n", ni, job_from_ni, nj, job_from_nj);
        PrintSolMatrix(ip);
        exit(-123424);
    }

    //Set position of job_from_nj in ni's route to be the same position as job_from_ni in nurse ni's route.
    ip->solMatrix[ni][job_from_nj] = ip->solMatrix[ni][job_from_ni]; //e.g. if job_from_ni is in position 5 in nurse ni's route, now job_from_nj is set as position 5 in nurse ni's route.
    ip->solMatrix[ni][job_from_ni] = aux; //Then, set position of job_from_ni in nurse ni's route to be aux (which should be -1), removing job_from_ni from ni's route.

    return 0;

} //END OF ExchangeJobsInRoute function.

int BestSwitch(struct INSTANCE* ip, int onlyInfeasible, double MAX_TIME){

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

    int** solMatrixInitial = malloc(nRows*sizeof(int*)); // Rows
    for(int i = 0; i < nRows; i++){
        solMatrixInitial[i] = malloc(nCols*sizeof(int)); // Cols
    }

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


    FreeMatrixInt(solMatrixInitial, ip->nNurses);
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

void FreeMatrixInt(int** matrix, int nRows){
    for(int i = 0; i < nRows; ++i){
        free(matrix[i]);
    }
    free(matrix);
}

int CopyIntMatrix(int** source, int** destination, int nRows, int nCols){
    for(int i = 0; i < nRows; ++i)
        for(int j = 0; j < nCols; ++j)
            destination[i][j] = source[i][j];
    return 0;
}

int NurseTwoExchange(struct INSTANCE* ip){

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

}

int TwoOptMove(struct INSTANCE* ip, int ni, int pos1, int pos2){

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

    int* nurseRoute = malloc(ip->nJobs*sizeof(int)); //size = 1 x nJobs (positions)
    GetNurseRoute(ip, ni, nurseRoute); //Fills array nurseRoute[position] = job for nurse ni's route

    if(nurseRoute[pos2] < 0){ // There's no job in position 2, so clearly not enough jobs in the route for this move
        free(nurseRoute);
        return -1;
    }

    // Start move:
    int* nurseRouteAux = malloc(ip->nJobs*sizeof(int));

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
    free(nurseRoute);
    free(nurseRouteAux);

    return 0;

} //END OF TwoOptMove function.

int FindSecondNurseDS(struct INSTANCE* ip, int job, int currentNurse){

    //For a 'job' that is a double service which currentNurse is assigned to, this function finds the other nurse, 'secondNurse', that is also assigned to 'job' with 'currentNurse'.

    int secondNurse = -1;
    for(int prevNurse = 0; prevNurse < ip->nNurses; ++prevNurse){
        if((ip->solMatrix[prevNurse][job] > -1) && (prevNurse!=currentNurse)){
            secondNurse = prevNurse;
            break;
        }
    }

    return secondNurse;
}

int SwitchNurse(struct INSTANCE* ip, int ni, int nj, int pi){

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
}

int Repair(struct INSTANCE* ip, int ni){
    // Try to fix waiting times and missed time windows
    // Find what the nurse is doing:
    int* nurseRoute = malloc(ip->nJobs*sizeof(int));
    GetNurseRoute(ip, ni, nurseRoute);

    // Find first missing time window or waiting time:
    int tardiness = 0;
    int waitingTime = 0;
    int problemAt = -1;
    int prevPoint = 0;
    int currentTime = ip->nurseWorkingTimes[ni][0];
    int job;
    int routePos = -1;
    for(int i = 0; i < ip->nJobs; ++i){
        if(nurseRoute[i] < 0)
            break;
        job = nurseRoute[i];
        if(i > 0){
            if(prevPoint > -0.5)
                currentTime += GetTravelTime(ip, prevPoint, job);// ip->od[prevPoint][job];
            else
                currentTime += TravelTimeFromDepot(ip, ni, job);
        }

        int arriveAt = currentTime;
        // int waitingTime = 0;
        prevPoint = job;
        if(arriveAt < ip->jobTimeInfo[job][0]){
            waitingTime = ip->jobTimeInfo[job][0] - arriveAt;
            problemAt = job;
            routePos = i;
            if(waitingTime > 0){
                // printf("Waiting time %d\n", waitingTime);
                break;
            }
        }

        // int tardiness = 0;
        if(arriveAt > ip->jobTimeInfo[job][1]){
            tardiness = arriveAt - ip->jobTimeInfo[job][1];
            problemAt = job;
            if(tardiness > 0){
                // printf("Found tardiness %d\n", tardiness);
                break;
            }

        }

        currentTime = currentTime + ip->jobTimeInfo[job][2] + waitingTime;
        // int leaveAt = currentTime;

        // totalTardiness += tardiness;
        // totalTime += leaveAt - ip->startTime;

    }

    // Repair it:
    if(tardiness > 0){
        if(SwapPoints(ip, ni, ni, job, prevPoint) < 0)
            printf("Failed repairing tardiness\n");
        else
            return 0;
    }
    else{
        // printf("Didnt find tardiness.\n");
        // return -1;
    }

    if(waitingTime > 0 && routePos > 0 && routePos < (ip->nJobs - 1) && nurseRoute[routePos + 1] > 0){
        if(SwapPoints(ip, ni, ni, job, nurseRoute[routePos + 1]) < 0)
            printf("Failed repairing waitingTime\n");
        else
            return 0;
    }
    else{
        // printf("Didnt find waiting time.\n");
        return -1;
    }

    free(nurseRoute);
    return 0;
}

int GetJobCount(struct INSTANCE* ip, int ni){

    //This function returns the number of jobs in nurse ni's route
    // +1 because if nurse has jobs in positons 0,1,2,3,4,5 then the highest position is 5 but the number of jobs is 6 (because of 0 index)

    int maxE = -1;
    for(int i = 0; i < ip->nJobs; ++i){
        if(ip->solMatrix[ni][i] > maxE)
            maxE = ip->solMatrix[ni][i];
    }
    return (maxE + 1);

} //END OF GetJobCount function.

int GetNurseJobCount(struct INSTANCE* ip, int nurse){

    //This function returns the number of jobs that are in nurse's route

    int count = 0;
    for(int i = 0; i < ip->nJobs; i++){
        if(ip->solMatrix[nurse][i] > -0.5){
            count++;
        }
    }

    return count;

} //END OF get_nurse_job_count function.

void SetAllNurseRoutes(struct INSTANCE* ip){
    // printf("Setting all -1\n");
    for(int ni = 0; ni < ip->nNurses; ni++){
        SetNurseRoute(ip, ni);
    }

}

void SetNurseRoute(struct INSTANCE* ip, int ni){
    // Same code as "get nurse route"
    //This function updates allNurseRoutes for nurse ni.

    //For nurse ni, set all jobs j in allNurseRoutes to -1
    for(int j = 0; j < ip->nJobs; ++j){
        ip->allNurseRoutes[ni][j] = -1;
    }

    //Then, set allNurseRoutes for nurse ni and position to job index
    for(int j = 0; j < ip->nJobs; ++j){
        if(ip->solMatrix[ni][j] >= 0){
            ip->allNurseRoutes[ni][ip->solMatrix[ni][j]] = j;
        }
    }
}

void AccessNurseRoute(struct INSTANCE* ip, int ni, int* nurseRoute){

    nurseRoute = ip->allNurseRoutes[ni];
    // printf("nuseRoute ACCESSED: value 0: %d, coming from %d\n", nurseRoute[0], ip->allNurseRoutes[ni][0]);
    // if (nurseRoute[0] > -1)
    // {
    // 	printf("Accessing...");
    // 	print_nurse_route(ip, ni, nurseRoute);
    // }

}

void GetNurseRoute(struct INSTANCE* ip, int ni, int* nurseRoute){
    int achange = 0; //what's this for?

    // NURSE ROUTE:
    for(int ii = 0; ii < ip->nJobs; ++ii){
        nurseRoute[ii] = -1;
    }

    for(int ii = 0; ii < ip->nJobs; ++ii){
        if(ip->solMatrix[ni][ii] >= 0){
            nurseRoute[ip->solMatrix[ni][ii]] = ii;
            // printf("Set nurseRoute[ip->solMatrix[ni][ii]] = nurseRoute[%d] to ii = %d\n", ip->solMatrix[ni][ii], ii);
            achange = 1;
            // if (ip->solMatrix[ni][ii] > max)
            // 	max = ip->solMatrix[ni][ii];
        }
    }
}

void PrintNurseRoute(struct INSTANCE* ip, int ni, int* nurseRoute){

    printf("\n--\nNurse route for nurse: %d\n", ni);
    PrintIntMatrixOne(nurseRoute, 1, ip->nJobs);
    printf("\n--\n");
}

void OldSetNurseTime(struct INSTANCE* ip, int nursej){

    // This function goes through the route of 'nursei' from the first job (position 0) in nursei's route to the final job (last position) in nurse i's route,
    // including to and from the depot (nursei's home) and calculates the time that each job in nursei's route starts and finishes.
    // The function goes through a for loop through all job positions in nursei's route, and uses the previous job in the route to calculate times for the next job in the route.

    int prevPoint = -1;
    double tTime; // Travel time for nursei from depot (their house) to the job or from a previous job to the job
    double arriveAt = 0; //This is used to keep up the current time, and the time that nursei arrives at a job
    double currentTime = (double) ip->nurseWorkingTimes[nursej][0]; // The start time of nursei (for the whole day)

    ip->nurseWaitingTime[nursej] = 0; // Reset total waiting time for nursei to 0.
    ip->nurseTravelTime[nursej] = 0; // Reset total travel time for nursei to 0.

    // Reset all times for nursei for all jobs to -1. Recall that timeMatrix stores time at which each nurse i (row) does each job j (column). If i doesn't do job j, then =-1.
    for(int j = 0; j < ip->nJobs; ++j){
        ip->timeMatrix[nursej][j] = -1;
    }

    for(int j = 0; j < ip->nJobs; ++j){ // Main for loop of function, going through all POSITIONS j=0,...,nJobs.
        /**CHECK THIS IF STATEMENT: if j= 0 then yes, the nurse isn't being used at all, otherwise if j > 0 and allNurseRoutes < 0 it doesn't mean the nurse isn't being used at all, just that there's no job in position j! **/
        if(ip->allNurseRoutes[nursej][j] < 0){ // If there is no job in position j of nursei's route, exit for loop. This means that nursei is not being used, as if it was, it should have a job in the first position of the route.
            break;
        }
        int job = ip->allNurseRoutes[nursej][j]; // job= = the job in position j of nursei's route.

        if(prevPoint < -0.5){ //If this 'job' is the first job in nursei's route, then calculate the distance from nursei's home to the 'job'.
            tTime = TravelTimeFromDepot(ip, nursej, job); //get_travel_time_from_depot function returns ip->nurseTravelFromDepot[nursei][job].
            if(ip->excludeNurseTravel){ // If we're NOT including the travel time from nursei's home to the first job to update the currentTime (i.e. we want the arrival time of the nurse to be the start of the job)
                if(ip->jobTimeInfo[job][0] > ip->nurseWorkingTimes[nursej][0] - 0.001){ // If the start time (start of time window) of 'job' is LATER than 'nursei's start timefor the day
                    currentTime = ip->jobTimeInfo[job][0]; // Set the current time in the route to be the start time (start of time window) of 'job' (rather than add the tTime it takes to travel to first job)
                    currentTime = currentTime + ip->twInterval; // Add the time window interval so that the job starts on time rather than at the start of the time window. DEBUG : HARDCODED, NEEDS TO BE PARAMETER (DONE)
                }
                else{
                    // TODO: Start of nurse's day (shift) is LATER than the start of the time window, should we check for this?
                     currentTime = ip->nurseWorkingTimes[nursej][0]; // NEW: 22/05/2021
                }
            }
            else{
                currentTime = currentTime + tTime;
            }
        }
        else if(prevPoint > -0.5){ // If there was a job before this 'job', then calculate the distance from the previous job to this 'job'.
            tTime = GetTravelTime(ip, prevPoint, job);  // get_travel_time function returns ip->od[prevPoint+1][job+1] (+1 because of extra row and column in od matrix).
            currentTime = currentTime + tTime; //The current time is set as the current time plus the time taken for nursei to travel to the job.
        }

        ip->nurseTravelTime[nursej] += tTime; // Updated nursei's travel time to include the time taken to travel from previous job/home to this 'job'

        arriveAt = currentTime;
        ip->timeMatrix[nursej][job] = currentTime; //Set time at which nursei does job 'job' to the current time.

        // In principle we use time window from the data, unless DS
        double startTW = ip->jobTimeInfo[job][0]; //start time window of 'job'
        double endTW = ip->jobTimeInfo[job][1]; //end time window of 'job'
        double startTWMK = ip->jobTimeInfo[job][0]; //start time window of 'job'
        double endTWMK = bigM;

        // Treat the time dependent jobs as a gap rather than precedence. It is only important they are separated by a certain time, but not who goes first (as in Ait H. paper, hence name aitOnly)
        // This is called earlier do_gap_not_precedence
        int aitOnly = 0;
        if(ip->algorithmOptions[12] + 0.001 > 1){
            aitOnly = 1;
        }

        int considerDependency = -1;

        if(ip->dependsOn[job] > -1){ // If 'job' depends on another job
            int otherJob = ip->dependsOn[job]; //otherJob = the job that 'job' depends on.
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){ // for each nurse index 0,...,nNurses
                int prevNurse = ip->nurseOrder[prevNurseInd]; //prevNurse = the number of the nurse in the nurseOrder array.
                if(prevNurse==nursej){ // If prevNurse is nursei
                    considerDependency = -1; //do not consider dependency, break out of for loop.
                    break;
                }
                if(ip->timeMatrix[prevNurse][otherJob] > 0){ //If prevNurse has a time at which it does job 'otherJob'
                    if(aitOnly > 0){
                        ip->mkMinD[job] = abs(ip->mkMinD[job]);
                        ip->mkMaxD[job] = abs(ip->mkMaxD[job]);

                        // Latest allowed arrival is:
                        double laa = ip->timeMatrix[prevNurse][otherJob] - ip->mkMaxD[job];

                        //If latest allowed arrival is later than or equal to the start of the time window, AND the current time is earlier than or equal to the latest allowed arrival.
                        if((laa >= startTW) && (arriveAt <= laa)){
                            // Then we can reverse the order of these dependencies!
                            ip->mkMinD[job] = -1*ip->mkMinD[job];
                            ip->mkMaxD[job] = -1*ip->mkMaxD[job];
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->mkMinD[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->mkMaxD[job];
                    considerDependency = 1;
                    break;
                }
            }
        }

        if(ip->doubleService[job] > 0){ //If 'job' is a double service, i.e. doubleService[job] == 1
            // Check previous nurses, is anyone serving this patient already?
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){ // for each nurse index 0,...,nNurses
                int prevNurse = ip->nurseOrder[prevNurseInd]; //prevNurse = the number of the nurse in the nurseOrder array
                if(prevNurse==nursej){ // If prevNurse is nursei, break out of for loop.
                    break;
                }
                if(ip->timeMatrix[prevNurse][job] > 0){ //If the time at which prevNurse does 'job' is > 0, i.e. prevNurse is assigned to do 'job'.
                    startTWMK = ip->timeMatrix[prevNurse][job];// + ip->mkMinD[job];
                    endTWMK = ip->timeMatrix[prevNurse][job]; //  + ip->mkMaxD[job];
                    break;
                }
            }
        }

        double waitingTime = 0;
        double worstStart = startTWMK;
        if(startTWMK < startTW){ //worstStart takes the latest start time window.
            worstStart = startTW;
        }

        if(arriveAt < worstStart){ //If arrival time is earlier than worst start time
            waitingTime = worstStart - arriveAt; //waiting time = time nurse has to wait from arriving at the job to starting the job.
            ip->nurseWaitingTime[nursej] += waitingTime; // update nurseWaitingTime for nursei
            ip->timeMatrix[nursej][job] += waitingTime; //update timeMatrix for nursei and 'job' so that the time nursei actually starts 'job' isn't just the arrival time, it's after the waiting time.
            arriveAt += waitingTime; //update current time to start of job.
        }

        if(arriveAt > endTWMK){ // Job starts late
            ip->violatedTWMK[job] += arriveAt - endTWMK; //gap between end of time window and time nurse arrives to the job.
        }

        double tardiness = 0; // Job starts late
        if(arriveAt > endTW){
            tardiness = arriveAt - endTW; //tardiness is how late the nurse is to the job, i.e. how long after endTW does the nurse arrive.
        }
        ip->violatedTW[job] += tardiness; //update violated time window for 'job'.

        prevPoint = job; //current job is now set as the previous job.
        currentTime = currentTime + ip->jobTimeInfo[job][2] + waitingTime; //current time = current time + time length of 'job' + waiting time, i.e. the current time is the time now after the job has been completed.

        // Add penalty for potential lateness and breaching of normal working hours
        // if (report > 0 && ip->verbose > 1){
        // 	printf("\tArrives at job %d at %.2f and leaves at %.2f\n", job, arriveAt, leaveAt);
        // 	if (waitingTime > 0)
        // 		printf("\t\tNeeds to wait for %.2f before starting the job\n", waitingTime);
        // 	if (tardiness > 0)
        // 		printf("\t\t*** Misses the time window by %.2f! ***\n", tardiness);
        // }

    } //End of for loop (j = 0; j < ip->nJobs; ++j)

    // Return to depot:
    if(prevPoint > -1){ // If a previous job is set, calculate the time it takes for the nurse to go from the last job (prevPoint) back to the depot (their home).
        ip->nurseTravelTime[nursej] += TravelTimeToDepot(ip, nursej, prevPoint); //get_travel_time_to_depot function returns ip->nurseTravelToDepot[nursei][prevPoint].
    }

} //END OF OldSetNurseTime function

void SetNurseTimeAitH(struct INSTANCE* ip, int nursej){

    int prevPoint = -1;
    double currentTime = (double) ip->nurseWorkingTimes[nursej][0];

    int fel_ac = ip->allNurseRoutes[nursej][0];


    ip->nurseWaitingTime[nursej] = 0;
    ip->nurseTravelTime[nursej] = 0;
    double arriveAt = 0;
    double tTime;
    for (int i = 0; i < ip->nJobs; ++i){
        ip->timeMatrix[nursej][i] = -1;
    }


    for (int i = 0; i < ip->nJobs; ++i){
        if (ip->allNurseRoutes[nursej][i] < 0){
            break;
        }
        int job = ip->allNurseRoutes[nursej][i];
        // Trip from depot:
        if (prevPoint > -0.5){
            tTime = GetTravelTime(ip, prevPoint, job);
        }
        else{
            tTime = TravelTimeFromDepot(ip, nursej, job);
        }


        // onlyTravelTime += tTime;

        ip->nurseTravelTime[nursej] += tTime;

        currentTime = currentTime + tTime;//ip->od[prevPoint][job];

        arriveAt = currentTime;
        ip->timeMatrix[nursej][job] = currentTime;

        // In principle we use time window from the data, unless DS
        double startTW = ip->jobTimeInfo[job][0];
        double endTW = ip->jobTimeInfo[job][1];
        double startTWMK = ip->jobTimeInfo[job][0];
        double endTWMK = bigM;


        // Treat the time dependent jobs as a gap rather than precedence.
        // It is only important they are separated by a certain time,
        // but not who goes first (as in Ait H. paper, hence name aitOnly)
        // This is called earlier do_gap_not_precedence
        int aitOnly = 0;
        if (ip->algorithmOptions[12] + 0.001 > 1){
            aitOnly = 1;
        }

        // int aitOnly = 1;
        // if (ip->quality_measure > 0.5)
        // 	aitOnly = -1;

        int considerDependency = -1;

        if (ip->dependsOn[job] > -1){
            int otherJob = ip->dependsOn[job];
            for (int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if (prevNurse == nursej){
                    considerDependency = -1;
                    break;
                }

                if (ip->timeMatrix[prevNurse][otherJob] > 0){
                    if (aitOnly > 0){

                        ip->mkMinD[job] = abs(ip->mkMinD[job]);
                        ip->mkMaxD[job] = abs(ip->mkMaxD[job]);

                        // Latest allowed arrival is:
                        double laa = ip->timeMatrix[prevNurse][otherJob] - ip->mkMaxD[job];

                        if ((laa >= startTW) && (arriveAt <= laa)){
                            // Then we can reverse the order of these dependencies!
                            ip->mkMinD[job] = -1*ip->mkMinD[job];
                            ip->mkMaxD[job] = -1*ip->mkMaxD[job];
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->mkMinD[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->mkMaxD[job];
                    considerDependency = 1;
                    break;
                }
            }
        }


        if (ip->doubleService[job] > 0){
            // Check previous nurses, is anyone serving this patient already?
            for (int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if (prevNurse == nursej){
                    break;
                }

                if (ip->timeMatrix[prevNurse][job] > 0){

                    // else
                    // {
                    startTWMK = ip->timeMatrix[prevNurse][job];// + ip->MK_mind[job];
                    endTWMK = ip->timeMatrix[prevNurse][job]; //  + ip->MK_maxd[job];

                    break;
                }
            }
        }


        // int doMKWay = 1;
        double waitingTime = 0;

        // if (doMKWay > 0)
        // {
        double worstStart = startTWMK;
        if (startTWMK < startTW){
            worstStart = startTW;
        }

        // if (ip->dependsOn[job] > -1 && considerDependency > 0)
        // {
        // 	// In case this job has to start latter than the other part of the job, allow a gap to do so.
        // 	if (worstStart < startTW + ip->MK_mind[job])
        // 		worstStart = startTW + ip->MK_mind[job];
        // }

        if (arriveAt < worstStart){
            waitingTime = worstStart - arriveAt;
            ip->nurseWaitingTime[nursej] += waitingTime;
            ip->timeMatrix[nursej][job] += waitingTime;
            arriveAt += waitingTime;
        }


        if (arriveAt > endTWMK){
            ip->violatedTWMK[job] += arriveAt - endTWMK;
        }


        double tardiness = 0;
        if (arriveAt > endTW){
            tardiness = arriveAt - endTW;
        }
        ip->violatedTW[job] += tardiness;



        prevPoint = job;
        currentTime = currentTime + ip->jobTimeInfo[job][2] + waitingTime;
        // leaveAt = currentTime;

        // totalTardiness += tardiness;
        // totalTime += leaveAt - ip->nurseWorkingTimes[j][0];


        // Add penalty for potential lateness and breaching of normal working hours
        // if (report > 0 && ip->verbose > 1)
        // {
        // 	printf("\tArrives at job %d at %.2f and leaves at %.2f\n", job, arriveAt, leaveAt);
        // 	if (waitingTime > 0)
        // 		printf("\t\tNeeds to wait for %.2f before starting the job\n", waitingTime);
        // 	if (tardiness > 0)
        // 		printf("\t\t*** Misses the time window by %.2f! ***\n", tardiness);
        // }
    }

    // Return to depot:
    // double tTime = get_travel_time(ip, prevPoint, -1);
    if (prevPoint > -1){
        //ip->nurseTravelTime[nursej] += get_travel_time_to_depot(ip, nursej, prevPoint);
        ip->nurseTravelTime[nursej] += TravelTimeToDepot(ip, nursej, prevPoint);
    }

} // End of SetNurseTimeAitH function

void SetNurseTimeOld(struct INSTANCE* ip, int nursei){

    // SET NURSE TIME FUNCTION THAT WAS ORIGINALLY USED PRIOR TO 17/02/2022
    /** NEW_SET_NURSE_TIME FUNCTION **/

    //printf("Here set_nurse_time, nursei: %d.\n", nursei);

    int prevPoint = -1; // previous job
    double tTime; // travel time
    double arriveAt = 0; // Keep up current time, and the time nursei arrives at a job
    double currentTime = (double) ip->nurseWorkingTimes[nursei][0]; // The start time of nursei (for the whole day)

    ip->nurseWaitingTime[nursei] = 0; // Reset waiting time for nursei
    ip->nurseTravelTime[nursei] = 0; // Reset travel time for nursei

    //NB: violatedTW is reset to all 0's in SetTimesFull function or SetTimesFrom function

    // Reset timeMatrix for nursei (time at which nursei does job j - if nursei does not do job j then timeMatrix[nursei][j] = -1)
    for(int j = 0; j < ip->nJobs; ++j){
        ip->timeMatrix[nursei][j] = -1;
    }

    // Reset nurseWaitingMatrix for nursei (waiting time for nursei doing job j - if nursei does not do job j or there is no waiting time then nurseWaitingMatrix[nursei][j] = 0)
    for(int j = 0; j < ip->nJobs; ++j){
        ip->nurseWaitingMatrix[nursei][j] = 0;
    }

    // Reset nurseTravelMatrix for nursei (travel time for nursei going to job j - if nursei does not do job j or there is no travel time then nurseTravelMatrix[nursei][j] = 0)
    for(int j = 0; j < ip->nJobs; ++j){
        ip->nurseTravelMatrix[nursei][j] = 0;
    }

    // Determine the number of unavailable 'shifts' for nursei
    int numUnavail = ip->nurseUnavail[nursei];

    // Main for loop:
    for(int j = 0; j < ip->nJobs; ++j){ // Main for loop of function, going through all POSITIONS j=0,...,nJobs.
        if(ip->allNurseRoutes[nursei][j] < 0){ // If there is no job in position j of nursei's route, then we have reached the final position of nursei, break out of for loop.
            break;
        }

        int job = ip->allNurseRoutes[nursei][j]; // job = the job in position j of nursei's route.
        //printf("nursei: %d, j: %d, job: %d.\n", nursei, j, job);
        // NB: PART ONE: determine 'currentTime' and 'arriveAt', which is the time the nurse is at the location for 'job'
        if(prevPoint < -0.5){ // 'job' is the first job in nursei's route - there is no previous job, so prevPoint = -1.
            tTime = TravelTimeFromDepot(ip, nursei, job); // get_travel_time_from_depot = ip->nurseTravelFromDepot[nursei][job]
            if(ip->excludeNurseTravel){ // Exclude tTime (from nursei's home to this first job) when updating currentTime, but still include tTime when updating ip->nurseTravelTime[nursei]
                if(ip->jobTimeInfo[job][0] > ip->nurseWorkingTimes[nursei][0] - 0.001){ // If the start TW of 'job' is LATER than the start time of nursei's shift for the day
                    currentTime = ip->jobTimeInfo[job][0]; // currentTime is updated to be the start of the TW for 'job'
                    currentTime = currentTime + ip->twInterval; // currentTime is updated to be the actual start time of the job (this is done by moving the currentTime forward by the TW interval)
                }
                else{ // TODO: If the start time of nursei's day is LATER than the start of the TW for 'job', should we check for this?
                    currentTime = ip->nurseWorkingTimes[nursei][0];
                }
            }
            else{ // Include tTime (from nursei's home to 'job') when updating currentTime and when updating ip->nurseTravelTime[nursei]
                ip->nurseTravelMatrix[nursei][job] = tTime;
                currentTime = currentTime + tTime; // currentTime is updated to be the time that nursei arrives at 'job'
            }
        }
        else if(prevPoint > -0.5){ // 'job' is NOT the first job in nursei's route
            tTime = GetTravelTime(ip, prevPoint, job); // get_travel_time = ip->od[prevPoint+1][job+1]
            ip->nurseTravelMatrix[nursei][job] = tTime;
            currentTime = currentTime + tTime; // currentTime is updated to be the time that nursei arrives at 'job'
        }

        ip->nurseTravelTime[nursei] += tTime; // Update the total travel time for nursei.

        arriveAt = currentTime; //arriveAt is updated to be the currentTime, i.e. the time that the nurse is ready at the location of the job.
        ip->timeMatrix[nursei][job] = currentTime; // Update timeMatrix for nursei and job to be the current time that the nurse is at the location of the job.
        // End part one

        double startTW = ip->jobTimeInfo[job][0]; // Start of TW for job
        double endTW = ip->jobTimeInfo[job][1]; // End of TW for job
        double startTWMK = ip->jobTimeInfo[job][0]; // Start of TW for job (MK)
        double endTWMK = bigM; //End of TW for job (MK)

        // Treat the time dependent jobs as a gap rather than precedence. It is only important they are separated by a certain time, but not who goes first (as in Ait H. paper, hence name aitOnly)
        // This is called earlier do_gap_not_precedence
        int aitOnly = 0;
        if(ip->algorithmOptions[12] + 0.001 > 1){
            aitOnly = 1;
        }

        int considerDependency = -1;

        // Dependent jobs:
        if(ip->dependsOn[job] > -1){
            int otherJob = ip->dependsOn[job];
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    considerDependency = -1;
                    break;
                }
                if(ip->timeMatrix[prevNurse][otherJob] > 0){
                    if(aitOnly > 0){
                        ip->mkMinD[job] = abs(ip->mkMinD[job]);
                        ip->mkMaxD[job] = abs(ip->mkMaxD[job]);

                        double laa = ip->timeMatrix[prevNurse][otherJob] - ip->mkMaxD[job];
                        if((laa >= startTW) && (arriveAt <= laa)){ //NB: arrive at is used here!
                            ip->mkMinD[job] = -1*ip->mkMinD[job];
                            ip->mkMaxD[job] = -1*ip->mkMaxD[job];
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->mkMinD[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->mkMaxD[job];
                    considerDependency = 1;
                    break;
                }
            }
        }

        // Double service jobs:
        if(ip->doubleService[job] > 0){
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    break;
                }
                if(ip->timeMatrix[prevNurse][job] > 0){
                    startTWMK = ip->timeMatrix[prevNurse][job];
                    endTWMK = ip->timeMatrix[prevNurse][job];
                    break;
                }
            }
        }

        // NB: PART TWO: Update arriveAt to be after waitingTime (if there is any) and calculate tardiness.
        double tardiness = 0;
        double waitingTime = 0;
        double worstStart = startTWMK;
        if(startTWMK < startTW){
            worstStart = startTW; //worstStart takes the latest earliest start time window.
        }

        if(arriveAt < worstStart){ // arriveAt is EARLIER than the earliest start time
            waitingTime = worstStart - arriveAt; // waiting time is incurred
            //ip->nurseWaitingTime[nursei] += waitingTime;
            ip->nurseWaitingMatrix[nursei][job] = waitingTime;
            ip->timeMatrix[nursei][job] += waitingTime; // Update timeMatrix for nursei and 'job' so that the time nursei actually starts 'job' isn't just the arrival time, it's after the waiting time.
            arriveAt += waitingTime; // Update arriveAt to be the start time of the actual job (start of time window)
        }

        if(arriveAt > endTWMK){ // Job starts late: arriveAt is LATER than the end of the time window
            ip->violatedTWMK[job] += arriveAt - endTWMK; //Gap between end of time window and time nurse arrives to the job.
        }

        if(arriveAt > endTW){ // Job starts late: arriveAt is LATER than the end of the time window
            tardiness = arriveAt - endTW; //tardiness is how late the nurse is to the job, i.e. how long after endTW does the nurse arrive.
        }
        ip->violatedTW[job] += tardiness; // Updated violated TW for 'job'

        // This only occurs if nursei has unavailable shifts, need to check that nursei isn't set to do job during an unavailable shift
        int first = 0;
        if(numUnavail > 0){
            for(int i = 0; i < numUnavail; ++i){
                // If currentTime is BEFORE the start of unavailable shift and 'job' ENDS after the start of the unavailable shift (so finished either during or after unavailable shift), then we need to move the
                // job so that it starts after the unavailable shift ends.
                if(arriveAt < ip->unavailMatrix[i][1][nursei] && arriveAt + ip->jobTimeInfo[job][2] > ip->unavailMatrix[i][1][nursei]){
                    waitingTime = ip->unavailMatrix[i][1][nursei] - arriveAt; //waiting time = time from currentTime to the start of the unavailable shift
                    //ip->nurseWaitingTime[nursei] += waitingTime;
                    ip->nurseWaitingMatrix[nursei][job] += waitingTime;
                    arriveAt = ip->unavailMatrix[i][2][nursei]; // update currentTime to be the end of the unavailable shift.
                    ip->timeMatrix[nursei][job] = arriveAt;
                    tardiness = arriveAt - endTW; // tardiness is how late the nurse is to the job, so how long after the end of the job TW does the nurse start the job.
                    ip->violatedTW[job] = tardiness; // Updated violated TW for 'job'
                    first = 1;
                }
                // Else if currentTime is at or AFTER the start of an unavailable shift (and before the end of the unavailable shift) (and could end within or after the unavailable shift ends (doesn't matter)),
                // then we need to move the job so that it starts when the unavailable shift ends.
                else if(arriveAt >= ip->unavailMatrix[i][1][nursei] && arriveAt < ip->unavailMatrix[i][2][nursei]){
                    if(first == 0 && currentTime < ip->unavailMatrix[i][1][nursei]){
                        waitingTime = ip->unavailMatrix[i][1][nursei] - currentTime;
                        ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                        first = 1;
                    }
                    arriveAt = ip->unavailMatrix[i][2][nursei]; // update currentTime to be the end of the unavailable shift.
                    ip->timeMatrix[nursei][job] = arriveAt;
                    if(arriveAt > endTW){ // Only calculate tardiness if the job is actually late, i.e. nursei arrives at job after the time window
                        tardiness = arriveAt - endTW; //tardiness is how late the nurse is to the job, so how long after the end of the job TW does the nurse start the job.
                    }
                    else{
                        tardiness = 0;
                    }
                    ip->violatedTW[job] = tardiness; // Updated violated TW for 'job'
                    first = 1;
                }
            } //End for loop numUnavail
        }// End if(numUnavail > 0)

        // NB: update current time to be the time after the waiting time (if any) and after the duration of the job, so currentTime is after the job has finished.
        prevPoint = job; // previous job is now set to the current job
        //currentTime = currentTime + ip->jobTimeInfo[job][2] + waitingTime; //current time = current time + time length of 'job' + waiting time, i.e. the current time is the time now after the job has been completed.
        currentTime = arriveAt + ip->jobTimeInfo[job][2]; //current time = arriveAt (which includes waitingTime) + time length of 'job', i.e. the current time is the time now after the job has been completed.

    } //End of for loop (j = 0; j < ip->nJobs; ++j)

    // Return to depot:
    if(prevPoint > -1){ // If a previous job is set, calculate the time it takes for the nurse to go from the last job (prevPoint) back to the depot (their home).
        double tTime2 = TravelTimeToDepot(ip, nursei, prevPoint); //get_travel_time_to_depot function returns ip->nurseTravelToDepot[nursei][prevPoint].
        //ip->nurseTravelTime[nursei] += get_travel_time_to_depot(ip, nursei, prevPoint); //get_travel_time_to_depot function returns ip->nurseTravelToDepot[nursei][prevPoint].
        ip->nurseTravelTime[nursei] += tTime2; //get_travel_time_to_depot function returns ip->nurseTravelToDepot[nursei][prevPoint].
        //ip->nurseTravelMatrix[nursei][prevPoint] = tTime2;
    }

    // Update nurseWaitingTime for nursei to be the sum of all waiting times for nursei in nurseWaitingMatrix
    for(int j = 0; j < ip->nJobs; ++j){
        ip->nurseWaitingTime[nursei] += ip->nurseWaitingMatrix[nursei][j];
    }

    //printf("End set_nurse_time.\n");
} //END OF new_set_nurse_time function

void SetNurseTime(struct INSTANCE* ip, int nursei){

    /** SetNurseTime funtion 17/02/2022 */
    // Called in SetTimesFrom and SetTimesFull, both in for loops going through all nurses i = 0 to ip->nNurses.

    int prevPoint = -1; // previous job
    double tTime; // travel time
    double arriveAt = 0; // Keep up current time, and the time nursei arrives at a job
    double currentTime = (double) ip->nurseWorkingTimes[nursei][0]; // The start time of nursei (for the whole day)

    ip->nurseWaitingTime[nursei] = 0; // Reset waiting time for nursei
    ip->nurseTravelTime[nursei] = 0; // Reset travel time for nursei

    //NB: violatedTW and violatedTWMK are reset to all 0's in SetTimesFull function or SetTimesFrom function

    // Reset timeMatrix for nursei (time at which nursei does job j - if nursei does not do job j then timeMatrix[nursei][j] = -1)
    for(int j = 0; j < ip->nJobs; ++j){
        ip->timeMatrix[nursei][j] = -1;
    }

    // Reset nurseWaitingMatrix for nursei (waiting time for nursei doing job j - if nursei does not do job j or there is no waiting time then nurseWaitingMatrix[nursei][j] = 0)
    for(int j = 0; j < ip->nJobs; ++j){
        ip->nurseWaitingMatrix[nursei][j] = 0;
    }

    // Reset nurseTravelMatrix for nursei (travel time for nursei going to job j - if nursei does not do job j or there is no travel time then nurseTravelMatrix[nursei][j] = 0)
    for(int j = 0; j < ip->nJobs; ++j){
        ip->nurseTravelMatrix[nursei][j] = 0;
    }

    // Determine the number of unavailable 'shifts' for nursei
    int numUnavail = ip->nurseUnavail[nursei];

    // Main for loop:
    for(int j = 0; j < ip->nJobs; ++j){ // Main for loop of function, going through all POSITIONS j=0,...,nJobs.
        if(ip->allNurseRoutes[nursei][j] < 0){ // If there is no job in position j of nursei's route, then we have reached the final position of nursei, break out of for loop.
            break;
        }

        int job = ip->allNurseRoutes[nursei][j]; // job = the job in position j of nursei's route.
        //printf("nursei: %d, j: %d, job: %d.\n", nursei, j, job);
        // NB: PART ONE: determine 'currentTime' and 'arriveAt', which is the time the nurse is at the location for 'job'
        if(prevPoint < -0.5){ // 'job' is the first job in nursei's route - there is no previous job, so prevPoint = -1.
            tTime = TravelTimeFromDepot(ip, nursei, job); // get_travel_time_from_depot = ip->nurseTravelFromDepot[nursei][job]
            if(ip->excludeNurseTravel){ // Exclude tTime (from nursei's home to this first job) when updating currentTime, but still include tTime when updating ip->nurseTravelTime[nursei]
                if(ip->jobTimeInfo[job][0] > ip->nurseWorkingTimes[nursei][0] - 0.001){ // If the start TW of 'job' is LATER than the start time of nursei's shift for the day
                    currentTime = ip->jobTimeInfo[job][0]; // currentTime is updated to be the start of the TW for 'job'
                    currentTime = currentTime + ip->twInterval; // currentTime is updated to be the actual start time of the job (this is done by moving the currentTime forward by the TW interval)
                }
                else{ // TODO: If the start time of nursei's day is LATER than the start of the TW for 'job', should we check for this?
                    currentTime = ip->nurseWorkingTimes[nursei][0];
                }
            }
            else{ // Include tTime (from nursei's home to 'job') when updating currentTime and when updating ip->nurseTravelTime[nursei]
                ip->nurseTravelMatrix[nursei][job] = tTime;
                currentTime = currentTime + tTime; // currentTime is updated to be the time that nursei arrives at 'job'
            }
        }
        else if(prevPoint > -0.5){ // 'job' is NOT the first job in nursei's route
            tTime = GetTravelTime(ip, prevPoint, job); // get_travel_time = ip->od[prevPoint+1][job+1]
            ip->nurseTravelMatrix[nursei][job] = tTime;
            currentTime = currentTime + tTime; // currentTime is updated to be the time that nursei arrives at 'job'
        }

        ip->nurseTravelTime[nursei] += tTime; // Update the total travel time for nursei.

        arriveAt = currentTime; //arriveAt is updated to be the currentTime, i.e. the time that the nurse is ready at the location of the job.
        ip->timeMatrix[nursei][job] = currentTime; // Update timeMatrix for nursei and job to be the current time that the nurse is at the location of the job.
        // End part one

        double startTW = ip->jobTimeInfo[job][0]; // Start of TW for job
        double endTW = ip->jobTimeInfo[job][1]; // End of TW for job
        double startTWMK = ip->jobTimeInfo[job][0]; // Start of TW for job (MK)
        double endTWMK = bigM; //End of TW for job (MK)

        // Treat the time dependent jobs as a gap rather than precedence. It is only important they are separated by a certain time, but not who goes first (as in Ait H. paper, hence name aitOnly)
        // This is called earlier do_gap_not_precedence
        int aitOnly = 0;
        if(ip->algorithmOptions[12] + 0.001 > 1){
            aitOnly = 1;
        }

        int considerDependency = -1;
        int considerDoubleService = -1; //NEW 17/02/2022
        int otherNurseDJ = -1; //NEW 17/02/2022
        int otherNurseDS = -1; //NEW 17/02/2022
        int otherJobDJ = -1; //NEW 17/02/2022

        // Dependent jobs:
        if(ip->dependsOn[job] > -1){
            otherJobDJ = ip->dependsOn[job];
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    otherNurseDJ = -1; //NEW 17/02/2022
                    considerDependency = -1;
                    break;
                }
                if(ip->timeMatrix[prevNurse][otherJobDJ] > 0){
                    if(aitOnly > 0){
                        ip->mkMinD[job] = abs(ip->mkMinD[job]);
                        ip->mkMaxD[job] = abs(ip->mkMaxD[job]);

                        // NB 17/02/2022: NEED TO CHECK THESE LINES FROM laa ip-?TM[prevNurse][otherjobDJ]... to otherNurseDJ = prevNurse
                        double laa = ip->timeMatrix[prevNurse][otherJobDJ] - ip->mkMaxD[job]; // NB 17/02/2022: HERE WE SUBTRACT ip->mkMaxD[job], IS IT THE RIGHT ONE? WHY SUBTRATCT, HOW DO WE KNOW WHICH JOB COMES FIRST?
                        if((laa >= startTW) && (arriveAt <= laa)){ //NB: arrive at is used here!
                            ip->mkMinD[job] = -1*ip->mkMinD[job]; // NB 17/02/2022: ARE THESE CORRECT? CHANGED IN THE PAPER
                            ip->mkMaxD[job] = -1*ip->mkMaxD[job];
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMinD[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMaxD[job];
                    considerDependency = 1; //NEW 17/02/2022
                    otherNurseDJ = prevNurse; // NB 17/02/2022: NEED TO CHECK THESE LINES FROM laa ip-?TM[prevNurse][otherjobDJ]... to otherNurseDJ = prevNurse
                    break;
                }
            }
        }

        // Double service jobs:
        if(ip->doubleService[job] > 0){
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    otherNurseDS = -1; //NEW 17/02/2022
                    considerDoubleService = -1; //NEW 17/02/2022
                    break;
                }
                if(ip->timeMatrix[prevNurse][job] > 0){
                    startTWMK = ip->timeMatrix[prevNurse][job];
                    endTWMK = ip->timeMatrix[prevNurse][job];
                    considerDoubleService = 1; //NEW 17/02/2022
                    otherNurseDS = prevNurse; //NEW 17/02/2022
                    break;
                }
            }
        }

        // NB: PART TWO: Update arriveAt to be after waitingTime (if there is any) and calculate tardiness.
        double tardiness = 0;
        double waitingTime = 0;
        double worstStart = startTWMK;
        if(startTWMK < startTW){ // NB: WHY?
            worstStart = startTW; //worstStart takes the latest earliest start time window.
        }

        if(arriveAt < worstStart){ // arriveAt is EARLIER than the earliest start time
            waitingTime = worstStart - arriveAt; // waiting time is incurred
            //ip->nurseWaitingTime[nursei] += waitingTime;
            ip->nurseWaitingMatrix[nursei][job] = waitingTime;
            ip->timeMatrix[nursei][job] += waitingTime; // Update timeMatrix for nursei and 'job' so that the time nursei actually starts 'job' isn't just the arrival time, it's after the waiting time.
            arriveAt += waitingTime; // Update arriveAt to be the start time of the actual job (start of time window)
        }

        if(arriveAt > endTWMK){ // Job starts late: arriveAt is LATER than the end of the time window
            ip->violatedTWMK[job] += arriveAt - endTWMK; //Gap between end of time window and time nurse arrives to the job.
        }

        if(arriveAt > endTW){ // Job starts late: arriveAt is LATER than the end of the time window
            tardiness = arriveAt - endTW; //tardiness is how late the nurse is to the job, i.e. how long after endTW does the nurse arrive.
        }
        ip->violatedTW[job] += tardiness; // Updated violated TW for 'job'

        // This only occurs if nursei has unavailable shifts, need to check that nursei isn't set to do job during an unavailable shift
        // NB NEW 17/02/2022: MAKE startTWMK and endTWMK suitable for the FindValidTime function
        double startTWFVT = startTWMK;
        double endTWFVT = endTWMK;
        if(considerDependency < 0 && considerDoubleService < 0){
            startTWFVT = -1;
            endTWFVT = -1;
        }

        // NB 17/02/2022: changed for loop from int i = 0 to int f = 0 to match with paper.
        // Recall that number of unavailable shifts (breaks) = number of actual shifts - 1, i.e. number of shifts = numUnavail + 1
        int first = 0;
        if(numUnavail > 0){
            for(int f = 0; f < numUnavail; ++f){ //NB NEED TO REMOVE THIS, should be at the top in a while loop
                // If currentTime is BEFORE the start of unavailable shift and 'job' ENDS after the start of the unavailable shift (so finished either during or after unavailable shift), then we need to move the
                // job so that it starts after the unavailable shift ends.
                if(arriveAt < ip->unavailMatrix[f][1][nursei] && arriveAt + ip->jobTimeInfo[job][2] > ip->unavailMatrix[f][1][nursei]){
                    //NB NEW 17/02/2022: CALLING FVT FUNCTION WHICH RETURNS AN int* array FOR IF ARRIVAL + SERVICE TIME IS INTO BREAK
                    double* timesArray;
                    timesArray = FindValidTime(ip, f, arriveAt, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWFVT, endTWFVT);

                    waitingTime = ip->unavailMatrix[f][1][nursei] - arriveAt; //waiting time = time from currentTime to the start of the unavailable shift
                    //ip->nurseWaitingTime[nursei] += waitingTime;
                    ip->nurseWaitingMatrix[nursei][job] += waitingTime;
                    arriveAt = ip->unavailMatrix[f][2][nursei]; // update currentTime to be the end of the unavailable shift.
                    ip->timeMatrix[nursei][job] = arriveAt;
                    tardiness = arriveAt - endTW; // tardiness is how late the nurse is to the job, so how long after the end of the job TW does the nurse start the job.
                    ip->violatedTW[job] = tardiness; // Updated violated TW for 'job'
                    first = 1;
                }
                    // Else if currentTime is at or AFTER the start of an unavailable shift (and before the end of the unavailable shift) (and could end within or after the unavailable shift ends (doesn't matter)),
                    // then we need to move the job so that it starts when the unavailable shift ends.
                else if(arriveAt >= ip->unavailMatrix[f][1][nursei] && arriveAt < ip->unavailMatrix[f][2][nursei]){
                    if(first == 0 && currentTime < ip->unavailMatrix[f][1][nursei]){ //NB: DO WE NEED THIS CONDITION NOW?
                        waitingTime = ip->unavailMatrix[f][1][nursei] - currentTime;
                        ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                        first = 1;
                    }
                    //NB NEW 17/02/2022: CALLING FVT FUNCTION WHICH RETURNS AN int* array FOR IF ARRIVAL IS WITHIN BREAK
                    double* timesArray;
                    timesArray = FindValidTime(ip, f, arriveAt, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWFVT, endTWFVT);

                    arriveAt = ip->unavailMatrix[f][2][nursei]; // update currentTime to be the end of the unavailable shift.
                    ip->timeMatrix[nursei][job] = arriveAt;
                    if(arriveAt > endTW){ // Only calculate tardiness if the job is actually late, i.e. nursei arrives at job after the time window
                        tardiness = arriveAt - endTW; //tardiness is how late the nurse is to the job, so how long after the end of the job TW does the nurse start the job.
                    }
                    else{
                        tardiness = 0;
                    }
                    ip->violatedTW[job] = tardiness; // Updated violated TW for 'job'
                    first = 1;
                }
            } //End for loop numUnavail
        }// End if(numUnavail > 0)

        // NB: update current time to be the time after the waiting time (if any) and after the duration of the job, so currentTime is after the job has finished.
        prevPoint = job; // previous job is now set to the current job
        //currentTime = currentTime + ip->jobTimeInfo[job][2] + waitingTime; //current time = current time + time length of 'job' + waiting time, i.e. the current time is the time now after the job has been completed.
        currentTime = arriveAt + ip->jobTimeInfo[job][2]; //current time = arriveAt (which includes waitingTime) + time length of 'job', i.e. the current time is the time now after the job has been completed.

    } //End of for loop (j = 0; j < ip->nJobs; ++j)

    // Return to depot:
    if(prevPoint > -1){ // If a previous job is set, calculate the time it takes for the nurse to go from the last job (prevPoint) back to the depot (their home).
        double tTime2 = TravelTimeToDepot(ip, nursei, prevPoint); //get_travel_time_to_depot function returns ip->nurseTravelToDepot[nursei][prevPoint].
        //ip->nurseTravelTime[nursei] += get_travel_time_to_depot(ip, nursei, prevPoint); //get_travel_time_to_depot function returns ip->nurseTravelToDepot[nursei][prevPoint].
        ip->nurseTravelTime[nursei] += tTime2; //get_travel_time_to_depot function returns ip->nurseTravelToDepot[nursei][prevPoint].
        //ip->nurseTravelMatrix[nursei][prevPoint] = tTime2;
    }

    // Update nurseWaitingTime for nursei to be the sum of all waiting times for nursei in nurseWaitingMatrix
    for(int j = 0; j < ip->nJobs; ++j){
        ip->nurseWaitingTime[nursei] += ip->nurseWaitingMatrix[nursei][j];
    }

    /* Let f be initial shift, i.e. f = 0, so startOfShift = ip->nurseWorkingTimes[nursei][0] = u(pi_0^i), and endOfShift = ip->unavailMatrix[0][1][nursei] = v(pi_0^i)
     * starting arrival time = 0
     * for each 'job' in nursei's route
     *      if 'job' = first job in nursei's route
     *      current arrival time = arrival time to previous job + waiting time at previous job + duration of previous job + travel time from previous job to 'job' (current job)
     *      if(max(current arrival time, startTW(e_j)) + duration of 'job' > endOfShift and this is not last shift for nursei
     *          run FVT, get array
     *          if array[0] = -1
     *              INFEASIBLE, exit
     *          else
     *              feasible solution found, set all times
     *      else if current arrival time > endOfShift and current arrival time < startofNEXTshift and this is not the last shift for nursei
     *          tempArrival = endOfShift
     *          run FVT, get array
     *          if array[0] = -1
     *              INFEASIBLE, exit
     *          else
     *              feasible solution found, set all times
     *      else
     *          waiting time = MaxNum(0, (startTW(e_j) - current arrival time))
     *          tardiness = MaxNum(0, (current arrival time - endTW(l_j)))
     *          startTime(t_j) = current arrival time + waiting time
     */

} //END OF SetNurseTime (new) function

void CalculateJobTimes(struct INSTANCE* ip, int nursei){

    int feasible = -1;

    //Sec: Reset all nursei's stored information:

    ip->nurseWaitingTime[nursei] = 0; // Reset nurseWaitingTime for nurse i (total waiting time for nursei)
    ip->nurseTravelTime[nursei] = 0; // Reset nurseTravelTime for nurse i (total travel time for nursei)

    for(int j = 0; j < ip->nJobs; ++j){
        ip->timeMatrix[nursei][j] = 0; // Reset timeMatrix for nursei (time at which nursei does job j - if nursei does not do job j then timeMatrix[nursei][j] = -1)
        ip->nurseWaitingMatrix[nursei][j] = 0; // Reset nurseWaitingMatrix for nursei (waiting time for nursei doing job j - if nursei does not do job j or there is no waiting time then nurseWaitingMatrix[nursei][j] = 0)
        ip->nurseTravelMatrix[nursei][j] = 0; // Reset nurseTravelMatrix for nursei (travel time for nursei going to job j - if nursei does not do job j or there is no travel time then nurseTravelMatrix[nursei][j] = 0)
    }

    // Number of breaks and shifts in nursei's day. Note: numUnavail = number of available shifts - 1; so numAvail = numUnavail + 1.
    int numUnavail = ip->nurseUnavail[nursei];
    int numShifts = numUnavail + 1; // Number of shifts in nursei's day
    int shift = 0;
    int endOfDay = -1;
    if(numShifts == 1){
        endOfDay = 1;
    }

    int prevJob = -1; //NB: NEED TO CHANGE/CHECK THIS
    //int serviceTime = 0; //NB: NEED TO CHANGE/CHECK THIS
    //double waitingTime = 0; //NB: NEED TO CHANGE/CHECK THIS
    //double travelTime = -1; // NB: NEED TO CHANGE/CHECK THIS
    double currentTime = (double)ip->nurseWorkingTimes[nursei][0]; //currentTime is the start of nursei's working day.

    // Sec: Main for loop of function:
    for(int p = 0; p < ip->nJobs; ++p){ //NOTE: p IS THE POSITION, not the job
        feasible = -1;

        if(ip->allNurseRoutes[nursei][p] < 0){ // If there is no job in position p of nursei's route, then we have reached the end of the route, no more jobs, break out of for loop
            break;
        }
        int job = ip->allNurseRoutes[nursei][p]; // job = the job number at position p in nursei's route.

        if(p == 0){ // If this is the first job in nursei's route, then we don't use the previous current time
            double travelTimeFirst = TravelTimeFromDepot(ip, nursei, job);
            if(ip->excludeNurseTravel){ //If we're not including the travel time to/from the first/last jobs in the nurses' routes
                if(ip->jobTimeInfo[job][0] > ip->nurseWorkingTimes[nursei][0] - 0.001){ //If the startTW (e_j) of job is later than time that nursei starts the day
                    currentTime = ip->jobTimeInfo[job][0]; // currentTime is set to be the startTW (e_j) of job
                    currentTime += ip->twInterval; // add the extra 15 minutes (or whatever twInterval is set to be) to make the currentTime the actual start time of the job
                }
                else{ // start time of nursei's day is later than the startTW (e_j) of job, should we check for this?
                    currentTime = ip->nurseWorkingTimes[nursei][0];
                }
            }
            else{// include travel time to/from the first/last jobs in the nurses' routes.
                currentTime += travelTimeFirst;
                ip->nurseTravelTime[nursei] += travelTimeFirst;
                ip->nurseTravelMatrix[nursei][job] = travelTimeFirst;
            }
        } // End if p == 0
        else if(p > 0){ // job is not the first job in nursei's route, so we need to update currentTime using the previous job's waiting time, service time, and add travelTime from previous job to this job
            double waitingTimePrev = ip->nurseWaitingMatrix[nursei][prevJob]; //NB CHECK THIS, waiting time at the previous job, prevJob, w_{j-1}
            int serviceTimePrev = ip->jobTimeInfo[prevJob][2]; // Duration of prevJob, s_{j-1}
            double travelTime = GetTravelTime(ip, prevJob, job); // Travel time from previous job to this job, T_i(j-1, j)
            //currentTime = prevJobArrival + waitingTime + serviceTime + travelTime;
            currentTime = currentTime + waitingTimePrev + serviceTimePrev + travelTime;
            ip->nurseTravelTime[nursei] += travelTime;
            ip->nurseTravelMatrix[nursei][job] = travelTime;
        } // End else if p > 0


        double ogArrivalTime = currentTime;
        double startTW = ip->jobTimeInfo[job][0]; // e_j
        double endTW = ip->jobTimeInfo[job][1]; // l_j
        double startTWMK = ip->jobTimeInfo[job][0]; // e_j^(j')
        double endTWMK = bigM; // l_j^(j')

        // Sec: Find out if job is a DS or a DJ.
        int gapAllowed = ip->algorithmOptions[12]; // if 1 then gapAllowed = true, else no gap allowed.
        int considerDependency = -1; // DJ = 1 if job is dependent and nursei is second nurse, = 0 otherwise
        int considerDoubleService = -1; // DS = 1 if job is double service and nursei is second nurse, = 0 otherwise
        int otherNurseDJ = -1; //if job is DJ (and therefore nursei is second nurse), this is the other nurse doing the dependent job, already set
        int otherNurseDS = -1; // if job is DS (and therefore nursei is second nurse), this is the other nurse doing the job at the same time, already set
        int otherJobDJ = -1; //if job is DJ (and therefore nursei is second nurse), this is the dependent job done by otherNurseDJ, already set
        int secondNurse = -1; // = 1 if job is DJ and DS and the other nurse has already been assessed, else = -1.
        int jobAfter = -1; // =1 if 'job' is DJ and time window of current 'job' is AFTER start time of otherJobDJ (which has already been set)

        // 1. Check if job is DJ (dependent job):
        if(ip->dependsOn[job] > -1){
            otherJobDJ = ip->dependsOn[job];
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    otherNurseDJ = -1; //NEW 17/02/2022
                    considerDependency = -1;
                    break;
                }
                if(ip->timeMatrix[prevNurse][otherJobDJ] > 0){
                    if(gapAllowed > 0){ // was if(aitOnly > 0)
                        ip->mkMinD[job] = abs(ip->mkMinD[job]);
                        ip->mkMaxD[job] = abs(ip->mkMaxD[job]);

                        // NB 17/02/2022: NEED TO CHECK THESE LINES FROM laa ip-?TM[prevNurse][otherjobDJ]... to otherNurseDJ = prevNurse
                        double laa = ip->timeMatrix[prevNurse][otherJobDJ] - ip->mkMaxD[job]; // NB 17/02/2022: HERE WE SUBTRACT ip->mkMaxD[job], IS IT THE RIGHT ONE? WHY SUBTRATCT, HOW DO WE KNOW WHICH JOB COMES FIRST?
                        //if((laa >= startTW) && (arriveAt <= laa)){ //NB: OLD - arrive at is used here!
                        if((laa >= startTW) && (currentTime <= laa)){ //NB: why is currentTime <= laa being checked here?
                            //ip->mkMinD[job] = -1*ip->mkMinD[job]; // NB 17/02/2022: ARE THESE CORRECT? CHANGED IN THE PAPER
                            //ip->mkMaxD[job] = -1*ip->mkMaxD[job];
                            ip->mkMinD[job] = -1*ip->mkMaxD[job]; // New, swapped to match paper
                            ip->mkMaxD[job] = -1*ip->mkMinD[job];
                            jobAfter = -1;

                        }
                        else{
                            jobAfter = 1; // 'job' is after otherJobDJ in the schedule.
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMinD[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMaxD[job];
                    considerDependency = 1; //NEW 17/02/2022
                    otherNurseDJ = prevNurse; // NB 17/02/2022: NEED TO CHECK THESE LINES FROM laa ip-?TM[prevNurse][otherjobDJ]... to otherNurseDJ = prevNurse
                    secondNurse = 1;
                    break;
                }
            }
        }// End dependent jobs

        //2. Check if job is DS (double service job):
        if(ip->doubleService[job] > 0){
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    otherNurseDS = -1; //NEW 17/02/2022
                    considerDoubleService = -1; //NEW 17/02/2022
                    break;
                }
                if(ip->timeMatrix[prevNurse][job] > 0){
                    startTWMK = ip->timeMatrix[prevNurse][job];
                    endTWMK = ip->timeMatrix[prevNurse][job];
                    considerDoubleService = 1; //NEW 17/02/2022
                    otherNurseDS = prevNurse; //NEW 17/02/2022
                    secondNurse = 1;
                    break;
                }
            }
        }// End double service jobs

        //Sec: Checks for DJ and DS jobs
        if(secondNurse > 0){
            if(considerDependency > 0){ // DJ = True
                if(currentTime > endTWMK && gapAllowed < 1){ //a(i,j) > t_{j'} + l_{j}^{(j')} and gapAllowed = False, current time later than latest allowed start time and no gaps allowed
                    feasible = -1;
                    break;
                    //exit(-1);
                }
                else if(currentTime > endTWMK && gapAllowed > 0){ //a(i,j) > t_{j'} + l_{j}^{(j')} and gapAllowed = True, current time later than latest allowed start time but gaps allowed
                    //if(endTWMK < ip->timeMatrix[otherNurseDJ][otherJobDJ]){ // t_{j'} + l_{j}^{(j')} < t_{j'}, gap too small, latest allowed start time is earlier than start time of other job
                    if(jobAfter < 0){ // t_{j'} + l_{j}^{(j')} < t_{j'}, gap too small, latest allowed start time is earlier than start time of other job, this 'job' occurs before otherJobDJ in the schedule.
                        feasible = -1;
                        break;
                        //exit(-1);
                    }
                }
            }//End DJ checks
            else if(considerDoubleService > 0){ // DS = True
                if(currentTime > ip->timeMatrix[otherNurseDS][job]){ // a(i,j) > t_{j'}
                    feasible = -1;
                    break;
                    //exit(-1);
                }
            }//End DS checks
        }// End checks


        // Sec: Main checks
        /* Note: *timesArray([0]) = feasible, *(timesArray+1)([1]) = currentTime, *(timesArray+2)([2]) = startTime, *(timesArray+3)([3]) = w_ij, *(timesArray+4)([4]) = z_j
         * *(timesArray+5)([5]) = \tilde{z}_j, *(timesArray+6)([6]) = shift f. */
        if(numShifts == 1 || shift >= numShifts-1){ //1. if nurse only has one shift (i.e. whole day, no breaks), or if this current shift is the nurse's final shift of the day
            feasible = 1;
            if(secondNurse > 0){
                if(considerDependency > 0){ // DJ
                    if((jobAfter > 0 && gapAllowed < 1) || (jobAfter < 0)){
                        double waitingTime = MaxNumDouble(0, (startTWMK - currentTime)); // startTWMK - cT or startTW - cT? (w_ij)
                        double startTime = currentTime + waitingTime; // t_j
                        double tardiness = MaxNumDouble(0, (currentTime - endTW)); // cT - endTWMK or cT - endTW? (z_j)
                        //no gap tardiness allowed, leave it
                        ip->timeMatrix[nursei][job] = startTime;
                        ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                        ip->nurseWaitingTime[nursei] += waitingTime;
                        ip->violatedTW[job] = tardiness;
                    }
                    else if(jobAfter > 0 && gapAllowed > 0){
                        double waitingTime = MaxNumDouble(0, (startTWMK - currentTime)); // startTWMK - cT or startTW - cT? (w_ij)
                        double startTime = currentTime + waitingTime; // t_j
                        double tardiness = MaxNumDouble(0, (currentTime - endTW)); // cT - endTWMK or cT - endTW? (z_j)
                        double gapTardiness = MaxNumDouble(0, (startTime - endTWMK)); // \tilde{z}_j
                        ip->timeMatrix[nursei][job] = startTime;
                        ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                        ip->nurseWaitingTime[nursei] += waitingTime;
                        ip->violatedTW[job] = tardiness;
                        ip->violatedTWMK[job] = gapTardiness;
                    }
                }
                else if(considerDoubleService > 0){ // DS
                    double waitingTime = MaxNumDouble(0, (ip->timeMatrix[otherNurseDS][job] - currentTime));
                    double startTime = ip->timeMatrix[otherNurseDS][job]; // same start time as other nurse already set doing this job
                    //don't set tardiness, it will override tardiness already set from otherNurseDS at 'job'.
                    //no gap tardiness for double services.
                    ip->timeMatrix[nursei][job] = startTime;
                    ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                    ip->nurseWaitingTime[nursei] += waitingTime;
                }
            }
            else{ //nursei is first nurse assessed or job is single/normal job
                double waitingTime = MaxNumDouble(0, (startTW - currentTime)); //w_ij
                double startTime = currentTime + waitingTime; // t_j
                double tardiness = MaxNumDouble(0, (currentTime - endTW)); // z_j
                ip->timeMatrix[nursei][job] = startTime;
                ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                ip->nurseWaitingTime[nursei] += waitingTime;
                ip->violatedTW[job] = tardiness;
            }
        }
        else if(currentTime > ip->unavailMatrix[shift][1][nursei] && currentTime < ip->unavailMatrix[shift][2][nursei]){ //2. a(i,j) > v(pi_f^i) and a(i,j) < u(pi_f+1^i), i.e. arrival is between breaks
            double tempCurrentTime = ip->unavailMatrix[shift][1][nursei]; // v \gets v(pi_f^i) in pseudocode
            double* timesArray;
            //timesArray = FindValidTime(ip, f, arriveAt, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWFVT, endTWFVT);
            timesArray = FindValidTime(ip, shift, tempCurrentTime, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWMK, endTWMK);
            if(*timesArray < 0){
                feasible = -1;
                break;
                //exit(-1);
            }
            else{
                feasible = 1;
                currentTime = *(timesArray+1); //check
                ip->timeMatrix[nursei][job] = *(timesArray+2);
                if(*(timesArray+3) >=0 ){ // check that waiting time returned from FVT is > 0
                    ip->nurseWaitingMatrix[nursei][job] = *(timesArray+3);
                    ip->nurseWaitingTime[nursei] += *(timesArray+3);
                }
                if(*(timesArray+4) >=0){
                    ip->violatedTW[job] = *(timesArray+4);
                }
                if(*(timesArray+5) >= 0){
                    ip->violatedTWMK[job] = *(timesArray+5);
                }
                shift = (int)*(timesArray+6);
            }
        }
        else if(MaxNumDouble(currentTime, startTW) + ip->jobTimeInfo[job][2] > ip->unavailMatrix[shift][1][nursei]){ //3. max(a(i,j), e_j) + s_j > v(pi_f^i), i.e. duration of job extended into break
            //FVT
            double* timesArray;
            //timesArray = FindValidTime(ip, f, arriveAt, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWFVT, endTWFVT);
            timesArray = FindValidTime(ip, shift, currentTime, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWMK, endTWMK);
            if(*timesArray < 0){
                feasible = -1;
                break;
                //exit(-1);
            }
            else{
                feasible = 1;
                currentTime = *(timesArray+1); // NB: CHECK THIS, IT WILL BE THE SAME AT THE START TIME T_J WHICH IS WRONG AS IT WILL MESS UP THE CALCULATION OF THE NEXT ARRIVAL TIME AT THE TOP OF THE LOOP.
                ip->timeMatrix[nursei][job] = *(timesArray+2);
                if(*(timesArray+3) >=0 ){ // check that waiting time returned from FVT is > 0
                    ip->nurseWaitingMatrix[nursei][job] = *(timesArray+3);
                    ip->nurseWaitingTime[nursei] += *(timesArray+3);
                }
                if(*(timesArray+4) >=0){
                    ip->violatedTW[job] = *(timesArray+4);
                }
                if(*(timesArray+5) >= 0){
                    ip->violatedTWMK[job] = *(timesArray+5);
                }
                shift = (int)*(timesArray+6);
            }
        }
        else{ //4. Other
            feasible = 1;
            if(secondNurse > 0){
                if(considerDependency > 0){ // DJ
                    if((jobAfter > 0 && gapAllowed < 1) || (jobAfter < 0)){
                        double waitingTime = MaxNumDouble(0, (startTWMK - currentTime)); // startTWMK - cT or startTW - cT? (w_ij)
                        double startTime = currentTime + waitingTime; // t_j
                        double tardiness = MaxNumDouble(0, (currentTime - endTW)); // cT - endTWMK or cT - endTW? (z_j)
                        //no gap tardiness allowed, leave it
                        ip->timeMatrix[nursei][job] = startTime;
                        ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                        ip->nurseWaitingTime[nursei] += waitingTime;
                        ip->violatedTW[job] = tardiness;
                    }
                    else if(jobAfter > 0 && gapAllowed > 0){
                        double waitingTime = MaxNumDouble(0, (startTWMK - currentTime)); // startTWMK - cT or startTW - cT? (w_ij)
                        double startTime = currentTime + waitingTime; // t_j
                        double tardiness = MaxNumDouble(0, (currentTime - endTW)); // cT - endTWMK or cT - endTW? (z_j)
                        double gapTardiness = MaxNumDouble(0, (startTime - endTWMK)); // \tilde{z}_j
                        ip->timeMatrix[nursei][job] = startTime;
                        ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                        ip->nurseWaitingTime[nursei] += waitingTime;
                        ip->violatedTW[job] = tardiness;
                        ip->violatedTWMK[job] = gapTardiness;
                    }
                }
                else if(considerDoubleService > 0){ // DS
                    double waitingTime = MaxNumDouble(0, (ip->timeMatrix[otherNurseDS][job] - currentTime));
                    double startTime = ip->timeMatrix[otherNurseDS][job]; // same start time as other nurse already set doing this job
                    //don't set tardiness, it will override tardiness already set from otherNurseDS at 'job'.
                    //no gap tardiness for double services.
                    ip->timeMatrix[nursei][job] = startTime;
                    ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                    ip->nurseWaitingTime[nursei] += waitingTime;
                }
            }
            else{ //nursei is first nurse assessed or job is single/normal job
                double waitingTime = MaxNumDouble(0, (startTW - currentTime)); //w_ij
                double startTime = currentTime + waitingTime; // t_j
                double tardiness = MaxNumDouble(0, (currentTime - endTW)); // z_j
                ip->timeMatrix[nursei][job] = startTime;
                ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                ip->nurseWaitingTime[nursei] += waitingTime;
                ip->violatedTW[job] = tardiness;
            }
        }// End Main checks

        //get prevjob for next loop
        prevJob = job;

    }// End for loop positions

    if(prevJob > -1){ //if there was a job before, i.e. nursei's route isn't empty
        if(ip->excludeNurseTravel == 0){ //If we are including the travel time back to the nurse's home from the last job
            double travelTimeLast = TravelTimeToDepot(ip, nursei, prevJob); //returns ip->nurseTravelToDepot[nursei][prevJob].
            ip->nurseTravelTime[nursei] += travelTimeLast;
        }
    }



}// End of CalculateJobTimes function.

/*void GetOtherDSDJ(struct INSTANCE* ip, int job, ){

    // Dependent jobs:
    if(ip->dependsOn[job] > -1){
        otherJobDJ = ip->dependsOn[job];
        for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
            int prevNurse = ip->nurseOrder[prevNurseInd];
            if(prevNurse == nursei){
                otherNurseDJ = -1; //NEW 17/02/2022
                considerDependency = -1;
                break;
            }
            if(ip->timeMatrix[prevNurse][otherJobDJ] > 0){
                if(aitOnly > 0){
                    ip->mkMinD[job] = abs(ip->mkMinD[job]);
                    ip->mkMaxD[job] = abs(ip->mkMaxD[job]);

                    // NB 17/02/2022: NEED TO CHECK THESE LINES FROM laa ip-?TM[prevNurse][otherjobDJ]... to otherNurseDJ = prevNurse
                    double laa = ip->timeMatrix[prevNurse][otherJobDJ] - ip->mkMaxD[job]; // NB 17/02/2022: HERE WE SUBTRACT ip->mkMaxD[job], IS IT THE RIGHT ONE? WHY SUBTRATCT, HOW DO WE KNOW WHICH JOB COMES FIRST?
                    if((laa >= startTW) && (arriveAt <= laa)){ //NB: arrive at is used here!
                        ip->mkMinD[job] = -1*ip->mkMinD[job]; // NB 17/02/2022: ARE THESE CORRECT? CHANGED IN THE PAPER
                        ip->mkMaxD[job] = -1*ip->mkMaxD[job];
                    }
                }
                startTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMinD[job];
                endTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMaxD[job];
                considerDependency = 1; //NEW 17/02/2022
                otherNurseDJ = prevNurse; // NB 17/02/2022: NEED TO CHECK THESE LINES FROM laa ip-?TM[prevNurse][otherjobDJ]... to otherNurseDJ = prevNurse
                break;
            }
        }
    }

    // Double service jobs:
    if(ip->doubleService[job] > 0){
        for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
            int prevNurse = ip->nurseOrder[prevNurseInd];
            if(prevNurse == nursei){
                otherNurseDS = -1; //NEW 17/02/2022
                considerDoubleService = -1; //NEW 17/02/2022
                break;
            }
            if(ip->timeMatrix[prevNurse][job] > 0){
                startTWMK = ip->timeMatrix[prevNurse][job];
                endTWMK = ip->timeMatrix[prevNurse][job];
                considerDoubleService = 1; //NEW 17/02/2022
                otherNurseDS = prevNurse; //NEW 17/02/2022
                break;
            }
        }
    }

}*/

double* FindValidTime(struct INSTANCE* ip, int f, double currentTime, int currentNurse, int job, int considerDependency, int otherNurseDJ, int otherJobDJ, int considerDoubleService, int otherNurseDS,
                      double startTWMK, double endTWMK){

    /** FIND VALID TIME CODE TO MATCH PAPER: 17/02/2022 **/
    // NB: CHECK THAT STARTTWMK AND ENDTWMK ARE CORRECT FROM SETNURSETIME!

    /* currentTime = arriveAt from SetNurseTime
     * currentNurse = nursei from SetNurseTime
     * job = job from SetNurseTime
     * considerDependency = from SetNurseTime, = -1 if the job is a DJ but currentNurse is the first nurse assessed, i.e. treat 'job' as though it is a single/independent job, else = 1 if the job is DJ
     * and currentNurse is the second nurse assessed, so otherNurseDJ is the nurse assigned to do the dependent job otherJobDJ, and the start time for otherJobDJ has already been set, i.e. currentNurse is secondNurse.
     * considerDoubleService = from SetNurseTime, = -1 if the job is a DS but currentNurse is the first nurse assessed, i.e. treat 'job' as though it is a single/independent job, else = 1 if the job is DS
     * and currentNurse is the second nurse assessed, so otherNurseDS is the other nurse assigned to do 'job', and so the start time for 'job' done by otherNurseDS has already been set, i.e. currentNurse is secondNurse.
     * int startTW and int endTW are the start and end TWs for the 'job'.
     */

    static double timesArray[7];
    // feasible (0,1), current time a(i,j), start time t_j, waiting time w_ij (if any), tardiness z_j (if any), gap tardiness \tilde{z}_j (if any for DJ if gap allowed, algorithmOptions_data[12]), and current shift f.
    timesArray[0] = -1; // feasible (0, 1)
    timesArray[1] = -1; // currentTime \tilde{a}(i,j)
    timesArray[2] = -1; // start time t_j
    timesArray[3] = -1; // waiting time w_ij (if any)
    timesArray[4] = -1; // tardiness z_j (if any)
    timesArray[5] = -1; // gap tardiness \tilde{z}_f (if any, only exists for dependent jobs DJ and gaps are allowed, i.e. algorithmsOptions[12] = 0)
    timesArray[6] = -1; // currrent shift f

    int shift = f;
    int secondNurse = 0;
    int gapTardinessAllowed = 0; // i.e. gap tardiness not set. = 1 if gaps allowed, = 0 otherwise.
    double feasible = 0.0;

    int numUnavail = ip->nurseUnavail[currentNurse];
    double originalArrivalTime = currentTime;


    if(otherNurseDJ > -1){ // 'job' is a DJ and the other job, otherJobsDJ, has already been set by otherNurseDJ; thus currentNurse is the secondNurse
        secondNurse = 1;
        gapTardinessAllowed = ip->algorithmOptions[12]; // = 1 if gap tardiness allowed, = 0 otherwise.
        if(currentTime > endTWMK && gapTardinessAllowed == 0){ // current time is later than latest allowed start time and no gap tardiness allowed, infeasible.
            // INFEASIBLE, exit.
            feasible = -1;
            timesArray[0] = feasible;
            return timesArray;
        }
    }
    else if(otherNurseDS > -1){ // 'job' is a DS, and the other nurse, otherNurseDS, assigned to 'job' has already been assessed, i.e. start time for 'job' has already been set; thus currentNurse is the secondNurse.
        secondNurse = 1;
        gapTardinessAllowed = 0; // gap tardiness doesn't count if DS.
        if(currentTime > startTWMK){ // current time is later than start time of 'job' already set by otherNurseDS, infeasible.
            // INFEASIBLE, exit.
            feasible = -1;
            timesArray[0] = feasible;
            return timesArray;
        }
    }

    int startOfShift = -1; //u(pi_f^i)
    int endOfShift = -1; // v(pi_f^i)
    int endOfDay = 0; // = 1 if endOfShift is the end of currentNurse's day, = 0 if NOT the end of currentNurse's day

    /** Main while loop **/
    while(feasible == 0 && f < numUnavail){ // NB check f < numUnavail
        //Move to start of next available shift using unavailMatrix
        startOfShift = ip->unavailMatrix[f][2][currentNurse]; // u(pi_{f}^{i}, i.e. start of shift
        f += 1;
        if(f == numUnavail){ // make the end of shift equal to the end of the nurse's day, as there is no row in unavailMatrix.
            endOfShift = ip->nurseWorkingTimes[currentNurse][1]; //end time of currentNurse's day
            endOfDay = 1;
        }
        else{
            endOfShift = ip->unavailMatrix[f][1][currentNurse]; // v(pi_{f}^{i}, i.e. end of shift, as f = f+1
        }
        currentTime = startOfShift;
        // CURRENT NURSE IS SECOND NURSE
        if(secondNurse == 1){ // otherNurseDJ/otherNurseDS has already been assessed, need to take into account time windows/starting time of otherJobDJ / 'job' done by otherNurseDS
            if(currentTime > endTWMK && gapTardinessAllowed == 0){ // if DJ: currentTime later than latest allowed start time and no gap tardiness allowed, if DS: currentTime later than start time of 'job' by otherNurseDS.
                // INFEASIBLE, exit.
                feasible = -1;
                timesArray[0] = feasible;
                return timesArray;
            }
            else if(currentTime > endTWMK && gapTardinessAllowed == 1){ // This condition only works for DJs, if job is DS program won't assess this condition.
                if(endTWMK > ip->timeMatrix[otherNurseDJ][otherJobDJ]){ // if this 'job' is LATER than otherJobDJ in the schedule, then gap tardiness can exists if needs be.
                    if(currentTime + ip->jobTimeInfo[job][2] > endOfShift && endOfDay == 0){ // cannot complete job without going into break period, and this shift is not the last shift
                        continue;
                    }
                    else{
                        // FEASIBLE, SOLUTION FOUND
                        feasible = 1; // feasible [0]
                        double startTime = currentTime; // t_j = \tilde{a}(i,j), start time [2]
                        double waitingTime = MaxNumDouble(0, (startTime - originalArrivalTime)); // w_ij = t_j - a(i,j), waiting time [3]
                        double tardiness = MaxNumDouble(0, (startTime - ip->jobTimeInfo[job][1])); // z_j = t_j - l_j, ip->jobTimeInfo[job][1] = the end TW of 'job', tardiness [4]
                        double gapTardiness = MaxNumDouble(0, (startTime - endTWMK)); // \tilde{z}_j = t_j - l_j^{j'}, gap tardiness [5]
                        int currentShift = f; //NB CHECK THIS, f = \tilde{f}, current shift [6], also int to double?
                        timesArray[0] = feasible; // feasible (0, 1)
                        //timesArray[1] = currentTime; // current time \tilde{a}(i,j)
                        timesArray[1] = originalArrivalTime; // current time \tilde{a}(i,j)
                        timesArray[2] = startTime; // start time t_j
                        timesArray[3] = waitingTime; // waiting time w_ij (if any)
                        timesArray[4] = tardiness; // tardiness z_j (if any)
                        timesArray[5] = gapTardiness; // gap tardiness \tilde{z}_f (if any, only exists for dependent jobs DJ and gaps are allowed, i.e. algorithmsOptions[12] = 0)
                        timesArray[6] = currentShift; // currrent shift f
                    }
                }
                else{ // latest start time of job is earlier in the schedule than otherJobDJ, and so the gap between the two jobs is too small.
                    // INFEASIBLE, exit.
                    feasible = -1;
                    timesArray[0] = feasible;
                    return timesArray;
                } // End if-else check of order of job and otherJobDJ in schedule
            }
            else if(MaxNumDouble(currentTime, startTWMK) + ip->jobTimeInfo[job][2] > endOfShift && endOfDay == 0){// cannot complete job without going into break period, and this shift is not the last shift.
                continue;
            }
            else{
                // FEASIBLE, SOLUTION FOUND
                feasible = 1; // feasible [0]
                double startTime = MaxNumDouble(currentTime, startTWMK); // t_j = \tilde{a}(i,j), start time [2]
                double waitingTime = MaxNumDouble(0, (startTime - originalArrivalTime)); // w_ij = t_j - a(i,j), waiting time [3]
                double tardiness = MaxNumDouble(0, (startTime - ip->jobTimeInfo[job][1])); // z_j = t_j - l_j, ip->jobTimeInfo[job][1] = the end TW of 'job', tardiness [4]
                // int gapTardiness = MaxNum(0, (startTime - endTWMK)); // \tilde{z}_j = t_j - l_j^{j'}, gap tardiness [5]
                int currentShift = f; //NB CHECK THIS, f = \tilde{f}, current shift [6], also int to double?
                timesArray[0] = feasible; // feasible (0, 1)
                //timesArray[1] = currentTime; // current time \tilde{a}(i,j)
                timesArray[1] = originalArrivalTime; // current time \tilde{a}(i,j)
                timesArray[2] = startTime; // start time t_j
                timesArray[3] = waitingTime; // waiting time w_ij (if any)
                timesArray[4] = tardiness; // tardiness z_j (if any)
                // timesArray[5] = gapTardiness; // gap tardiness \tilde{z}_f (if any, only exists for dependent jobs DJ and gaps are allowed, i.e. algorithmsOptions[12] = 0)
                timesArray[6] = currentShift; // currrent shift f
            }
        }
        // CURRENT NURSE IS FIRST NURSE OR JOB IS SINGLE/INDEPENDENT
        else if(secondNurse == 0){ // either job is a DJ/DS but currentNurse is the first job to be assessed, i.e. no otherNurseDJ/otherNurseDS exists, or job is a single/independent job.
            if(currentTime + ip->jobTimeInfo[job][2] > endOfShift && endOfDay == 0){ // cannot complete job without going into break period, and this shift is not the last shift.
                continue;
            }
            else{
                //FEASIBLE, solution found.
                feasible = 1; // feasible [0]
                double startTime = MaxNumDouble(currentTime, startTWMK); // t_j = \tilde{a}(i,j), start time [2]
                double waitingTime = MaxNumDouble(0, (startTime - originalArrivalTime)); // w_ij = t_j - a(i,j), waiting time [3]
                double tardiness = MaxNumDouble(0, (startTime - ip->jobTimeInfo[job][1])); // z_j = t_j - l_j, ip->jobTimeInfo[job][1] = the end TW of 'job', tardiness [4]
                // int gapTardiness = MaxNum(0, (startTime - endTWMK)); // \tilde{z}_j = t_j - l_j^{j'}, gap tardiness [5]
                int currentShift = f; //NB CHECK THIS, f = \tilde{f}, current shift [6], also int to double?
                timesArray[0] = feasible; // feasible (0, 1)
                //timesArray[1] = currentTime; // current time \tilde{a}(i,j)
                timesArray[1] = originalArrivalTime; // current time \tilde{a}(i,j)
                timesArray[2] = startTime; // start time t_j
                timesArray[3] = waitingTime; // waiting time w_ij (if any)
                timesArray[4] = tardiness; // tardiness z_j (if any)
                // timesArray[5] = gapTardiness; // gap tardiness \tilde{z}_f (if any, only exists for dependent jobs DJ and gaps are allowed, i.e. algorithmsOptions[12] = 0)
                timesArray[6] = currentShift; // currrent shift f
            }
        }

    }

    return timesArray;

    /* if ip->dependsOn[job] > -1 (i.e. if the job is dependent),
     *      int otherJob = ip->dependsOn[job];
     *      get otherNurse doing otherJob
     *      if otherNurse is earlier in nurseOrder than currentNurse, then we need to take into account the dependent TWs, currentNurse is the second nurse
     *          secondNurse = true
     *          create dependent TW
     *          if currentTime > latest allowed arrival time and gap == false,
     *              infeasible, exit, job cannot have tardiness
     *      else currentNurse is the first nurse being assessed, can continue as if it was a single job
     *
     * if ip->doubleService[job] > 0
     *      int otherNurse = the other nurse assigned to job
     *      if otherNurse is earlier in nurseOrder than currentNurse, then currentNurse is the second nurse, and we need to make sure it starts at the same time that otherNurse starts the job
     *          secondNurse = true
     *          if currentTime > start time of otherNurse at job
     *              infeasible, exit - jobs must start at the same time
     *      else currentNurse is the first nurse being assessed, can continue as if it was a single job
     *
     * while !feasible
     *      move to start of next available shift using unavailMatrix
     *      currentTime = start time of next available shift
     *      if secondNurse = True
     *          if currentTime > start time of otherNurse at job (DS) or currentTime > latest allowed arrival time and gap == false (DJ)
     *              infeasible, exit
     *          else if currentTime > latest allowed arrival time and gap == true (for DJ only)
     *              if latest allowed arrival time > start time of otherJob
     *                  if currentTime + service time of job > end of current shift, and current shift is not the last shift
     *                      continue
     *                  else
     *                      feasible = true, solution found
     *              else feasible = false, exit, infeasible
     *          else if max(currentTime, earliest start time of job/otherJob) + service time of job > end of current shift, and current shift is not the last shift
     *              continue
     *          else
     *              feasible = true, solution found
     *      else if secondNurse = false
     *          if currentTime + service time of job > end of current shift, and current shift is not the last shift
     *              continue
     *          else
     *              feasible = true, solution found
     */

} // End FindValidTime function

void SetTimesFull(struct INSTANCE* ip){
    for(int i = 0; i < ip->nJobs; ++i){
        ip->violatedTW[i] = 0;
        ip->violatedTWMK[i] = 0;
    }
    for(int j = 0; j < ip->nNurses; ++j){
        int nurse = ip->nurseOrder[j];
        // printf("Setting nurse time of: %d", nurse);
        //SetNurseTime(ip, nurse);
        CalculateJobTimes(ip, nurse); // new 24/02/2022, to replace SetNurseTime function.
    }
} // End SetTimesFull function

void SetTimesFrom(struct INSTANCE* ip, int firstNurse){

    int has_appeared = -1;
    int nurse = -1;
    int jobdue = -1;

    for(int i = 0; i < ip->nNurses; ++i){ //For each nurse i = 0,...,nNurses
        nurse = ip->nurseOrder[i]; //nurse = nurse i in nurseOrder array
        if(has_appeared < 0){ //If we have not yet assessed first_nurse
            if(nurse==firstNurse){ //if the current nurse we are assessing is also first_nurse (nurse ni)
                has_appeared = 1; //then first_nurse has appeared
            }
            else{ //move on to next nurse in nurseOrder
                continue;
            }
        }
        for(int j = 0; j < ip->nJobs; ++j){ //for all POSITIONS j=0,...,nJobs
            jobdue = ip->allNurseRoutes[nurse][j]; //jobdue = the job in position j of the nurse's route
            /** CHECK THIS IF STATEMENT! If j = 0 and jobdue < 0, then the nurse isn't being used, BUT if j > 0 and jobdue < 0, surely this just means that the nurse doesn't have a job in that position, but could still
             * have jobs before hand and is therefore still being used!**/
            if(jobdue < 0){ //If there is no job scheduled in position j of the nurses's route, then this means that nurse isn't being used.
                break; //exit for loop.
            }
            //if there is a job 'jobdue' scheduled in position j of nurse's route
            ip->violatedTW[jobdue] = 0; //"lateness" of jobdue is 0 (not late, set to start on time).
            ip->violatedTWMK[jobdue] = 0; //"lateness" of jobdue is 0 (not late, set to start on time).
        }

        //SetNurseTime(ip, nurse); //Note that this set_nurse_time function also checks to make sure that nurse is being used.
        CalculateJobTimes(ip, nurse); // new 24/02/2022, to replace SetNurseTime function.
    }
}

int SynchroniseJobi(struct INSTANCE* ip, int job, int nurse1, int nurse2){
    // Make nurse1 the early one:
    if(ip->timeMatrix[nurse1][job] > ip->timeMatrix[nurse2][job]){
        int temp = nurse1;
        nurse1 = nurse2;
        nurse2 = temp;
    }

    double timeDiff = ip->timeMatrix[nurse2][job] - ip->timeMatrix[nurse1][job];
    printf("Nurse %d arrives at %.2f, and nurse %d at %.2f to job %d, so nurse %d needs to wait for %.2f\n",
           nurse2, ip->timeMatrix[nurse2][job], nurse1, ip->timeMatrix[nurse1][job], job, nurse1, timeDiff);

    ip->nurseWaitingTime[nurse1] += timeDiff;
    GetNurseRoute(ip, nurse1, ip->nurseRoute);
    int afterThis = -1;
    for(int i = 0; i < ip->nJobs; ++i){
        int ji = ip->nurseRoute[i];
        if(ji < -0.1)
            break;

        if(ji==job)
            afterThis = 1;

        if(afterThis > 0){
            ip->timeMatrix[nurse1][ji] += timeDiff;
            printf("\tIncreased job %d in nurse %d by %.2f\n", ji, nurse1, timeDiff);
        }
    }
    printf("Now nurse %d arrives at %.2f\n", nurse1, ip->timeMatrix[nurse1][job]);
    return 0;
}

double SolutionQualityGlobal(struct INSTANCE* ip, int report){
    /*
		Recalculate the full solution quality.
		Should be avoided if the change only affects
		to 1 or 2 nurses. See SolutionQualityOptimised below
	*/
    // Set all nurse routes, as these get used multiple times in this function
    SetAllNurseRoutes(ip);
    SetTimesFull(ip);

    return -1;
}

double SolutionQualityLight(struct INSTANCE* ip){
    /*
		Same as SolutionQuality, but assumes that nurse routes
		are already correct, and are not recalculated
	*/
    double TOL = 0.0001;
    SetTimesFull(ip);
    // printf("\n\n-----------------------vvvvvvvvvvvvvvv-----------------------\n");

    // printf("\n\n----------------------- Before recalc -----------------------\n");
    // Objective(ip, 12);
    // print_int_matrix(ip->allNurseRoutes, ip->nNurses, ip->nJobs);

    //double lightQuality = Objective(ip, -1);
    double lightQuality = ObjectiveNew(ip, -1); // For testing, 05/11/2021
    // printf("\n\n-----------------------|||||||||||||||-----------------------\n");

    // double real_quality = SolutionQuality(ip, -1);
    // printf("\n\n----------------------- After recalc -----------------------\n");
    // Objective(ip, 21);
    // printf("\n\n-----------------------^^^^^^^^^^^^^^-----------------------\n");

    // print_int_matrix(ip->allNurseRoutes, ip->nNurses, ip->nJobs);
    // printf("Sol matrix:\n");
    // print_solmatrix(ip);

    // if (abs(real_quality - light_quality) > TOL)
    // {
    // 	printf("\nERROR: Sol quality and sol quality light have different values.\n");
    // 	printf("Real %.2f vs Light %.2f.\n", real_quality, light_quality);
    // 	exit(-4323452);
    // }
    return (lightQuality);
}

double SolutionQualityOptimised(struct INSTANCE* ip, int n1, int n2){
    printf("\nUSING DEPRECATED FUNCTION SolutionQualityOptimised, EXIT!!!!\n");
    exit(-22222222);
    /*
		Calculate the quality of the solution based only
		on a change in nurses n1 and n2
	*/
    SetNurseRoute(ip, n1);

    if((n2 > -1) && (n1!=n2)){
        SetNurseRoute(ip, n2);
        // n1 = min(n1, n2);
        if(n2 < n1)
            n1 = n2;
    }

    SetTimesFrom(ip, n1);

    //return Objective(ip, -1);
    return ObjectiveNew(ip, -1); //For testing, 05/11/2021
}

double SolutionQuality(struct INSTANCE* ip, int report){
    if(report > 0){
        printf("-------- SolutionQuality(ip, %d) --------\n\n", report);
    }

    // printf("\n\n********\nStarted SolutionQuality():\nSetting times...\n");

    if (report == -98765){
        printf("Initial allNurseRoutes:\n");
        PrintAllNurseRoutes(ip);
    }

    // printf("Solmatrix when starting SolutionQuality\n");
    // print_solmatrix(ip);

    // Set all nurse routes, as these get used multiple times in this function
    SetAllNurseRoutes(ip);
    if (report == -98765){
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
    double quality = ObjectiveNew(ip, report); // for testing, 05/11/2021

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
}

int SynchroniseJobs(struct INSTANCE* ip){
    // int retVal = -1;
    double TOL = 0.01;
    int MAX_CHECKS = 2*ip->nJobs;
    int checks = 0;
    for(int job = 1; job < ip->nJobs; ++job){
        if(ip->doubleService[job] < 0)
            continue;

        checks++;
        if(checks > MAX_CHECKS){
            printf("ERROR: A loop in SynchroniseJobs() went too far, exit!\n");
            break;
        }

        if(ip->verbose > 10)
            printf("Checking DS, job %d\n", job);
        double timeN1 = -1;
        double timeN2 = -1;
        int nurse1 = -1;
        int nurse2 = -1;
        for(int j = 0; j < ip->nNurses; ++j){
            if(ip->timeMatrix[j][job] > -0.5){
                if(timeN1 < -0.1){
                    timeN1 = ip->timeMatrix[j][job];
                    nurse1 = j;
                }
                else{
                    timeN2 = ip->timeMatrix[j][job];
                    nurse2 = j;
                    break;
                }
            }
        }
        if(timeN2 < 0 || timeN1 < 0){
            if(ip->verbose > 10)
                printf("\nERROR: Synchronised job %d does not have a correct nurse assignment!\n", job);
            continue;
        }
        if(abs(timeN1 - timeN2) > TOL){
            if(ip->verbose > 10)
                printf("One nurse arrives at %.2f and the other at %.2f, needs to synchronise!\n", timeN1, timeN2);
            SynchroniseJobi(ip, job, nurse1, nurse2);
            job = 0; // Start over checking!
        }
        else{
            if(ip->verbose > 10)
                printf("DS job %d OK, (%.2f and %.2f)\n", job, timeN1, timeN2);
            continue; // Done.
        }
    }

    return 0;
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
}

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
}

double Objective(struct INSTANCE* ip, int report){
    if(ip->verbose < 0){
        report = -1;
    }

    //report = -595959;

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
                double real_ds_tardiness = ip->violatedTW[job]/2;
                totalTardiness += real_ds_tardiness;
                if(real_ds_tardiness > maxTardiness){
                    maxTardiness = real_ds_tardiness;
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
        sparetime = max(0.0, -1*overtime);
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


    /*//Calculate the number of jobs that have actually been assigned.
    int* jobsAssigned = malloc(ip->nJobs * sizeof(int));
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

    //if(numJobsAssigned < ip->nJobs){
    if(numJobsAssigned != ip->nJobsIncDS){
        //printf("Not all jobs have been assigned. numJobsAssigned: %d, nJobs: %d \n", numJobsAssigned, ip->nJobs);
        printf("IncorrectNumber of jobs assigned. numJobsAssigned: %d, nJobsIncDS: %d \n", numJobsAssigned, ip->nJobsIncDS);
        *//*printf("jobsAssigned:\n");
        for(int i = 0; i < ip->nJobs; ++i){
            printf("%d ", jobsAssigned[i]);
        }
        printf("\n");*//*
    }
    else if(numJobsAssigned == ip->nJobsIncDS){
        printf("CORRECT number of jobs assigned. numJobsAssigned: %d, nJobsIncDS: %d \n", numJobsAssigned, ip->nJobsIncDS);
    }*/

    /*
		Mankowska objective: Z = A*D + B*T + C*Tmax
		A = B = C = 1/3 (User set weights)
		D = Total distance travelled by caregivers (OD-matrix)
		T = Total tardiness
		Tmax = Max tardiness
		double quality = (totalTime + totalTardiness + maxTardiness)/3;
	*/
    // double quality = -1*totalTravelTime -10000000000000*totalTardiness; // TSP-TW
    double TOL = 1e-4;
    double mk_allowed_tardiness = totalOvertime + totalTardiness;
    // double mk_allowed_tardiness = totalTardiness;
    double mk_max_tardiness = maxTardiness;
    if(maxOvertime > mk_max_tardiness){
        mk_max_tardiness = maxOvertime;
    }

    totalTime = totalWaitingTime + totalTravelTime + ip->totalServiceTime;

    double quality = 0.0;
    double ait_quality = 0.0;
    double mk_quality = 0.0;
    double wb_quality = 0.0;
    double paper_quality = 0.0;
    // int qualityType = ip->qualityMeasure;
    double infeasibility_M1 = ip->nNurses*12*60; // Multiplier
    double infeasibility_M2 = ip->nNurses*12*60; // Chunk sum

    //Ait H.
    ait_quality = -1*(0.3*totalTravelTime + ip->totalPref); // Real obj
    // Avoid infeasibility:
    ait_quality += -10000*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
        ait_quality += -10000;
    }

    // Mankowska
    mk_quality = -1*(totalTravelTime + mk_allowed_tardiness + mk_max_tardiness)/3; // Mankowska
    if(totalMKTardiness > TOL){
        mk_quality += -1000 - 10000*(totalMKTardiness);
    }

    // Workload Balance
    double dayDiff = (longestDay - shortestDay);
    wb_quality = -1*(0.3*totalTravelTime + ip->totalPref) - 0.1*dayDiff + 0.1*minSpareTime; // Real obj
    // Avoid infeasibility:
    wb_quality += -10000*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
        wb_quality += -10000;
    }

    // Paper
    paper_quality = ip->algorithmOptions[51]*totalTravelTime +
            ip->algorithmOptions[52]*totalWaitingTime +
            ip->algorithmOptions[53]*mk_allowed_tardiness +
            ip->algorithmOptions[54]*totalOvertime +
            ip->algorithmOptions[55]*minSpareTime +
            ip->algorithmOptions[56]*ip->totalPref +
            ip->algorithmOptions[57]*mk_max_tardiness;
    if(ip->algorithmOptions[50] > 0.5){
        // No tardiness of any type is allowed (including overtime)
        if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
            paper_quality += -infeasibility_M1*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
            paper_quality += -infeasibility_M2;
        }
    }
    else{
        // Tardiness and overtime are allowed, but infeasibleTardiness is not (double ups, gaps)
        if(totalMKTardiness > TOL){
            paper_quality += -infeasibility_M1*totalMKTardiness;
            paper_quality += -infeasibility_M2;
        }
    }

    if(ip->qualityMeasure == 0){
        quality = ait_quality;
    }
    else if(ip->qualityMeasure == 1){
        quality = mk_quality;
    }
    else if(ip->qualityMeasure == 5){
        quality = wb_quality;
    }
    else if(ip->qualityMeasure == 6){
        quality = paper_quality;
    }


    if(report==12345){
        double infeasibility_M1 = ip->nNurses*12*60; // Multiplier
        double infeasibility_M2 = ip->nNurses*12*60; // Chunk sum
        double dayDiff = longestDay - shortestDay;

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
        printf("---------------------------------\n");
        printf("ALG OPTIONS:\n");
        printf("algOp[50] = %.2f (if 1, tardiness/overtime are infeasible)\n", ip->algorithmOptions[50]);
        printf("alpha1 [51] (totalTravelTime) = %.2f\n", ip->algorithmOptions[51]);
        printf("alpha2 [52] (totalWaitingTime) = %.2f\n", ip->algorithmOptions[52]);
        printf("alpha3 [53] (mk_allowed_tardiness) = %.2f\n", ip->algorithmOptions[53]);
        printf("alpha4 [54] (totalOvertime) = %.2f\n", ip->algorithmOptions[54]);
        printf("alpha5 [55] (minSpareTime) = %.2f\n", ip->algorithmOptions[55]);
        printf("alpha6 [56] (totalPref) = %.2f\n", ip->algorithmOptions[56]);
        printf("alpha7 [57] (mk_max_tardiness) = %.2f\n", ip->algorithmOptions[57]);

        printf("\n-------------------\n");
        printf("\nOUTPUT VARIABLES:\n");
        printf("Total time (totalTime): %.2f\n", totalTime);
        printf("Total Travel Time (totalTravelTime) = %.2f\n", totalTravelTime);
        printf("Total Waiting Time (totalWaitingTime): %.2f\n", totalWaitingTime);
        printf("Total Service Time (totalServiceTime): %.2f\n", ip->totalServiceTime);
        printf("Total Overtime (totalOvertime) = %.2f\n", totalOvertime);
        printf("Total Violated TW (totalTardiness) = %.2f\n", totalTardiness);
        printf("Total Violated TWMK (totalMKTardiness) = %.2f\n", totalMKTardiness);
        printf("Total Tardiness MK (mk_allowed_tardiness) = %.2f (totalOvertime + totalTardiness)\n", mk_allowed_tardiness);
        printf("Max Spare Time (maxSpareTime): %.2f\n", maxSpareTime);
        printf("Min Spare Time (minSpareTime): %.2f\n", minSpareTime);
        printf("Shortest Shift (shortestDay): %.2f\n", shortestDay);
        printf("Longest Shift (longestDay): %.2f\n", longestDay);
        printf("Shift Difference (dayDiff): %.2f\n", dayDiff);
        printf("Total Preference Score (ip->totalPref) = %.2f\n", ip->totalPref);
        printf("Quality = %.2f\n", quality);

        printf("\n------------------------------\n");
        printf("ALL QUALITIES:\n");
        printf("Ait H: %.2f\n", ait_quality);
        printf("Mankowska: %.2f\n", mk_quality);
        printf("Workload Balance: %.2f\n", wb_quality);
        printf("Paper: %.2f\n", paper_quality);
        //exit(-1);

    }

    // if (totalTardiness > 0)
    // 	ip->isFeasible = 0;
    // else
    // 	ip->isFeasible = 1;
    ip->isFeasible = feasible;


    // Save the details:
    // NB: updated, added new variables: 04/06/2021
    ip->objTime = totalTime;
    ip->objWaiting = totalWaitingTime;
    ip->objTravel = totalTravelTime;
    ip->objService = ip->totalServiceTime;
    ip->objTardiness = totalTardiness;
    ip->objMaxTardiness = maxTardiness;
    ip->objMKTardiness = totalMKTardiness;
    ip->objMKAllowedTardiness = mk_allowed_tardiness;
    ip->objOvertime = totalOvertime;
    ip->objMaxOvertime = maxOvertime;
    ip->objMinSpareTime = minSpareTime;
    ip->objMaxSpareTime = maxSpareTime;
    ip->objShortestDay = shortestDay;
    ip->objLongestDay = longestDay;
    ip->objAitHQuality = ait_quality;
    ip->objMKQuality = mk_quality;
    ip->objWBQuality = wb_quality;
    ip->objPaperQuality = paper_quality;
    ip->objQuality = quality;
    return quality;

} // End of Objective function.

double ObjectiveNew(struct INSTANCE* ip, int report){
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
        sparetime = max(0.0, -1*overtime);
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

    int* jobsAssigned = malloc(ip->nJobs * sizeof(int));
    int* nurseServiceTime = malloc(ip->nNurses * sizeof(int));
    double* nurseWorkload = malloc(ip->nNurses * sizeof(double));
    double* diffWorkloads = malloc(ip->nNurses * sizeof(double));

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
    free(jobsAssigned);
    free(nurseServiceTime);
    free(nurseWorkload);
    free(diffWorkloads);


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
    ip->objAitHQuality = aitQuality;
    ip->objMKQuality = mkQuality;
    ip->objWBQuality = wbQuality;
    ip->objPaperQuality = paperQuality;
    ip->objQuality = quality;
    ip->objMaxTravelTime = maxTravelTime;
    ip->objMaxWaitingTime = maxWaitingTime;
    ip->objMaxDiffWorkload = maxDiffWorkload;
    return quality;

} // End of ObjectiveNew function.

double AlternativeQuality(struct INSTANCE* ip, int report){
    double totalTardiness = 0;
    double totalTime = 0;
    // int * nurseRoute = ip->nurseRoute; // malloc(ip->nJobs * sizeof(int));
    double onlyTravelTime = 0;
    double arriveAt, leaveAt;
    double tTime = -1;
    double finishShiftAt = -1;
    for(int j = 0; j < ip->nNurses; ++j){
        int prevPoint = -1;
        double currentTime = (double) ip->nurseWorkingTimes[j][0];
        if(report > 0 && ip->verbose > 1)
            printf("Nurse %d starts at %.2f\n", j, currentTime);

        GetNurseRoute(ip, j, ip->nurseRoute);

        for(int i = 0; i < ip->nJobs; ++i){
            if(ip->nurseRoute[i] < 0)
                break;
            int job = ip->nurseRoute[i];
            // Trip from depot:
            if(prevPoint > -0.5)
                tTime = GetTravelTime(ip, prevPoint, job);
            else
                tTime = TravelTimeFromDepot(ip, j, job);

            onlyTravelTime += tTime;
            currentTime = currentTime + tTime;//ip->od[prevPoint][job];

            arriveAt = currentTime;
            double waitingTime = 0;
            if(arriveAt < ip->jobTimeInfo[job][0])
                waitingTime = ip->jobTimeInfo[job][0] - arriveAt;

            double tardiness = 0;
            if(arriveAt > ip->jobTimeInfo[job][1])
                tardiness = arriveAt - ip->jobTimeInfo[job][1];
            ip->violatedTW[job] = tardiness;

            prevPoint = job;
            currentTime = currentTime + ip->jobTimeInfo[job][2] + waitingTime;
            leaveAt = currentTime;

            totalTardiness += tardiness;
            // Add penalty for potential lateness and breaching of normal working hours
            if(report > 0 && ip->verbose > 1){
                printf("\tArrives at job %d at %.2f and leaves at %.2f\n", job, arriveAt, leaveAt);
                if(waitingTime > 0)
                    printf("\t\tNeeds to wait for %.2f before starting the job\n", waitingTime);
                if(tardiness > 0)
                    printf("\t\t*** Misses the time window by %.2f! ***\n", tardiness);
            }
        }
        if(ip->nurseRoute[0] > -1){
            totalTime += (leaveAt - ip->nurseWorkingTimes[j][0]);

            // Return to depot:
            tTime = TravelTimeToDepot(ip, j, prevPoint);
            // tTime = get_travel_time(ip, prevPoint, -1);
            onlyTravelTime += tTime;
            totalTime += tTime;
            finishShiftAt = leaveAt + tTime;
        }
        else
            finishShiftAt = ip->nurseWorkingTimes[j][0];

        if(report > 0 && ip->verbose > 1){
            printf("\tFinishes at the depot at %.2f.\n", finishShiftAt);
            if(finishShiftAt > ip->nurseWorkingTimes[j][1]){
                printf("\t\t*** This nurse is finishing late (by %.2f)\n", finishShiftAt - (double) ip->nurseWorkingTimes[j][1]);
            }
            // else
            // {
            // 	printf("\t\t Finishing before %.2f (end of shift)\n", (double)ip->nurseWorkingTimes[j][1]);
            // }

        }

    }
    if(report > 0 && ip->verbose > 1){
        printf("\nTotal travel time: %.2f", onlyTravelTime);
        printf("\n");
    }
    // free(nurseRoute);
    double quality = -100000*totalTardiness - totalTime;
    // int MAX_ALLOWED_WORKING = 100000;
    // int quality = ip->nNurses*MAX_ALLOWED_WORKING - totalTime;
    if(totalTardiness > 0)
        ip->isFeasible = 0;
    else
        ip->isFeasible = 1;

    return quality;

}

int RandomInteger(int min_val, int max_val){
    //return random value between min_val and max_val
    return min_val + (rand()%(max_val + 1));
}

void TwoExchange(int* array, int i, int j){ //In GRASP: two_exchange(ip->nurseOrder, exch1, exch2), and RandomTwoExchange: TwoExchange(array, (*i), (*j))

    //This function swaps the elements in array[i] and array[j] (element that was in array[i] is now in array [j] and vice vera)

    int t = array[j];
    array[j] = array[i];
    array[i] = t;

} //END OF TwoExchange.

void RandomTwoExchange(int* array, size_t n, int* i, int* j){ //In GRASP: RandomTwoExchange(ip->nurseOrder, ip->nNurses, &exch1, &exch2)

    if(n < 2){ //If nNurses < 2, then &exch1 and &exch2 = 0, and exit.
        i = 0;
        j = 0;
        return;
    }

    //Otherwise, if nNurses >=2, then use RandomInteger function to set *i and *j to random ints between 0 and n-1
    (*i) = RandomInteger(0, n - 1);
    (*j) = RandomInteger(0, n - 1);

    //If the random_integer function coincidentally set *i and *j to be the same, use RandomInteger to choose another value for *j until *i and *j are different.
    while(*i==*j)
        (*j) = RandomInteger(0, n - 1);

    // Then, perform TwoExchange - swap the elements in array[i] and array[j].
    TwoExchange(array, (*i), (*j));
}

void Shuffle(int* array, size_t n){ //In GRASP: Shuffle(ip->nurseOrder, (size_t)ip->nNurses)

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

void PrintVector(int* array, size_t n){
    printf("\n[%d", array[0]);
    for(int i = 1; i < n; ++i)
        printf(", %d", array[i]);
    printf("]\n");
}

double MaxNumDouble(double num1, double num2){
    return (num1 > num2) ? num1 : num2;
}

int MaxNumInt(int num1, int num2){
    return (num1 > num2) ? num1 : num2;
}
