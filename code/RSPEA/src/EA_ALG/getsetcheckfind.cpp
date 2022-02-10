/*--------------/
EA_ALG
getset.cpp
UoS
01/02/2022
/--------------*/

#include "getsetcheckfind.h"

void SetNurseRoute(struct Instance* ip, int ni){
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

}// End SetNurseRoute function

void SetAllNurseRoutes(struct Instance* ip){
    // printf("Setting all -1\n");
    for(int ni = 0; ni < ip->nNurses; ni++){
        SetNurseRoute(ip, ni);
    }

} // End SetAllNurseRoutes function

void SetNurseTime(struct Instance* ip, int nursei){

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

} //END OF SetNurseTime function

void SetTimesFrom(struct Instance* ip, int firstNurse){

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

        SetNurseTime(ip, nurse); //Note that this set_nurse_time function also checks to make sure that nurse is being used.
    }

} // End SetTimesFrom function

void SetTimesFull(struct Instance* ip){

    for(int i = 0; i < ip->nJobs; ++i){
        ip->violatedTW[i] = 0;
        ip->violatedTWMK[i] = 0;
    }

    for(int j = 0; j < ip->nNurses; ++j){
        int nurse = ip->nurseOrder[j];
        // printf("Setting nurse time of: %d", nurse);
        SetNurseTime(ip, nurse);
    }

} // End SetTimesFull function


int CheckSkills(struct Instance* ip, int job, int nurse){
    //This function returns 1 if carer is skilled to do the job, and returns 0 otherwise.

    return ip->nurseSkilled[nurse][job];
}

int CheckSkillsDS(struct Instance* ip, int job, int nursei, int nursej){

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

int CheckSkillsDSFirst(struct Instance* ip, int job, int nursei){

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


double GetTravelTime(struct Instance* ip, int i, int j){
    // DEBUG : SPEED we can remove the error check if the -1 call is never made anymore
    if(i < 0 || j < 0){
        printf("\nERROR: Calling travel time with a -1 in get_travel_time(struct INSTANCE * ip, int jobi, int jobj).\n");
        printf("This is not valid since we allow for nurses to have different starting locations.\n");
        exit(-4323452);
    }

    return ip->od[i + 1][j + 1];

}// End of GetTravelTime function


double TravelTimeFromDepot(struct Instance* ip, int nurse, int job){

    if(ip->nurseTravelFromDepot[nurse][job] > 100000000000){
        printf("\nError! when calling with FROM nurse %d job %d, travel reported is %.3f", nurse, job, ip->nurseTravelFromDepot[nurse][job]);
    }

    return ip->nurseTravelFromDepot[nurse][job];

} // End of TravelTimeFromDepot function


double TravelTimeToDepot(struct Instance* ip, int nurse, int job){

    if(ip->nurseTravelToDepot[nurse][job] > 100000000000){
        printf("\nError! when calling with TO nurse %d job %d, travel reported is %.3f", nurse, job, ip->nurseTravelToDepot[nurse][job]);
    }

    return ip->nurseTravelToDepot[nurse][job];

}// End of TravelTimeToDepot function

int GetNurseJobCount(struct Instance* ip, int nurse){

    //This function returns the number of jobs that are in nurse's route

    int count = 0;
    for(int i = 0; i < ip->nJobs; i++){
        if(ip->solMatrix[nurse][i] > -0.5){
            count++;
        }
    }

    return count;

} //END OF get_nurse_job_count function.

void GetNurseRoute(struct Instance* ip, int ni, std::vector<int>& nurseRoute){
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

}// End GetNurseRoute function

int FindSecondNurseDS(struct Instance* ip, int job, int currentNurse){

    //For a 'job' that is a double service which currentNurse is assigned to, this function finds the other nurse, 'secondNurse', that is also assigned to 'job' with 'currentNurse'.

    int secondNurse = -1;
    for(int prevNurse = 0; prevNurse < ip->nNurses; ++prevNurse){
        if((ip->solMatrix[prevNurse][job] > -1) && (prevNurse!=currentNurse)){
            secondNurse = prevNurse;
            break;
        }
    }

    return secondNurse;

}// End of FindSecondNurseDS function