/*--------------/
GRASP_VNS
constructive.c
UoS
10/07/2021
/--------------*/

#include <stdlib.h>
#include "Python.h"
#include "constructive.h"
#include "grasp.h"



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


    double eps = 1e-6;
    int printInputData = (int)algorithmOptions_data[99] + eps;

    int qualityMeasureModAPI = (int)(algorithmOptions_data[0] + eps);
    int doGapNotPrecedence = (int)(algorithmOptions_data[12] + eps);

    if(qualityMeasureModAPI == 0 && doGapNotPrecedence == 0){
        printf("WARNING: Solving with AIT H quality measure but with PRECEDENCE rather than GAP.\n");
    }


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


    struct INSTANCE inst = InstanceFromPython(nJobs_data, nNurses_data, nSkills_data, verbose_data, MAX_TIME_SECONDS, twInterval_data, excludeNurseTravel_data, od_data, nurseTravelFromDepot_data,
                                              nurseTravelToDepot_data, unavailMatrix_data, nurseUnavail_data, nurseWorkingTimes_data, jobTimeInfo_data, jobRequirements_data, nurseSkills_data,
                                              doubleService_data, dependsOn_data, mkMinD_data, mkMaxD_data, capabilityOfDoubleServices_data, prefScore_data, algorithmOptions_data);


    if(printInputData > 0){
        printf("nurseSkilled = \n");
        PrintIntMatrix(inst.nurseSkilled, nNurses_data, nJobs_data);
        printf("\n---------------  end of input data in C  ---------------\n\n");
    }


    double MMM = 10000;
    double minPref = 0.0;
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
        minPref += best_a;
    }
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
        minPref += best_a;
    }
    printf("THE MIN PREF OF THIS INSTANCE IS:\n");
    printf("<-< minPref = %.2f >->\n", minPref);

    struct INSTANCE* ip = &inst;
    if(randomSeed >= 0){
        srand(randomSeed);
    }
    else{
        srand(time(NULL));
    }

    int thenum = rand();

    int retvalue = 0;
    double int_tol = 0.0001;
    if(ip->algorithmOptions[98] >= -int_tol && ip->algorithmOptions[98] <= int_tol){
        retvalue = MainWithOutput(ip, od_data, solMatrixPointer, timeMatrixPointer, nurseWaitingTimePointer, nurseTravelTimePointer, violatedTWPointer, nurseWaitingMatrixPointer, nurseTravelMatrixPointer, totalsArrayPointer);
    }
    else if(ip->algorithmOptions[98] >= 1 - int_tol && ip->algorithmOptions[98] <= 1 + int_tol){
        evaluate_given_solution(ip, solMatrixPointer, od_data);
    }
    else if(ip->algorithmOptions[98] >= 2 - int_tol && ip->algorithmOptions[98] <= 2 + int_tol){
        printf("Solving with option '%d' is not implemented.", (int) ip->algorithmOptions[98]);
        retvalue = -1;
    }
    else{
        printf("Solving with option '%d' is not implemented.", (int) ip->algorithmOptions[98]);
        retvalue = -1;
    }


    if(qualityMeasureModAPI==0 && doGapNotPrecedence==0){
        printf("[WARNING]: Solving with AIT H quality measure but with PRECEDENCE rather than GAP.\n");
    }

    return retvalue;

}

int evaluate_given_solution(struct INSTANCE* ip, int* solMatrixPointer, double* odmat_pointer){

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

    GRASP(ip);

    double finalQuality = SolutionQuality(ip, -1);
    if(ip->verbose > 0){
        printf("Final solution quality is: %.2f\n", finalQuality);
    }

    if(odmat_pointer!=NULL){
        odmat_pointer[0] = finalQuality;
    }


    if(ip->verbose > 10)
        printf("Finishing stuff...\n");

    SolnToPythomFormat(ip, solMatrixPointer, timeMatrixPointer, nurseWaitingTimePointer, nurseTravelTimePointer, violatedTWPointer, nurseWaitingMatrixPointer, nurseTravelMatrixPointer,
                       totalsArrayPointer);

    if(ip->verbose > 5)
        printf("End of program.\nFreeing memory...\n");
    FreeInstanceMemory(ip);
    if(ip->verbose > 5)
        printf("Done.");

    return 0;
}



void SolnToPythomFormat(struct INSTANCE* ip, int* solMatrixPointer, double* timeMatrixPointer, double* nurseWaitingTimePointer, double* nurseTravelTimePointer, double* violatedTWPointer,
                        double* nurseWaitingMatrixPointer, double* nurseTravelMatrixPointer, double* totalsArrayPointer){
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

    SetNurseRoute(ip, ni);
    SetTimesFrom(ip, ni);

    return 0;
}

int BestJobInsertion(struct INSTANCE* ip, int job, int ni){
    // This function starts at position 0 of nurse ni's route, and trials adding 'job' into each position 0,...,nJobs + 10, calculating the quality of each solution created using the job in the new position.
    // Note that a job is inserted into a position, the quality is calculated, and then the job is removed from that position (i.e. the solution is returned to the original) before the function attempts to
    // insert the job into the next position.
    // Therefore, this function repeatedly uses the function 'InsertJobAtPosition' to test out every position.
    // The position that 'job' is inserted into in nurse ni's route that produces the best quality solution (bestQuality) is stored (bestPosition), and this position is then used to produce a solution,
    // InsertJobAtPosition(ip, job, ni, bestPosition).
    // This function returns 0 if 'job' has been inserted successfully into nurse ni's route, otherwise it returns -1 (job cannot be inserted into any position in nurse ni's route).

    int bestPosition = -1*bigM;
    double bestQuality = -1*bigM;
    int firstSwitch = 0;

    if(InsertJobAtPosition(ip, job, ni, 0) < 0){
        if(ip->solMatrix[ni][job] < 0){
            printf("ERROR: Job %d cannot be inserted into nurse %d in position 0, but job is NOT in nurse's route!\n", job, ni);
        }
        return -1;
    }
    else{ // Job has been inserted into nurse ni's route in position 0

        bestPosition = 0;
        bestQuality = SolutionQualityLight(ip); // Solution quality of ip with new job insertion
        RemoveJob(ip, job, ni); // Reverse job insertion, go back to original solution

    }

    double propQuality = -1;
    for(int j = 1; j < ip->nJobs; ++j){
        if(InsertJobAtPosition(ip, job, ni, j) < 0){
            break;
        }
        else{
            propQuality = SolutionQualityLight(ip); // Job has been inserted into nurse ni's route into position j, so calculate quality of this new solution
            if(propQuality > bestQuality){ // If this solution has better quality than current best quality solution
                bestPosition = j; // The best position to insert job is updated to position j
                bestQuality = propQuality; // The quality of solution with best position of inserted job is updated.
                if(firstSwitch > 0){
                    return 0;
                }
            }
            RemoveJob(ip, job, ni); //Reverse job insertion, go back to original solution.
        }
    }

    //We have found the best position in nurse ni's route to put job, so now we insert job into that best position.
    if(bestPosition >= 0){
        InsertJobAtPosition(ip, job, ni, bestPosition);
        return 0;
    }
    else{
        printf("ERROR: bestPosition < 0, code should not reach here!");
        return -1;
    }

}

