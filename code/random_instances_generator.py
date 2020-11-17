import numpy as np
import copy
import sys
import os

# sys.path.insert(1, '..')
import instance_handler as hhc


### Start data generation ###
def vector_of_scrambled_ones(length, n_ones):
    '''
    Returns a numpy array of length "length"
    with "n_ones" 
    '''
    vector = np.zeros(length)
    vector[:int(n_ones)] = 1
    np.random.shuffle(vector)
    return vector

def random_pick_from_prob_vector(v):
    '''
    "v" is a list/array containing probability values
    adding up to one, eg:
    v = [0.2, 0.1, 0.7]
    The output is a random index of the vector, with probability
    as listed in v. In the previous example, there is three
    possible outcomes:
    0 - with 20% probability,
    1 - with 10% probability or
    2 - with 70% probability
    '''
    r = np.random.random()
    csum = 0
    for i, val in enumerate(v):
        csum += val
        if csum > r:
            return i

def calculate_od_matrix(cities):
    n_cities = cities.shape[0]
    od_matrix = np.zeros((n_cities, n_cities))
    for i in range(n_cities):
        od_matrix[:,i] = (cities[:,0] - cities[i,0])**2 + (cities[:,1] - cities[i,1])**2
    
    return np.sqrt(od_matrix)

def biased_pick(values_vector, probability_vector):
    '''
    Returns randomly one value from "values_vector"
    using the probabilities from "probability_vector"
    '''
    return values_vector[random_pick_from_prob_vector(probability_vector)]

