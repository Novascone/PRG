######################################################
# module: cs3430_s21_hw01.py
# Hannah Swenson
# A02231817
######################################################

import numpy as np
import numpy.linalg
import random

### ============= Problem 1 (Gauss-Jordan Elimination) ===============
def swap(M, r1, r2):
    M[[r1,r2]] = M[[r2,r1]]
    return M

def scale(M, r, s):
    M1 = M.copy()
    M1[r,:] = s * M[r,:]
    return M1

def sum(M, r1, r2, s):
    M1 = M.copy()
    M1[r1,:] = M1[r1,:] + s * M1[r2,:]
    return M1

def gje(a, b):
    """ Gauss-Jordan Elimination to solve Ax = b. """
    #append a and b for the augmented matrix
    A = np.concatenate((a,b), axis = 1)

    #find which rows are only zeros
    zr = np.where(~A.any(axis=1))[0]
    #if there's zero rows, move them down
    if (len(zr) != 0):
        for i in range(len(zr)):
            swap(A, zr[i], len(A) - i - 1)
        zr = np.where(~A.any(axis=1))[0]
        swap(A, zr[0], len(A) - 1)

    #loop 
    for j in range (len(A[0])-1):

        #find which columns are only zeros
        zc = np.where(~A.any(axis=0))[0]

        #assume column we're on is nonzero
        col = j

        #check to make sure we aren't wrong
        if (len(zc) != 0):
            for i in range(j, len(A[j])):
                if zc[i] != i:
                    #change col if we are 
                    col = i
                else:
                    #the whole matrix is zeros, meaning there are infinite solutions
                    return None

        #get a non zero pivot
        if A[j][col] == 0:
            for i in range (j, len(A) - 1):
                if A[i][col] != 0:
                    A = swap(A, col, i)
        
        #scale pivot to 1
        if A[j][col] == 0:
            # if we're going to divide by zero ever, there is more than one solution
            return None
        else:
            A = scale(A, j, 1 / A[j][col])


        #turn other numbers in col to 0
        for i in range(len(A)):
            if A[i][j] != 0 and i != j:
                A = sum(A, i, j, -A[i][j])
    
    #isolate the "b" column
    x = np.delete(A, np.s_[:-1], axis = 1)
                        
    return x


## ============== Problem 2 (Determinants) ========================

def random_mat(nr, nc, lower, upper):
    """ Generate an nrxnc matrix of random numbers in [lower, upper]. """
    m = np.zeros((nr, nc))
    for r in range(nr):
        for c in range(nc):
            m[r][c] = random.randint(lower, upper)
    return m

def leibniz_det(a):
    """ Compute determinant of nxn matrix a with Leibniz's Formula. """
    A = a
    cf = []
    #find the minors of row 1
    for i in range(len(A)):
        mn = A.copy()

        #rdelete row 1 and col i
        mn = np.delete(mn, np.s_[:1], axis=0)
        mn = np.delete(mn, np.s_[i], axis=1)
        
        #find determinant of the minors -> the cofactors
        cf.append((-1)**(i) * np.linalg.det(mn))

    #find det
    x = []
    for i in range(len(A)):
        x.append(A[0][i] * cf[i])
    
    det = 0
    for i in range(len(A)):
        det += x[i]

    return det


def gauss_det(a):
    """ Compute determinant of nxn matrix a with Gaussian elimination. """
    A, s = gje_re(a)

    #isolate the pivots
    pivots = []
    for i in range (len(A)):
        pivots.append(A[i][i])
    
    #multiply them together
    pmul = 1
    for i in range (len(pivots)):
        pmul *= pivots[i]

    det = (-1)**s * pmul

    return det
    

def gje_re(a):
    """ Row Echelon Form & number of swaps """
    def swap(M, r1, r2):
        M[[r1,r2]] = M[[r2,r1]]
        return M

    def scale(M, r, s):
        M1 = M.copy()
        M1[r,:] = s * M[r,:]
        return M1

    def sum(M, r1, r2, s):
        M1 = M.copy()
        M1[r1,:] = M1[r1,:] + s * M1[r2,:]
        return M1

    #rename variable
    A = a

    #swaps
    swaps = 0

    #find which rows are only zeros
    zr = np.where(~A.any(axis=1))[0]
    #if there's zero rows, move them down
    if (len(zr) != 0):
        for i in range(len(zr)):
            swap(A, zr[i], len(A) - i - 1)
            swaps += 1
        zr = np.where(~A.any(axis=1))[0]
        swap(A, zr[0], len(A) - 1)

    #loop 
    for j in range (len(A[0])-1):

        #find which columns are only zeros
        zc = np.where(~A.any(axis=0))[0]

        #assume column we're on is nonzero
        col = j

        #check to make sure we aren't wrong
        if (len(zc) != 0):
            for i in range(j, len(A[j])):
                if zc[i] != i:
                    #change col if we are 
                    col = i
                else:
                    #the whole matrix is zeros, meaning there are infinite solutions
                    return None

        #get a non zero pivot
        if A[j][col] == 0:
            for i in range (j, len(A) - 1):
                if A[i][col] != 0:
                    A = swap(A, col, i)
                    swaps += 1
                
        # #scale pivot to 1
        if A[j][col] == 0:
            # if we're going to divide by zero ever, there is more than one solution
            return None
        
        #turn other numbers below pivots in col to 0
        for i in range(j, len(A)):
            if A[i][j] != 0 and i != j:
                s = A[i][j] / A[j][j]
                A = scale(A, j, s)
                A = sum(A, i, j, -1)
                A = scale(A, j, 1/s)

    return A, swaps


## ============== Problem 3 (Cramer's Rule) ======================

def cramer(A, b):
    """ Solve Ax = b with Cramer's Rule. """
    x = []
    detA = np.linalg.det(A)

    #find the bk matrices and divide their det by det(A)
    for i in range(len([A][0])):
        bk = A.copy()
        adim = A.shape[0]
        bk[:adim,i] = b[:,0]

        x.append(np.linalg.det(bk)/detA)
    
    x = np.array(x)
    x = x.reshape(-1,1)

    return x


if __name__ == '__main__':
    pass