int InsertJobAtPosition(struct INSTANCE* ip, int job, int ni, int posi){
    // This function attempts to push forward all of the positions of jobs in nurse ni's route and add a new job in a particular position, 'posi' in nurse ni's route.
    // This function returns retVal which is either 0 (job has been inserted into position 'posi' successfully) or -1 (job cannot be inserted into position 'posi').

    if(ip->solMatrix[ni][job] > -1){ //The job is already assigned to that nurse.
        return -1;
    }

    if(CheckSkills(ip, job, ni) < 1 && ip->doubleService[job] < 0.5){
        printf(" <> Unskilled <>\n");
        return -1;
    }

    int retVal = -1;
    int avJobs = 0;

    for(int i = 0; i < ip->nJobs; ++i){
        if(ip->solMatrix[ni][i] > -1){
            avJobs++;
        }
        else{
            continue;
        }

        if(ip->solMatrix[ni][i] >= posi){
            retVal = 0;
            ip->solMatrix[ni][i]++;
        }
    }

    if((retVal==0) || (avJobs==posi)){
        ip->solMatrix[ni][job] = posi;
        SetNurseRoute(ip, ni);
        SetTimesFrom(ip, ni);
        return 0;
    }
    SetNurseRoute(ip, ni);

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

    int firstSwitch = 1;
    double TOL = 0.0001;
    int nRows = ip->nNurses;
    int nCols = ip->nJobs;

    double baseQuality = SolutionQuality(ip, DEBUG_PRINT - 30);
    double overAllBestFitness = -1*bigM;
    int overalBestJob = -1;
    int overall_bestA = -1;
    int overall_bestB = -1;
    int overall_bestAPos = -1;
    int overall_bestANurse = -1;
    int overall_bestBPos = -1;
    int overall_bestBNurse = -1;
    int getOut = 0;

    for(int i = 0; i < ip->nJobs; ++i){

        int considerJob = -1;
        int nurseA = -1;
        int nurseB = -1;
        int jobA = i;
        int jobB = i;
        int posJobA = -1;
        int posJobB = -1;
        if(ip->doubleService[i] > 0){
            considerJob = 1;
            for(int nu = 0; nu < ip->nNurses; ++nu){
                if(ip->solMatrix[nu][i] > -1){
                    if(nurseA < 0){
                        nurseA = nu;
                        posJobA = ip->solMatrix[nu][i];
                    }
                    else{
                        nurseB = nu;
                        posJobB = ip->solMatrix[nu][i];
                        break;
                    }
                }
            }
        }
        else if(ip->dependsOn[i] >= i){
            jobB = ip->dependsOn[i];
            considerJob = 1;
            for(int nu = 0; nu < ip->nNurses; ++nu){
                if(ip->solMatrix[nu][jobA] > -1){
                    nurseA = nu;
                    posJobA = ip->solMatrix[nu][jobA];
                }
                if(ip->solMatrix[nu][jobB] > -1){
                    nurseB = nu;
                    posJobB = ip->solMatrix[nu][jobB];
                }
            }
        }
        else{
            considerJob = -1;
        }
        if(considerJob < 1){
            continue;
        }

        if((nurseA < 0) || (nurseB < 0)){
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

        RemoveJob(ip, jobA, nurseA);

        if(DEBUG_PRINT > 0){
            printf("About to remove %d from nurse %d\n", jobB, nurseB);
            printf("Currently at position (posJobB) %d\t", posJobB);
            printf("Currently at position %d\n", ip->solMatrix[nurseB][jobB]);
        }

        RemoveJob(ip, jobB, nurseB);

        int jobABestNurse = -1;
        int jobBBestNurse = -1;
        int jobABestPos = -1;
        int jobBBestPos = -1;
        int allPositionsCovered = 0;
        double bestJobPosQuality = baseQuality;

        if(allPositionsCovered > 0){
            continue;
        }

        for(int tn = 0; tn < ip->nNurses; ++tn){
            if(DEBUG_PRINT > 0){
                printf("Testing job A (%d) in nurse %d\n", jobA, tn);
            }
            if(ip->doubleService[jobA]){
                if(CheckSkillsDSFirst(ip, jobA, tn)
                        < 1){
                    if(DEBUG_PRINT > 0){
                        printf("\t(!) Not possible, does not cover first part of DS.\n");
                    }
                    continue;
                }
            }
            else{
                if(CheckSkills(ip, jobA, tn) < 1){
                    if(DEBUG_PRINT > 0){
                        printf("\t(!) Not possible, unskilled.\n");
                    }
                    continue;
                }
            }
            for(int tpos = 0; tpos < ip->nJobs; ++tpos){
                if(DEBUG_PRINT > 0){
                    printf("\t(-) Testing position %d ", tpos);
                }
                if((tn==nurseA) && (tpos==posJobA)){
                    if(DEBUG_PRINT > 0){
                        printf("- skipping, it was already here before.\n");
                    }
                    continue;
                }

                // Insert jobA:
                int insertAValue = InsertJobAtPosition(ip, jobA, tn, tpos);
                if(insertAValue < 0){
                    if(DEBUG_PRINT > 0){
                        printf("- insertion failed (index too high?)\n");
                    }
                    allPositionsCovered = 1;
                    break;
                }
                else if(DEBUG_PRINT > 0){
                    printf("- part A inserted in Nurse %d at position %d (Ret %d)\n", tn, tpos, insertAValue);
                }

                int finalTNB = -1;
                int finalPosB = -1;
                int bPossible = 0;
                double bestTargetQuality = -1*bigM;
                double tQuality = bestTargetQuality;
                if(DEBUG_PRINT > 0){
                    printf("\tStart checks for Position B\n");
                }
                for(int tnb = 0; tnb < ip->nNurses; ++tnb){
                    if(DEBUG_PRINT > 0){
                        printf("\tTesting job B (%d) in nurse %d\n", jobB, tnb);
                    }
                    if(ip->doubleService[jobA] && CheckSkillsDS(ip, jobB, tn, tnb) < 1){
                        if(DEBUG_PRINT > 0){
                            printf("\t\t(!) Not possible, unskilled to do second part of DS.\n");
                        }
                        continue;
                    }
                    else if((!ip->doubleService[jobA]) && CheckSkills(ip, jobB, tnb) < 1){
                        if(DEBUG_PRINT > 0){
                            printf("\t\t(!) Not possible, unskilled.\n");
                        }
                        continue;
                    }
                    if(DEBUG_PRINT > 0){
                        printf("\t\t(-) Possible, calling BestJobInsertion\n");
                    }

                    int insertValue = BestJobInsertion(ip, jobB, tnb);
                    if(insertValue > -1){
                        bPossible = 1;

                        tQuality = SolutionQuality(ip, -5);
                        if(tQuality > bestTargetQuality){
                            bestTargetQuality = tQuality;
                            finalTNB = tnb;
                            finalPosB = ip->solMatrix[tnb][jobB];
                        }
                        if(DEBUG_PRINT > 0){
                            printf("\t>>> Inserted job B in nurse %d (Ins val %d, quality %.2f) <<<\n", tnb, insertValue, tQuality);
                        }
                        RemoveJob(ip, jobB, tnb);
                    }
                    else if(DEBUG_PRINT > 0){
                        printf("\tInsert not possible (%d)\n", insertValue);
                        printf("\tjobB = %d\n", jobB);
                        printf("\ttnb = %d\n", tnb);
                        printf("\tip->solMatrix[tnb][jobB] = %d\n", ip->solMatrix[tnb][jobB]);
                    }
                }

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

                if(bestTargetQuality > bestJobPosQuality){
                    bestJobPosQuality = bestTargetQuality;
                    jobABestNurse = tn;
                    jobBBestNurse = finalTNB;
                    jobABestPos = tpos;
                    jobBBestPos = finalPosB;
                    if(DEBUG_PRINT > 0){
                        printf("\t\t * best target quality %.2f*\n", bestJobPosQuality);
                    }
                    if(firstSwitch){
                        getOut = 100;
                    }
                }

                if(DEBUG_PRINT > 0){
                    printf("\t >> Finished testing job A, nurse %d, position %d <<\n", tn, tpos);
                }
                RemoveJob(ip, jobA, tn);
                if(getOut > 0){
                    break;
                }
            }
            if(getOut > 0){
                break;
            }
        }

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
    }

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

        for(int nn = 0; nn < ip->nNurses; ++nn){
            if(ip->solMatrix[nn][overall_bestA] > -1){
                RemoveJob(ip, overall_bestA, nn);
            }
            if(ip->solMatrix[nn][overall_bestB] > -1){
                RemoveJob(ip, overall_bestB, nn);
            }
        }

        if(InsertJobAtPosition(ip, overall_bestA, overall_bestANurse, overall_bestAPos) < 0){
            printf("ERROR: Cannot insert the move found on local search!!!\n");
            exit(-34234523);
        }
        if(InsertJobAtPosition(ip, overall_bestB, overall_bestBNurse, overall_bestBPos) < 0){
            printf("ERROR: Cannot insert the move found on local search!!!\n");
            exit(-34234523);
        }
        if(DEBUG_PRINT > 0){
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
            double dcheckQual = SolutionQuality(ip, DEBUG_PRINT - 30);
            printf(">    Not doing best_double_switch, started with %.2f and best was only %.2f    <\n", baseQuality, overAllBestFitness);
        }
        if(DEBUG_PRINT > 0){
            printf("--- -1 FINISHED BestSyncDoubleSwitch ---");
        }
        return -1;
    }

}

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
    double iniQuality = SolutionQuality(ip, -12121);
    double currentQuality = iniQuality;

    for(int ni = 0; ni < ip->nNurses - 1; ++ni){
        for(int job_from_ni = 0; job_from_ni < ip->nJobs; ++job_from_ni){
            if(ip->solMatrix[ni][job_from_ni] < 0){
                continue;
            }
            secondNurseI = -1;
            if(ip->doubleService[job_from_ni]){
                secondNurseI = FindSecondNurseDS(ip, job_from_ni, ni);
            }
            for(int nj = ni + 1; nj < ip->nNurses; ++nj){
                if(nj==secondNurseI){
                    continue;
                }

                if(ip->doubleService[job_from_ni]){
                    if(CheckSkillsDS(ip, job_from_ni, secondNurseI, nj)
                            < 1){
                        continue;
                    }
                }
                else if(CheckSkills(ip, job_from_ni, nj) < 1){
                    continue;
                }

                for(int job_from_nj = 0; job_from_nj < ip->nJobs; ++job_from_nj){
                    if(ip->solMatrix[nj][job_from_nj] < 0){
                        continue;
                    }
                    if(job_from_ni==job_from_nj){
                        continue;
                    }

                    int secondNurseJ = -1;
                    if(ip->doubleService[job_from_nj]){
                        secondNurseJ = FindSecondNurseDS(ip, job_from_nj, nj);
                    }

                    if(secondNurseJ==ni){
                        continue;
                    }

                    if(ip->doubleService[job_from_nj]){
                        if(CheckSkillsDS(ip, job_from_nj, secondNurseJ, ni) < 1){
                            continue;
                        }
                    }
                    else if(CheckSkills(ip, job_from_nj, ni) < 1){
                        continue;
                    }

                    ExchangeJobsInRoute(ip, ni, job_from_ni, nj, job_from_nj);

                    double tentQuality = SolutionQuality(ip, -123121);
                    if(tentQuality > currentQuality + TOL){
                        retVal = 0;
                        if(firstImprovement < 1){
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
                            return retVal;
                        }
                    }

                    ExchangeJobsInRoute(ip, ni, job_from_nj, nj, job_from_ni);
                }
            }
        }
    }

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

}

