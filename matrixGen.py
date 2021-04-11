import numpy as np
import random

"""A simple matrix generator that writes to a given file. To use, 
   call createFile() with the name of the text file you want to be
   filled in (you may have to edit the path for it to work), the
   smallest you want your matrices to be, the largest you want
   them to be, the smallest number in the matrix, the largest
   number in the matrix, and how many matrices you want to generate. """


def random_mat(nr, nc, lower, upper):
    """ Generate an nrxnc matrix of random numbers in [lower, upper]. """
    m = np.zeros((nr, nc))
    for r in range(nr):
        for c in range(nc):
            m[r][c] = random.randint(lower, upper)
    return m

def createFile(file, s1, s2, r1, r2, num): #name of file, lower size, upper size, lower range, upper range, how many matrices
    path = 'project/' + file
    f = open(path, "w")
    for i in range(num):
        size = random.randint(s1, s2)
        mat = str(random_mat(size, size + 1, r1, r2))
        f.write(mat)
        f.write("\n \n")
    f.close()

# #example call: creates 5 matrices of random size ranged 3-10, 
# # with the values of 0-50 and writes it to 'test.txt':
# createFile('test.txt', 3, 10, 0, 50, 5)


createFile('mat10.txt', 5, 15, 0, 99, 10)
createFile('mat50.txt', 5, 15, 0, 99, 50)
createFile('mat100.txt', 5, 15, 0, 99, 100)
