import random_instances_generator as rig
from default_options import OPTIONS
from itertools import product
import numpy as np

# Output folder:
destination_folder = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Code\HHCRSP\testbed_1rep'

# Each type of instance has various realisations:
instances_per_type = 1

# Number of nurses
number_of_nurses_list = [10, 12, 14]

# Skills
skills_and_bands = ['low', 'medium', 'high']

# Time windows
tw_quantity_list = [10, 20] # Percentage of jobs with TW
tw_size_list = ['small', 'large', 'mixed']

# Double services quantity (percentage of total jobs)
ds_quantity_list = [10, 20]

# Job dependencies
dep_quantity_list = [10, 20] # Percentage of jobs with dependencies




for instance_specs in product(number_of_nurses_list, skills_and_bands, tw_quantity_list, tw_size_list, ds_quantity_list, dep_quantity_list):
	
	# Unpack the specifications of the instance:
	number_of_nurses, s_n_b_type, tw_quantity, tw_size, ds_quantity, dep_quantity = instance_specs

	# Generate the options object for the instance generator:
	op = OPTIONS()
	op.number_of_instances = instances_per_type
	op.destination_folder = destination_folder
	stem = 'n' + str(number_of_nurses) + '_'
	# Skills
	if s_n_b_type == 'low':
		stem += 'sl_'
		op.nskills = 2
		op.skill_service_times = [10, 20]
		n_bands = 1
		op.band_fixed_skills = [1]
		op.prob_of_additional_skill = [-1.0] # Not allowed
		op.skills_per_job = [1]
		op.probs_skills_per_job = [1.1]
	elif s_n_b_type == 'medium':
		stem += 'sm_'
		op.nskills = 5
		n_bands = 2
		op.band_fixed_skills = [1, 2]
		op.prob_of_additional_skill = [0.1, 0.5]
		op.skill_service_times = [10, 20, 20, 30, 45]
		op.skills_per_job = [1, 2]
		op.probs_skills_per_job = [0.8, 0.2]
	elif s_n_b_type == 'high':
		stem += 'sh_'
		op.nskills = 10
		op.skill_service_times = [10, 10, 10, 20, 20, 20, 30, 30, 30, 45]
		n_bands = 3
		op.band_fixed_skills = [1, 2, 3]
		op.prob_of_additional_skill = [0.1, 0.2, 0.5]
		op.skills_per_job = [1, 2, 3]
		op.probs_skills_per_job = [0.8, 0.15, 0.05]
	else:
		print('This type of skills and band specification is not defined "' + str(s_n_b_type) + '"')
		exit(-1)
		
		
	# Spread nurses evenly in each band, with more nurses on the lowest if non divisible
	n_per_band = np.floor(number_of_nurses/n_bands)
	n_lowest_band = number_of_nurses - n_per_band*(n_bands - 1)
	op.nurses_in_each_band = np.full(n_bands, n_per_band, dtype=np.int32)
	op.nurses_in_each_band[0] = n_lowest_band

	# Time windows:
	stem += 'tw' + str(tw_quantity)
	op.g3 = tw_quantity/100.0
	op.time_window_start_times = [0, 60, 360, 480, 540]
	op.time_window_start_probability = [0.3, 0.15, 0.1, 0.15, 0.3]
	if tw_size == 'small':
		stem += 's_'
		op.time_window_durations = [60]
		op.time_window_duration_probability = [1.1]
	elif tw_size == 'large':
		stem += 'l_'
		op.time_window_durations = [180]
		op.time_window_duration_probability = [1.1]
	elif tw_size ==  'mixed':
		stem += 'm_'
		op.time_window_durations = [60, 120, 180]
		op.time_window_duration_probability = [0.3, 0.4, 0.3]
	else:
		print('This type of time window specification is not defined "' + str(tw_size) + '"')
		exit(-1)

	stem += 'ds' + str(ds_quantity) + '_'
	op.g4 = ds_quantity/100.0 # Percentage of double services
	stem += 'de' + str(dep_quantity)
	op.g5 = dep_quantity/100.0 # Percentage of jobs with time dependencies

	op.stem_name = stem
	# Generate (and save) the instances for this spec.
	rig.generate_random_instances(op, print_summary=False)