int ExchangeJobsInRoute(struct INSTANCE* ip, int ni, int job_from_ni, int nj, int job_from_nj){



    int aux = ip->solMatrix[nj][job_from_ni];

    if(aux > -1){
        printf("\n---\n---\nWARNING: Exchanging a job maybe we should not????\n");
        printf("ip->solMatrix[%d][%d] = %d\n", nj, job_from_ni, aux);
        printf("\tTrying to exchange (n%d, j%d) with (n%d, j%d).\nSolmatrix:\n", ni, job_from_ni, nj, job_from_nj);
        PrintSolMatrix(ip);
        exit(-123424);
    }

    ip->solMatrix[nj][job_from_ni] = ip->solMatrix[nj][job_from_nj];
    ip->solMatrix[nj][job_from_nj] = aux;

    aux = ip->solMatrix[ni][job_from_nj];

    if(aux > -1){
        printf("\n---\n---\nWARNING: Exchanging a job maybe we should not????\n");
        printf("ip->solMatrix[%d][%d] = %d\n", ni, job_from_nj, aux);
        printf("\tTrying to exchange (n%d, j%d) with (n%d, j%d).\nSolmatrix:\n", ni, job_from_ni, nj, job_from_nj);
        PrintSolMatrix(ip);
        exit(-123424);
    }

    ip->solMatrix[ni][job_from_nj] = ip->solMatrix[ni][job_from_ni];
    ip->solMatrix[ni][job_from_ni] = aux;

    return 0;

}

