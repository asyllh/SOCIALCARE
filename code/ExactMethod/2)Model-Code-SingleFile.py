#!/usr/bin/env python
# coding: utf-8

import sys
from collections import defaultdict
import gurobipy as gp
from gurobipy import GRB
import numpy as np
import xlrd
import xlsxwriter
import matplotlib.pyplot as plt

import read_helper_functions as rhf

# Determine the file to read:
file_to_read = '45-10-s-2b'
# inst = rhf.read_ait_h_instance(file_to_read, split=False) # Normal instance
inst = rhf.read_ait_h_instance(file_to_read, split=True) # Split instance into skills.
# exit(-1)

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

#print('Shape of the OD matrix:')
#print(inst.od.shape)
#print('Shape of the preference matrix:')
#print(inst.CoutPref.shape)
#print(inst.depl)
#print(inst.CoutPref)


# In[4]:


class Customer():
    def __init__(self,CID,CPref,SDemand,TWinS,TWinE,SGap):
        self.CID = CID
        self.CPref = CPref
        self.SDemand = SDemand
        self.TWinS = TWinS
        self.TWinE = TWinE
        self.SGap = SGap
    def __str__(self):
        return f"Customer ID: {self.CID}\n Customer Pref: {self.CPref}\n Skill Required: {self.SDemand}\n Time Window Starting: {self.TWinS}\n Time Window Ending: {self.TWinE}\n Gap between two services: {self.SGap}"


# In[5]:


class Vehicle():
    def __init__(self,VID,STPS,AS,CTWinS,CTWinE):
        self.VID = VID
        self.STPS = STPS
        self.AS = AS
        self.CTWinS=CTWinS
        self.CTWinE=CTWinE
    def __str__(self):
        return f"Vehicle ID: {self.VID}\n Service time per skill: {self.STPS}\n Available Skills: {self.AS}\n Caregiver Availability Starting Time: {self.CTWinS}\n Caregiver Availability Ending Time: {self.CTWinE}"


# In[6]:


#Read Customer Data
Customers=[]
for c in range(inst.NbClients):
    thisCustomer = Customer(c,inst.CoutPref[c],inst.Demandes[c],inst.WindowsC[c][0],inst.WindowsC[c][1],inst.gap[c])
    Customers.append(thisCustomer)


# In[7]:


#Read Vehicle Data
Vehicles=[]
for v in range(inst.NbVehicules):
    thisVehicle = Vehicle(v,inst.DureeDeVisite,inst.Offres,inst.WindowsD[v][0],inst.WindowsD[v][1])
    Vehicles.append(thisVehicle)


# In[8]:


#Read Location Data
VLoc = []

for i in range(inst.od.shape[0]):
    thisVLocs = np.hstack((inst.od[i],inst.od[i][0]))
    VLoc.append(thisVLocs) 
VLoc.append(VLoc[0])


