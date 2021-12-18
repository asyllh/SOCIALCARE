from read_helper_functions import read_ait_h_instance


# Determine the file to read:
file_to_read = '45-10-m-4'
inst = read_ait_h_instance(file_to_read)


# Now you can access this data fields, using a dot:
# CoutPref - Matrix of the preference scores between patients/carers
# Demandes - Matrix with skills required by the client
# DureeDeVisite - Service time per skill -> DureeDeVisite[job][skill]
# NbClients - Number of patients/clients
# NbServices - Number of different skills
# NbVehicles - Number of nurses available
# Offres - For each nurse, what skills they have. Binary: Offres[nurse][skill]
# WindowsC - For each job, WindowsC[job][0] is the start of the time window, WindowsC[job][1] the finishing time
# WindowsD - WindowsD[nurse][0] time the nurse starts working, WindowsD[nurse][1] time the nurse finishes
# od - od[i,j] as the distance between two patients. row/col 0 represent the nurses location (same for all of them)
# gap - Gap between two services (for those that require two skills only!) if the gap is 0, it means they are simultaneous

print('The number of nurses in the instance is', inst.NbVehicules, 'and the number of patients is', inst.NbClients)
print(len(inst.gap))

print('Shape of the OD matrix:')
print(inst.od.shape)

print('Shape of the preference matrix:')
print(inst.CoutPref.shape)
