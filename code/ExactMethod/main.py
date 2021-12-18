import sys
from collections import defaultdict
import gurobipy as gp
from gurobipy import GRB
import numpy as np
import xlrd
import xlsxwriter
import read_helper_functions as rhf
import networkx_graph as nxg
import testnetworkx as testnx

# filenames=['18-4-s-2a','18-4-s-2b','18-4-s-2c','18-4-s-2d','18-4-m-2a','18-4-m-2b','18-4-m-2c','18-4-m-2d','18-4-m-2e','18-4-l-2a','18-4-l-2b','18-4-l-2c','18-4-l-2d','18-4-l-2e']
filenames=['18-4-s-2a']
# plot_graph = True
plot_graph = False
# workbook = xlsxwriter.Workbook('C:/Users/ah4c20/Asyl/PostDoc/SOCIALCARE/code/ExactMethod/Values.xlsx')
# worksheet = workbook.add_worksheet()
# worksheet.write(0, 0, 'Filename' )
# worksheet.write(0, 1, 'Objective value' )
for fl in range(len(filenames)):
    # Determine the file to read:
    file_to_read = filenames[fl]
    inst = rhf.read_ait_h_instance(file_to_read, split=False) # Normal instance
    # inst = rhf.read_ait_h_instance(file_to_read, split=True) # the instance file is a split file
    # inst = rhf.split_instance(file_to_read, print_statements=True)
    # exit(-1)
    
    # NbVehicles - Number of nurses available
    # NbClients - Number of patients/clients
    # NbServices - Number of different skills
    # Demandes - NbClients x NbServices, binary matrix with skills required by the client. Demandes[client][skill] = 1 if client requires skill, 0 otherwise
    # Offres - NbVehicules x NbServices, binary matrix: for each nurse, what skills they have. Binary: Offres[nurse][skill] = 1 if nurse can perform skill, = 0 otherwise
    # od - NbClients+1 x NbClients+1, distance between patients. row/col 0 represent the nurses location (same for all of them, single depot)
    # WindowsC - NbClients x 2, for each job, WindowsC[job][0] is the start of the time window, WindowsC[job][1] the finishing time
    # DureeDeVisite - NbClients x NbServices, service time per skill, DureeDeVisite[client][skill]
    # WindowsD - NbVehicules x 2, WindowsD[nurse][0] time the nurse starts working, WindowsD[nurse][1] time the nurse finishes
    # CoutPref - NbClients x NbVehicules, matrix of the preference scores between clients and nurses
    # gap - 1 x NbClients, gap between two services (for those that require two skills only!) if the gap is 0, it means they are simultaneous

    
    class CLIENT():
        def __init__(self, clientID, clientPref, clientSkillReq, clientStartTW, clientEndTW, clientServiceGap):
            self.clientID = clientID # CLIENT ID CID
            self.clientPref = clientPref # CLIENT preference CPref
            self.clientSkillReq = clientSkillReq # Skill required SDemand
            self.clientStartTW = clientStartTW # Start of job TW (WindowsC[i][0]) TWinS
            self.clientEndTW = clientEndTW # End of job TW (WindowsC[i][1]) TWinE
            self.clientServiceGap = clientServiceGap # Gap between two services SGap
        def __str__(self):
            return f"Client ID: {self.clientID}\n Client Pref: {self.clientPref}\n Skill Required: {self.clientSkillReq}\n Time Window Starting: {self.clientStartTW}\n Time Window Ending: {self.clientEndTW}\n Gap between two services: {self.clientServiceGap}"
    
    
    class NURSE():
        def __init__(self, nurseID, serviceTimePerSkill, availSkills, nurseStartTW, nurseEndTW):
            self.nurseID = nurseID # NURSE ID VID
            self.serviceTimePerSkill = serviceTimePerSkill # Service time per skill STPS
            self.availSkills = availSkills # Available skills AS
            self.nurseStartTW = nurseStartTW # Nurse starting time (WindowsD[i][0]) CTWinS
            self.nurseEndTW = nurseEndTW # Nurse ending time (WindowsD[i][1]) CTWinE
        def __str__(self):
            return f"Nurse ID: {self.nurseID}\n Service time per skill: {self.serviceTimePerSkill}\n Available Skills: {self.availSkills}\n Nurse Availability Starting Time: {self.nurseStartTW}\n Nurse Availability Ending Time: {self.nurseEndTW}"
    
    
    #Read CLIENT Data
    Clients = []
    for c in range(inst.NbClients):
        thisClient = CLIENT(c,inst.CoutPref[c],inst.Demandes[c],inst.WindowsC[c][0],inst.WindowsC[c][1],inst.gap[c])
        Clients.append(thisClient)
        
    #Read NURSE Data
    Nurses = []
    for v in range(inst.NbVehicules):
        thisNurse = NURSE(v,inst.DureeDeVisite,inst.Offres,inst.WindowsD[v][0],inst.WindowsD[v][1])
        Nurses.append(thisNurse)
        
    #Read Location Data - create copy of od matrix and add first column of matrix as a new column on the end of matrix, same for row
    Locs = []
    for i in range(inst.od.shape[0]):
        thisLocs = np.hstack((inst.od[i],inst.od[i][0]))
        Locs.append(thisLocs) 
    Locs.append(Locs[0])
    
    
    def solve_BaseM(Clients,Nurses):

        # Sets
        V = [i for i in range(inst.NbClients+2)] # 0,...,n, n+1 (clients N and initial depot 0 and final depot n+1)
        Vnf = [d for d in range(inst.NbClients+1)] # V but not including f, 0, ..., n, (clients and initial depot 0)
        Vnd = [f for f in range(1,inst.NbClients+2)] # V but not including d, 1, ..., n, n+1  (clients and final depot n+1)
            
        N = [i for i in range(1,inst.NbClients+1)] # just clients, 1,..., n
        K = [k.nurseID for k in Nurses] # Vehicles 0,..., NbVehicules
        S = [s for s in range(inst.NbServices)] # Services 0,...,NbServices
        
        Ksub = [] # List of S lists, one for each skill, each containing the nurses than can perform that skill
        for s in S:
            Ksub.append([])
            for k in K:
                if inst.Offres[k][s] > 0.5:
                    Ksub[s].append(k)
       
        Ssub = [] # List of NbClients lists, one for each client, each containing the skill that is required by that client
        for a in range(inst.NbClients):
            Ssub.append([])
            for s in S:
                if inst.Demandes[a][s] > 0.5:
                    Ssub[a].append(s)        
        Ssub.insert(0,[0,1]) # for initial depot 0, added at beginning of Ssub
        Ssub.append([0,1]) # for final depot n+2, added to end of Ssub
        
        mis = np.array(inst.Demandes)
        
        # Model
        m = gp.Model("VRP-1")
        
        # Decision Variables
        # x = m.addVars(V,V,K, vtype=GRB.BINARY, name="x")
        x = m.addVars(V,V,K, name="x", lb=0, ub=1)
        # z = m.addVars(N, vtype=GRB.BINARY, name="z")
        z = m.addVars(N, name="z", lb=0, ub=1)
        t = m.addVars(V,K, name="t")

        #### --------------- Constraints --------------- ####
        
        # Constraint 2: sum(j in N) x_djk = 1 for all k in K
        for k in K:
            m.addConstr(gp.quicksum(x[0,j,k] for j in N) == 1, name="Constraint-2-" + 'k='+ str(k))     

        # Constraint 3: sum(i in N) x_ifk = 1 for all k in K
        for k in K:
            m.addConstr(gp.quicksum(x[i,inst.NbClients+1,k] for i in N) == 1, name="Constraint-3-" + 'k='+ str(k))
        
        # Constraint 4: sum(i in V\{f}) x_ihk = sum(i in V\{d}) x_hik for all h in N, for all k in K
        for h in N:
            for k in K:
                m.addConstr((gp.quicksum(x[i,h,k] for i in Vnf) == gp.quicksum(x[h,i,k] for i in Vnd)), name="Constraint-4-"+ 'h=' + str(h) + '_k=' + str(k))
    
        # Constraint 5: sum(j in V\{d}) sum(k in K_s) x_ijk = m_is for all i in N, for all s in S
        for i in N:
            for s in S:
                m.addConstr((gp.quicksum(x[i,j,k] for j in Vnd for k in Ksub[s]) == mis[i-1,s]), name="Constraint-5-"+ 'i=' + str(i) + '_s=' + str(s))
            
        # Constraint 6: t_ik + (T_ij + D_is).x_ijk <= t_jk + b_i(1 - x_ijk) for all i, j in V, for all s in S : s in S_i \cap S_j, for all k in K_s
        for i in V:
            for j in V:
                for s in S:
                    if s in Ssub[i] and s in Ssub[j]: 
                        for k in Ksub[s]:
                            #print(i,j,s,k)
                            if i == 0 or i == len(V)-1:
                                #print('0',Nurses[0].nurseStartTW)
                                m.addConstr(( t[i,k] + (Locs[i][j] + 0)* x[(i,j,k)] <= t[j,k] + Nurses[k].nurseEndTW * (1-x[(i,j,k)])), name="Constraint-6-" + 'i=' + str(i) + '_j=' + str(j) + '_s=' + str(s)+ '_k=' + str(k))    
                                #print(NLoc[i][j])
                            else:
                                #print(Nurses[k].serviceTimePerSkill[i-1][s],Clients[i-1].clientEndTW)
                                m.addConstr(( t[i,k] + (Locs[i][j] + Nurses[k].serviceTimePerSkill[i-1][s]) * x[(i,j,k)] <= t[j,k] + Clients[i-1].clientEndTW * (1-x[(i,j,k)])), name="Constraint-6-" + 'i=' + str(i) + '_j=' + str(j) + '_s=' + str(s)+ '_k=' + str(k))    
                                #print(NLoc[i][j])
       
        # Constraint 7: a_i.sum(j in N) x_ijk <= t_ik <= b_i.sum(j in N) x_ijk for all i in N, for all s in S_i, for all k in K_s
        for i in N:
            for s in Ssub[i]:
                #print(s,Ssub[i])
                for k in Ksub[s]:
                    #print(i,s,k)
                    #print(Clients[i-1].clientStartTW,Clients[i-1].clientEndTW)
                    m.addConstr((Clients[i-1].clientStartTW * gp.quicksum(x[(i,j,k)] for j in Vnd) <= t[i,k] ), name="Constraint-7L-" + 'i=' + str(i) + '_s=' + str(s)+ '_k=' + str(k))
                    m.addConstr((t[i,k] <= Clients[i-1].clientEndTW * gp.quicksum(x[(i,j,k)] for j in Vnd) ), name="Constraint-7R-" + 'i=' + str(i) + '_s=' + str(s)+ '_k=' + str(k))   
    
        # Constraint 8: alpha_k <= t_dk <= beta_k for all k in K
        for k in K:
            m.addConstr((Nurses[k].nurseStartTW <= t[0,k]),name="Constraint-8L-" + 'k='+ str(k))
            m.addConstr((t[0,k] <= Nurses[k].nurseEndTW),name="Constraint-8R-" + 'k='+ str(k))
            
        # Constraint 9: alpha_k <= t_fk <= beta_k for all k in K
        for k in K:
            m.addConstr( (Nurses[k].nurseStartTW <= t[0,k]) ,name="Constraint-9L-" + 'k='+ str(k))
            m.addConstr( (t[0,k] <= Nurses[k].nurseEndTW) ,name="Constraint-9R-" + 'k='+ str(k))
   
        # Constraint 10: sum(k in K_r) t_ik - sum(k in K_s) t_ik <= gap_isr for all i in N, for all s, r in S_i : r != s
        for i in N:
            for s in Ssub[i]:
                for r in Ssub[i]:
                    if r<s:
                        m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) <= Clients[i-1].clientServiceGap ,name="Constraint-10")

        # Constraint 11: sum(k in K_r) t_ik - sum(k in K_s) t_ik >= -gap_isr for all i in N, for all s, r in S_i : r != s
        for i in N:
            for s in Ssub[i]:
                for r in Ssub[i]:
                    if r < s:
                        m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) >= -1 * Clients[i-1].clientServiceGap ,name="Constraint-11")
        
        # Constraint 12: sum(k in K_r) t_ik - sum(k in K_s) t_ik >= gap_isr - M.z_i for all i in N, for all s, r in S_i : r != s
        for i in N:
            for s in Ssub[i]:
                for r in Ssub[i]:
                    if r != s:
                        m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) >= Clients[i-1].clientServiceGap - (695*2) * z[i] ,name="Constraint-12")            
                    
        # Constraint 13: sum(k in K_r) t_ik - sum(k in K_s) t_ik <= -gap_isr + M.(1 - z_i) for all i in N, for all s, r in S_i : r != s
        for i in N:
            for s in Ssub[i]:
                for r in Ssub[i]:
                    if r < s:
                        m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) >=  -1 * Clients[i-1].clientServiceGap - (695*2)*(1 - z[i]) ,name="Constraint-13")                

        # Constraint 14
        for i in N:
            for s in Ssub[i]:
                if len(Ssub[i]) < inst.NbServices and Ssub[i] == Ssub[i+1]:
                    for k in K:
                        if k not in Ksub[s]:
                            m.addConstr(x[i, i+1, k] == 0, name="Constraint-14-" + 'i =' + str(i) + '_s =' + str(s)+ '_k =' + str(k))
                            #print(i,i+1,k)

        for j in V:
            for k in K:
                m.addConstr(x[j,j,k] == 0)
        
        for i in V:
            for j in V:
                if i < j:
                    continue
                for k in K:
                    m.addConstr(x[i,j,k] + x[j,i,k] <= 1)

        ### --------------- End Constraints --------------- ###
        
        # Objective Function: minimise sum(i in V\{f}) sum(j in V\{d}) sum(k in K) C_ij.x_ijk + sum(i in N) sum(j in V\{d}) sum(k in K) Pref_ik.x_ijk
        m.setObjective(gp.quicksum(0.3 * Locs[i][j] * x[i,j,k] for i in Vnf for j in Vnd for k in K) + gp.quicksum(Clients[i-1].clientPref[k] * x[i,j,k] for i in N for j in Vnd for k in K), GRB.MINIMIZE)
        m.write("ModellingCode_lp.lp")
        m.optimize()

        # DEBUG_CONSTRAINT WITH OBJ FUNCTION
        # m.addConstr(gp.quicksum(0.3 * VLoc[i][j] * x[i,j,k] for i in Vnf for j in Vnd for k in K) + gp.quicksum(Customers[i-1].clientPref[k] * x[i,j,k] for i in N for j in Vnd for k in K)<=220,name="DebugConst")
    
        # m.computeIIS()
        # m.write("ModellingCode_ilp.ilp")
    
        print("Objective Value = ", m.objVal)
        print("filename = ", file_to_read)

        print('Initial depot node: ', 0)
        print('Final depot node: ', inst.NbClients+1)
        print('\n')
        for v in m.getVars():
            if v.x > 0:
                print(v.varName, v.x)

        edge_set = []
        res = []
        for k in K:
            leavingFrom = []
            goingTo = []
            time = []
            nurse = []
            nurseEdges = []
            for i in Vnf:
                for j in Vnd:
                    if x[i,j,k].X>0:
                        nurse.append(k)
                        leavingFrom.append(i)
                        goingTo.append(j)
                        #print(i,j,k)
                        time.append(t[j,k].X)
                        nurseEdges.append(tuple([i, j]))
                        if k == 0:
                            print('x', i, j, k, ': ', x[i,j,k].X)
                        #print('j=',j,'k=',k,t[j,k].X)
            edge_set.append(nurseEdges)
            kResult = np.column_stack((nurse, leavingFrom, goingTo, time)) # stacking lists
            kResult = kResult[np.argsort(kResult[:,3])] # sorting lists of lists in order based on value of time
            res.append(kResult)
        print(res)            
        print('edges:')
        print(edge_set)
        # print('edges size: ', len(edge_set))
        # print('edge inner size: ', len(edge_set[0]))

        # edge_set1 = [[(0, 4), (4, 0), (3, 2), (2, 1)]]
        # exit(-1)

        if plot_graph:
            # solution_graph = nxg.create_graph(file_to_read, inst.NbClients+2, edge_set, nurse_index=[2,3], is_nurse_list=True)
            # solution_graph = nxg.create_graph(file_to_read, inst.NbClients+2, edge_set, nurse_index=999, is_nurse_list=False)
            solution_graph = testnx.create_graph(file_to_read, inst.NbClients+2, edge_set, nurse_index=999, is_nurse_list=False)
            # solution_graph = testnx.create_graph(file_to_read, inst.NbClients+2, edge_set, nurse_index=3, is_nurse_list=False)
            # solution_graph = nxg.create_graph(file_to_read, inst.NbClients+2, edge_set)
            solution_graph.show()

        
        # worksheet.write(fl+1, 0, file_to_read )
        # worksheet.write(fl+1, 1, m.objVal )
    ### --- End def solve_BaseM --- ###
    
    def printScen(scenStr):
        sLen = len(scenStr)
        print("\n" + "*"*sLen + "\n" + scenStr + "\n" + "*"*sLen + "\n")
    ### --- End def printScen --- ###
        
        
    if __name__ == "__main__":
        # Base model
        printScen("Solving base scenario model")
        solve_BaseM(Clients,Nurses)

# workbook.close()     
