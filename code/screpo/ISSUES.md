# Issues: GRASP program

### Important:
* At some point in the code, a worse solution is allowed to be taken. Is this allowed? Where is this happening? 

### Issues:
* In `switch_nurse` function, we need to check that the nurses `ni` and `nj` aren't the same nurse, perhaps do this in `best_switch` function in the for loops? - DONE: the same nurse can be used, we can switch a job from one position in a nurse's route to a better position in the same nurse's route! Nothing needs to be changed.__
* In `best_switch`, the for loops go through each nurse `ni`, each job `job_from_ni`, and then each nurse `nj`, however the break statements in the loops means that as soon as one solution has been found that has better quality than `baseQuality`, then the search ceases, and that solution is taken to be the best one. It only finds one solution. Is this what the function is supposed to do? 
* In `best_switch`, at the end of the function, it has `if (firstSwitch < 1)` which is never true, and then `switch_nurse(ip, besti, bestj, best_job)` which is never executed. It actually doesn't need to be executed because the solution has been updated already in the for loops.
* In `standard_local_search_test`, `int userNurseOrderChange = 0` (from instance_handler.py), which means that neighbourhood NE03 is not used. Why?
* In `rcl_pick` function (grasp.c) `int rd_int = 0`, and then `int el = RCL_seeds[rd_int]`, which means that the element picked is *always* `RCL_seeds[0]`. The function is meant to pick values at random, so `int rd_int` needs to be modified to pick a random integer between 0 and `rcl_size` (the size of the RCL). __Done: removed selection of RCL_seeds[0], replaced with random integer pick.__
* In `path_relinking` function (grasp.c) (line1767 approx), there is `if(insert_job_at_position(ip, mvjob, current_nurse, current_nurse_pos))`, should this be `<0`? Because the `insert` function returns 0 if successful and -1 if not successful. __Done: added <0 to the statement.__
* In `directed_path_relinking` function (grasp.c), `int source_sol_1 =0`, and can be changed to -1, so `source_sol_1` is only ever 0 or -1. However, there's an if statement `if(source_sol_1 > 0)`. This will never be true, should it be `if(source_sol_1 > -1)`? __Done: changed >0 to >-1.__
* In `find_arc_destination` function (grasp.c), `int n_pos = c_pos++`, which I believe is to set `n_pos` as the next position AFTER position `c_pos` in the nurse's route. However, `n_pos = c_pos++` means that `n_pos = c_pos` and THEN `c_pos` is incremented by one, so really `n_pos` is just the same position as `c_pos`, only `c_pos` has changed. Should this be `n_pos = ++c_pos` instead, or just `n_pos = c_pos + 1`? __Done: changed `cpos++` to `cpos+1`.__
* Note that currently, `ov[3] = 0.0` in instance_handler.py, which means that in `standard_local_search_test` (grasp.c), `int useNurderOrderChange = 0`, and so neighbourhood NE03 `nurse_two_exchange` is currently not active.
* In `best_job_insertion` function (constructive.c) `int firstSwitch = 0` is defined but never altered, however it is then used in the function: `if(firstSwitch > 0) {return 0; }`. Surely, as `firstSwitch` is always 0, isn't this redundant?
* The function `void grap_ls` not used? Declared in grasp.h and defined in grasp.c, but never used.
* In `randomised_constructive` function (grasp.c), during the 'seeds' part of the function, `cJob` and `cNurse` are selected using `rcl_pick` and if `cJob` is successfully inserted into `cNurse`'s route, `allocatedJobs[cJob]` is updated. `nurseSeed` is also updated, but `nurseSeed[nurse] = cJob`. Should it be `nurseSeed[cNurse] = cJob` instead?
* Potentially declare/define `int check_skills` function as inline for optimisation?
* `randomised_constructive` parameter `int randomness` is never used.
* `best_job_insertion`: for loop is `nJobs+10`, is this necessary? Test.
* `set_nurse_times`: `int fel_ac` is never used.
* `GRASP`: `double ca_time = elapsedTimeIT` never used.
* In `best_switch` function (constructive.c), `int firstSwitch = 1`, and is used in the function but is never altered, it always stays as 1. Why?
* In `standard_local_search_test` (grasp.c), in the 2-OPT section, `int nurseJobCount = get_nurse_job_count(ip, nurse);` is used to count the number of jobs in `nurse`'s route, but `nurseJobCount` is then never used.
* In `stamdard_local_search_test`, `int backandforceTrials = bigM` is never used.
* In `path_relinking` function, we should add a check in the while loop to make sure that only the ip solution is being modified, and the guiding solution stays the same and is not accidentally modified.
* In `standard_local_search` and `standard_local_search_test`, the parameter `MAX_ITERATIONS` is never used.
* In `best_switch` function (constructive.c), what does the variable `onlyInfeasible` do?
