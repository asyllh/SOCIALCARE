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
    struct INSTANCE inst = generate_instance();
    struct INSTANCE* ip = &inst;
    ip->verbose = 4;
    return main_with_output(ip, NULL, NULL);
}

MODULE_API int python_entry(int nJobs_data, int nNurses_data, int nSkills_data, int verbose_data, float MAX_TIME_SECONDS, int tw_interval_data, bool exclude_nurse_travel_data,
                            double* od_data, double* nurse_travel_from_depot_data, double* nurse_travel_to_depot_data, int* unavail_matrix_data, int* nurse_unavail_data,
                            int* nurseWorkingTimes_data, int* jobTimeInfo_data, int* jobRequirements_data, int* nurseSkills_data, int* solMatrixPointer,
                            int* doubleService_data, int* dependsOn_data, int* mk_mind_data, int* mk_maxd_data, int* capabilityOfDoubleServices,
                            double* prefScore, double* algorithmOptions_data, int randomSeed){

    // printf("Inside C function. Random seed = %d.\n", randomSeed);

    double eps = 1e-6;
    int printInputData = (int) algorithmOptions_data[99] + eps;

    int qm = (int) (algorithmOptions_data[0] + eps); // printf("Quality measure	= %d\n\n", qm);
    int do_gap_not_precedence = (int) (algorithmOptions_data[12] + eps); // printf("Use gaps for dependent jobs (rather than precedence)	= %d\n", do_gap_not_precedence);

    if(qm==0 && do_gap_not_precedence==0)
        printf("WARNING: Solving with AIT H quality measure but with PRECEDENCE rather than GAP.\n");

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
        print_double_matrix_one(od_data, nJobs_data + 1, nJobs_data + 1);
        printf("nurse_travel_from_depot_data = \n");
        print_double_matrix_one(nurse_travel_from_depot_data, nNurses_data, nJobs_data);
        printf("nurse_travel_to_depot_data = \n");
        print_double_matrix_one(nurse_travel_to_depot_data, nNurses_data, nJobs_data);
        printf("nurseWorkingTimes_data = \n");
        print_int_matrix_one(nurseWorkingTimes_data, nNurses_data, 3);
        printf("jobTimeInfo_data = \n");
        print_int_matrix_one(jobTimeInfo_data, nJobs_data, 3);
        printf("jobRequirements_data = \n");
        print_int_matrix_one(jobRequirements_data, nJobs_data, nSkills_data);
        printf("nurseSkills_data = \n");
        print_int_matrix_one(nurseSkills_data, nNurses_data, nSkills_data);
        printf("solMatrixPointer = \n");
        print_int_matrix_one(solMatrixPointer, nNurses_data, nJobs_data);
        printf("mk_mind_data = \n");
        print_int_matrix_one(mk_mind_data, 1, nJobs_data);
        printf("mk_maxd_data = \n");
        print_int_matrix_one(mk_maxd_data, 1, nJobs_data);
        printf("doubleService_data = \n");
        print_int_matrix_one(doubleService_data, 1, nJobs_data);
        printf("dependsOn_data = \n");
        print_int_matrix_one(dependsOn_data, 1, nJobs_data);
        int nDS = 0;
        for(int i = 0; i < nJobs_data; ++i){
            nDS += doubleService_data[i];
        }
        printf("capabilityOfDoubleServices (only first nurses*nurses, jobs)= \n");
        print_int_matrix_one(capabilityOfDoubleServices, nDS, nNurses_data*nNurses_data);
        printf("prefScore = \n");
        print_double_matrix_one(prefScore, nJobs_data, nNurses_data);
        printf("algorithmOptions_data = \n");
        print_double_matrix_one(algorithmOptions_data, 1, 100);
    }

    // exit(-1);
    // printf("In C.\nReading input...\n");
    struct INSTANCE inst = instance_from_python(nJobs_data, nNurses_data, nSkills_data, verbose_data, MAX_TIME_SECONDS, tw_interval_data, exclude_nurse_travel_data, od_data, nurse_travel_from_depot_data, nurse_travel_to_depot_data,
                                                unavail_matrix_data, nurse_unavail_data,
                                                nurseWorkingTimes_data, jobTimeInfo_data, jobRequirements_data, nurseSkills_data, doubleService_data, dependsOn_data, mk_mind_data, mk_maxd_data,
                                                capabilityOfDoubleServices,
                                                prefScore, algorithmOptions_data);

    // printf("Done.\nBack in python_entry()\n");
    if(printInputData > 0){
        printf("nurseSkilled = \n");
        print_int_matrix(inst.nurseSkilled, nNurses_data, nJobs_data);
        printf("\n---------------  end of input data in C  ---------------\n\n");
    }

    //Calculating the minimum preference score
    double MMM = 10000;
    double min_pref = 0.0;
    // First, ignore doubleServices (Ds's)
    for(int i = 0; i < inst.nJobs; ++i){
        if(inst.doubleService[i] > 0)
            continue;

        double best_a = MMM;
        for(int j = 0; j < inst.nNurses; ++j){
            if(check_skills(&inst, i, j)){
                if(inst.prefScore[i][j] < best_a)
                    best_a = inst.prefScore[i][j];
            }
        }
        min_pref += best_a;
    }
    // Add DS's now
    for(int i = 0; i < inst.nJobs; ++i){
        if(inst.doubleService[i] < 1)
            continue;

        double best_a = MMM;
        for(int j = 0; j < inst.nNurses; ++j){
            for(int k = 0; k < inst.nNurses; ++k){
                if(check_skills_ds(&inst, i, j, k)){
                    double comb_pref = inst.prefScore[i][j] + inst.prefScore[i][k];
                    if(comb_pref < best_a)
                        best_a = comb_pref;
                }
            }
        }

        min_pref += best_a;
    }
    printf("THE MIN PREF OF THIS INSTANCE IS:\n");
    printf("<-< minPref = %.2f >->\n", min_pref);

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
        retvalue = main_with_output(ip, solMatrixPointer, od_data, exclude_nurse_travel_data);
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

    if(qm==0 && do_gap_not_precedence==0)
        printf("WARNING: Solving with AIT H quality measure but with PRECEDENCE rather than GAP.\n");

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
    print_solmatrix(ip);
    printf("-\n");

    printf("SolMatrix Quality\t%.4f\n", sol_quality(ip, 11));

    printf("-- TESTING A SOLUTION, NOT SOLVING -- \n");
    free_instance_memory(ip);

    return 0;
}

int main_with_output(struct INSTANCE* ip, int* solMatrixPointer, double* odmat_pointer, bool exclude_nurse_travel_data){

    // Call the GRASP algorithm
    GRASP(ip);


    // printf("Swiches %d\n", count);
    double finalQuality = sol_quality(ip, -1);
    if(ip->verbose > 0){
        // report_solution(ip);
        printf("Final solution quality is: %.2f\n", finalQuality);

        // printf("After %d swaps and %d switches, solution quality is: %.2f\n", performedSwaps, performedSwitches, finalQuality);
    }

    // printf("Saving final quality in OD...\n");
    if(odmat_pointer!=NULL){
        odmat_pointer[0] = finalQuality;
    }
    // printf("Done.\n");

    printf("\n------\nQuality indicators:\n");
    printf("Travel time   (min)\t%.2f\n", ip->objTravel);
    printf("Total time    (min)\t%.2f\n", ip->objTime);
    printf("Total tard.   (min)\t%.2f\n", ip->objTardiness);
    printf("Longest shift (min)\t%.2f\n", ip->objLongestDay);
    printf("Total Overtime(min)\t%.2f\n", ip->objOvertime);
    printf("\n------\n");

    // if (finalQuality > 31)
    // 	print_solmatrix(ip);
    if(ip->verbose > 10)
        printf("Finishing stuff...\n");

    solmatrix_to_python_format(ip, solMatrixPointer);

    exclude_nurse_travel_data = false;

    // solMatrixPointer = ip->solMatrix;
    if(ip->verbose > 5)
        printf("End of program.\nFreeing memory...\n");
    free_instance_memory(ip);
    if(ip->verbose > 5)
        printf("Done.");

    return 0;
}

void solmatrix_to_python_format(struct INSTANCE* ip, int* solMatrixPointer){
    int ct = 0;
    if(solMatrixPointer!=NULL){
        if(ip->verbose > 10)
            printf("Allocating stuff back to python\n");

        for(int i = 0; i < ip->nNurses; ++i){
            for(int j = 0; j < ip->nJobs; ++j){
                solMatrixPointer[ct] = ip->solMatrix[i][j];
                ct++;
            }

            if(ip->verbose > 10)
                printf("\n");
        }
    }
}

void constructive_basic(struct INSTANCE* ip){

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
        shuffle(jobPermutation, (size_t) ip->nJobs);
        if(ip->verbose > 2){
            printf("Generated a permutation of jobs:\n");
            print_vector(jobPermutation, (size_t) ip->nJobs);
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
                    best_job_insertion(ip, i, nursej);
                    // printf("Job %d assigned to nurse %d in position %d\n", i, j, jobsCount[nursej]);
                    jobsCount[nursej] = jobsCount[nursej] + 1;
                    jobsAssigned = jobsAssigned + 1;
                    chosenNurse++;
                    // if ((float) jobsCount[j] > balancedTarget)
                    // 	chosenNurse = (chosenNurse + 1) % ip->nNurses;
                    // ip->doubleService[i] = -1;
                    // printf("About to check DS...\n");
                    if(ip->doubleService[i] > 0){
                        best_job_insertion(ip, i, ((nursej + 1)%ip->nNurses));
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
            print_int_matrix(ip->nurseSkilled, ip->nNurses, ip->nJobs);

            printf("jobRequirements: \n");
            print_int_matrix(ip->jobRequirements, ip->nJobs, ip->nSkills);

            printf("nurseSkills: \n");
            print_int_matrix(ip->nurseSkills, ip->nNurses, ip->nSkills);

        }
        else if(ip->verbose > 2){
            printf("Assigned all jobs correctly.\n");
            report_solution(ip);
        }
    }
    else{
        initial_job_assignment(ip);
    }
    // standard_local_search(ip, ip->MAX_TIME_SECONDS);

}

int check_skills(struct INSTANCE* ip, int job, int nurse){
    //This function returns 1 if nurse is skilled to do the job, and returns 0 otherwise.

    return ip->nurseSkilled[nurse][job];
}

int check_skills_ds_first(struct INSTANCE* ip, int job, int nursei){

    // This function checks to see if there is another nurse that can do the double service with nurse i.
    // Function returns 1 if there is another nurse, and 0 if there is not.

    // This for loop is to count up the number of double service jobs up to our current one, 'job'.
    // This is because the capabilityOfDoubleServices 3d matrix is nNurses x nNurses x nDoubleServices! So we need to find out what number of double service our 'job' is.
    int jobdsindex = 0;
    for(int i = 0; i < job; ++i){ // Only count up to (but not including) our 'job', this ensures that jobdsindex is correct and not out of bounds.
        jobdsindex += ip->doubleService[i];
    }

    //Now go through all nurses and try to find another nurse, i, that can do our double service job, jobdsindex, together with our current nurse, nursei.
    for(int i = 0; i < ip->nNurses; ++i){
        if(i==nursei){
            continue;
        }
        // Is there any other nurse that can do it?
        if(ip->capabilityOfDoubleServices[nursei][i][jobdsindex] > 0){ // nursei and i are capable of performing the job together!
            return 1;
        }
    }

    return 0;

} // END OF check_skills_ds_first function.

int check_skills_ds(struct INSTANCE* ip, int job, int nursei, int nursej){

    // This function checks whether nursei and nursej are capable of doing double service 'job' together.
    // Returns 1 if yes, and 0 if no (capabilityOfDoubleService 3D matrix is a 0-1 matrix).

    // This for loop is to count up the number of double service jobs up to our current one, 'job'.
    // This is because the capabilityOfDoubleServices 3d matrix is nNurses x nNurses x nDoubleServices! So we need to find out what number of double service our 'job' is.
    int jobdsindex = 0;
    for(int i = 0; i < job; ++i){
        jobdsindex += ip->doubleService[i];
    }

    return ip->capabilityOfDoubleServices[nursei][nursej][jobdsindex];
}

