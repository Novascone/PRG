#include <iostream>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

int mpiRank, mpiSize, data;

void print(vector<double> &B);
void printMore(vector< vector<double> > &M);
void change(vector< vector<double> > &M, int r1, int r2);
void scale(vector< vector<double> > &M, int r1, double s);
void addUp(vector< vector<double> > &M, int r1, int r2, double s);
double gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs);

int main(int argc,char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &mpiRank); 
    MPI_Comm_size(MCW, &mpiSize); 

    // Creating an array matrix to pass into a 2d vector
    int A[3][4] = {{0.0 ,1.0 ,-3.0, -5.0}, {2.0, 3.0, -1.0, 7.0}, {4.0, 5.0, -2.0, 10.0}};

    vector< vector<double> > M(3);
    vector<double> B(3);

    // creating the 2d part of the vector and saving the values of A
    for (int i = 0; i < 3; ++i) {
        M[i] = vector<double>(4);
        for (int j = 0; j < 4; ++j) {
            M[i][j] = A[i][j];
        }
    }

    // performs gaussian-jordan elimination
    gje(M, B, mpiSize);

    MPI_Finalize();
    return 0;
}

// Used to print the final result of the elimination
void print(vector<double> &B){
    for(int i = 0; i < B.size(); i++){
        cout << B[i] << " " << endl;
    }
}

// used to print a 2d vector
void printMore(vector< vector<double> > &M){
    for(int i = 0; i < M.size(); i++){
        for(int j = 0; j < M[i].size(); j++){
            cout << M[i][j] << " ";
        }
        cout << endl;
    }
}

// helper function that swaps rows of matrix
void change(vector< vector<double> > &M, int r1, int r2){
    for(int i = 0; i < M[r1].size(); i++){
        swap(M[r1][i],M[r2][i]);
    }
    cout << "After changing rows " << r1 << " and " << r2 << ":" << endl;
    printMore(M);
}

// helper function that scales rows to make the pivot 1
void scale(vector< vector<double> > &M, int r1, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = s * M[r1][i];
    }
    cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    printMore(M);
}

// helper function to make other rows in pivot column 0
void addUp(vector< vector<double> > &M, int r1, int r2, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
    cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    printMore(M);
}

// The gaussian-jordan elimination function that utilizes helper functions to get results
double gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs){
    MPI_Comm_rank(MCW, &mpiRank); 
    MPI_Comm_size(MCW, &mpiSize); 
    MPI_Status myStatus;

    // temp used for storing the passed vectors when using MPI_Recv
    vector< vector<double> > temp(1);
    temp[0] = vector<double>(M[0].size());
    bool done = false; // used to tell worker threads to stop
    int rowCount = 0; // used to know how many rows have been used and when we run out
    
    if (mpiRank == 0) {
        // if rank == 0, perform swaps before sending rows to other processes
        for (int i = 0; i < M.size(); ++i) {
            if(M[i][i] == 0){
                for(int j = 0; j < M.size(); j++){
                    if(M[j][i] != 0){
                        change(M, i, j);
                    }
                }
            }
        }

        // sending a row to each process
        for (int i = rowCount; i < numberOfProcs - 1 && i < M.size(); ++i, ++rowCount) {
            MPI_Send(&M[i], M[i].size(), MPI_DOUBLE, (i + 1), rowCount, MCW);
            cout << "Master sending row " << i << " to processor " << (i + 1) << endl;
        }

        // while loop runs until there are no more rows to send out
        while(rowCount < M.size()) {
            int myFlag, received;
            int epithet = -1;

            // probing to ensure there is a message to receive
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
            if (myFlag) {
                // once received, we save the received double to B which houses the results
                // using MPI_TAG to know pivot/row of result
                MPI_Recv(&received, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myStatus);
                B[myStatus.MPI_TAG] = received;
                rowCount++;

                // checking to see if there are more rows to send
                // if there aren't, we send a kill tag to other processors
                if(rowCount == M.size()) {
                    MPI_Send(&epithet, 1, MPI_INT, myStatus.MPI_SOURCE, -1, MCW);
                } else {
                    MPI_Send(&M[rowCount], M[0].size(), MPI_DOUBLE, myStatus.MPI_SOURCE, rowCount, MCW);
                }
            }
        }
    } 
    // worker threads below
    else {
        while(!done) {
            int myFlag;
            // probing to ensure the is a message to receive
            MPI_Iprobe(0, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
            if (myFlag) {
                // if kill message is received, we get out of the loop
                if (myStatus.MPI_TAG == -1) {
                    done = true;
                } else {
                    MPI_Recv(&temp, (M[0].size() * M.size()), MPI_DOUBLE, 0, MPI_ANY_TAG, MCW, &myStatus);
                    int pivot = myStatus.MPI_TAG;

                    // scale pivot to 1
                    if(temp[0][pivot] == 0){
                        return -1;
                    }else{
                        scale(temp, 0, 1.0 / temp[0][pivot]);
                    }

                    for(int i = 0; i < temp[0].size(); i++){
                        // Turn other numbers in column to 0
                        if(temp[0][i] != 0 && i != pivot){
                            addUp(temp, 0, i, -temp[0][i]);
                        }
                    }
                    MPI_Send(&temp[pivot][temp[0].size() - 1], 1, MPI_DOUBLE, 0, pivot, MCW);
                }
            }
        }
    }

    // print the results of the function
    cout << "Result:" << endl;
    print(B);

    return 0;
}