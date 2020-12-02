import os
import instance_handler as hhc
import matplotlib.pyplot as plt
import call_concorde as cc
import numpy as np

def tsp_lower_bound(instance_name, full_file_path, itype='ait_h', show_image=False):
	# Most of these are not necessary here:
	instance_type = itype
	quality_measure = itype
	ds_skill_type = 'strictly-shared'
	max_time_seconds = 600
	verbose_level = 100
	load_from_disk = True
	options_vector = hhc.default_options_vector()
	# options_list = [full_file_path, instance_type, quality_measure, ds_skill_type,
 #                    max_time_seconds, verbose_level, load_from_disk, False, options_vector]
	inst = hhc.prepare_instance(full_file_path, instance_type, quality_measure, ds_skill_type,
							 max_time_seconds, verbose_level, load_from_disk, False, options_vector)
	inst.solMatrix -= 1

	print('Reading instance "', instance_name, '" from file:')
	print('> "', full_file_path, '"')

	if False:
	# if len(inst.xy) < 1:
		print('XY coordinates are not present, generating automatically...')
		inst.xy = generate_xy_from_euclidian_od(inst.od)
		print('Done.')

	# Initialise TSP matrix to start modifying the problem
	TSP_matrix = inst.od

	# Artificial distances:
	free = 0
	impossible = 1e4

	# Prepare OD matrix for TSP
	customers = [i + 1 for i in range(inst.nJobs)]

	earliest_departure = []
	# Calculate earliest arrivals and latest departures

	for i in range(inst.nJobs):
		min_time_from_depot = np.min(inst.nurse_travel_from_depot[:, i])
		min_start = min(min_time_from_depot, inst.jobTimeInfo[i, 0])
		earliest_departure.append(min_start + inst.jobTimeInfo[i, 2])

	# Weight the arcs that will incur a delay (or forbid)
	
	weight_type = 'impossible'
	wprop = 1
	if itype == 'mankowska':
		weight_type = 'proportional'
		wprop = 1
	start_time_arc_removals = 0
	for i in range(inst.nJobs):
		for j in range(inst.nJobs):
			if i == j:
				continue
			i_to_j_travel = inst.od[i + 1, j + 1]
			earliest_arrival_from_i = earliest_departure[i] + i_to_j_travel
			tardiness = earliest_arrival_from_i - inst.jobTimeInfo[j, 1]
			if tardiness > -0.00001:
				# print('Cannot go from ', i, ' to ', j)
				# print('\tEarliest arrival: ', earliest_arrival_from_i)
				# print('\tTime window of j finishes:', inst.jobTimeInfo[j, 1])
				new_arc_value = impossible
				if weight_type == 'proportional':
					new_arc_value = i_to_j_travel + tardiness*wprop
					
				change_distance_to_list(TSP_matrix, customers[i], [customers[j]], new_arc_value, direction='forward')
				start_time_arc_removals += 1
			# else:
			# 	print('Customers ', i, ' and ', j, 'are cool')
			# 	print('\tEarliest arrival: ', earliest_arrival_from_i)
			# 	print('\tTime window of j finishes:', inst.jobTimeInfo[j, 1])			

	print('Arcs forbidden by time dependencies:', start_time_arc_removals)

	# Duplicate double services, and list them as customers:
	dspairs = []
	artificial_ds = []
	corresponding_ds_customer = np.zeros(inst.nJobs, dtype=np.int32)
	for i in range(inst.nJobs):
		if inst.doubleService[i] > 0:
			TSP_matrix, index = duplicate_node_od(TSP_matrix, 0)
			dspairs.append((customers[i], index))
			customers.append(index)
			artificial_ds.append(index)
			corresponding_ds_customer[i] = index




	# Create depots:
	nurse_depots_init = []
	nurse_depots_finish = []
	for nn in range(inst.nNurses):
		TSP_matrix, index = duplicate_node_od(TSP_matrix, 0)
		nurse_depots_init.append(index)

		TSP_matrix, index = duplicate_node_od(TSP_matrix, 0)
		nurse_depots_finish.append(index)

	# Fix distances between nurse depots:
	for pos, ni in enumerate(nurse_depots_init):
		change_distance_to_list(TSP_matrix, ni, nurse_depots_init, impossible, direction='both')
		change_distance_to_list(TSP_matrix, ni, nurse_depots_finish, impossible, direction='forward')
		change_distance_to_list(TSP_matrix, ni, [nurse_depots_finish[pos]], free, direction='forward')
		change_distance_to_list(TSP_matrix, ni, customers, impossible, direction='backward')
	
	for pos, nf in enumerate(nurse_depots_finish):
		change_distance_to_list(TSP_matrix, nf, customers, impossible, direction='forward')
		change_distance_to_list(TSP_matrix, nf, nurse_depots_init, impossible, direction='forward')

		# Ensure nurses appear in order in the route:
		if pos < len(nurse_depots_init) - 1:
			change_distance_to_list(TSP_matrix, nf, [nurse_depots_init[pos + 1]], free, direction='forward')



	# Create sink:
	sink = 0
	change_distance_to_list(TSP_matrix, sink, nurse_depots_init, free, direction='forward')
	change_distance_to_list(TSP_matrix, sink, nurse_depots_finish + customers, impossible, direction='forward')
	change_distance_to_list(TSP_matrix, sink, nurse_depots_finish, free, direction='backward')
	change_distance_to_list(TSP_matrix, sink, nurse_depots_init + customers, impossible, direction='backward')


	# Check basic nurse skills
	nurse_can_serve_customer = np.zeros((inst.nNurses, max(customers) + 1))
	for nurse in range(inst.nNurses):
		for j in range(inst.nJobs):
			if inst.doubleService[j] > 0:
				if np.sum(inst.jobSkillsRequired[j,:]) == 2: 
					# Then we can split it
					s1, s2 = np.argwhere(inst.jobSkillsRequired[j,:] > 0)[:,0]
					if inst.nurseSkills[nurse, s1] > 0:
						nurse_can_serve_customer[nurse, customers[j]]
					if inst.nurseSkills[nurse,s2] > 0:
						nurse_can_serve_customer[nurse, corresponding_ds_customer[j]]
				else:
					# Does the nurse have any of the skills at least
					if np.max(inst.jobSkillsRequired[j,:] + inst.nurseSkills[nurse,:]) >= 2:
						nurse_can_serve_customer[nurse, customers[j]]
						nurse_can_serve_customer[nurse, corresponding_ds_customer[j]]
			else:
				if check_single_skills(inst, nurse, j):
					nurse_can_serve_customer[nurse, customers[j]] = 1

	# Forbid the same nurse doing a double service:
	for pair in dspairs:
		ci, cj = pair
		change_distance_to_list(TSP_matrix, ci, [cj], impossible, direction='both')


	# Prevent nurses from starting at jobs where they are not skilled:
	for nurse in range(inst.nNurses):
		for j in range(inst.nJobs):
			if nurse_can_serve_customer[nurse, customers[j]] < 1:
				change_distance_to_list(TSP_matrix, nurse_depots_init[nurse], [customers[j]], impossible, direction='forward')
				change_distance_to_list(TSP_matrix, customers[j], [nurse_depots_finish[nurse]], impossible, direction='forward')

	# Prevent arcs between "disjoint" jobs:
	skill_arc_removals = 0
	for cj1 in customers:
		for cj2 in customers:
			if cj1 == cj2:
				continue
			if np.sum(nurse_can_serve_customer[:, cj2]) > 1:
				continue
			# If the nurses that can serve the jobs
			# are different, then we can delete the link

			max_jobs_nurse_can_do = np.max(nurse_can_serve_customer[:, cj1] + nurse_can_serve_customer[:, cj2])

			if max_jobs_nurse_can_do < 1.5:
				# print('Removing arc between customers ', cj1, '<->', cj2)
				skill_arc_removals += 1
				change_distance_to_list(TSP_matrix, cj1, [cj2], impossible, direction='both')


	print('Depot indices:')
	print('nurse_depots_init', nurse_depots_init)
	print('nurse_depots_finish', nurse_depots_finish)

	# Use TSP solver for lower bound:
	# print(inst.xy)
	# trivial_solution(inst)
	tour, city_list = cc.solve_with_concorde(TSP_matrix)
	# print(city_list)

	current_nurse = -1
	current_pos = 0
	for city in city_list:
		print('City: ', city, end=' >> ')
		if city == sink:
			print('Is the sink. Ignore.')

		elif city in nurse_depots_finish:
			print('Is the finish depot for nurse', nurse_depots_finish.index(city),'. Ignore.')

		elif city in nurse_depots_init:
			current_nurse = nurse_depots_init.index(city)
			print('Is an initial depot. Changing to nurse:', current_nurse)
			current_pos = 0

		elif city in artificial_ds:
			for pair in dspairs:
				original, duplicate = pair
				if city == duplicate:
					print('Is part of DS, ', original,'added to', current_nurse, ' as job ', city - 1, 'in position', current_pos)
					inst.solMatrix[current_nurse, original - 1] = current_pos
					current_pos += 1

		else:
			print('Is a customer, added to', current_nurse, ' as job ', city - 1, 'in position', current_pos)
			inst.solMatrix[current_nurse, city - 1] = current_pos
			current_pos += 1

	# print('solMatrix:\n', inst.solMatrix)
	tot_arcs = (inst.nJobs + 1)*(inst.nJobs + 1)
	rem_arcs = skill_arc_removals + start_time_arc_removals
	print('Total arcs:', tot_arcs)
	print('Removed   :', rem_arcs, '(', np.round(rem_arcs/tot_arcs*100, 1), '%)')
	print('TSP Cost: ', tour)
	print('TSP Cost/3: ', tour/3)
	print('TSP Cost*0.3: ', tour*0.3)

	if show_image == True:
		inst.simple_solution_plot()
		plt.show()