void initial_job_assignment(struct INSTANCE* ip){
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
            nurseSeed[cNurse] = find_furthest_unallocated_point_for_nurse(ip, allocatedJobs, cNurse);
            if(nurseSeed[cNurse] > -1 && best_job_insertion(ip, nurseSeed[cNurse], cNurse) > -1){
                allocatedJobs[nurseSeed[cNurse]] = 1;
                ajc++;
                // printf("Allocated seed for nurse %d, job %d (total of %d out of %d)\n", cNurse, nurseSeed[cNurse], ajc, ip->nJobs);

            }
            else{
                printf("WARNING: Could not find any job for nurse %d at initial_job_assignment()\n", cNurse);
                printf("Seed was: %d\n\n", nurseSeed[cNurse]);
                // exit(-1);
            }
        }
        else{
            int nextPoint = find_closest_unallocated_point_for_nurse(ip, allocatedJobs, nurseSeed[cNurse], cNurse);
            int wentWrong = 0;
            if(nextPoint < 0){
                wentWrong = 1;
                printf("WARNING: Got as results: %d\n", nextPoint);
            }
            else{
                wentWrong = best_job_insertion(ip, nextPoint, cNurse);
                if(wentWrong < 0){
                    printf("best_job_insertion returned %d\n", wentWrong);
                    wentWrong = 1;
                    if(ip->doubleService[nextPoint]){
                        printf("Job is a DS\n");
                    }
                    printf("ip->nurseSkills:\n");
                    print_int_matrix(ip->nurseSkills, ip->nNurses, ip->nSkills);
                    printf("ip->jobRequirements:\n");
                    print_int_matrix(ip->jobRequirements, ip->nJobs, ip->nSkills);

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
            double intTravel = get_travel_time(ip, nurseSeed[j], i);

            if(intTravel < get_travel_time(ip, i, nurseSeed[j]))
                intTravel = get_travel_time(ip, i, nurseSeed[j]);

            if(intTravel < bestTravel && ip->solMatrix[j][i] < 0){
                toThisNurse = j;
                bestTravel = intTravel;
            }
        }
        if(toThisNurse < 0 || best_job_insertion(ip, i, toThisNurse) < 0){
            printf("\tWARNING: Could not allocate double service %d (toThisNurse = %d)!\n", i, toThisNurse);
            // free_instance_memory(ip);
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

int find_closest_unallocated_point_for_nurse(struct INSTANCE* ip, int* allocatedJobs, int job, int nurse){
    int closestPoint = -1;
    double bestDistance = bigM;
    double cdist;
    for(int i = 0; i < ip->nJobs; ++i){
        if(allocatedJobs[i] > 0){
            continue;
        }
        if((ip->doubleService[i] < 1 && check_skills(ip, i, nurse) < 1)){
            continue;
        }

        // debug : speed // these could be two separate loops, so the if-statement is only made once
        if(job < -0.5)
            cdist = get_travel_time_from_depot(ip, nurse, i);
        else{
            cdist = get_travel_time(ip, job, i);
            if(cdist < get_travel_time(ip, i, job))
                cdist = get_travel_time(ip, i, job);
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

int find_furthest_unallocated_point_for_nurse(struct INSTANCE* ip, int* allocatedJobs, int nurse){
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
            if(ip->doubleService[i] < 1 && check_skills(ip, i, nurse) < 0){
                continue;
                printf("\tSkipping job %d, because nurse %d is unskilled!\n", i, nurse);
            }


            // debug : speed // these could be two separate loops, so the if-statement is only made once
            if(job < -0.5)
                cdist = get_travel_time_from_depot(ip, nurse, i);
            else{
                cdist = get_travel_time(ip, job, i);
                if(cdist > get_travel_time(ip, i, job))
                    cdist = get_travel_time(ip, i, job);
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

void print_int_matrix(int** matrix, int nRows, int nCols){
    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
}

void print_int_matrix_one(int* matrix, int nRows, int nCols){
    int count = 0;
    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            printf("%d\t", matrix[count]);
            count++;
        }
        printf("\n");
    }
}

void print_double_matrix_one(double* matrix, int nRows, int nCols){
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

void print_double_matrix(double** matrix, int nRows, int nCols){
    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            printf("%.4f\t", matrix[i][j]);
        }
        printf("\n");
    }
}

void print_solmatrix(struct INSTANCE* ip){
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

void print_allNurseRoutes(struct INSTANCE* ip){
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

void print_timeMatrix(struct INSTANCE* ip){
    for(int i = 0; i < ip->nNurses; ++i){
        for(int j = 0; j < ip->nJobs; ++j){
            printf("%.2f\t", ip->timeMatrix[i][j]);
        }
        printf("\n");
    }
}

double get_travel_time(struct INSTANCE* ip, int jobi, int jobj){
    // DEBUG : SPEED we can remove the error check if the -1 call is never made anymore
    if(jobi < 0 || jobj < 0){
        printf("\nERROR: Calling travel time with a -1 in get_travel_time(struct INSTANCE * ip, int jobi, int jobj).\n");
        printf("This is not valid since we allow for nurses to have different starting locations.\n");
        exit(-4323452);
    }

    return ip->od[jobi + 1][jobj + 1];
}

double get_travel_time_from_depot(struct INSTANCE* ip, int nurse, int job){
    if(ip->nurse_travel_from_depot[nurse][job] > 100000000000)
        printf("\nError! when calling with FROM nurse %d job %d, travel reported is %.3f", nurse, job, ip->nurse_travel_from_depot[nurse][job]);
    return ip->nurse_travel_from_depot[nurse][job];
}

double get_travel_time_to_depot(struct INSTANCE* ip, int nurse, int job){
    if(ip->nurse_travel_to_depot[nurse][job] > 100000000000)
        printf("\nError! when calling with TO nurse %d job %d, travel reported is %.3f", nurse, job, ip->nurse_travel_to_depot[nurse][job]);
    return ip->nurse_travel_to_depot[nurse][job];
}

double get_travel_time_bnc(struct INSTANCE* ip, int nodei, int nodej){
    // The difference here is that the first ip->nNurses indices
    // correspond to nurse depots, the rest are jobs
    if(nodei < ip->nNurses && nodej < ip->nNurses){
        printf("\nWARNING: Trying to access distance between two nurse depots in 'get_travel_time_bnc(...)', which is not defined.\n");
        return -1;  //	This is not defined
    }

    if(nodei < ip->nNurses)
        return get_travel_time_from_depot(ip, nodei, nodej - ip->nNurses);
    else if(nodej < ip->nNurses)
        return get_travel_time_from_depot(ip, nodej, nodei - ip->nNurses);
    else
        return get_travel_time(ip, nodei - ip->nNurses, nodej - ip->nNurses);
}

int report_solution(struct INSTANCE* ip){
    sol_quality(ip, 1);
    return 0;
}

int swap_points(struct INSTANCE* ip, int ni, int nj, int pi, int pj){
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

int remove_job(struct INSTANCE* ip, int job, int ni){
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
    set_nurse_route(ip, ni);
    set_times_from(ip, ni);

    return 0;
}

int job_insertion_last(struct INSTANCE* ip, int job, int ni){
    double baseQuality = -1*bigM;
    // int nurse_job_count = get_nurse_job_count(ip, ni);

    if(insert_job_at_position(ip, job, ni, 0) < 0){
        return -1;
    }
    else{
        baseQuality = sol_quality(ip, -2000);
        return 0;
    }

}

int best_job_insertion(struct INSTANCE* ip, int job, int ni){
    // This function starts at position 0 of nurse ni's route, and trials adding 'job' into each position 0,...,nJobs + 10, calculating the quality of each solution created using the job in the new position.
    // Note that a job is inserted into a position, the quality is calculated, and then the job is removed from that position (i.e. the solution is returned to the original) before the function attempts to
    // insert the job into the next position.
    // Therefore, this function repeatedly uses the function 'insert_job_at_position' to test out every position.
    // The position that 'job' is inserted into in nurse ni's route that produces the best quality solution (bestQuality) is stored (bestPosition), and this position is then used to produce a solution,
    // insert_job_at_position(ip, job, ni, bestPosition).
    // This function returns 0 if 'job' has been inserted successfully into nurse ni's route, otherwise it returns -1 (job cannot be inserted into any position in nurse ni's route).

    int bestPosition = -1*bigM; //New
    double bestQuality = -1*bigM;
    int firstSwitch = 0; // Is this needed?

    if(insert_job_at_position(ip, job, ni, 0) < 0){ // Position = 0, if job cannot be inserted into nurse ni's route in position 0.
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
        bestQuality = sol_quality_light(ip); // Solution quality of ip with new job insertion
        remove_job(ip, job, ni); // Reverse job insertion, go back to original solution
        //printf("Insertion into position 0 successful. Job: %d, Nurse: %d\n", job, ni);
    }

    //int bestPosition = 0; // OLD: Because currently we've only inserted a job in position 0 of nurse ni's route (above), so best position is 0.
    // NOTE: This is incorrect (16/05/2021), as above we've removed the 'return -1' if a job can't be inserted into position 0. So, the bestPosition is not necessarily 0!
    // Changing this so that bestPosition = -1*bigM at the start of the function, and is set to =0 if insertion into position =0 is successful
    double propQuality = -1;
    for(int j = 1; j < ip->nJobs; ++j){ // Why nJobs + 10? start from j=1 because we've already done position 0 (above).
        if(insert_job_at_position(ip, job, ni, j) < 0){ // If job cannot be inserted into nurse ni's route in position j, exit for loop.
            break;
        }
        else{
            propQuality = sol_quality_light(ip); // Job has been inserted into nurse ni's route into position j, so calculate quality of this new solution
            if(propQuality > bestQuality){ // If this solution has better quality than current best quality solution
                bestPosition = j; // The best position to insert job is updated to position j
                bestQuality = propQuality; // The quality of solution with best position of inserted job is updated.
                if(firstSwitch > 0){ // Is this needed?
                    return 0;
                }
            }
            remove_job(ip, job, ni); //Reverse job insertion, go back to original solution.
        }
    }

    //We have found the best position in nurse ni's route to put job, so now we insert job into that best position.
    //insert_job_at_position(ip, job, ni, bestPosition);
    //return 0;
    if(bestPosition >= 0){
        insert_job_at_position(ip, job, ni, bestPosition);
        return 0;
    }
    else{
        printf("ERROR: bestPosition < 0, code should not reach here!");
        return -1;
    }

} //END OF best_job_insertion function.

int insert_job_at_position(struct INSTANCE* ip, int job, int ni, int posi){
    // This function attempts to push forward all of the positions of jobs in nurse ni's route and add a new job in a particular position, 'posi' in nurse ni's route.
    // This function returns retVal which is either 0 (job has been inserted into position 'posi' successfully) or -1 (job cannot be inserted into position 'posi').

    // Check the job is not there
    if(ip->solMatrix[ni][job] > -1){ //The job is already assigned to that nurse.
        //printf(" <> Job there. job: %d, nurse: %d, solMatrix position: %d, posi: %d <>\n", job, ni, ip->solMatrix[ni][job], posi);
        return -1;
    }

    // Check nurse can do the job
    if(check_skills(ip, job, ni) < 1 && ip->doubleService[job] < 0.5){ //why 0.5? doubleService is an int array.
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
        set_nurse_route(ip, ni); //Update allNurseRoutes for nurse ni.
        set_times_from(ip, ni);
        return 0;
    }
    set_nurse_route(ip, ni); // Update allNurseRoutes for nurse ni.

    return retVal;
}

int best_sync_double_switch(struct INSTANCE* ip){

    //This function removes a double service job from two nurse's routes and tries to add the job back into two other routes in the best possible positions such that the solution quality improves
    //OR removes two dependent jobs from two nurse's routes and tries to add the two jobs back into two other routes in the best possible positions such that the solution quality improves.
    //Returns 1 if successful and -1 otherwise.
    //Note that if double service, then jobA and jobB are the same job, otherwise if dependent job then jobA and jobB are different jobs.

    int DEBUG_PRINT = -1;
    if(DEBUG_PRINT > 0){
        printf("--- STARTING best_sync_double_switch ---");
        printf("Initial solmatrix:\n");
        print_solmatrix(ip);
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
    double baseQuality = sol_quality(ip, DEBUG_PRINT - 30); //original solution quality of ip
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
                print_solmatrix(ip);
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
        remove_job(ip, jobA, nurseA);

        if(DEBUG_PRINT > 0){
            printf("About to remove %d from nurse %d\n", jobB, nurseB);
            printf("Currently at position (posJobB) %d\t", posJobB);
            printf("Currently at position %d\n", ip->solMatrix[nurseB][jobB]);
        }

        //Remove jobB from nurseB's route
        remove_job(ip, jobB, nurseB);

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
                if(check_skills_ds_first(ip, jobA, tn)
                        < 1){ //If no other nurse can do jobA with nurse tn. (capabilityOfDoubleService, if there's another nurse that can do the job with tn then =1, else = 0)
                    if(DEBUG_PRINT > 0){
                        printf("\t(!) Not possible, does not cover first part of DS.\n");
                    }
                    continue; //Go to next nurse tn (++tn)
                }
            }
            else{ //If jobA is not a double service
                if(check_skills(ip, jobA, tn) < 1){ //If nurse tn is not skilled to do jobA (nurseSkilled[tn][jobA] = 0)
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
                int insertAValue = insert_job_at_position(ip, jobA, tn, tpos); //Insert jobA into nurse tn's route in position tpos, =0 if successful, =-1 otherwise.
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
                    if(ip->doubleService[jobA] && check_skills_ds(ip, jobB, tn, tnb) < 1){ //If jobA is a double service AND nurse tn and nursetnb are NOT skilled to do jobB together
                        if(DEBUG_PRINT > 0){
                            printf("\t\t(!) Not possible, unskilled to do second part of DS.\n");
                        }
                        continue; //Go to next nurse tnb (++tnb)
                    }
                    else if((!ip->doubleService[jobA]) && check_skills(ip, jobB, tnb) < 1){ //If jobA is NOT a double service AND nurse tnb is NOT skilled to do jobB
                        if(DEBUG_PRINT > 0){
                            printf("\t\t(!) Not possible, unskilled.\n");
                        }
                        continue; //Go to next nurse tnb (++tnb)
                    }
                    if(DEBUG_PRINT > 0){
                        printf("\t\t(-) Possible, calling best_job_insertion\n");
                    }

                    int insertValue = best_job_insertion(ip, jobB, tnb); //Try to insert jobB into nurse tnb's route, =0 if successful, = -1 otherwise.
                    if(insertValue > -1){ //If jobB inserted into nurse tnb's route successfully
                        bPossible = 1;
                        tQuality = sol_quality(ip, -5);
                        if(tQuality > bestTargetQuality){ //If solution quality has improved
                            bestTargetQuality = tQuality;
                            finalTNB = tnb; //nurse tnb
                            finalPosB = ip->solMatrix[tnb][jobB]; //position of jobB in nurse tnb's route
                        }
                        if(DEBUG_PRINT > 0){
                            printf("\t>>> Inserted job B in nurse %d (Ins val %d, quality %.2f) <<<\n", tnb, insertValue, tQuality);
                        }
                        remove_job(ip, jobB, tnb); //Return solution back to before
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
                remove_job(ip, jobA, tn); //Remove jobA from nurse tn's route
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
            if(insert_job_at_position(ip, jobA, nurseA, posJobA) < 0){
                printf("ERROR! Solmatrix:\n");
                print_solmatrix(ip);
                printf("ERROR: Cannot insert %d where it was!\n", jobA);
                printf("jobA = %d, nurseA = %d, posJobA = %d\n", jobA, nurseA, posJobA);
                printf("jobB = %d, nurseB = %d, posJobB = %d\n", jobB, nurseB, posJobB);
                exit(-34234523);
            }
        }

        if(insert_job_at_position(ip, jobB, nurseB, posJobB) < 0){
            printf("ERROR! Solmatrix:\n");
            print_solmatrix(ip);
            printf("ERROR: Cannot insert the %d where it was!\n", jobB);
            printf("jobA = %d, nurseA = %d, posJobA = %d\n", jobA, nurseA, posJobA);
            printf("jobB = %d, nurseB = %d, posJobB = %d\n", jobB, nurseB, posJobB);
            exit(-34234523);
        }

        // Repeat the previous code. This is to avoid the case where both are inserted in the same nurse,
        // and A is the last job, which would mess up the indices.
        if(!(posJobA < posJobB)){
            if(insert_job_at_position(ip, jobA, nurseA, posJobA) < 0){
                printf("ERROR! Solmatrix:\n");
                print_solmatrix(ip);
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


    // double fQual = sol_quality(ip, 0);
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
                printf("--- -1 early FINISHED best_sync_double_switch ---");
            }
            return -1;
        }

        // First find them and remove them:
        for(int nn = 0; nn < ip->nNurses; ++nn){
            if(ip->solMatrix[nn][overall_bestA] > -1){
                remove_job(ip, overall_bestA, nn);
            }
            if(ip->solMatrix[nn][overall_bestB] > -1){
                remove_job(ip, overall_bestB, nn);
            }
        }
        // Insert them back in the new positions:
        if(insert_job_at_position(ip, overall_bestA, overall_bestANurse, overall_bestAPos) < 0){
            printf("ERROR: Cannot insert the move found on local search!!!\n");
            exit(-34234523);
        }
        if(insert_job_at_position(ip, overall_bestB, overall_bestBNurse, overall_bestBPos) < 0){
            printf("ERROR: Cannot insert the move found on local search!!!\n");
            exit(-34234523);
        }
        if(DEBUG_PRINT > 0){
            double dcheckQual = sol_quality(ip, DEBUG_PRINT - 30);
            printf("DOUBLE CHECK: Real sol quality: %.2f, expected sol quality: %.2f\n", dcheckQual, overAllBestFitness);
        }
        if(DEBUG_PRINT > 0){
            printf("--- 1 FINISHED best_sync_double_switch ---");
        }
        return 1;
    }
    else{
        if(DEBUG_PRINT > 0){
            double dcheckQual = sol_quality(ip, DEBUG_PRINT - 30);
            printf(">    Not doing best_double_switch, started with %.2f and best was only %.2f    <\n", baseQuality, overAllBestFitness);
        }
        if(DEBUG_PRINT > 0){
            printf("--- -1 FINISHED best_sync_double_switch ---");
        }
        return -1;
    }

} // END OF best_sync_double_switch function.

int route_two_exchange(struct INSTANCE* ip, int firstImprovement){

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
    double iniQuality = sol_quality(ip, -12121);
    double currentQuality = iniQuality;

    for(int ni = 0; ni < ip->nNurses - 1; ++ni){ //For each nurse ni 0,...,nNurses-1
        for(int job_from_ni = 0; job_from_ni < ip->nJobs; ++job_from_ni){ //For each job_from_ni 0,...,nJobs
            if(ip->solMatrix[ni][job_from_ni] < 0){ //If job_from_ni is not in ni's route, go to next job_from_ni (++job_from_ni)
                continue;
            }
            secondNurseI = -1;
            if(ip->doubleService[job_from_ni]){ //If job_from_ni is a doubleService, find the other nurse (secondNurseI) that does the job with nurse ni.
                secondNurseI = find_second_nurse_ds(ip, job_from_ni, ni);
            }
            for(int nj = ni + 1; nj < ip->nNurses; ++nj){ //For each nurse nj = ni+1,..., nNurses
                if(nj==secondNurseI){ // nj is the same nurse as secondNurseI, so nurse nj is already doing the job, go to next nurse nj (++nj)
                    continue;
                }

                // Can nj do job_from_ni?
                if(ip->doubleService[job_from_ni]){ //If job_from_ni is a double service
                    if(check_skills_ds(ip, job_from_ni, secondNurseI, nj)
                            < 1){ // returns ip->capabilityOfDoubleServices[secondNurseI][nj][jobdsindex], <1 means nj is not skilled to do the job with secondNurseI
                        continue;
                    }
                }
                else if(check_skills(ip, job_from_ni, nj) < 1){ // If ip->nurseSkilled[nj][job_from_ni] <1, nurse nj is not skilled to do job_from_ni, go to next nurse nj (++nj)
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
                        secondNurseJ = find_second_nurse_ds(ip, job_from_nj, nj);
                    }

                    //NEW: FIXED BUG - need to check that secondNurseJ isn't actually the nurse ni that's already doing the job!
                    if(secondNurseJ==ni){
                        continue;
                    }

                    // Can ni do job_from_nj?
                    if(ip->doubleService[job_from_nj]){ //If job_from_nj is a double service
                        if(check_skills_ds(ip, job_from_nj, secondNurseJ, ni) < 1){ //If ni is not skilled to do job_from_nj with secondNurseJ, go to next job_from_nj
                            continue;
                        }
                    }
                    else if(check_skills(ip, job_from_nj, ni) < 1){ // If ip->nurseSkilled[ni][job_from_nj] < 1, nurse ni is not skilled to do job_from_nj, go to next job_from_nj
                        continue;
                    }

                    // If we are here, it's ok to swap job_from_i with job_from_j
                    exchange_jobs_in_route(ip, ni, job_from_ni, nj, job_from_nj); //put job_from_ni in nj's route in job_from_nj's position, and vice versa.

                    double tentQuality = sol_quality(ip, -123121); // quality of new solution after exchanging jobs
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
                    exchange_jobs_in_route(ip, ni, job_from_nj, nj, job_from_ni); // printf("\n\tExchange reversed. "); printf("Quality back to %.2f\n", sol_quality(ip, -123451));
                } //End for job_from_j loop
            } //End for nj loop
        } //End for job_from_i loop
    } //End for ni loop

    if(retVal > -1){
        exchange_jobs_in_route(ip, bestni, bestji, bestnj, bestjj);
        if(ip->verbose > 5){
            printf("\tThe best two exchange was (n%d, j%d) by (n%d, j%d)\n", bestni, bestji, bestnj, bestjj);
        }
    }
    else if(ip->verbose > 5){
        printf("\tNo exchange was good enough!\n");
    }

    return retVal;

} //END OF route_two_exchange function.

int exchange_jobs_in_route(struct INSTANCE* ip, int ni, int job_from_ni, int nj, int job_from_nj){

    //This function removes job_from_nj from nj's route and inserts job_from_ni into nj's route in the same position that job_from_nj was in, and
    //removes job_from_ni from ni's route and inserts job_from_nj into ni's route in the same position that job_from_ni was in.
    /// Maybe we should check ip->solMatrix[nj][job_from_ni] and ip->solMatrix[ni][job_from_nj] at the same time at the beginning of the function instead?

    //1. Put job_from_ni in nurse nj's route in the same position that job_from_nj was in, and remove job_from_nj from nj's route.
    int aux = ip->solMatrix[nj][job_from_ni]; //Position of job_from_ni in nj's route, should be =-1 as job_from_ni shouldn't be in nj's route!

    if(aux > -1){ // If job_from_ni is already in nj's route, then there's an error - we shouldn't do the swap.
        printf("\n---\n---\nWARNING: Exchanging a job maybe we should not????\n");
        printf("ip->solMatrix[%d][%d] = %d\n", nj, job_from_ni, aux);
        printf("\tTrying to exchange (n%d, j%d) with (n%d, j%d).\nSolmatrix:\n", ni, job_from_ni, nj, job_from_nj);
        print_solmatrix(ip);
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
        print_solmatrix(ip);
        exit(-123424);
    }

    //Set position of job_from_nj in ni's route to be the same position as job_from_ni in nurse ni's route.
    ip->solMatrix[ni][job_from_nj] = ip->solMatrix[ni][job_from_ni]; //e.g. if job_from_ni is in position 5 in nurse ni's route, now job_from_nj is set as position 5 in nurse ni's route.
    ip->solMatrix[ni][job_from_ni] = aux; //Then, set position of job_from_ni in nurse ni's route to be aux (which should be -1), removing job_from_ni from ni's route.

    return 0;

} //END OF exchange_jobs_in_route function.

int best_switch(struct INSTANCE* ip, int onlyInfeasible, double MAX_TIME){

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

    copy_int_matrix(ip->solMatrix, solMatrixInitial, nRows, nCols); //solMatrixInitial = solMatrix for all [i][j].
    double baseQuality = sol_quality(ip, -6);

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
                if(switch_nurse(ip, ni, nj, job_from_ni) > -1){ //switch_nurse attempts to remove job_from_ni from ni's route and insert it into nj's route. If successful, =0, else = -1.
                    double propQuality = sol_quality(ip, -7);
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


    free_matrix_int(solMatrixInitial, ip->nNurses);
    // printf("Finished best switch, with quality %d\n", baseQuality);

    // Finished, perform the best switch:
    if(besti > -1){
        // printf("Switched job %d from nurse %d to nurse %d.\n-------\n\n", best_job, besti, bestj);
        if(firstSwitch < 1){ //This is never true!
            switch_nurse(ip, besti, bestj, best_job);
        }
        return 0;
    }

    if(ip->verbose > 5){
        printf("Best switch could NOT find anything good\n");
        printf("besti = %d, bestj = %d,  best_job = %d, baseQuality=%.2f\n", besti, bestj, best_job, baseQuality);
        // exit(-698696969696);
    }

    return -1;

} //END OF best_switch function.

void free_matrix_int(int** matrix, int nRows){
    for(int i = 0; i < nRows; ++i){
        free(matrix[i]);
    }
    free(matrix);
}

int copy_int_matrix(int** source, int** destination, int nRows, int nCols){
    for(int i = 0; i < nRows; ++i)
        for(int j = 0; j < nCols; ++j)
            destination[i][j] = source[i][j];
    return 0;
}

int nurse_two_exchange(struct INSTANCE* ip){

    //This function swaps positions of pairs of nurses in nurseOrder array and evaluates solution quality
    //Returns 1 if solution has improved by swapping two nurses, otherwise returns -1.

    double cq = sol_quality(ip, -66643);

    for(int i = 0; i < ip->nNurses - 1; ++i){ //For all nurses i=0,...,nNurses-1
        for(int j = i + 1; j < ip->nNurses; ++j){ //For all nurses j=i+1,...,nNurses (so never swapping a nurse with itself)
            two_exchange(ip->nurseOrder, i, j); //Swap the order of nurses in indicies i and j in nurseOrder array: int t = array[j]; array[j]=array[i]; array[i] = t
            double tentQ = sol_quality(ip, -66644);
            if(tentQ > cq){ //If quality of solution with order of nurses i and j swapped is better than previous solution
                return 1; //successful, exit
            }
            else{ //If quality of solution is not better with order of nurses i and j swapped, then swap the nurses back.
                two_exchange(ip->nurseOrder, j, i);
            }
        }
    }
    return -1;

}

int two_opt_move(struct INSTANCE* ip, int ni, int pos1, int pos2){

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
    get_nurse_route(ip, ni, nurseRoute); //Fills array nurseRoute[position] = job for nurse ni's route

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

} //END OF two_opt_move function.

int find_second_nurse_ds(struct INSTANCE* ip, int job, int currentNurse){

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

int switch_nurse(struct INSTANCE* ip, int ni, int nj, int pi){

    //This function takes a job 'pi' out of nurse ni's route and attempts to put the job 'pi' into nurse nj's route.
    //If it's not possible to put 'pi' into nurse nj's route, then 'pi' is just put back into nurse ni's route in its original position.
    //Function returns -1 if no switch has occurred, and returns 0 if job 'pi' has been taken out of ni's route and put into nj's route successfully.

    /// Surely need to check that nurse ni and nurse nj are not the same nurse? Perhaps do this in best_switch function before this switch_nurse function is called?

    if(ip->solMatrix[ni][pi] < 0){ //Job pi is not assigned to nurse ni
        return -1; // Nothing to switch
    }

    if(ip->nurseSkilled[nj][pi] < 1 && ip->doubleService[pi] < 1){ //Nurse ni is not skilled to do job pi, and job pi is not a double service
        return -1; // Nothing to switch
    }

    if(ip->doubleService[pi] > 0){ // If job pi is a double service
        int secondNurse = find_second_nurse_ds(ip, pi, ni); //secondNurse = the other nurse that is assigned to job 'pi' along with nurse 'ni'.

        if(secondNurse==-1){ //If there is no other nurse that has been assigned to do job 'pi' with nurse 'ni'
            printf("ERROR: When attempting to switch job %d from nurse %d to nurse %d\n", pi, ni, nj);
            printf("-> Double service was not found in any other nurse.\n");
            exit(-1);
        }
        // If secondNurse and nurse 'nj' are not capable of doing the double service 'pi' together
        if(check_skills_ds(ip, pi, secondNurse, nj) < 1){ //returns ip->capabilityOfDoubleServices[secondNurse][nj][jobdsindex];
            return -1;
        }

    }
    int oldPos = ip->solMatrix[ni][pi]; //oldPos = position of job pi in nurse ni's route (in current solution before it gets modified below)

    remove_job(ip, pi, ni); //Remove job pi from nurse ni's route.

    if(best_job_insertion(ip, pi, nj) < 0){ //If job pi has NOT been inserted in nurse nj's route.
        insert_job_at_position(ip, pi, ni, oldPos); //Then insert job back into nurse ni's route in it's previous position.
        return -1;
    }

    //Otherwise, best_job_insertion(ip,pi,nj) >= 0 which means that job pi has been inserted into nurse nj's route successfully.

    return 0;
}

int repair(struct INSTANCE* ip, int ni){
    // Try to fix waiting times and missed time windows
    // Find what the nurse is doing:
    int* nurseRoute = malloc(ip->nJobs*sizeof(int));
    get_nurse_route(ip, ni, nurseRoute);

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
                currentTime += get_travel_time(ip, prevPoint, job);// ip->od[prevPoint][job];
            else
                currentTime += get_travel_time_from_depot(ip, ni, job);
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
        if(swap_points(ip, ni, ni, job, prevPoint) < 0)
            printf("Failed repairing tardiness\n");
        else
            return 0;
    }
    else{
        // printf("Didnt find tardiness.\n");
        // return -1;
    }

    if(waitingTime > 0 && routePos > 0 && routePos < (ip->nJobs - 1) && nurseRoute[routePos + 1] > 0){
        if(swap_points(ip, ni, ni, job, nurseRoute[routePos + 1]) < 0)
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

int get_job_count(struct INSTANCE* ip, int ni){

    //This function returns the number of jobs in nurse ni's route
    // +1 because if nurse has jobs in positons 0,1,2,3,4,5 then the highest position is 5 but the number of jobs is 6 (because of 0 index)

    int maxE = -1;
    for(int i = 0; i < ip->nJobs; ++i){
        if(ip->solMatrix[ni][i] > maxE)
            maxE = ip->solMatrix[ni][i];
    }
    return (maxE + 1);

} //END OF get_job_count function.

int get_nurse_job_count(struct INSTANCE* ip, int nurse){

    //This function returns the number of jobs that are in nurse's route

    int count = 0;
    for(int i = 0; i < ip->nJobs; i++){
        if(ip->solMatrix[nurse][i] > -0.5){
            count++;
        }
    }

    return count;

} //END OF get_nurse_job_count function.

void set_all_nurse_routes(struct INSTANCE* ip){
    // printf("Setting all -1\n");
    for(int ni = 0; ni < ip->nNurses; ni++){
        set_nurse_route(ip, ni);
    }

}

void set_nurse_route(struct INSTANCE* ip, int ni){
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

void access_nurse_route(struct INSTANCE* ip, int ni, int* nurseRoute){

    nurseRoute = ip->allNurseRoutes[ni];
    // printf("nuseRoute ACCESSED: value 0: %d, coming from %d\n", nurseRoute[0], ip->allNurseRoutes[ni][0]);
    // if (nurseRoute[0] > -1)
    // {
    // 	printf("Accessing...");
    // 	print_nurse_route(ip, ni, nurseRoute);
    // }

}

void get_nurse_route(struct INSTANCE* ip, int ni, int* nurseRoute){
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

void print_nurse_route(struct INSTANCE* ip, int ni, int* nurseRoute){

    printf("\n--\nNurse route for nurse: %d\n", ni);
    print_int_matrix_one(nurseRoute, 1, ip->nJobs);
    printf("\n--\n");
}

void old_set_nurse_time(struct INSTANCE* ip, int nursei){

    // This function goes through the route of 'nursei' from the first job (position 0) in nursei's route to the final job (last position) in nurse i's route,
    // including to and from the depot (nursei's home) and calculates the time that each job in nursei's route starts and finishes.
    // The function goes through a for loop through all job positions in nursei's route, and uses the previous job in the route to calculate times for the next job in the route.

    int prevPoint = -1;
    double tTime; // Travel time for nursei from depot (their house) to the job or from a previous job to the job
    double arriveAt = 0; //This is used to keep up the current time, and the time that nursei arrives at a job
    double currentTime = (double) ip->nurseWorkingTimes[nursei][0]; // The start time of nursei (for the whole day)

    ip->nurseWaitingTime[nursei] = 0; // Reset total waiting time for nursei to 0.
    ip->nurseTravelTime[nursei] = 0; // Reset total travel time for nursei to 0.

    // Reset all times for nursei for all jobs to -1. Recall that timeMatrix stores time at which each nurse i (row) does each job j (column). If i doesn't do job j, then =-1.
    for(int j = 0; j < ip->nJobs; ++j){
        ip->timeMatrix[nursei][j] = -1;
    }

    for(int j = 0; j < ip->nJobs; ++j){ // Main for loop of function, going through all POSITIONS j=0,...,nJobs.
        /**CHECK THIS IF STATEMENT: if j= 0 then yes, the nurse isn't being used at all, otherwise if j > 0 and allNurseRoutes < 0 it doesn't mean the nurse isn't being used at all, just that there's no job in position j! **/
        if(ip->allNurseRoutes[nursei][j] < 0){ // If there is no job in position j of nursei's route, exit for loop. This means that nursei is not being used, as if it was, it should have a job in the first position of the route.
            break;
        }
        int job = ip->allNurseRoutes[nursei][j]; // job= = the job in position j of nursei's route.

        if(prevPoint < -0.5){ //If this 'job' is the first job in nursei's route, then calculate the distance from nursei's home to the 'job'.
            tTime = get_travel_time_from_depot(ip, nursei, job); //get_travel_time_from_depot function returns ip->nurse_travel_from_depot[nursei][job].
            if(ip->exclude_nurse_travel){ // If we're NOT including the travel time from nursei's home to the first job to update the currentTime (i.e. we want the arrival time of the nurse to be the start of the job)
                if(ip->jobTimeInfo[job][0] > ip->nurseWorkingTimes[nursei][0] - 0.001){ // If the start time (start of time window) of 'job' is LATER than 'nursei's start timefor the day
                    currentTime = ip->jobTimeInfo[job][0]; // Set the current time in the route to be the start time (start of time window) of 'job' (rather than add the tTime it takes to travel to first job)
                    currentTime = currentTime + ip->tw_interval; // Add the time window interval so that the job starts on time rather than at the start of the time window. DEBUG : HARDCODED, NEEDS TO BE PARAMETER (DONE)
                }
                else{
                    // TODO: Start of nurse's day (shift) is LATER than the start of the time window, should we check for this?
                     currentTime = ip->nurseWorkingTimes[nursei][0]; // NEW: 22/05/2021
                }
            }
            else{
                currentTime = currentTime + tTime;
            }
        }
        else if(prevPoint > -0.5){ // If there was a job before this 'job', then calculate the distance from the previous job to this 'job'.
            tTime = get_travel_time(ip, prevPoint, job);  // get_travel_time function returns ip->od[prevPoint+1][job+1] (+1 because of extra row and column in od matrix).
            currentTime = currentTime + tTime; //The current time is set as the current time plus the time taken for nursei to travel to the job.
        }

        ip->nurseTravelTime[nursei] += tTime; // Updated nursei's travel time to include the time taken to travel from previous job/home to this 'job'

        arriveAt = currentTime;
        ip->timeMatrix[nursei][job] = currentTime; //Set time at which nursei does job 'job' to the current time.

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
                if(prevNurse==nursei){ // If prevNurse is nursei
                    considerDependency = -1; //do not consider dependency, break out of for loop.
                    break;
                }
                if(ip->timeMatrix[prevNurse][otherJob] > 0){ //If prevNurse has a time at which it does job 'otherJob'
                    if(aitOnly > 0){
                        ip->MK_mind[job] = abs(ip->MK_mind[job]);
                        ip->MK_maxd[job] = abs(ip->MK_maxd[job]);

                        // Latest allowed arrival is:
                        double laa = ip->timeMatrix[prevNurse][otherJob] - ip->MK_maxd[job];

                        //If latest allowed arrival is later than or equal to the start of the time window, AND the current time is earlier than or equal to the latest allowed arrival.
                        if((laa >= startTW) && (arriveAt <= laa)){
                            // Then we can reverse the order of these dependencies!
                            ip->MK_mind[job] = -1*ip->MK_mind[job];
                            ip->MK_maxd[job] = -1*ip->MK_maxd[job];
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->MK_mind[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->MK_maxd[job];
                    considerDependency = 1;
                    break;
                }
            }
        }

        if(ip->doubleService[job] > 0){ //If 'job' is a double service, i.e. doubleService[job] == 1
            // Check previous nurses, is anyone serving this patient already?
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){ // for each nurse index 0,...,nNurses
                int prevNurse = ip->nurseOrder[prevNurseInd]; //prevNurse = the number of the nurse in the nurseOrder array
                if(prevNurse==nursei){ // If prevNurse is nursei, break out of for loop.
                    break;
                }
                if(ip->timeMatrix[prevNurse][job] > 0){ //If the time at which prevNurse does 'job' is > 0, i.e. prevNurse is assigned to do 'job'.
                    startTWMK = ip->timeMatrix[prevNurse][job];// + ip->MK_mind[job];
                    endTWMK = ip->timeMatrix[prevNurse][job]; //  + ip->MK_maxd[job];
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
            ip->nurseWaitingTime[nursei] += waitingTime; // update nurseWaitingTime for nursei
            ip->timeMatrix[nursei][job] += waitingTime; //update timeMatrix for nursei and 'job' so that the time nursei actually starts 'job' isn't just the arrival time, it's after the waiting time.
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
        ip->nurseTravelTime[nursei] += get_travel_time_to_depot(ip, nursei, prevPoint); //get_travel_time_to_depot function returns ip->nurse_travel_to_depot[nursei][prevPoint].
    }

} //END OF set_nurse_time function

void set_nurse_time(struct INSTANCE* ip, int nursei){

    /** NEW_SET_NURSE_TIME FUNCTION **/
    int report = -1;
    /*if(nursei == 0){
        report = 1;
    }*/
    //printf("Here set_nurse_time, nursei: %d.\n", nursei);
    if(report == 1){
        printf("Start, nursei : %d\t", nursei);
    }
    int prevPoint = -1; // previous job
    double tTime; // travel time
    double arriveAt = 0; // Keep up current time, and the time nursei arrives at a job
    double currentTime = (double) ip->nurseWorkingTimes[nursei][0]; // The start time of nursei (for the whole day)
    if(report == 1){
        printf("currentTime: %.2f\t", currentTime);
    }
    ip->nurseWaitingTime[nursei] = 0; // Reset waiting time for nursei
    ip->nurseTravelTime[nursei] = 0; // Reset travel time for nursei
    if(report == 1){
        printf("After waitingTime and travelTime\t");
    }

    // Reset timeMatrix for nursei (time at which nursei does job j - if nursei does not do job j then timeMatrix[nursei][j] = -1)
    for(int j = 0; j < ip->nJobs; ++j){
        ip->timeMatrix[nursei][j] = -1;
    }
    if(report == 1){
        printf("after timeMatrix\n");
    }

    // Determine the number of unavailable 'shifts' for nursei
    if(report == 1){
        for(int i = 0; i < ip->nNurses; ++i){
            printf("%d, ", ip->nurseUnavail[i]);
        }
        printf("\n");
    }
    int numUnavail = ip->nurseUnavail[nursei];
    if(report == 1){
        printf("numUnavail: %d\n", numUnavail);
    }

    // Main for loop:
    for(int j = 0; j < ip->nJobs; ++j){ // Main for loop of function, going through all POSITIONS j=0,...,nJobs.
        if(ip->allNurseRoutes[nursei][j] < 0){ // If there is no job in position j of nursei's route, then we have reached the final position of nursei, break out of for loop.
            break;
        }

        int job = ip->allNurseRoutes[nursei][j]; // job = the job in position j of nursei's route.
        //printf("nursei: %d, j: %d, job: %d.\n", nursei, j, job);
        // NB: PART ONE: determine 'currentTime' and 'arriveAt', which is the time the nurse is at the location for 'job'
        if(prevPoint < -0.5){ // 'job' is the first job in nursei's route - there is no previous job, so prevPoint = -1.
            tTime = get_travel_time_from_depot(ip, nursei, job); // get_travel_time_from_depot = ip->nurse_travel_from_depot[nursei][job]
            if(ip->exclude_nurse_travel){ // Exclude tTime (from nursei's home to this first job) when updating currentTime, but still include tTime when updating ip->nurseTravelTime[nursei]
                if(ip->jobTimeInfo[job][0] > ip->nurseWorkingTimes[nursei][0] - 0.001){ // If the start TW of 'job' is LATER than the start time of nursei's shift for the day
                    currentTime = ip->jobTimeInfo[job][0]; // currentTime is updated to be the start of the TW for 'job'
                    currentTime = currentTime + ip->tw_interval; // currentTime is updated to be the actual start time of the job (this is done by moving the currentTime forward by the TW interval)
                }
                else{ // TODO: If the start time of nursei's day is LATER than the start of the TW for 'job', should we check for this?
                    currentTime = ip->nurseWorkingTimes[nursei][0];
                }
            }
            else{ // Include tTime (from nursei's home to 'job') when updating currentTime and when updating ip->nurseTravelTime[nursei]
                currentTime = currentTime + tTime; // currentTime is updated to be the time that nursei arrives at 'job'
            }
        }
        else if(prevPoint > -0.5){ // 'job' is NOT the first job in nursei's route
            tTime = get_travel_time(ip, prevPoint, job); // get_travel_time = ip->od[prevPoint+1][job+1]
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
                if(prevNurse==nursei){
                    considerDependency = -1;
                    break;
                }
                if(ip->timeMatrix[prevNurse][otherJob] > 0){
                    if(aitOnly > 0){
                        ip->MK_mind[job] = abs(ip->MK_mind[job]);
                        ip->MK_maxd[job] = abs(ip->MK_maxd[job]);

                        double laa = ip->timeMatrix[prevNurse][otherJob] - ip->MK_maxd[job];
                        if((laa >= startTW) && (arriveAt <= laa)){ //NB: arrive at is used here!
                            ip->MK_mind[job] = -1*ip->MK_mind[job];
                            ip->MK_maxd[job] = -1*ip->MK_maxd[job];
                        }
                    }
                    startTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->MK_mind[job];
                    endTWMK = ip->timeMatrix[prevNurse][otherJob] + ip->MK_maxd[job];
                    considerDependency = 1;
                    break;
                }
            }
        }

        // Double service jobs:
        if(ip->doubleService[job] > 0){
            for(int prevNurseInd = 0; prevNurseInd < ip->nNurses; ++prevNurseInd){
                int prevNurse = ip->nurseOrder[prevNurseInd];
                if(prevNurse==nursei){
                    break;
                }
                if(ip->timeMatrix[prevNurse][job] > 0){
                    startTWMK = ip->timeMatrix[prevNurse][job];
                    endTWMK = ip->timeMatrix[prevNurse][job];
                    break;
                }
            }
        }

        double waitingTime = 0;
        double tardiness = 0;

        // NB: PART TWO: Update arriveAt to be after waitingTime (if there is any) and calculate tardiness.
        //double waitingTime = 0;
        double worstStart = startTWMK;
        if(startTWMK < startTW){
            worstStart = startTW; //worstStart takes the latest start time window.
        }

        if(arriveAt < worstStart){ // arriveAt is EARLIER than the latest start time
            waitingTime = worstStart - arriveAt; // waiting time is incurred
            ip->nurseWaitingTime[nursei] += waitingTime;
            ip->timeMatrix[nursei][job] += waitingTime; // Update timeMatrix for nursei and 'job' so that the time nursei actually starts 'job' isn't just the arrival time, it's after the waiting time.
            arriveAt += waitingTime; // Update arriveAt to be the start time of the actual job (start of time window)
        }

        if(arriveAt > endTWMK){ // Job starts late: arriveAt is LATER than the end of the time window
            ip->violatedTWMK[job] += arriveAt - endTWMK; //Gap between end of time window and time nurse arrives to the job.
        }

        //double tardiness = 0;
        if(arriveAt > endTW){ // Job starts late: arriveAt is LATER than the end of the time window
            tardiness = arriveAt - endTW; //tardiness is how late the nurse is to the job, i.e. how long after endTW does the nurse arrive.
        }
        ip->violatedTW[job] += tardiness; // Updated violated TW for 'job'


        if(numUnavail > 0){
            for(int i = 0; i < numUnavail; ++i){
                // If currentTime is BEFORE the start of unavailable shift and 'job' ENDS after the start of the unavailable shift (so finished either during or after unavailable shift), then we need to move the
                // job so that it starts after the unavailable shift ends.
                if(arriveAt < ip->unavailMatrix[i][1][nursei] && arriveAt + ip->jobTimeInfo[job][2] > ip->unavailMatrix[i][1][nursei]){
                    waitingTime = ip->unavailMatrix[i][1][nursei] - arriveAt; //waiting time = time from currentTime to the start of the unavailable shift
                    ip->nurseWaitingTime[nursei] += waitingTime;
                    arriveAt = ip->unavailMatrix[i][2][nursei]; // update currentTime to be the end of the unavailable shift.
                    ip->timeMatrix[nursei][job] = arriveAt;
                    tardiness = arriveAt - endTW; // tardiness is how late the nurse is to the job, so how long after the end of the job TW does the nurse start the job.
                    ip->violatedTW[job] = tardiness; // Updated violated TW for 'job'
                    //break; //don't check for further unavailable shifts (may need to change this in case there is another unavailable shift that the job would violate)
                }
                // Else if currentTime is at or AFTER the start of an unavailable shift (and before the end of the unavailable shift) (and could end within or after the unavailable shift ends (doesn't matter)),
                // then we need to move the job so that it starts after the unavailable shift ends.
                else if(arriveAt >= ip->unavailMatrix[i][1][nursei] && arriveAt < ip->unavailMatrix[i][2][nursei]){
                    // no waiting time
                    arriveAt = ip->unavailMatrix[i][2][nursei]; // update currentTime to be the end of the unavailable shift.
                    ip->timeMatrix[nursei][job] = arriveAt;
                    tardiness = arriveAt - endTW; //tardiness is how late the nurse is to the job, so how long after the end of the job TW does the nurse start the job.
                    ip->violatedTW[job] = tardiness; // Updated violated TW for 'job'
                    //break;
                }
            }
        }

        // NB: update current time to be the time after the waiting time (if any) and after the duration of the job, so currenTime is after the job has finished.
        prevPoint = job; // previous job is now set to the current job
        //currentTime = currentTime + ip->jobTimeInfo[job][2] + waitingTime; //current time = current time + time length of 'job' + waiting time, i.e. the current time is the time now after the job has been completed.
        currentTime = arriveAt + ip->jobTimeInfo[job][2]; //current time = arriveAt (which includes waitingTime) + time length of 'job', i.e. the current time is the time now after the job has been completed.

    } //End of for loop (j = 0; j < ip->nJobs; ++j)

    // Return to depot:
    if(prevPoint > -1){ // If a previous job is set, calculate the time it takes for the nurse to go from the last job (prevPoint) back to the depot (their home).
        ip->nurseTravelTime[nursei] += get_travel_time_to_depot(ip, nursei, prevPoint); //get_travel_time_to_depot function returns ip->nurse_travel_to_depot[nursei][prevPoint].
    }

    //printf("End set_nurse_time.\n");
} //END OF new_set_nurse_time function

void set_times_full(struct INSTANCE* ip){
    for(int i = 0; i < ip->nJobs; ++i){
        ip->violatedTW[i] = 0;
        ip->violatedTWMK[i] = 0;
    }
    for(int j = 0; j < ip->nNurses; ++j){
        int nurse = ip->nurseOrder[j];
        // printf("Setting nurse time of: %d", nurse);
        set_nurse_time(ip, nurse);
    }
}

void set_times_from(struct INSTANCE* ip, int first_nurse){

    int has_appeared = -1;
    int nurse = -1;
    int jobdue = -1;

    for(int i = 0; i < ip->nNurses; ++i){ //For each nurse i = 0,...,nNurses
        nurse = ip->nurseOrder[i]; //nurse = nurse i in nurseOrder array
        if(has_appeared < 0){ //If we have not yet assessed first_nurse
            if(nurse==first_nurse){ //if the current nurse we are assessing is also first_nurse (nurse ni)
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

        set_nurse_time(ip, nurse); //Note that this set_nurse_time function also checks to make sure that nurse is being used.
    }
}

int synchronise_job_i(struct INSTANCE* ip, int job, int nurse1, int nurse2){
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
    get_nurse_route(ip, nurse1, ip->nurseRoute);
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

double sol_quality_global(struct INSTANCE* ip, int report){
    /*
		Recalculate the full solution quality.
		Should be avoided if the change only affects
		to 1 or 2 nurses. See sol_quality_optimised below
	*/
    // Set all nurse routes, as these get used multiple times in this function
    set_all_nurse_routes(ip);
    set_times_full(ip);

    return -1;
}

double sol_quality_light(struct INSTANCE* ip){
    /*
		Same as sol_quality, but assumes that nurse routes
		are already correct, and are not recalculated
	*/
    double TOL = 0.0001;
    set_times_full(ip);
    // printf("\n\n-----------------------vvvvvvvvvvvvvvv-----------------------\n");

    // printf("\n\n----------------------- Before recalc -----------------------\n");
    // obj_from_times(ip, 12);
    // print_int_matrix(ip->allNurseRoutes, ip->nNurses, ip->nJobs);

    double light_quality = obj_from_times(ip, -1);
    // printf("\n\n-----------------------|||||||||||||||-----------------------\n");

    // double real_quality = sol_quality(ip, -1);
    // printf("\n\n----------------------- After recalc -----------------------\n");
    // obj_from_times(ip, 21);
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
    return (light_quality);
}

double sol_quality_optimised(struct INSTANCE* ip, int n1, int n2){
    printf("\nUSING DEPRECATED FUNCTION sol_quality_optimised, EXIT!!!!\n");
    exit(-22222222);
    /*
		Calculate the quality of the solution based only
		on a change in nurses n1 and n2
	*/
    set_nurse_route(ip, n1);

    if((n2 > -1) && (n1!=n2)){
        set_nurse_route(ip, n2);
        // n1 = min(n1, n2);
        if(n2 < n1)
            n1 = n2;
    }

    set_times_from(ip, n1);

    return obj_from_times(ip, -1);
}

double sol_quality(struct INSTANCE* ip, int report){
    if(report > 0){
        printf("-------- sol_quality(ip, %d) --------\n\n", report);
    }

    // printf("\n\n********\nStarted sol_quality():\nSetting times...\n");

    if (report == -98765){
        printf("Initial allNurseRoutes:\n");
        print_allNurseRoutes(ip);
    }

// printf("Solmatrix when starting sol_quality\n");
// print_solmatrix(ip);

    // Set all nurse routes, as these get used multiple times in this function
    set_all_nurse_routes(ip);
    if (report == -98765){
        printf("After set_all_nurse_routes:\n");
        print_allNurseRoutes(ip);
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
    set_times_full(ip);
    // if (report > 0)
    // 	printf("Done.\nSynchronising jobs...\n");
    // synchronise_jobs(ip);
    // if (report > 0)
    // 	printf("Done.\nReporting quality...\n");

    double quality = obj_from_times(ip, report);

    if(report > 0){
        printf("Nurse order: ( ");
        for(int i = 0; i < ip->nNurses; ++i){
            printf("%d,  ", ip->nurseOrder[i]);
        }
        printf(" )\n");
        // printf("Done.\nFinished. sol_quality().\n********\n\n");
        printf("-------- finished. sol_quality(ip, %d) --------\n\n", report);
    }
    // exit(-32423452359238749);
    // printf("\nsol_quality() = %.2f", quality);

    return quality;
}

int synchronise_jobs(struct INSTANCE* ip){
    // int retVal = -1;
    double TOL = 0.01;
    int MAX_CHECKS = 2*ip->nJobs;
    int checks = 0;
    for(int job = 1; job < ip->nJobs; ++job){
        if(ip->doubleService[job] < 0)
            continue;

        checks++;
        if(checks > MAX_CHECKS){
            printf("ERROR: A loop in synchronise_jobs() went too far, exit!\n");
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
            synchronise_job_i(ip, job, nurse1, nurse2);
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

int* mins_to_time(double time){
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

int* mins_to_minsecs(double time){
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

double obj_from_times(struct INSTANCE* ip, int report){
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
    double totalTravel = 0;
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
    ip->totPref = 0;

    for(int i = 0; i < ip->nNurses; ++i){ // For each index i in nurseOrder
        int ni = ip->nurseOrder[i]; //ni - nurse[i] in nurseOrder
        totalTravel += ip->nurseTravelTime[ni]; //Add (total) travel time for nurse ni to total travel time (for all nurses).
        totalWaitingTime += ip->nurseWaitingTime[ni]; // Add (total) waiting time for nurse ni to total waiting time (for all nurses)
        if(report > 0){
            int* timeNWTStart;
            timeNWTStart = mins_to_time((double) (ip->nurseWorkingTimes[ni][0]));
            // printf("Nurse %d. Start time: %.2f\t", ni, (double)(ip->nurseWorkingTimes[ni][0]));
            printf("Nurse %d. Start time: %d:%d:%d, ", ni, *timeNWTStart, *(timeNWTStart + 1), *(timeNWTStart + 2));
            int* timeNWTEnd;
            timeNWTEnd = mins_to_time((double) (ip->nurseWorkingTimes[ni][1]));
            printf("End time: %d:%d:%d, ", *timeNWTEnd, *(timeNWTEnd + 1), *(timeNWTEnd + 2));
            int* timeNWTShift;
            timeNWTShift = mins_to_time((double) (ip->nurseWorkingTimes[ni][2]));
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
                printf("\tEmpty route for nurse %d, setting spare time to: %.2f\n\n", ni, sparetime);
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
                    lastPosition = ip->allNurseRoutes[ni][p
                            - 1]; //last position = job at the last position of ni's route (p-1 since job = -1 so p is out of bounds, need to go to previous position for last job)
                }
                else{ //position = 0, so there is no job before in the route.
                    lastPosition = -1;
                }
                // printf("\t>> That was the last job for nurse %d (lastPosition = %d).\n", ni, lastPosition);
                break;
            }

            ip->totPref += ip->prefScore[job][ni];
            arriveAt = ip->timeMatrix[ni][job];
            totalMKTardiness += ip->violatedTWMK[job];

            if(ip->doubleService[job] < 0.5){
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
                if(ip->doubleService[job] > 0){
                    int* timeArriveDS;
                    timeArriveDS = mins_to_time(arriveAt);
                    // printf("\tJob %d (DS) arrives at %.2f, ", job, arriveAt);
                    printf("\tJob %d (DS) arrives at %d:%d:%d, ", job, *timeArriveDS, *(timeArriveDS + 1), *(timeArriveDS + 2));
                }
                else{
                    int* timeArrive;
                    timeArrive = mins_to_time(arriveAt);
                    // printf("\tJob %d arrives at %.2f, ", job, arriveAt);
                    printf("\tJob %d arrives at %d:%d:%d, ", job, *timeArrive, *(timeArrive + 1), *(timeArrive + 2));
                }
                int* timeTWStart;
                timeTWStart = mins_to_time((double) (ip->jobTimeInfo[job][0]));
                // printf("prefScore: %.2f, [TW %d - %d], ", ip->prefScore[job][ni], ip->jobTimeInfo[job][0], ip->jobTimeInfo[job][1]);
                printf("prefScore: %.2f, [TW %d:%d:%d", ip->prefScore[job][ni], *timeTWStart, *(timeTWStart + 1), *(timeTWStart + 2));
                int* timeTWEnd;
                timeTWEnd = mins_to_time((double) (ip->jobTimeInfo[job][1]));
                printf(" - %d:%d:%d], ", *timeTWEnd, *(timeTWEnd + 1), *(timeTWEnd + 2));

                if(ip->dependsOn[job] > -1){
                    printf("(Dep %d [%d - %d]) ", ip->dependsOn[job], ip->MK_mind[job], ip->MK_maxd[job]);
                }

                double readyToNext = arriveAt + (double) ip->jobTimeInfo[job][2];
                int* timeDuration;
                timeDuration = mins_to_time((double) (ip->jobTimeInfo[job][2]));
                // printf("duration(ST): %.d ", ip->jobTimeInfo[job][2]);
                printf("duration(ST): %d:%d:%d", *timeDuration, *(timeDuration + 1), *(timeDuration + 2));
                if((p < ip->nJobs - 1) && (ip->allNurseRoutes[ni][p + 1] > -1)){
                    int* timeTR;
                    double travelTime = get_travel_time(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]);
                    timeTR = mins_to_minsecs(travelTime);
                    // printf("travel(TR): %.2f ", get_travel_time(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]));
                    printf(", travel(TR): %d:%d ", *timeTR, *(timeTR + 1));
                    readyToNext += get_travel_time(ip, ip->allNurseRoutes[ni][p], ip->allNurseRoutes[ni][p + 1]);
                }
                int* timeRTN;
                timeRTN = mins_to_time(readyToNext);
                // printf("[ready at: %.2f]", readyToNext);
                printf("[ready at: %d:%d:%d]", *timeRTN, *(timeRTN + 1), *(timeRTN + 2));

                //printf("\n");
                if(ip->violatedTW[job] > 0.1){
                    int* timeViolatedTW;
                    timeViolatedTW = mins_to_minsecs(ip->violatedTW[job]);
                    // printf(" *** Misses TW by %.2f ***", ip->violatedTW[job]);
                    printf(" *** Misses TW by %d:%d ***", *timeViolatedTW, *(timeViolatedTW + 1));
                }

                if(ip->violatedTWMK[job] > 0.1){
                    //printf("\t *** Misses MK TW by %.2f (Mind %d, Maxd %d)***\n", ip->violatedTWMK[job], ip->MK_mind[job], ip->MK_maxd[job]);
                    printf(" *** Misses MK TW by %.2f (Mind %d, Maxd %d)***", ip->violatedTWMK[job], ip->MK_mind[job], ip->MK_maxd[job]);
                }
                printf("\n");
            }
        } // End for (int p = 0; p < ip->nJobs + 1; ++p) (positions), end analysing route of nurse ni

        if(lastPosition > -0.5){ // If the nurse went somewhere...
            // // printf("\t>> Adding return to depot from job %d.\n", lastPosition);

            // CORRECT:
            // tTime = ip->jobTimeInfo[lastPosition][2] + get_travel_time(ip, lastPosition, -1);
            // finishTime = (double)(ip->timeMatrix[ni][lastPosition] + tTime);
            // dayWork = (double)finishTime - (double)ip->nurseWorkingTimes[ni][0];

            // printf("\t>> Adding return to depot from job %d.\n", lastPosition);
            tTime = (double) get_travel_time_to_depot(ip, ni, lastPosition);
            // tTime = (double) get_travel_time(ip, lastPosition, -1);
            tTime += (double) ip->jobTimeInfo[lastPosition][2];
            // printf("Correct tTime: %.2f\n", tTime);
            // tTime += ip->timeMatrix[ni][lastPosition];
            // printf("Incorrect tTime %.2f\n", tTime);
            // totalTime -= ip->nurseWorkingTimes[ni][0]; // Remove start time
            finishTime = (double) (ip->timeMatrix[ni][lastPosition] + tTime);
            dayWork = (double) finishTime - (double) ip->nurseWorkingTimes[ni][0];
            /*if (shortestDay > finishTime) {
                shortestDay = finishTime;
            }*/
            if(shortestDay > dayWork){
                shortestDay = dayWork;
            }
            //dayWork = (double)finishTime - (double)ip->nurseWorkingTimes[ni][0];
        }

        overtime = finishTime - (double) ip->nurseWorkingTimes[ni][1];
        sparetime = max(0.0, -1*overtime);
        totalTime += dayWork;

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
            timeFinish = mins_to_time(finishTime);
            // printf("Finishes: %.2f Overtime: %.2f Waiting time: %.2f\n", finishTime, overtime, ip->nurseWaitingTime[ni]);
            printf("Finish time: %d:%d:%d, ", *timeFinish, *(timeFinish + 1), *(timeFinish + 2));
            if(overtime < 0){
                double overtimePositive = overtime*-1;
                int* timeOTPositive;
                timeOTPositive = mins_to_time(overtimePositive);
                printf("Overtime: -%d:%d:%d, ", *timeOTPositive, *(timeOTPositive + 1), *(timeOTPositive + 2)); // Negative overtime, put '-' in front of time.
            }
            else if(overtime > 0){ // Postive overtime.
                int* timeOT;
                timeOT = mins_to_time(overtime);
                printf("Overtime: %d:%d:%d, ", *timeOT, *(timeOT + 1), *(timeOT + 2));
            }
            else{
                printf("Overtime: %.2f, ", overtime);
            }
            int* timeWaiting;
            timeWaiting = mins_to_time(ip->nurseWaitingTime[ni]);
            printf("Waiting time: %d:%d:%d\n\n", *timeWaiting, *(timeWaiting + 1), *(timeWaiting + 2));
        }

    } // End for (int i = 0; i < ip->nNurses; ++i) loop

    /*
		Mankowska objective: Z = A*D + B*T + C*Tmax
		A = B = C = 1/3 (User set weights)
		D = Total distance travelled by caregivers (OD-matrix)
		T = Total tardiness
		Tmax = Max tardiness
		double quality = (totalTime + totalTardiness + maxTardiness)/3;
	*/
    // double quality = -1*totalTravel -10000000000000*totalTardiness; // TSP-TW
    double TOL = 1e-4;
    double mk_allowed_tardiness = totalOvertime + totalTardiness;
    // double mk_allowed_tardiness = totalTardiness;
    double mk_max_tardiness = maxTardiness;
    if(maxOvertime > mk_max_tardiness){
        mk_max_tardiness = maxOvertime;
    }

    double quality = 0.0;
    double ait_quality = 0.0;
    double mk_quality = 0.0;
    double wb_quality = 0.0;
    double paper_quality = 0.0;
    int qualityType = ip->quality_measure;
    double infeasibility_M1 = ip->nNurses*12*60; // Multiplier
    double infeasibility_M2 = ip->nNurses*12*60; // Chunk sum

    //Ait H.
    ait_quality = -1*(0.3*totalTravel + ip->totPref); // Real obj
    // Avoid infeasibility:
    ait_quality += -10000*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
        ait_quality += -10000;
    }

    // Mankowska
    mk_quality = -1*(totalTravel + mk_allowed_tardiness + mk_max_tardiness)/3; // Mankowska
    if(totalMKTardiness > TOL){
        mk_quality += -1000 - 10000*(totalMKTardiness);
    }

    // Workload Balance
    double dayDiff = (longestDay - shortestDay);
    wb_quality = -1*(0.3*totalTravel + ip->totPref) - 0.1*dayDiff + 0.1*minSpareTime; // Real obj
    // Avoid infeasibility:
    wb_quality += -10000*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
    if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
        wb_quality += -10000;
    }

    // Paper
    paper_quality = ip->algorithmOptions[51]*totalTravel +
            ip->algorithmOptions[52]*totalWaitingTime +
            ip->algorithmOptions[53]*mk_allowed_tardiness +
            ip->algorithmOptions[54]*totalOvertime +
            ip->algorithmOptions[55]*minSpareTime +
            ip->algorithmOptions[56]*ip->totPref +
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

    if(qualityType == 0){
        quality = ait_quality;
    }
    else if(qualityType == 1){
        quality = mk_quality;
    }
    else if(qualityType == 5){
        quality = wb_quality;
    }
    else if(qualityType == 6){
        quality = paper_quality;
    }


    /*if(qualityType==1){ // Mankowska
        if(report > 0){
            printf("\nUsing Mankowska et al. as quality measure.\n");
        }
        // Mankowska
        quality = -1*(totalTravel + mk_allowed_tardiness + mk_max_tardiness)/3; // Mankowska
        if(totalMKTardiness > TOL){
            quality += -1000 - 10000*(totalMKTardiness);
        }
    }
    else if(qualityType==99){ // Constructive algorithm
        quality = -1*(totalTime + ip->totPref);
        // Avoid infeasibility:
        quality += -10000*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);

        if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
            quality += -10000;
            feasible = -1;
        }
    }*/

    /*if(qualityType==6){ // Paper
        // To get rid of infeasibilities, assume everybody has a twelve hour shift
        double infeasibility_M1 = ip->nNurses*12*60; // Multiplier
        double infeasibility_M2 = ip->nNurses*12*60; // Chunk sum


        double travelMinutes = totalTravel/60.0;
        double totalWaitingMinutes = totalWaitingTime/60.0;
        double minSpareTimeMinutes = minSpareTime/60.0;

        double TWTardinessMinutes = mk_allowed_tardiness/60.0;
        double TWMaxTardinessMinutes = mk_max_tardiness/60.0;
        double infeasibleTardinessMinutes = totalMKTardiness/60.0;
        double overtimeMinutes = totalOvertime/60.0;


        // a1 1 Travel time
        // a2 1 Waiting time
        // a3 5 Tardiness
        // a4 5 Overtime
        // a5 0.5 Workload balance
        // a6 1 Preference score
        quality = ip->algorithmOptions[51]*totalTravel +
                ip->algorithmOptions[52]*totalWaitingTime +
                ip->algorithmOptions[53]*mk_allowed_tardiness +
                ip->algorithmOptions[54]*totalOvertime +
                ip->algorithmOptions[55]*minSpareTime +
                ip->algorithmOptions[56]*ip->totPref +
                ip->algorithmOptions[57]*mk_max_tardiness;

        *//*quality = ip->algorithmOptions[51]*travelMinutes +
                ip->algorithmOptions[52]*totalWaitingMinutes +
                ip->algorithmOptions[53]*TWTardinessMinutes +
                ip->algorithmOptions[54]*overtimeMinutes +
                ip->algorithmOptions[55]*minSpareTimeMinutes +
                ip->algorithmOptions[56]*ip->totPref +
                ip->algorithmOptions[57]*TWMaxTardinessMinutes;*//*

        // Avoid infeasibility:
        int feasible = 1;
        if(ip->algorithmOptions[50] > 0.5){
            // No tardiness of any type is allowed (including overtime)
            if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
                quality += -infeasibility_M1*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
                quality += -infeasibility_M2;
                feasible = -1;
            }
        }
        else{
            // Tardiness and overtime are allowed, but infeasibleTardiness is not (double ups, gaps)
            if(totalMKTardiness > TOL){
                quality += -infeasibility_M1*totalMKTardiness;
                quality += -infeasibility_M2;
                feasible = -1;
            }
        }
        *//*if(ip->algorithmOptions[50] > 0.5){
            // No tardiness of any type is allowed (including overtime)
            if((infeasibleTardinessMinutes > TOL) || (TWTardinessMinutes > TOL) || (overtimeMinutes > TOL)){
                quality += -infeasibility_M1*(infeasibleTardinessMinutes + TWTardinessMinutes + overtimeMinutes);
                quality += -infeasibility_M2;
                feasible = -1;
            }
        }
        else{
            // Tardiness and overtime are allowed, but infeasibleTardiness is not (double ups, gaps)
            if(infeasibleTardinessMinutes > TOL){
                quality += -infeasibility_M1*infeasibleTardinessMinutes;
                quality += -infeasibility_M2;
                feasible = -1;
            }
        }*//*
    }*/

    /*if(qualityType==5){ // Work_balance
        double dayDiff = (longestDay - shortestDay);
        quality = -1*(0.3*totalTravel + ip->totPref) - 0.1*dayDiff + 0.1*minSpareTime; // Real obj
        // Avoid infeasibility:
        quality += -10000*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
        int feasible = 1;
        if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
            quality += -10000;
            feasible = -1;
        }
    }*/

    /*if(qualityType==0){ // Ait H.
        quality = -1*(0.3*totalTravel + ip->totPref); // Real obj
        // Avoid infeasibility:
        quality += -10000*(totalMKTardiness + mk_allowed_tardiness + totalOvertime);
        int feasible = 1;
        if((totalMKTardiness > TOL) || (mk_allowed_tardiness > TOL) || (totalOvertime > TOL)){
            quality += -10000;
            feasible = -1;
        }

        if(report > 0){
            printf("\nUsing Ait Haddadene et al. as quality measure.");        // GRASP x ILS
            if(feasible > 0){
                printf("\n\tFeasible.");
            }
            else{
                printf("\n\t >< INFEASIBLE ><");
            }
            printf("\n\tTravel %.2f ", totalTravel); // GRASP x ILS
            printf("\n\tPref %.2f ", ip->totPref); // GRASP x ILS
            printf("\n\tQuality = %.2f + %.2f = %.2f\n\n", totalTravel*0.3, ip->totPref, -1*quality); // GRASP x ILS
        }
    }*/

    /*if(report > 0){
        printf("Used quality: %2.f\n", quality);
        printf("Mankowska values:\n");
        printf("\ttotalTravel = %.2f", totalTravel);
        printf("\ttotPref = %.4f", ip->totPref);
        printf("\tmk_allowed_tardiness = %.2f\t", mk_allowed_tardiness);
        printf("\tmk_max_tardiness = %.2f\n", mk_max_tardiness);
        printf("totalTardiness: %.2f\t", totalTardiness);
        printf("totalMKTardiness: %.2f\t", totalMKTardiness);
        printf("totalOvertime: %.2f\n", totalOvertime);
        printf("totalWaitingTime: %.2f\n", totalWaitingTime);
        printf("maxSpareTime: %.2f\t", maxSpareTime);
        printf("minSpareTime: %.2f\t", minSpareTime);
        printf("totalTime: %.2f\t", totalTime);
        printf("Mankowska obj: %.2f\n", (totalTravel + totalTardiness + maxTardiness)/3);
        printf("\n------------------------\n");
        //double dayDiff = pow(longestDay - shortestDay, 2);
        double dayDiff = longestDay - shortestDay;
        printf("shortestDay = %.2f\n", shortestDay);
        printf("longestDay = %.2f\n", longestDay);
        printf("dayDiff = %.2f\n", dayDiff);
        printf("minSpareTime = %.2f\n", minSpareTime);
        printf("maxSpareTime = %.2f\n", maxSpareTime);
        printf("\n------------------------\n");
    }*/

    if(report==12345){
        double infeasibility_M1 = ip->nNurses*12*60; // Multiplier
        double infeasibility_M2 = ip->nNurses*12*60; // Chunk sum
        double dayDiff = longestDay - shortestDay;

        if(qualityType == 0){
            printf("QUALITY MEASURE: Ait. H (%d)\n", qualityType);
        }
        else if(qualityType == 1){
            printf("QUALITY MEASURE: Mankowska (%d)\n", qualityType);
        }
        else if(qualityType == 5){
            printf("QUALITY MEASURE: Workload Balance (%d)\n", qualityType);
        }
        else if(qualityType == 6){
            printf("QUALITY MEASURE: Paper (%d)\n", qualityType);
        }
        else{
            printf("QUALITY MEASURE:  No specific type given (%d)\n", qualityType);
        }
        printf("---------------------------------\n");
        printf("ALG OPTIONS:\n");
        printf("algOp[50] = %.2f (if 1, tardiness/overtime are infeasible)\n", ip->algorithmOptions[50]);
        printf("alpha1 [51] (totalTravel) = %.2f\n", ip->algorithmOptions[51]);
        printf("alpha2 [52] (totalWaitingTime) = %.2f\n", ip->algorithmOptions[52]);
        printf("alpha3 [53] (mk_allowed_tardiness) = %.2f\n", ip->algorithmOptions[53]);
        printf("alpha4 [54] (totalOvertime) = %.2f\n", ip->algorithmOptions[54]);
        printf("alpha5 [55] (minSpareTime) = %.2f\n", ip->algorithmOptions[55]);
        printf("alpha6 [56] (totPref) = %.2f\n", ip->algorithmOptions[56]);
        printf("alpha7 [57] (mk_max_tardiness) = %.2f\n", ip->algorithmOptions[57]);

        printf("\n-------------------\n");
        printf("\nOUTPUT VARIABLES:\n");
        printf("Total time (totalTime): %.2f\n\n", totalTime);
        printf("Total Travel Time (totalTravel) = %.2f\n", totalTravel);
        printf("Total Waiting Time (totalWaitingTime): %.2f\n", totalWaitingTime);
        printf("Total Overtime (totalOvertime) = %.2f\n", totalOvertime);
        printf("Total Violated TW (totalTardiness) = %.2f\n", totalTardiness);
        printf("Total Violated TWMK (totalMKTardiness) = %.2f\n", totalMKTardiness);
        printf("Total Tardiness MK (mk_allowed_tardiness) = %.2f (totalOvertime + totalTardiness)\n", mk_allowed_tardiness);
        printf("Max Spare Time (maxSpareTime): %.2f\n", maxSpareTime);
        printf("Min Spare Time (minSpareTime): %.2f\n", minSpareTime);
        printf("Shortest Shift (shortestDay): %.2f\n", shortestDay);
        printf("Longest Shift (longestDay): %.2f\n", longestDay);
        printf("Shift Difference (dayDiff): %.2f\n", dayDiff);
        printf("Total Preference Score (ip->totPref) = %.2f\n", ip->totPref);
        printf("Quality = %.2f\n", quality);

        printf("\n------------------------------\n");
        printf("ALL QUALITIES:\n");
        printf("Ait H: %.2f\n", ait_quality);
        printf("Mankowska: %.2f\n", mk_quality);
        printf("Workload Balance: %.2f\n", wb_quality);
        printf("Paper: %.2f\n", paper_quality);



        /*printf("PAPER EXTRA BIT\n");
        printf("(0.3*totalTravel + ip->totPref) = %.2f\n", (0.3*totalTravel + ip->totPref));
        printf("totalTravel = %.2f\n", totalTravel);
        printf("ip->totPref = %.2f\n", ip->totPref);
        printf("quality = %.2f\n", quality);
        printf("totalMKTardiness = %.2f\n", totalMKTardiness);
        printf("mk_allowed_tardiness = %.2f\n", mk_allowed_tardiness);
        printf("totalWaitingTime: %.2f\n", totalWaitingTime);
        printf("totalOvertime = %.2f\n", totalOvertime);
        printf("totalTime: %.2f\n\n", totalTime);*/

        /*printf("travelMins = %.2f\n", travelMinutes);
        printf("TWTardinessMinutes = %.2f\n", TWTardinessMinutes);
        printf("TWMaxTardinessMinutes = %.2f\n", TWMaxTardinessMinutes);
        printf("infeasibleTardinessMinutes: %.2f\n", infeasibleTardinessMinutes);
        printf("overtimeMinutes: %.2f\n", overtimeMinutes);
        printf("totalWaitingMinutes: %.2f\n", totalWaitingMinutes);
        printf("minSpareTimeMinutes: %.2f\n", minSpareTimeMinutes);
        printf("totalTime: %.2f\n", totalTime);*/

        /*printf("ip->algorithmOptions[50] = %.2f (if 1, tardiness/overtime are infeasible)\n", ip->algorithmOptions[50]);
        printf("ip->algorithmOptions[51] = %.2f\n", ip->algorithmOptions[51]);
        printf("ip->algorithmOptions[51]*travelMinutes = %.2f\n", ip->algorithmOptions[51]*travelMinutes);
        printf("ip->algorithmOptions[52] = %.2f\n", ip->algorithmOptions[52]);
        printf("ip->algorithmOptions[52]*totalWaitingMinutes = %.2f\n", ip->algorithmOptions[52]*totalWaitingMinutes);
        printf("ip->algorithmOptions[53] = %.2f\n", ip->algorithmOptions[53]);
        printf("ip->algorithmOptions[53]*totalMKTardiness = %.2f\n", ip->algorithmOptions[53]*totalMKTardiness);
        printf("ip->algorithmOptions[54] = %.2f\n", ip->algorithmOptions[54]);
        printf("ip->algorithmOptions[54]*totalOvertime = %.2f\n", ip->algorithmOptions[54]*totalOvertime);
        printf("ip->algorithmOptions[55] = %.2f\n", ip->algorithmOptions[55]);
        printf("ip->algorithmOptions[55]*minSpareTimeMinutes = %.2f\n", ip->algorithmOptions[55]*minSpareTimeMinutes);
        printf("ip->algorithmOptions[56] = %.2f\n", ip->algorithmOptions[56]);
        printf("ip->algorithmOptions[56]*ip->totPref = %.2f\n", ip->algorithmOptions[56]*ip->totPref);
        printf("quality = %.2f\n", quality);*/

        /*printf("\n--------------------------\n");
        printf("PAPER QUALITY: ORIGINAL\n");
        printf("-1*totalTravel = %.2f\n", -1*totalTravel);
        printf("-1*totalWaitingTime = %.2f\n", -1*totalWaitingTime);
        printf("-5*mk_allowed_tardiness = %.2f\n", -5*mk_allowed_tardiness);
        printf("-5*totalOvertime = %.2f\n", -5*totalOvertime);
        printf("0.5*minSpareTime= %.2f\n", 0.5*minSpareTime);
        printf("-1*ip->totPref= %.2f\n", -1*ip->totPref);
        printf("-0*mk_max_tardiness= %.2f\n", -0*mk_max_tardiness);
        printf("----------------------\n\n");*/

        /*printf("PAPER QUALITY: DIVIDED BY 60\n");
        printf("-1*travelMinutes = %.2f\n", -1*travelMinutes);
        printf("-0*totalWaitingMinutes = %.2f\n", -0*totalWaitingMinutes);
        printf("-5*totalMKTardiness = %.2f\n", -5*totalMKTardiness);
        printf("-5*totalOvertime = %.2f\n", -5*totalOvertime);
        printf("0.0*minSpareTimeMinutes= %.2f\n", 0.0*minSpareTimeMinutes);
        printf("-1*ip->totPref= %.2f\n", -1*ip->totPref);
        printf("--------------------------\n\n");*/

        /*printf("IF IT WAS MANKOWSKA\n");
        printf("quality = -1*(totalTravel + mk_allowed_tardiness + mk_max_tardiness)/3 = %.2f\n", -1*(totalTravel + mk_allowed_tardiness + mk_max_tardiness)/3);
        printf("totalTravel = %.2f\n", totalTravel);
        printf("mk_allowed_tardiness = %.2f\n", mk_allowed_tardiness);
        printf("mk_max_tardiness = %.2f\n", mk_max_tardiness);
        printf("------------------------------------\n");*/

        /*printf("<- VALUES FOR COMPARISON OF QUALITY ->\n");
        printf("<-< totalTravel = %.2f >->\n", totalTravel);
        printf("<-< totalWaitingMinutes = %.2f >->\n", totalWaitingMinutes);
        printf("<-< ip->totPref = %.2f >->\n", ip->totPref);
        printf("<-< shortestDay = %.2f >->\n", shortestDay);
        printf("<-< longestDay = %.2f >->\n", longestDay);
        printf("<-< dayDifference = %.2f >->\n", longestDay - shortestDay);
        printf("<-< totalOvertime = %.2f >->\n", totalOvertime);
        printf("<-< maxSpareTime: %.2f >->\n", maxSpareTime);
        printf("<-< minSpareTime: %.2f >->\n", minSpareTime);
        printf("<-< totalTime: %.2f >->\n", totalTime);
        printf("<-< quality: %.2f >->\n", quality);
        printf("<- END OF COMPARISON OF QUALITY ->\n");*/

    }


    // double quality = -1*totalTime; // Time <> Greedy

    /*
	// EXTENDED OBJECTIVE F. //
	double tardinessCoef = -10000000000;
	double overtimeCoeff = tardinessCoef;
	double minSpareTimeCoeff = 1;
	double maxSpareTimeCoeff = -1;
	double totalTimeCoeff = -1;
	double maxTardinessCoeff = 0;
	double longestDayCoeff = -10;
	double quality =tardinessCoef*totalTardiness +
					overtimeCoeff*totalOvertime +
					maxTardinessCoeff*maxTardiness +
					minSpareTimeCoeff*minSpareTime +
					maxSpareTimeCoeff*maxSpareTime +
					longestDayCoeff*longestDay +
					totalTimeCoeff*totalTime;
	if (report > 0)
	{
		printf("\nTotal travel = %.2f\n", totalTravel);
		printf("\ntardinessCoef = %.2f,\ttotalTardiness = %.2f\n", tardinessCoef, totalTardiness);
		printf("overtimeCoeff = %.2f,\ttotalOvertime = %.2f\n", overtimeCoeff, totalOvertime);
		printf("maxTardinessCoeff = %.2f,\tmaxTardiness = %.2f\n", maxTardinessCoeff, maxTardiness);
		printf("minSpareTimeCoeff = %.2f,\tminSpareTime = %.2f\n", minSpareTimeCoeff, minSpareTime);
		printf("maxSpareTimeCoeff = %.2f,\tmaxSpareTime = %.2f\n", maxSpareTimeCoeff, maxSpareTime);
		printf("longestDayCoeff = %.2f,\tlongestDay = %.2f\n", longestDayCoeff, longestDay);
		printf("totalTimeCoeff = %.2f,\ttotalTime = %.2f\n", totalTimeCoeff, totalTime);
	}
	*/

    // int MAX_ALLOWED_WORKING = 100000;
    // int quality = ip->nNurses*MAX_ALLOWED_WORKING - totalTime;
    // if (totalTardiness > 0)
    // 	ip->isFeasible = 0;
    // else
    // 	ip->isFeasible = 1;
    ip->isFeasible = feasible;

    // print_timeMatrix(ip);

    // Save the details:
    // Debug: Move all this to the top, why define new variables??????
    ip->objTravel = totalTravel;
    ip->objTime = totalTime;
    ip->objTardiness = totalTardiness;
    ip->objLongestDay = longestDay;
    ip->objOvertime = totalOvertime;
    return quality;

} // End of obj_from_times function.

double alternative_quality(struct INSTANCE* ip, int report){
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

        get_nurse_route(ip, j, ip->nurseRoute);

        for(int i = 0; i < ip->nJobs; ++i){
            if(ip->nurseRoute[i] < 0)
                break;
            int job = ip->nurseRoute[i];
            // Trip from depot:
            if(prevPoint > -0.5)
                tTime = get_travel_time(ip, prevPoint, job);
            else
                tTime = get_travel_time_from_depot(ip, j, job);

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
            tTime = get_travel_time_to_depot(ip, j, prevPoint);
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

int random_integer(int min_val, int max_val){
    //return random value between min_val and max_val
    return min_val + (rand()%(max_val + 1));
}

void two_exchange(int* array, int i, int j){ //In GRASP: two_exchange(ip->nurseOrder, exch1, exch2), and random_two_exchange: two_exchange(array, (*i), (*j))

    //This function swaps the elements in array[i] and array[j] (element that was in array[i] is now in array [j] and vice vera)

    int t = array[j];
    array[j] = array[i];
    array[i] = t;

} //END OF two_exchange.

void random_two_exchange(int* array, size_t n, int* i, int* j){ //In GRASP: random_two_exchange(ip->nurseOrder, ip->nNurses, &exch1, &exch2)

    if(n < 2){ //If nNurses < 2, then &exch1 and &exch2 = 0, and exit.
        i = 0;
        j = 0;
        return;
    }

    //Otherwise, if nNurses >=2, then use random_integer function to set *i and *j to random ints between 0 and n-1
    (*i) = random_integer(0, n - 1);
    (*j) = random_integer(0, n - 1);

    //If the random_integer function coincidentally set *i and *j to be the same, use random_integer to choose another value for *j until *i and *j are different.
    while(*i==*j)
        (*j) = random_integer(0, n - 1);

    // Then, perform two_exchange - swap the elements in array[i] and array[j].
    two_exchange(array, (*i), (*j));
}

void shuffle(int* array, size_t n){ //In GRASP: shuffle(ip->nurseOrder, (size_t)ip->nNurses)

    // Based on the idea from: https://benpfaff.org/writings/clc/shuffle.html
    // Or from: https://stackoverflow.com/questions/6127503/shuffle-array-in-c
    // (Ben Pfaff's Writings)

    //Randomly shuffle elements of the array (nurseOrder)
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

void print_vector(int* array, size_t n){
    printf("\n[%d", array[0]);
    for(int i = 1; i < n; ++i)
        printf(", %d", array[i]);
    printf("]\n");
}