int BestSwitch(struct INSTANCE* ip, int onlyInfeasible, double MAX_TIME){

    clock_t start = clock();
    clock_t end = clock();
    double elapsedTime = 0;
    int firstSwitch = 1;
    double TOL = 0.0001;
    int nRows = ip->nNurses;
    int nCols = ip->nJobs;

    int** solMatrixInitial = malloc(nRows*sizeof(int*));
    for(int i = 0; i < nRows; i++){
        solMatrixInitial[i] = malloc(nCols*sizeof(int));
    }

    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            solMatrixInitial[i][j] = ip->solMatrix[i][j];
        }
    }

    CopyIntMatrix(ip->solMatrix, solMatrixInitial, nRows, nCols);
    double baseQuality = SolutionQuality(ip, -6);

    int besti = -1;
    int bestj = -1;
    int best_job = -1;

    for(int ni = 0; ni < ip->nNurses; ++ni){
        end = clock();
        elapsedTime = (float) (end - start)/CLOCKS_PER_SEC;
        if(elapsedTime > MAX_TIME){
            break;
        }
        for(int job_from_ni = 0; job_from_ni < ip->nJobs; ++job_from_ni){
            if(ip->solMatrix[ni][job_from_ni] < 0){
                continue;
            }
            if((onlyInfeasible > 0.5) && (ip->violatedTW[job_from_ni] < TOL)){
                continue;
            }
            for(int nj = 0; nj < ip->nNurses; ++nj){

                if(SwitchNurse(ip, ni, nj, job_from_ni) > -1){
                    double propQuality = SolutionQuality(ip, -7);
                    if(propQuality > baseQuality){
                        baseQuality = propQuality;
                        besti = ni;
                        bestj = nj;
                        best_job = job_from_ni;
                        if(firstSwitch > 0){
                            break;
                        }
                    }
                    else{
                        for(int i = 0; i < ip->nJobs; ++i){
                            ip->solMatrix[ni][i] = solMatrixInitial[ni][i];
                            ip->solMatrix[nj][i] = solMatrixInitial[nj][i];
                        }
                    }
                }
            }
            if(firstSwitch > 0 && besti > -1){
                break;
            }
        }
        if(firstSwitch > 0 && besti > -1){
            break;
        }
    }


    FreeMatrixInt(solMatrixInitial, ip->nNurses);

    if(besti > -1){
        if(firstSwitch < 1){
            SwitchNurse(ip, besti, bestj, best_job);
        }
        return 0;
    }

    if(ip->verbose > 5){
        printf("Best switch could NOT find anything good\n");
        printf("besti = %d, bestj = %d,  best_job = %d, baseQuality=%.2f\n", besti, bestj, best_job, baseQuality);
    }

    return -1;

}

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

    double cq = SolutionQuality(ip, -66643);

    for(int i = 0; i < ip->nNurses - 1; ++i){
        for(int j = i + 1; j < ip->nNurses; ++j){
            TwoExchange(ip->nurseOrder, i, j);

            double tentQ = SolutionQuality(ip, -66644);
            if(tentQ > cq){
                return 1;
            }
            else{
                TwoExchange(ip->nurseOrder, j, i);
            }
        }
    }
    return -1;

}

int TwoOptMove(struct INSTANCE* ip, int ni, int pos1, int pos2){

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

    int* nurseRoute = malloc(ip->nJobs*sizeof(int));
    GetNurseRoute(ip, ni, nurseRoute);

    if(nurseRoute[pos2] < 0){
        free(nurseRoute);
        return -1;
    }

    int* nurseRouteAux = malloc(ip->nJobs*sizeof(int));

    for(int i = 0; i < ip->nJobs; i++){
        nurseRouteAux[i] = nurseRoute[i];
    }

    nurseRouteAux[pos2] = nurseRoute[pos1];
    nurseRouteAux[pos1] = nurseRoute[pos2];

    int revCounter = 0;
    for(int i = pos1; i < pos2; i++){
        nurseRouteAux[pos1 + revCounter] = nurseRoute[pos2 - revCounter];
        revCounter++;
    }

    for(int j = 0; j < ip->nJobs; j++){
        if(nurseRouteAux[j] < 0){
            break;
        }
        ip->solMatrix[ni][nurseRouteAux[j]] = j;

    }
    free(nurseRoute);
    free(nurseRouteAux);

    return 0;

}

int FindSecondNurseDS(struct INSTANCE* ip, int job, int currentNurse){

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

    if(ip->solMatrix[ni][pi] < 0){
        return -1;
    }

    if(ip->nurseSkilled[nj][pi] < 1 && ip->doubleService[pi] < 1){
        return -1;
    }

    if(ip->doubleService[pi] > 0){
        int secondNurse = FindSecondNurseDS(ip, pi, ni);

        if(secondNurse==-1){
            printf("ERROR: When attempting to switch job %d from nurse %d to nurse %d\n", pi, ni, nj);
            printf("-> Double service was not found in any other nurse.\n");
            exit(-1);
        }
        if(CheckSkillsDS(ip, pi, secondNurse, nj) < 1){
            return -1;
        }

    }
    int oldPos = ip->solMatrix[ni][pi];

    RemoveJob(ip, pi, ni);

    if(BestJobInsertion(ip, pi, nj) < 0){
        InsertJobAtPosition(ip, pi, ni, oldPos);
        return -1;
    }

    return 0;
}

int GetJobCount(struct INSTANCE* ip, int ni){

    int maxE = -1;
    for(int i = 0; i < ip->nJobs; ++i){
        if(ip->solMatrix[ni][i] > maxE)
            maxE = ip->solMatrix[ni][i];
    }
    return (maxE + 1);

}

int GetNurseJobCount(struct INSTANCE* ip, int nurse){

    int count = 0;
    for(int i = 0; i < ip->nJobs; i++){
        if(ip->solMatrix[nurse][i] > -0.5){
            count++;
        }
    }

    return count;

}

void SetAllNurseRoutes(struct INSTANCE* ip){
    for(int ni = 0; ni < ip->nNurses; ni++){
        SetNurseRoute(ip, ni);
    }

}

void SetNurseRoute(struct INSTANCE* ip, int ni){

    for(int j = 0; j < ip->nJobs; ++j){
        ip->allNurseRoutes[ni][j] = -1;
    }

    for(int j = 0; j < ip->nJobs; ++j){
        if(ip->solMatrix[ni][j] >= 0){
            ip->allNurseRoutes[ni][ip->solMatrix[ni][j]] = j;
        }
    }
}


void GetNurseRoute(struct INSTANCE* ip, int ni, int* nurseRoute){
    int achange = 0;

    for(int ii = 0; ii < ip->nJobs; ++ii){
        nurseRoute[ii] = -1;
    }

    for(int ii = 0; ii < ip->nJobs; ++ii){
        if(ip->solMatrix[ni][ii] >= 0){
            nurseRoute[ip->solMatrix[ni][ii]] = ii;
            achange = 1;
        }
    }
}

