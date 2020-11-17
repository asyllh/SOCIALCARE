'''
Times are in minutes. We start at time 0 and finish at time 720 (12 hours with service)
'''

### Object to set parameters ###
class OPTIONS(object):
	def __init__(self):
		# Number of instances of this type to be generated
		self.number_of_instances = 100

		# Destination folder where the options will be stored (must exist beforehand):
		self.destination_folder = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Code\HHCRSP\default_set'

		# Instance stem name, instances will be called stem_name_1, stem_name_2, etc.
		self.stem_name = 'default_set'


		# WORKFORCE #
		self.nskills = 10 # at least 2

		# Nurses are split in bands (the number of bands can be altered)
		#   - Each band has a number of nurses "nurses_in_each_band"
		#     (the total number of nurses is the sum of all of these)
		#   - Each band _at least_ a fixed number of skills, as detailed in "band_fixed_skills"
		#   - The fixed skills are assigned randomly in the skill vector
		#     all the other skills might be present with probability as detailed in "prob_of_additional_skill"
		self.nurses_in_each_band = [4, 3, 3]
		self.band_fixed_skills = [1, 2, 3]
		self.prob_of_additional_skill = [0.1, 0.2, 0.5]

		# Only jobs that can be fulfilled are generated. If a job is not
		# valid, the code generates another one until it is suitable.
		# This is the max number of attempts to generate a valid job
		# (so we do not go into infinite loops)
		self.max_skill_generation_attempts = 10

		# Number of minutes at work and they probability to occur in the workforce:
		self.working_hours_types = [240, 480]
		self.shift_probability = [0.2, 0.8]

		# Starting times of the workforce and their probability to occur:
		self.nurse_start_times = [0, 120]
		self.start_time_probability = [0.8, 0.2]


		# SERVICES #
		self.services_per_nurse_min = 9
		self.services_per_nurse_max = 11
		# skill_service_times = [10, 20, 45]
		self.skill_service_times = [10, 10, 10, 20, 20, 20, 30, 30, 30, 45]

		self.g1 = 0.1 # Percentage of jobs in repeated locations

		# G2 NOT USED!
		self.g2 = 0.2 # Percentage of jobs requiring a second skill

		self.skills_per_job = [1, 2, 3]
		self.probs_skills_per_job = [0.8, 0.15, 0.05]


		self.g3 = 0.3 # Probability of job having a time window

		# For the time window generation:
		self.time_window_start_times = [0, 60, 360, 480, 540]
		self.time_window_start_probability = [0.3, 0.15, 0.1, 0.15, 0.3]

		self.time_window_durations = [60, 120, 180]
		self.time_window_duration_probability = [0.3, 0.4, 0.3]

		# For jobs with no TW:
		self.earliest_start = 0
		self.latest_start = 720

		# Double services
		self.g4 = 0.1 # Percentage of double services
		self.g5 = 0.1 # Percentage of jobs with time dependencies

		self.gap_durations = [120, 240, 360] # 2, 4, 6 hours
		self.prob_gap_durations = [1./3., 1./3., 0.34] # Last one larger to avoid numerical errors