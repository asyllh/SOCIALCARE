import numpy as np 
import matplotlib.pyplot as plt
import read_helper_functions as rhf

def get_od(points):
    npts = np.max(points.shape)
    od = np.zeros((npts,npts))
    for i in range(npts):
        for j in range(npts):
            od[i,j] = np.linalg.norm(points[i,:] - points[j,:])
    
    return od
### --- End def get_od --- ###

def get_M(D): 
    # Create matrix M using values in D (od matrix)
    n,m = D.shape # n = number of rows in od matrix, m = number of columns in od matrix
    M = np.zeros((n,m)) # Create empty matrix M of size n x m

    # Fill M matrix using od matrix, M matrix is a Gram Matrix:
    for i in range(n): 
        for j in range(m):
            M[i,j] = (D[0,j]**2 + D[i,0]**2 - D[i,j]**2)/2

    return M
### --- End def get_M --- ###

def coords_from_M(M):
    # Eigenvalue decomposition
    lambd, U = np.linalg.eig(M) # lambd = eigenvalues, U = eigenvector
    S = np.round(np.diag(lambd), 8) # np.diag(lambd) = create diagonal matrix with eignvalues, round eignvalues to 8dp
    # S = np.diag(lambd)
    X = U.dot(np.sqrt(S)) # if M = USU^T (because od is a real symmetric matrix), then the matrix X = U.sqrt(S) contains the position of the points (each row corresponding to one point)
    return X
### --- End def coords_from_M --- ###

def coords_from_od(od):
    M = get_M(od) # Create Gram matrix M
    return coords_from_M(M) # Get matrix X of position of points (each row corresponds to one point)
### --- End def coords_from_od --- ###

file_to_read = '18-4-l-2a'
inst = rhf.read_ait_h_instance(file_to_read)
nbc = inst.NbClients

# T = np.zeros((nbc+2, nbc+2))
# T[:nbc+1, :nbc+1] = inst.od

# Complete last column and last row:
# T[nbc+1, :] = T[0, :] # Last row equal to first row
# T[:, nbc+1] = T[:, 0] # Last column equal to first column

T = np.zeros((3,3))
T[0,1] = 3
T[1,0] = 3
T[0,2] = 4
T[2,0] = 4
T[1,2] = 5
T[2,1] = 5

print(T)
# exit(-1)

new_points = coords_from_od(T)
print(new_points)

plt.plot(new_points[:, 0], new_points[:, 1], 'ro')
plt.show()

# new_od = get_od(new_points)
# np.round(T - new_od, 4)