void CalculateJobTimes(struct INSTANCE* ip, int nursei){

    int feasible = -1;

    ip->nurseWaitingTime[nursei] = 0;
    ip->nurseTravelTime[nursei] = 0;

    for(int j = 0; j < ip->nJobs; ++j){
        ip->timeMatrix[nursei][j] = 0;
        ip->nurseWaitingMatrix[nursei][j] = 0;
        ip->nurseTravelMatrix[nursei][j] = 0;
    }

    int numUnavail = ip->nurseUnavail[nursei];
    int numShifts = numUnavail + 1;
    int f = 0;
    int endOfDay = -1;
    if(numShifts == 1){
        endOfDay = 1;
    }

    int prevJob = -1;
    double currentTime = (double)ip->nurseWorkingTimes[nursei][0];

    for(int p = 0; p < ip->nJobs; ++p){
        feasible = -1;

        if(ip->allNurseRoutes[nursei][p] < 0){
            break;
        }
        int job = ip->allNurseRoutes[nursei][p];

        if(p == 0){
            double travelTimeFirst = TravelTimeFromDepot(ip, nursei, job);
            if(ip->excludeNurseTravel){
                if(ip->jobTimeInfo[job][0] > ip->nurseWorkingTimes[nursei][0] - 0.001){
                    currentTime = ip->jobTimeInfo[job][0];
                    currentTime += ip->twInterval;
                }
                else{
                    currentTime = ip->nurseWorkingTimes[nursei][0];
                }
            }
            else{
                currentTime += travelTimeFirst;
                ip->nurseTravelTime[nursei] += travelTimeFirst;
                ip->nurseTravelMatrix[nursei][job] = travelTimeFirst;
            }
        }
        else if(p > 0){
            double waitingTimePrev = ip->nurseWaitingMatrix[nursei][prevJob];
            int serviceTimePrev = ip->jobTimeInfo[prevJob][2];
            double travelTime = GetTravelTime(ip, prevJob, job);

            currentTime = currentTime + waitingTimePrev + serviceTimePrev + travelTime;
            ip->nurseTravelTime[nursei] += travelTime;
            ip->nurseTravelMatrix[nursei][job] = travelTime;
        }

        double ogArrivalTime = currentTime;
        ip->arrivalTimes[nursei][job] = ogArrivalTime;
        double startTW = ip->jobTimeInfo[job][0];
        double endTW = ip->jobTimeInfo[job][1];
        double startTWMK = ip->jobTimeInfo[job][0];
        double endTWMK = ip->jobTimeInfo[job][1];

        int gapAllowed = ip->algorithmOptions[12];
        int considerDependency = -1;
        int considerDoubleService = -1;
        int otherNurseDJ = -1;
        int otherNurseDS = -1;
        int otherJobDJ = -1;
        int secondNurse = -1;
        int jobAfter = -1;

        if(ip->dependsOn[job] > -1){
            otherJobDJ = ip->dependsOn[job];
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    otherNurseDJ = -1;
                    considerDependency = -1;
                    break;
                }
                if(ip->timeMatrix[prevNurse][otherJobDJ] > 0){
                    if(gapAllowed > 0){ // was if(aitOnly > 0)
                        ip->mkMinD[job] = abs(ip->mkMinD[job]);
                        ip->mkMaxD[job] = abs(ip->mkMaxD[job]);

                        double laa = ip->timeMatrix[prevNurse][otherJobDJ] - ip->mkMaxD[job];
                        if((laa >= startTW) && (currentTime <= laa)){
                            ip->mkMinD[job] = -1*ip->mkMaxD[job];
                            ip->mkMaxD[job] = -1*ip->mkMinD[job];
                            jobAfter = -1;

                        }
                        else{
                            jobAfter = 1;
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMinD[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJobDJ] + ip->mkMaxD[job];
                    considerDependency = 1;
                    otherNurseDJ = prevNurse;
                    secondNurse = 1;
                    break;
                }
            }
        }

        if(ip->doubleService[job] > 0){
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse == nursei){
                    otherNurseDS = -1;
                    considerDoubleService = -1;
                    break;
                }
                if(ip->timeMatrix[prevNurse][job] > 0){
                    startTWMK = ip->timeMatrix[prevNurse][job];
                    endTWMK = ip->timeMatrix[prevNurse][job];
                    considerDoubleService = 1;
                    otherNurseDS = prevNurse;
                    secondNurse = 1;
                    break;
                }
            }
        }

        if(numUnavail == 0){
            feasible = 1;
            double waitingTime = MaxNumDouble(0, (startTWMK - currentTime));
            double startTime = currentTime + waitingTime;
            double tardiness = MaxNumDouble(0, (startTime - endTW));
            ip->nurseWaitingMatrix[nursei][job] = waitingTime;
            ip->nurseWaitingTime[nursei] += waitingTime;
            ip->timeMatrix[nursei][job] = startTime;
            if(considerDependency > 0){
                double gapTardiness = MaxNumDouble(0, (startTime - endTWMK));
                ip->violatedTWMK[job] = gapTardiness;
                ip->violatedTW[job] = tardiness;
            }
            else if(considerDoubleService > 0){
                if(tardiness > ip->violatedTW[job]){
                    ip->violatedTW[job] = tardiness;
                }
            }
            else{
                ip->violatedTW[job] = tardiness;
            }
        }
        else{
            if(f < numUnavail){
                if(currentTime > ip->unavailMatrix[f][1][nursei] && currentTime < ip->unavailMatrix[f][2][nursei]){
                    double tempCurrentTime = ip->unavailMatrix[f][1][nursei];
                    ip->arrivalTimes[nursei][job] = tempCurrentTime;
                    double* timesArray;
                    timesArray = FindValidTime(ip, f, tempCurrentTime, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWMK, endTWMK);
                    feasible = 1;
                    currentTime = *(timesArray+1);
                    f = (int)*(timesArray+6);
                    ip->timeMatrix[nursei][job] = *(timesArray+2);
                    if(*(timesArray+3) >= 0){
                        ip->nurseWaitingMatrix[nursei][job] = *(timesArray+3);
                        ip->nurseWaitingTime[nursei] += *(timesArray+3);
                    }
                    double tardiness = *(timesArray+4);
                    double gapTardiness = *(timesArray+5);
                    if(considerDependency > 0){
                        ip->violatedTW[job] = tardiness;
                        if(gapTardiness >= 0){
                            ip->violatedTWMK[job] = gapTardiness;
                        }
                    }
                    else if(considerDoubleService > 0){
                        if(tardiness > ip->violatedTW[job]){
                            ip->violatedTW[job] = tardiness;
                        }
                    }
                    else{
                        ip->violatedTW[job] = tardiness;
                    }
                }
                else if(MaxNumDouble(currentTime, startTW) + ip->jobTimeInfo[job][2] > ip->unavailMatrix[f][1][nursei]){
                    double* timesArray;
                    timesArray = FindValidTime(ip, f, currentTime, nursei, job, considerDependency, otherNurseDJ, otherJobDJ, considerDoubleService, otherNurseDS, startTWMK, endTWMK);
                    feasible = 1;
                    currentTime = *(timesArray+1);
                    f = (int)*(timesArray+6);
                    ip->timeMatrix[nursei][job] = *(timesArray+2);
                    if(*(timesArray+3) >= 0){
                        ip->nurseWaitingMatrix[nursei][job] = *(timesArray+3);
                        ip->nurseWaitingTime[nursei] += *(timesArray+3);
                    }
                    double tardiness = *(timesArray+4);
                    double gapTardiness = *(timesArray+5);
                    if(considerDependency > 0){
                        ip->violatedTW[job] = tardiness;
                        if(gapTardiness >= 0){
                            ip->violatedTWMK[job] = gapTardiness;
                        }
                    }
                    else if(considerDoubleService > 0){
                        if(tardiness > ip->violatedTW[job]){
                            ip->violatedTW[job] = tardiness;
                        }
                    }
                    else{
                        ip->violatedTW[job] = tardiness;
                    }
                }
                else{
                    feasible = 1;
                    double waitingTime = MaxNumDouble(0, (startTWMK - currentTime));
                    double startTime = currentTime + waitingTime;
                    double tardiness = MaxNumDouble(0, (startTime - endTW));
                    ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                    ip->nurseWaitingTime[nursei] += waitingTime;
                    ip->timeMatrix[nursei][job] = startTime;
                    if(considerDependency > 0){
                        double gapTardiness = MaxNumDouble(0, (startTime - endTWMK));
                        ip->violatedTWMK[job] = gapTardiness;
                        ip->violatedTW[job] = tardiness;
                    }
                    else if(considerDoubleService > 0){
                        if(tardiness > ip->violatedTW[job]){
                            ip->violatedTW[job] = tardiness;
                        }
                    }
                    else{
                        ip->violatedTW[job] = tardiness;
                    }
                }
            }
            else{
                if(currentTime < ip->unavailMatrix[numUnavail-1][2][nursei]){
                    printf("[ERROR]: currentTime %.2f for nurse %d is within a break", currentTime, nursei);
                    printf("\t end of final break for nurse %d is at time %d", nursei, ip->unavailMatrix[numUnavail-1][2][nursei]);
                    exit(-1);
                }
                else{
                    feasible = 1;
                    double waitingTime = MaxNumDouble(0, (startTWMK - currentTime));
                    double startTime = currentTime + waitingTime;
                    double tardiness = MaxNumDouble(0, (startTime - endTW));
                    ip->nurseWaitingMatrix[nursei][job] = waitingTime;
                    ip->nurseWaitingTime[nursei] += waitingTime;
                    ip->timeMatrix[nursei][job] = startTime;
                    if(considerDependency > 0){ // DJ
                        double gapTardiness = MaxNumDouble(0, (startTime - endTWMK));
                        ip->violatedTWMK[job] = gapTardiness;
                        ip->violatedTW[job] = tardiness;
                    }
                    else if(considerDoubleService > 0){
                        if(tardiness > ip->violatedTW[job]){
                            ip->violatedTW[job] = tardiness;
                        }
                    }
                    else{
                        ip->violatedTW[job] = tardiness;
                    }
                }
            }


        }
        prevJob = job;

    }

    if(prevJob > -1){
        if(ip->excludeNurseTravel == 0){
            double travelTimeLast = TravelTimeToDepot(ip, nursei, prevJob);
            ip->nurseTravelTime[nursei] += travelTimeLast;
        }
    }



}