# In[26]:
def solve_BaseM(Customers,Vehicles):
    V = [i for i in range(inst.NbClients+2)]
    Vnf = [d for d in range(inst.NbClients+1)]
    Vnd = [f for f in range(1,inst.NbClients+2)]
        
    N = [i for i in range(1,inst.NbClients+1)]
    K = [k.VID for k in Vehicles]
    S = [s for s in range(inst.NbServices)]
    
    Ksub = []
    for s in S:
        Ksub.append([])
        for k in K:
            if inst.Offres[k][s] > 0.5:
                Ksub[s].append(k)
   
    Ssub = []
    for a in range(inst.NbClients):
        Ssub.append([])
        for s in S:
            if inst.Demandes[a][s] > 0.5:
                Ssub[a].append(s)        
    Ssub.insert(0,[0,1])
    Ssub.append([0,1])
    
    mis = np.array(inst.Demandes)
    
    ##Model
    m = gp.Model("VRP-1")
    
    #Decision Variables
    x = m.addVars(V,V,K, vtype=GRB.BINARY, name="x")
    
    z = m.addVars(N, vtype=GRB.BINARY, name="z")
    
    t = m.addVars(V,K, name="t")
    
    #Constraints
    #2
    for k in K:
        m.addConstr(gp.quicksum(x[0,j,k] for j in N) == 1, name="Constraint-2-" + 'k='+ str(k))     
    #3
    for k in K:
        #print(k)
        m.addConstr(gp.quicksum(x[i,inst.NbClients+1,k] for i in N) == 1, name="Constraint-3-" + 'k='+ str(k))
    
    #4
    for h in N:
        for k in K:
            #print(h,k)
            m.addConstr((gp.quicksum(x[i,h,k] for i in Vnf) == gp.quicksum(x[h,i,k] for i in Vnd)), name="Constraint-4-"+ 'h=' + str(h) + '_k=' + str(k))

    #5:
    for i in N:
        for s in S:
            m.addConstr((gp.quicksum(x[i,j,k] for j in Vnd for k in Ksub[s]) == mis[i-1,s]), name="Constraint-5-"+ 'i=' + str(i) + '_s=' + str(s))
            #print(mis)
            #print("i=",i,"s=",s)
            #print("i=",i,"i-1=",i-1)
           
    #6
    for i in V:
        for j in V:
            for s in S:
                if s in Ssub[i] and s in Ssub[j]: 
                    for k in Ksub[s]:
                        #print(i,j,s,k)
                        if i==0:
                            #print('0',Vehicles[0].CTWinS)
                            m.addConstr(( t[i,k] + (VLoc[i][j] + 0)* x[(i,j,k)] <= t[j,k] + Vehicles[k].CTWinE * (1-x[(i,j,k)])), name="Constraint-6-" + 'i=' + str(i) + '_j=' + str(j) + '_s=' + str(s)+ '_k=' + str(k))    
                            #print(VLoc[i][j])
                        elif i==len(V)-1:
                            #print('0',Vehicles[0].CTWinE)
                            m.addConstr(( t[i,k] + (VLoc[i][j] + 0)* x[(i,j,k)] <= t[j,k] + Vehicles[k].CTWinE * (1-x[(i,j,k)])), name="Constraint-6-" + 'i=' + str(i) + '_j=' + str(j) + '_s=' + str(s)+ '_k=' + str(k))    
                            #print(VLoc[i][j])
                        else:
                            #print(Vehicles[k].STPS[i-1][s],Customers[i-1].TWinE)
                            m.addConstr(( t[i,k] + (VLoc[i][j] + Vehicles[k].STPS[i-1][s]) * x[(i,j,k)] <= t[j,k] + Customers[i-1].TWinE * (1-x[(i,j,k)])), name="Constraint-6-" + 'i=' + str(i) + '_j=' + str(j) + '_s=' + str(s)+ '_k=' + str(k))    
                            #print(VLoc[i][j])
   
    #7
    for i in N:
        for s in Ssub[i]:
            #print(s,Ssub[i])
            for k in Ksub[s]:
                #print(i,s,k)
                #print(Customers[i-1].TWinS,Customers[i-1].TWinE)
                m.addConstr((Customers[i-1].TWinS * gp.quicksum(x[(i,j,k)] for j in Vnd) <= t[i,k] ), name="Constraint-7L-" + 'i=' + str(i) + '_s=' + str(s)+ '_k=' + str(k))
                m.addConstr((t[i,k] <= Customers[i-1].TWinE * gp.quicksum(x[(i,j,k)] for j in Vnd) ), name="Constraint-7R-" + 'i=' + str(i) + '_s=' + str(s)+ '_k=' + str(k))   

    #8
    for k in K:
        m.addConstr((Vehicles[k].CTWinS <= t[0,k]),name="Constraint-8L-" + 'k='+ str(k))
        m.addConstr((t[0,k] <= Vehicles[k].CTWinE),name="Constraint-8R-" + 'k='+ str(k))
        
    #9
    for k in K:
        m.addConstr( (Vehicles[k].CTWinS <= t[0,k]) ,name="Constraint-9L-" + 'k='+ str(k))
        m.addConstr( (t[0,k] <= Vehicles[k].CTWinE) ,name="Constraint-9R-" + 'k='+ str(k))
   
    #10
    for i in N:
        for s in Ssub[i]:
            for r in Ssub[i]:
                if r<s:
                    m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) <= Customers[i-1].SGap ,name="Constraint-10")
    #11
    for i in N:
        for s in Ssub[i]:
            for r in Ssub[i]:
                if r<s:
                    m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) >= -1 * Customers[i-1].SGap ,name="Constraint-11")
    #12
    for i in N:
        for s in Ssub[i]:
            for r in Ssub[i]:
                if r!=s:
                    m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) >= Customers[i-1].SGap - (695*2) * z[i] ,name="Constraint-12")            
    #13
    for i in N:
        for s in Ssub[i]:
            for r in Ssub[i]:
                if r<s:
                    m.addConstr((gp.quicksum(t[i,k] for k in Ksub[r])) - (gp.quicksum(t[i,k] for k in Ksub[s])) >=  -1 * Customers[i-1].SGap - (695*2) * (1-z[i]) ,name="Constraint-13")                
    #14
    for i in N:
        for s in Ssub[i]:
            if len(Ssub[i])<inst.NbServices and Ssub[i]==Ssub[i+1]:
                for k in K:
                    if k not in Ksub[s]:
                        m.addConstr(x[i,i+1,k] == 0, name="Constraint-14-" + 'i=' + str(i) + '_s=' + str(s)+ '_k=' + str(k))
                        #print(i,i+1,k)              
    ##DEBUG_CONSTRAINT WITH OBJ FUNCTION
    #m.addConstr(gp.quicksum(0.3 * VLoc[i][j] * x[i,j,k] for i in Vnf for j in Vnd for k in K) + gp.quicksum(Customers[i-1].CPref[k] * x[i,j,k] for i in N for j in Vnd for k in K)<=220,name="DebugConst")
    
    #Objective Function
    m.setObjective(gp.quicksum(0.3 * VLoc[i][j] * x[i,j,k] for i in Vnf for j in Vnd for k in K) + gp.quicksum(Customers[i-1].CPref[k] * x[i,j,k] for i in N for j in Vnd for k in K), GRB.MINIMIZE)
    m.write("ModellingCode_lp.lp")
    m.optimize()

    #m.computeIIS()
    #m.write("ModellingCode_ilp.ilp")

    print("Objective Value = ",m.objVal)
    print("filename = ",file_to_read)
    for v in m.getVars():
        if v.x > 0:
            print(v.varName, v.x)

    res=[]
    for k in K:
        frm=[]
        to=[]
        tym=[]
        
        nurse=[]
        for i in Vnf:
            for j in Vnd:
                if x[i,j,k].X>0:
                    nurse.append(k)
                    frm.append(i)
                    to.append(j)
                    #print(i,j,k)
                    tym.append(t[j,k].X)
                    #print('j=',j,'k=',k,t[j,k].X)
        stck=np.column_stack((nurse,frm,to,tym))
        stck=stck[np.argsort(stck[:,3])]
        res.append(stck)
    print(res)            

# In[27]:  

def printScen(scenStr):
    sLen = len(scenStr)
    print("\n" + "*"*sLen + "\n" + scenStr + "\n" + "*"*sLen + "\n")


# In[28]:


if __name__ == "__main__":
    # Base model
    printScen("Solving base scenario model")
    solve_BaseM(Customers,Vehicles)

