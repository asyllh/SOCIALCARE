import numpy as np 
import matplotlib.pyplot as plt

M = np.zeros((3,3))
M[1,1] = 9
M[2,2] = 16

print('M:\n', M)

lambd, U = np.linalg.eig(M)

print('lambd:\n', lambd)
print('U:\n', U)