double* FindValidTime(struct INSTANCE* ip, int f, double currentTime, int currentNurse, int job, int considerDependency, int otherNurseDJ, int otherJobDJ, int considerDoubleService, int otherNurseDS,
                              double startTWMK, double endTWMK){


    static double timesArray[7];
    timesArray[0] = -1;
    timesArray[1] = -1;
    timesArray[2] = -1;
    timesArray[3] = -1;
    timesArray[4] = -1;
    timesArray[5] = -1;
    timesArray[6] = -1;

    int secondNurse = -1;
    int gapAllowed = -1;
    double feasible = 0.0;

    int numUnavail = ip->nurseUnavail[currentNurse];
    double originalArrivalTime = currentTime;

    if(considerDependency > 0 || considerDoubleService > 0){
        secondNurse = 1;
        if(considerDependency > 0){
            gapAllowed = ip->algorithmOptions[12];
        }
        else{
            gapAllowed = -1;
        }
    }

    int startOfShift = -1;
    int endOfShift = -1;
    int endOfDay = 0;

    while(feasible < 1 ){
        startOfShift = ip->unavailMatrix[f][2][currentNurse];
        f += 1;
        if(f >= numUnavail){
            endOfShift = ip->nurseWorkingTimes[currentNurse][1];
            endOfDay = 1;
        }
        else{
            endOfShift = ip->unavailMatrix[f][1][currentNurse];
        }
        currentTime = startOfShift;

        if(secondNurse > 0){
            if(currentTime > endTWMK){
                if(currentTime + ip->jobTimeInfo[job][2] > endOfShift && endOfDay == 0){
                    continue;
                }
                else{
                    feasible = 1;
                    double startTime = currentTime;
                    double waitingTime = MaxNumDouble(0, (startTime - originalArrivalTime));
                    double tardiness = MaxNumDouble(0, (startTime - ip->jobTimeInfo[job][1]));
                    double gapTardiness = -1;
                    if(considerDependency > 0){
                        gapTardiness = MaxNumDouble(0, (startTime - endTWMK));
                    }

                    timesArray[0] = feasible;
                    timesArray[1] = originalArrivalTime;
                    timesArray[2] = startTime;
                    timesArray[3] = waitingTime;
                    timesArray[4] = tardiness;
                    timesArray[5] = gapTardiness;
                    timesArray[6] = (double)(f);
                    return timesArray;
                }
            }
            else if(MaxNumDouble(currentTime, startTWMK) + ip->jobTimeInfo[job][2] > endOfShift && endOfDay == 0){
                continue;
            }
            else{
                feasible = 1;
                double startTime = currentTime;
                double waitingTime = MaxNumDouble(0, (startTime - originalArrivalTime));
                double tardiness = MaxNumDouble(0, (startTime - ip->jobTimeInfo[job][1]));
                double gapTardiness = -1;
                if(considerDependency > 0){
                    gapTardiness = MaxNumDouble(0, (startTime - endTWMK));
                }
                timesArray[0] = feasible;
                timesArray[1] = originalArrivalTime;
                timesArray[2] = startTime;
                timesArray[3] = waitingTime;
                timesArray[4] = tardiness;
                timesArray[5] = gapTardiness;
                timesArray[6] = (double)(f);
                return timesArray;
            }
        }
        else if(secondNurse < 1){
            if(currentTime + ip->jobTimeInfo[job][2] > endOfShift && endOfDay == 0){
                continue;
            }
            else{

                feasible = 1;
                double startTime = MaxNumDouble(currentTime, ip->jobTimeInfo[job][0]);
                double waitingTime = MaxNumDouble(0, (startTime - originalArrivalTime));
                double tardiness = MaxNumDouble(0, (startTime - ip->jobTimeInfo[job][1]));
                double gapTardiness = -1;
                timesArray[0] = feasible;
                timesArray[1] = originalArrivalTime;
                timesArray[2] = startTime;
                timesArray[3] = waitingTime;
                timesArray[4] = tardiness;
                timesArray[5] = gapTardiness;
                timesArray[6] = (double)(f);
                return timesArray;
            }
        }

    }

    return timesArray;

}



void SetTimesFull(struct INSTANCE* ip){

    for(int i = 0; i < ip->nJobs; ++i){
        ip->violatedTW[i] = 0;
        ip->violatedTWMK[i] = 0;
    }
    for(int j = 0; j < ip->nNurses; ++j){
        int nurse = ip->nurseOrder[j];
        CalculateJobTimes(ip, nurse);
    }

}

void SetTimesFrom(struct INSTANCE* ip, int firstNurse){

    int has_appeared = -1;
    int nurse = -1;
    int jobdue = -1;

    for(int i = 0; i < ip->nNurses; ++i){
        nurse = ip->nurseOrder[i];
        if(has_appeared < 0){
            if(nurse==firstNurse){
                has_appeared = 1;
            }
            else{
                continue;
            }
        }
        for(int j = 0; j < ip->nJobs; ++j){
            jobdue = ip->allNurseRoutes[nurse][j];
            if(jobdue < 0){
                break;
            }

            ip->violatedTW[jobdue] = 0;
            ip->violatedTWMK[jobdue] = 0;
        }

        CalculateJobTimes(ip, nurse);
    }
}