def generate_xy_from_euclidian_od(od):
	# Based on this answer: 
	# https://math.stackexchange.com/questions/156161/finding-the-coordinates-of-points-from-distance-matrix

	# TODO: This is not working / tested
	print('ERROR: Functionality not implemented "generate_xy_from_euclidian_od()"')
	exit(-1)

	npoints = len(od)
	M = -(od**2)
	for n in range(npoints):
		M[n,:] += od[n,0]**2
		M[:,n] += od[0,n]**2
	M = M/2.0
	Mb = np.zeros((npoints, npoints))
	for i in range(npoints):
		for j in range(npoints):
			Mb[i,j] = (od[i,0]**2 + od[0,j]**2 - od[i,j]**2)/2
	M = Mb
	S, U, St = np.linalg.svd(M)
	xy = np.matmul(U, np.sqrt(S))
	# print('od', od)
	# print('M', M)
	# print('M_rank', np.linalg.matrix_rank(M))
	# print('Mb_rank', np.linalg.matrix_rank(Mb))

	# print('S', S)
	# print('U', U)
	# print('St', St)
	# print('np.sqrt(S)', np.sqrt(S))
	# print('xy', xy)
	# print('U.shape', U.shape)
	# print('S.shape', S.shape)
	# print('xy.shape', xy.shape)
	return xy