### Generate the instances ####
def generate_random_instances(options, print_summary=True, save_txt_summary=True):
    for inst_idx in range(options.number_of_instances):
        inst = hhc.INSTANCE()   
        instance_name = options.stem_name + '_' + str(inst_idx)

        inst.name = instance_name
        inst.full_file_name = os.path.join(options.destination_folder, instance_name + '.p')

        summary = ''

        options.skill_service_times = np.array(options.skill_service_times)

        total_nurses = 0
        for n in options.nurses_in_each_band:
            total_nurses += n

        inst.nNurses = total_nurses
        inst.nSkills = options.nskills

        services_per_nurse = options.services_per_nurse_min + np.random.random()*(options.services_per_nurse_max - options. services_per_nurse_min)

        inst.nJobs = int(np.floor(total_nurses * services_per_nurse))

        # Start summary:
        summary += '\nInstance Name: ' + instance_name
        summary += '\n' + 'nNurses : ' + str(inst.nNurses)
        summary += '\n' + 'nJobs : ' + str(inst.nJobs)
        summary += '\n' + 'nSkills : ' + str(inst.nSkills)


        ### Nurses ###
        inst.nurseWorkingTimes = np.zeros((inst.nNurses, 3))
        inst.nurseSkills = np.zeros((inst.nNurses, inst.nSkills))
        nurse_idx = -1
        # print('-')
        avoid_previous = True
        if inst.nSkills < 2:
            avoid_previous = 0
        for band, nurses_in_band in enumerate(options.nurses_in_each_band):
            fixed_s = options.band_fixed_skills[band]
            prob_add = options.prob_of_additional_skill[band]

            for nurse in range(nurses_in_band):
                nurse_idx += 1
                # Define working hours:
                inst.nurseWorkingTimes[nurse_idx, 0] = biased_pick(options.nurse_start_times, options.start_time_probability)
                inst.nurseWorkingTimes[nurse_idx, 1:] = inst.nurseWorkingTimes[nurse_idx, 0] + biased_pick(options.working_hours_types, options.shift_probability)

                # Define their skills:
                # 1 - Fixed skills
                is_equal = True
                trials_avoiding = 0
                while avoid_previous and is_equal:
                    trials_avoiding += 1
                    if trials_avoiding > 1000:
                        print('ERROR: Cannot avoid generating the same skill!')
                        exit(-1234)
                    rnd_vector = vector_of_scrambled_ones(inst.nSkills, fixed_s)
                    # print(rnd_vector)
                    inst.nurseSkills[nurse_idx,:] = rnd_vector
                    # print('Nurse ' , nurse_idx, ' adding ', fixed_s, ' fixed skills :', inst.nurseSkills[nurse_idx,:]*(np.arange(inst.nSkills) + 1))
                    # 2 - Check if they have some more:
                    for s in range(inst.nSkills):
                        randnum = np.random.random()
                        if (inst.nurseSkills[nurse_idx,s] < 1) and (randnum < prob_add):
                            # print('\trandnum', randnum, 'prob', prob_add, ' - adding skill', s)
                            inst.nurseSkills[nurse_idx,s] = 1
                        # else:
                        #   print('\trandnum', randnum, 'prob', prob_add, ' - *not* adding skill', s)
                    if nurse_idx > 0 and np.allclose(inst.nurseSkills[nurse_idx,:], inst.nurseSkills[nurse_idx - 1,:]):
                        is_equal = True
                    else:
                        is_equal = False
        # print('-')

        # Update summary
        summary += '\n' + 'Staff : \n'
        count = -1
        for band, nurses_in_band in enumerate(options.nurses_in_each_band):
            summary += '\n\t - Band ' + str(band) + ' - '
            for nurse in range(nurses_in_band):
                count += 1
                summary += '\n\tNurse ' + str(count) + ')'
                summary += '\n\t\tSkills: ' + str(inst.nurseSkills[count,:]*(np.arange(inst.nSkills) + 1))
                summary += '\n\t\tWorking times: ' + str(inst.nurseWorkingTimes[nurse,:])

        ### Jobs ###


        inst.xy = np.random.random((inst.nJobs + 1, 2))*100

        # Repeat some locations (match first "rep_locs" to a random location of the ones remaining):
        rep_locs = int(np.floor(options.g1*inst.nJobs))
        for i in range(rep_locs):
            copy_from = np.random.randint(rep_locs, inst.nJobs)
            inst.xy[i,:] = inst.xy[copy_from, :]

        inst.od = calculate_od_matrix(inst.xy)

        jobs_with_tw = np.floor(inst.nJobs*options.g3)

        summary += '\n' + 'nTimeWindows : ' + str(jobs_with_tw)


        time_window_locations = vector_of_scrambled_ones(inst.nJobs, jobs_with_tw)


        # Double services and time dependencies:
        number_of_ds = np.floor(options.g4*inst.nJobs)
        # print('number_of_ds ', number_of_ds)
        n_of_dependencies = np.floor(options.g5*inst.nJobs)
        # print('n_of_dependencies ', n_of_dependencies)

        summary += '\n' + 'nDoubleServices : ' + str(number_of_ds)
        summary += '\n' + 'nDependencies : ' + str(n_of_dependencies)


        # To avoid overlap, generate all, the first ones are ds's the last ones dependencies
        ds_and_dep = vector_of_scrambled_ones(inst.nJobs, number_of_ds + n_of_dependencies)

        inst.doubleService = np.zeros(inst.nJobs, dtype=np.int32)
        dependency_locations = np.zeros(inst.nJobs, dtype=np.int32)

        # Check where we need to split:
        ds_count = 0

        while(np.sum(ds_and_dep[:ds_count]) < number_of_ds):
            ds_count += 1

        # Do the split:
        inst.doubleService[:ds_count] = ds_and_dep[:ds_count]
        dependency_locations[ds_count:] = ds_and_dep[ds_count:]
        # print('ds_count ', ds_count)
        # print('number_of_ds ', number_of_ds)
        # print('n_of_dependencies ', n_of_dependencies)
        # print('ds_and_dep: ', ds_and_dep*(np.arange(inst.nJobs) + 1))

        # print('DS locations: ', inst.doubleService*(np.arange(inst.nJobs) + 1))
        # print('Dependency locations: ', dependency_locations*(np.arange(inst.nJobs) + 1))

        inst.jobSkillsRequired = np.zeros((inst.nJobs, inst.nSkills), dtype=np.int32)
        inst.jobTimeInfo = np.zeros((inst.nJobs, 3))



        for job in range(inst.nJobs):
            # print('GENERATING JOB ', job)
             # Deal with time windows:
            if time_window_locations[job] > 0:
                # Job has a time window
                inst.jobTimeInfo[job,0] = biased_pick(options.time_window_start_times, options.time_window_start_probability)
                inst.jobTimeInfo[job,1] = inst.jobTimeInfo[job,0] + biased_pick(options.time_window_durations, options.time_window_duration_probability)
            else:
                inst.jobTimeInfo[job,0] = options.earliest_start
                inst.jobTimeInfo[job,1] = options.latest_start

            # Re-generate skills until there is one nurse that can do it
            skills_ok = False
            generate_skills_attempts = -1

            failed_jobs = []
            while not skills_ok:
                generate_skills_attempts += 1

                if generate_skills_attempts > options.max_skill_generation_attempts:
                    print('ERROR: It was not possible to generate suitable jobs for this workforce in ' + str(generate_skills_attempts) + ' attempts.')
                    print('Please, increase the capabilities of the workforce or diminish the requirements of jobs.')
                    print('Instance so far: ')
                    print(summary)
                    print('---------------------------')
                    print('Generating DS: ' + str(inst.doubleService[job]))
                    print('Jobs tried: ')
                    [print(iiii) for iiii in failed_jobs]
                    print('ERROR GENERATING JOBS')
                    exit(-234523)

                # Generate skill required:
                skills_on_this_job = biased_pick(options.skills_per_job, options.probs_skills_per_job)

                # Double services always have 2 skills
                if inst.doubleService[job] > 0:
                    skills_on_this_job = 2

                inst.jobSkillsRequired[job,:] = vector_of_scrambled_ones(inst.nSkills, skills_on_this_job)

                # Check if a nurse (or pair of nurses) can do the job:
                if inst.doubleService[job] < 0.5:
                    # print('\n - Checking a single service: ', inst.doubleService[job])
                    for nu, nurse_skills in enumerate(inst.nurseSkills):
                        if inst.jobTimeInfo[job, 1] < inst.nurseWorkingTimes[nu, 0]:
                            # print('Nurse A', nu, ' cannot do it (time)', inst.nurseWorkingTimes[nu, 0], 'job time info: ', inst.jobTimeInfo[job,:])
                            continue # The nurse cannot do it if the job TW finishes before they start
                        if np.max(inst.jobSkillsRequired[job,:] - nurse_skills) <= 0:
                            skills_ok = True
                            break
                else:
                    # print('\n - Checking a double service: ', inst.doubleService[job])
                    for nu, nurse_skills in enumerate(inst.nurseSkills):
                        if inst.jobTimeInfo[job, 1] < inst.nurseWorkingTimes[nu, 0]:
                            # print('Nurse A', nu, ' cannot do it (time)', inst.nurseWorkingTimes[nu, 0], 'job time info: ', inst.jobTimeInfo[job,:])
                            continue # The nurse cannot do it if the job TW finishes before they start
                        for nu_b, nurse_skills_b in enumerate(inst.nurseSkills):
                            if nu_b == nu:
                                # print('not checking A ', nu, ' and B ', nu_b)
                                continue
                            if inst.jobTimeInfo[job, 1] < inst.nurseWorkingTimes[nu_b, 0]:
                                # print('Nurse B', nu, ' cannot do it (time)', inst.nurseWorkingTimes[nu, 0], 'job time info: ', inst.jobTimeInfo[job,:])
                                continue # The nurse cannot do it if the job TW finishes before they start
                            if np.max(inst.jobSkillsRequired[job,:] - nurse_skills - nurse_skills_b) <= 0:
                                # print('They can, yay!')
                                skills_ok = True
                                break
                            # else:
                            #     print('----')
                            #     print('Nurses: ')
                            #     print('\t', nurse_skills)
                            #     print('\t', nurse_skills_b)
                            #     print('Cannot do: ')
                            #     print('\t', inst.jobSkillsRequired[job,:])
                            #     print('----')
                        if skills_ok:
                            break

                if not skills_ok:
                    failed_jobs.append('Skills: ' + str(inst.jobSkillsRequired[job,:]) + 'TW: ' + str(inst.jobTimeInfo[job,:]))
                #     print('This job is not valid regarding skills/time window! Trying again!')
                #     print('skills_on_this_job ', skills_on_this_job)
                #     print('inst.doubleService[job] ', inst.doubleService[job])
                #     print('Job generated: ', inst.jobSkillsRequired[job,:]*(np.arange(inst.nSkills) + 1))

            # print('----------------------------------------------------------\n\n')

            # Set job duration:
            index_array = inst.jobSkillsRequired[job,:] > 0
            inst.jobTimeInfo[job,2] = np.max(options.skill_service_times[inst.jobSkillsRequired[job,:]  > 0])




        # Set the dependencies:
        inst.mk_mind = np.zeros(inst.nJobs)
        inst.mk_maxd = np.zeros(inst.nJobs)
        inst.dependsOn = -1*np.ones(inst.nJobs, dtype=np.int32)
        job_list = np.array(range(inst.nJobs))

        non_dependency_jobs = -1*(dependency_locations - 1)
        non_dep_sum = np.sum(non_dependency_jobs)
        for dp in range(inst.nJobs):# dependency_locations:
            if dependency_locations[dp] < 1:
                continue

            inst.dependsOn[dp] = biased_pick(job_list, 1/non_dep_sum*non_dependency_jobs)
            non_dependency_jobs[inst.dependsOn[dp]] = 0
            non_dep_sum -= 1
            inst.dependsOn[inst.dependsOn[dp]] = dp
            min_gap = biased_pick(options.gap_durations, options.prob_gap_durations)
            max_gap = min_gap + biased_pick(options.time_window_durations, options.time_window_duration_probability)

            inst.mk_mind[dp] = min_gap
            inst.mk_maxd[dp] = max_gap
            inst.mk_mind[inst.dependsOn[dp]] = -1*max_gap
            inst.mk_maxd[inst.dependsOn[dp]] = -1*min_gap

        # Add element on the front, to have the same format as Ait H. instances we read from files:
        ext_mk = np.zeros(inst.nJobs + 1)
        ext_mk[0] = -12345
        ext_mk[1:] = inst.mk_mind
        inst.mk_mind = copy.copy(ext_mk)
        ext_mk[1:] = inst.mk_maxd
        inst.mk_maxd= copy.copy(ext_mk)



        # Completely random preferences:
        inst.prefScore = options.preference_max_value*2*(np.random.random((inst.nNurses, inst.nJobs)) - 0.5)


        ################ Generate single depots ##################
        summary += '\n' + 'SingleOrMultipleDepots : ' + 'Single'
        inst.nurse_travel_from_depot = np.empty((inst.nNurses, inst.nJobs), dtype=np.float64)
        inst.nurse_travel_to_depot = np.empty((inst.nNurses, inst.nJobs), dtype=np.float64)
        for nurse in range(inst.nNurses):
            inst.nurse_travel_from_depot[nurse,:] = inst.od[0,1:]
            inst.nurse_travel_to_depot[nurse,:] = inst.od[1:,0]

        inst.nurse_travel_from_depot = np.ascontiguousarray(inst.nurse_travel_from_depot)
        inst.nurse_travel_to_depot = np.ascontiguousarray(inst.nurse_travel_to_depot)

        inst.init_job_and_nurse_objects()
        inst.solMatrix = np.zeros((inst.nNurses, inst.nJobs))
        ############################################################################


        ### Finish summary:


        summary += '\n' + 'Jobs :'
        for job in range(inst.nJobs):
            summary += '\n\tJob ' + str(job)
            if inst.doubleService[job] > 0:
                summary += ', double service'
            if inst.dependsOn[job] > -1:
                summary += ', depends on ' + str(inst.dependsOn[job])
                summary += ' (gap ' + str(inst.mk_mind[job]) + ' - ' + str(inst.mk_maxd[job]) + ')'
            
            summary += '\n\t\tDuration: ' + str(inst.jobTimeInfo[job,2])
            summary += '\n\t\tTime Window: ' + str(inst.jobTimeInfo[job,:2])
            summary += '\n\t\tSkills: ' + str(inst.jobSkillsRequired[job,:]*(np.arange(inst.nSkills) + 1))

        ### Report:
        if print_summary:
            print('::: Instance Summary :: ' + str(instance_name) + ' :::')
            print(summary)
            print('\n::::::::::::::::::::::::\n')

        # Save as p-file for the moment:
        inst.pickle(inst.full_file_name)

        if save_txt_summary:
            # And the summary as txt:
            f = open(os.path.join(options.destination_folder, instance_name + '.txt'), 'w')
            f.write(summary)
            f.close()