int SynchroniseJobi(struct INSTANCE* ip, int job, int nurse1, int nurse2){

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


double SolutionQualityLight(struct INSTANCE* ip){

    double TOL = 0.0001;
    SetTimesFull(ip);

    double lightQuality = ObjectiveNew(ip, -1);
    return (lightQuality);
}


double SolutionQuality(struct INSTANCE* ip, int report){
    if(report > 0){
        printf("-------- SolutionQuality(ip, %d) --------\n\n", report);
    }

    if (report == -98765){
        printf("Initial allNurseRoutes:\n");
        PrintAllNurseRoutes(ip);
    }

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

    SetTimesFull(ip);

    double quality = ObjectiveNew(ip, report);

    if(report > 0){
        printf("Nurse order: ( ");
        for(int i = 0; i < ip->nNurses; ++i){
            printf("%d,  ", ip->nurseOrder[i]);
        }
        printf(" )\n");
        printf("-------- finished. SolutionQuality(ip, %d) --------\n\n", report);
    }

    return quality;
}


int* MinsToTime(double time){
    static int s_time[3];
    s_time[0] = 0;
    s_time[1] = 0;
    s_time[2] = 0;
    int minsRound = floor(time);
    double decimalMins = time - minsRound;
    int hours = minsRound/60;
    int minutes = minsRound%60;
    int seconds = round(decimalMins*60);
    s_time[0] = hours;
    s_time[1] = minutes;
    s_time[2] = seconds;

    return s_time;
}

int* MinsToMinSecs(double time){

    static int s_minSecs[2];
    s_minSecs[0] = 0;
    s_minSecs[1] = 0;
    int minutes = floor(time);
    double decimalMins = time - minutes;
    int seconds = round(decimalMins*60);
    s_minSecs[0] = minutes;
    s_minSecs[1] = seconds;

    return s_minSecs;
}


double ObjectiveNew(struct INSTANCE* ip, int report){
    if(ip->verbose < 0){
        report = -1;
    }

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

    ip->totalPref = 0;

    for(int i = 0; i < ip->nNurses; ++i){
        int ni = ip->nurseOrder[i];
        totalTravelTime += ip->nurseTravelTime[ni];
        totalWaitingTime += ip->nurseWaitingTime[ni];
        if(report > 0){
            int* timeNWTStart;
            timeNWTStart = MinsToTime((double) (ip->nurseWorkingTimes[ni][0]));
            printf("Carer %d. Start time: %d:%d:%d, ", ni, *timeNWTStart, *(timeNWTStart + 1), *(timeNWTStart + 2));
            int* timeNWTEnd;
            timeNWTEnd = MinsToTime((double) (ip->nurseWorkingTimes[ni][1]));
            printf("End time: %d:%d:%d, ", *timeNWTEnd, *(timeNWTEnd + 1), *(timeNWTEnd + 2));
            int* timeNWTShift;
            timeNWTShift = MinsToTime((double) (ip->nurseWorkingTimes[ni][2]));
            printf("Length of shift: %d:%d:%d\n", *timeNWTShift, *(timeNWTShift + 1), *(timeNWTShift + 2));
        }

        if(ip->allNurseRoutes[ni][0] < -0.5){
            sparetime = (double) ip->nurseWorkingTimes[ni][1] - (double) ip->nurseWorkingTimes[ni][0];
            avgSpare += sparetime;
            finishTime = (double) ip->nurseWorkingTimes[ni][0];
            shortestDay = 0.0;
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

        for(int p = 0; p < ip->nJobs + 1; ++p){
            if(p >= ip->nJobs){
                job = -1;
            }
            else{
                job = ip->allNurseRoutes[ni][p];
            }
            if(job < -0.1){
                if(p > 0){
                    lastPosition = ip->allNurseRoutes[ni][p-1];
                }
                else{
                    lastPosition = -1;
                }
                break;
            }

            ip->totalPref += ip->prefScore[job][ni];
            arriveAt = ip->timeMatrix[ni][job];
            totalMKTardiness += ip->violatedTWMK[job];

            if(ip->doubleService[job] < 0.5){
                totalTardiness += ip->violatedTW[job];
                if(ip->violatedTW[job] > maxTardiness){
                    maxTardiness = ip->violatedTW[job];
                }
            }
            else{
                double realDSTardiness = ip->violatedTW[job]/2;
                totalTardiness += realDSTardiness;
                if(realDSTardiness > maxTardiness){
                    maxTardiness = realDSTardiness;
                }
            }

            if(report > 0){
                if(ip->doubleService[job] > 0){
                    int* timeArriveDS;
                    timeArriveDS = MinsToTime(arriveAt);
                    printf("\tJob %d (DS) arrives at %d:%d:%d, ", job, *timeArriveDS, *(timeArriveDS + 1), *(timeArriveDS + 2));
                }
                else{
                    int* timeArrive;
                    timeArrive = MinsToTime(arriveAt);
                    printf("\tJob %d arrives at %d:%d:%d, ", job, *timeArrive, *(timeArrive + 1), *(timeArrive + 2));
                }
                int* timeTWStart;
                timeTWStart = MinsToTime((double) (ip->jobTimeInfo[job][0]));
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
                printf("duration(ST): %d:%d:%d", *timeDuration, *(timeDuration + 1), *(timeDuration + 2));
                if((p < ip->nJobs - 1) && (ip->allNurseRoutes[ni][p + 1] > -1)){
                    int* timeTR;
                    double travelTime = GetTravelTime(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]);
                    timeTR = MinsToMinSecs(travelTime);
                    printf(", travel(TR): %d:%d ", *timeTR, *(timeTR + 1));
                    readyToNext += GetTravelTime(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]);
                }
                int* timeRTN;
                timeRTN = MinsToTime(readyToNext);
                printf("[ready at: %d:%d:%d]", *timeRTN, *(timeRTN + 1), *(timeRTN + 2));

                if(ip->violatedTW[job] > 0.1){
                    int* timeViolatedTW;
                    timeViolatedTW = MinsToMinSecs(ip->violatedTW[job]);
                    printf(" *** Misses TW by %d:%d ***", *timeViolatedTW, *(timeViolatedTW + 1));
                }

                if(ip->violatedTWMK[job] > 0.1){
                    printf(" *** Misses MK TW by %.2f (Mind %d, Maxd %d)***", ip->violatedTWMK[job], ip->mkMinD[job], ip->mkMaxD[job]);
                }
                printf("\n");
            }
        }

        if(lastPosition > -0.5){
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
            printf("Finish time: %d:%d:%d, ", *timeFinish, *(timeFinish + 1), *(timeFinish + 2));
            if(overtime < 0){
                double overtimePositive = overtime*-1;
                int* timeOTPositive;
                timeOTPositive = MinsToTime(overtimePositive);
                printf("Overtime: -%d:%d:%d, ", *timeOTPositive, *(timeOTPositive + 1), *(timeOTPositive + 2));
            }
            else if(overtime > 0){
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

    }

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

    for(int i = 0; i < ip->nJobs; ++i){
        jobsAssigned[i] = 0;
    }

    for(int i = 0; i < ip->nNurses; ++i){
        for(int p = 0; p < ip->nJobs; ++p){
            if(ip->allNurseRoutes[i][p] < 0){
                continue;
            }
            else{
                jobsAssigned[ip->allNurseRoutes[i][p]] += 1;
            }
        }
    }

    int numJobsAssigned = 0;
    for(int i = 0; i < ip->nJobs; ++i){
        numJobsAssigned += jobsAssigned[i];
    }


    if(numJobsAssigned == ip->nJobsIncDS){
        for(int i = 0; i < ip->nNurses; ++i){
            int ni = ip->nurseOrder[i];
            nurseServiceTime[ni] = 0;
            for(int p = 0; p < ip->nJobs; ++p){
                if(ip->allNurseRoutes[ni][p] < 0){
                    break;
                }
                else{
                    nurseServiceTime[ni] += ip->jobTimeInfo[ip->allNurseRoutes[ni][p]][2];
                }
            }
        }

        for(int i = 0; i < ip->nNurses; ++i){
            int ni = ip->nurseOrder[i];
            nurseWorkload[ni] = 0;
            if(nurseServiceTime[ni] == 0){
                continue;
            }
            else{
                nurseWorkload[ni] = (double)nurseServiceTime[ni] / (double)ip->nurseWorkingTimes[ni][4];
            }
        }

        double totalNurseWorkload = 0;
        for(int i = 0; i < ip->nNurses; ++i){
            totalNurseWorkload += nurseWorkload[i];
        }

        double avgNurseWorkload = totalNurseWorkload / (double)ip->nNurses;

        int bigNum = -99999;

        for(int i = 0; i < ip->nNurses; ++i){
            int ni = ip->nurseOrder[i];
            diffWorkloads[ni] = -bigNum;
            if(nurseWorkload[ni] == 0){
                continue;
            }
            else{
                diffWorkloads[ni] = nurseWorkload[ni] - avgNurseWorkload;
            }
        }

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

        for(int i = 0; i < ip->nNurses; ++i){
            if(ip->nurseTravelTime[i] > maxTravelTime){
                maxTravelTime = ip->nurseTravelTime[i];
                maxTravelTimeNurseIndex = i;
            }
        }


        for(int i = 0; i < ip->nNurses; ++i){
            if(ip->nurseWaitingTime[i] > maxWaitingTime){
                maxWaitingTime = ip->nurseWaitingTime[i];
                maxWaitingTimeNurseIndex = i;
            }
        }

    }



    double TOL = 1e-4;
    double mkAllowedTardiness = totalOvertime + totalTardiness;
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
    double infeasibilityM1 = ip->nNurses*12*60;
    double infeasibilityM2 = ip->nNurses*12*60;


    aitQuality = -1*(0.3*totalTravelTime + ip->totalPref);
    aitQuality += -10000*(totalMKTardiness + mkAllowedTardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mkAllowedTardiness > TOL) || (totalOvertime > TOL)){
        aitQuality += -10000;
    }

    mkQuality = -1*(totalTravelTime + mkAllowedTardiness + mkMaxTardiness)/3; // Mankowska
    if(totalMKTardiness > TOL){
        mkQuality += -1000 - 10000*(totalMKTardiness);
    }


    double dayDiff = (longestDay - shortestDay);
    wbQuality = -1*(0.3*totalTravelTime + ip->totalPref) - 0.1*dayDiff + 0.1*minSpareTime;
    wbQuality += -10000*(totalMKTardiness + mkAllowedTardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mkAllowedTardiness > TOL) || (totalOvertime > TOL)){
        wbQuality += -10000;
    }


    paperQuality = ip->algorithmOptions[51]*totalTravelTime
            + ip->algorithmOptions[52]*totalWaitingTime
            + ip->algorithmOptions[53]*mkAllowedTardiness
            + ip->algorithmOptions[54]*totalOvertime
            + ip->algorithmOptions[56]*ip->totalPref
            + ip->algorithmOptions[57]*mkMaxTardiness
            + ip->algorithmOptions[58]*maxTravelTime
            + ip->algorithmOptions[59]*maxWaitingTime
            + ip->algorithmOptions[60]*maxDiffWorkload;
    if(ip->algorithmOptions[50] > 0.5){
        if((totalMKTardiness > TOL) || (mkAllowedTardiness > TOL) || (totalOvertime > TOL)){
            paperQuality += -infeasibilityM1*(totalMKTardiness + mkAllowedTardiness + totalOvertime);
            paperQuality += -infeasibilityM2;
        }
    }
    else{

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

    ip->isFeasible = feasible;


    free(jobsAssigned);
    free(nurseServiceTime);
    free(nurseWorkload);
    free(diffWorkloads);


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

}


int RandomInteger(int min_val, int max_val){
    //return random value between min_val and max_val
    return min_val + (rand()%(max_val + 1));
}

void TwoExchange(int* array, int i, int j){

    int t = array[j];
    array[j] = array[i];
    array[i] = t;

}

void RandomTwoExchange(int* array, size_t n, int* i, int* j){

    if(n < 2){
        i = 0;
        j = 0;
        return;
    }


    (*i) = RandomInteger(0, n - 1);
    (*j) = RandomInteger(0, n - 1);


    while(*i==*j)
        (*j) = RandomInteger(0, n - 1);


    TwoExchange(array, (*i), (*j));
}

void Shuffle(int* array, size_t n){

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


double MaxNumDouble(double num1, double num2){
    return (num1 > num2) ? num1 : num2;
}

void RemoveBreaksWaitingTime(struct INSTANCE* ip){


    for(int i = 0; i < ip->nNurses; ++i){
        int nurse = ip->nurseOrder[i];
        int numUnavail = ip->nurseUnavail[nurse];
        ip->nurseWaitingTime[nurse] = 0;
        for(int p = 0; p < ip->nJobs; ++p){
            if(ip->allNurseRoutes[nurse][p] < 0){
                break;
            }
            int job = ip->allNurseRoutes[i][p];
            double originalArrivalTime = ip->arrivalTimes[nurse][job];
            double startTime = ip->timeMatrix[nurse][job];
            double breakTimeTotal = 0;
            if(numUnavail > 0){
                for(int f = 0; f < numUnavail; ++f){
                    if(ip->unavailMatrix[f][1][nurse] >= originalArrivalTime && ip->unavailMatrix[f][2][nurse] <= startTime){
                        breakTimeTotal += ip->unavailMatrix[f][3][nurse];
                    }
                }
                double actualWaitingTime = ip->nurseWaitingMatrix[nurse][job] - breakTimeTotal;
                ip->nurseWaitingMatrix[nurse][job] = actualWaitingTime;
            }
        }

        double totalWaitingNurse = 0;
        for(int k = 0; k < ip->nJobs; ++k){
            if(ip->nurseWaitingMatrix[nurse][k] > 0){
                totalWaitingNurse += ip->nurseWaitingMatrix[nurse][k];
            }
        }
        ip->nurseWaitingTime[nurse] = totalWaitingNurse;
    }

}





