def check_single_skills(inst, nurse, job):
	if np.min(inst.nurseSkills[nurse,:] - inst.jobSkillsRequired[job,:]) > -0.5:
		return True
	else:
		return False

def duplicate_node_od(od, node):
	'''
	Duplicates a node by adding it
	at the end
	'''
	nr,nc = od.shape
	new_od = np.zeros((nr + 1, nc + 1))
	new_od[:nr, :nc] = od.copy()
	new_od[-1,:-1] = od[node, :]
	new_od[:-1, -1] = od[:, node]
	new_index = nr
	return new_od, new_index

def change_distance_to_list(od, node, node_list, new_distance, direction='forward'):
	back = False
	both = False
	if direction.lower() == 'backward':
		back = True
	elif direction.lower() == 'both':
		both = True

	for dn in node_list:
		if back or both:
			od[dn, node] = new_distance
		if not back:
			od[node, dn] = new_distance



def trivial_solution(inst):
	for i in range(len(inst.solMatrix[0,:])):
		inst.solMatrix[0,i] = i


if __name__ == '__main__':

	directory = 'C:\\Users\\clf1v16\\docs_offline\\01.HHCRSP\\Instances\\Ait_Haddadene\\adapted\\'
	# file_list = ['18-4-s-2a', '18-4-s-2b', '18-4-s-2c', '18-4-s-2d', '18-4-m-2a', '18-4-m-2b', '18-4-m-2c', '18-4-m-2d', '18-4-m-2e', '18-4-l-2a', '18-4-l-2b', '18-4-l-2c', '18-4-l-2d', '18-4-l-2e']
	# file_list = ['18-4-s-2a']
	# itype = 'ait_h'

	directory = r'C:\Users\clf1v16\docs_offline\01.HHCRSP\Instances\Mankowska\all_mk\\'
	file_list = []
	# file_list += ['saved_InstanzCPLEX_HCSRP_10_1.mat', 'saved_InstanzCPLEX_HCSRP_10_2.mat', 'saved_InstanzCPLEX_HCSRP_10_3.mat', 'saved_InstanzCPLEX_HCSRP_10_4.mat', 'saved_InstanzCPLEX_HCSRP_10_5.mat', 'saved_InstanzCPLEX_HCSRP_10_6.mat', 'saved_InstanzCPLEX_HCSRP_10_7.mat', 'saved_InstanzCPLEX_HCSRP_10_8.mat', 'saved_InstanzCPLEX_HCSRP_10_9.mat', 'saved_InstanzCPLEX_HCSRP_10_10.mat']
	# file_list += ['saved_InstanzCPLEX_HCSRP_25_1.mat', 'saved_InstanzCPLEX_HCSRP_25_2.mat', 'saved_InstanzCPLEX_HCSRP_25_3.mat', 'saved_InstanzCPLEX_HCSRP_25_4.mat', 'saved_InstanzCPLEX_HCSRP_25_5.mat', 'saved_InstanzCPLEX_HCSRP_25_6.mat', 'saved_InstanzCPLEX_HCSRP_25_7.mat', 'saved_InstanzCPLEX_HCSRP_25_8.mat', 'saved_InstanzCPLEX_HCSRP_25_9.mat', 'saved_InstanzCPLEX_HCSRP_25_10.mat']
	# file_list += ['saved_InstanzCPLEX_HCSRP_50_1.mat', 'saved_InstanzCPLEX_HCSRP_50_2.mat', 'saved_InstanzCPLEX_HCSRP_50_3.mat', 'saved_InstanzCPLEX_HCSRP_50_4.mat', 'saved_InstanzCPLEX_HCSRP_50_5.mat', 'saved_InstanzCPLEX_HCSRP_50_6.mat', 'saved_InstanzCPLEX_HCSRP_50_7.mat', 'saved_InstanzCPLEX_HCSRP_50_8.mat', 'saved_InstanzCPLEX_HCSRP_50_9.mat', 'saved_InstanzCPLEX_HCSRP_50_10.mat']
	# file_list += ['saved_InstanzCPLEX_HCSRP_75_1.mat', 'saved_InstanzCPLEX_HCSRP_75_2.mat', 'saved_InstanzCPLEX_HCSRP_75_3.mat', 'saved_InstanzCPLEX_HCSRP_75_4.mat', 'saved_InstanzCPLEX_HCSRP_75_5.mat', 'saved_InstanzCPLEX_HCSRP_75_6.mat', 'saved_InstanzCPLEX_HCSRP_75_7.mat', 'saved_InstanzCPLEX_HCSRP_75_8.mat', 'saved_InstanzCPLEX_HCSRP_75_9.mat', 'saved_InstanzCPLEX_HCSRP_75_10.mat']
	file_list = ['saved_InstanzCPLEX_HCSRP_10_1.mat']
	itype = 'mankowska'


	pause_between_instances = True
	for instance in file_list:
		full_file_path = os.path.join(directory, instance)

		tsp_lower_bound(instance, full_file_path, itype=itype, show_image=True)
		print('That was instance: "', instance, '"')
		if pause_between_instances and instance != file_list[-1]:
			answer = input("Press enter to continue, or type 'N' to stop")
			if answer.lower() == 'n':
				break
