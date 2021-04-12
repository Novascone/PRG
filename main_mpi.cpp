#include <iostream>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

int mpiRank, size, data;

void print(vector< vector<double> > &M);
void change(vector< vector<double> > &M, int r1, int r2);
void scale(vector< vector<double> > &M, int r1, double s);
void addUp(vector< vector<double> > &M, int r1, int r2, double s);
double gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs);

// pivot is equal to int i
// master process get matrix and does swaps
// each worker gets copy of matrix and row/pivot to work on
// get pivot to 1 and everything to 0
// each worker returns a double to master
// read in multiple matrices
// keep track of time? Maybe

int main(int argc,char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &mpiRank); 
    MPI_Comm_size(MCW, &size); 

    int A[3][4] = {{0.0 ,1.0 ,-3.0, -5.0}, {2.0, 3.0, -1.0, 7.0}, {4.0, 5.0, -2.0, 10.0}};

    vector< vector<double> > M(3);
    vector<double> B(3);

    for (int i = 0; i < 3; ++i) {
        M[i] = vector<double>(4);
        for (int j = 0; j < 4; ++j) {
            M[i][j] = A[i][j];
        }
    }

    gje(M, B, size);

    MPI_Finalize();
    return 0;
}

void print(vector< vector<double> > &M){
    for(int i = 0; i < M.size(); i++){
        for(int j = 0; j < M[i].size(); j++){
            cout << M[i][j] << " ";
        }
        cout << endl;
    }
}

void change(vector< vector<double> > &M, int r1, int r2){
    for(int i = 0; i < M[r1].size(); i++){
        swap(M[r1][i],M[r2][i]);
    }
    cout << "After changing rows " << r1 << " and " << r2 << ":" << endl;
    print(M);
}

void scale(vector< vector<double> > &M, int r1, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = s * M[r1][i];
    }
    cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    print(M);
}

void addUp(vector< vector<double> > &M, int r1, int r2, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
    cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    print(M);
}

void gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs){
    vector< vector<double> > temp(M.size())

    for (int i = 0; i <  temp.size(); ++i) {
        temp[i] = vector<double>(M[0].size());
    }

    bool done = false;
    int rowCount = 0;
    if (mpiRank == 0) {
        for (int i = 0; i < M.size(); ++i) {
            if(M[i][i] == 0){
                for(int j = 0; j < M.size(); j++){
                    if(M[j][i] != 0){
                        change(M, i, j);
                    }
                }
            }
        }

        // sending row to each process
        for (int i = rowCount; i < numberOfProcs - 1 && i < M.size(); ++i, ++rowCount) {
            MPI_Send(&M[0][0], (M[i].size() * M.size()), MPI_DOUBLE, (i + 1), rowCount, MCW);
            cout << "Master sending row " << i << " to processor " << (i + 1) << endl;
        }

        while(rowCount < M.size()) {
            int myFlag, myStatus, received;
            int epithet = -1;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
            if (myFlag) {
                MPI_RECV(&received, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myStatus);
                B[myStatus.MPI_TAG] = received;
                rowCount++;

                if(rowCount == M.size()) {
                    MPI_SEND(&epithet, 1, MPI_INT, myStatus.MPI_SOURCE, -1, MCW);
                } else {
                    MPI_SEND(&M[0][0], (M[i].size() * M.size()), MPI_DOUBLE, myStatus.MPI_SOURCE, rowCount, MCW);
                }
            }

        }
    } else {
        while(!done) {
            int myFlag, myStatus;
            MPI_Iprobe(0, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
            if (myFlag) {
                if (myStatus.MPI_TAG == -1) {
                    done = true;
                } else {
                    MPI_RECV(&temp, (M[0].size() * M.size()), MPI_DOUBLE, 0, MPI_ANY_TAG, MCW, &myStatus);
                    // Looping through X
                    int pivot = myStatus.MPI_TAG;
                    for(int i = 0; i < temp.size(); i++){

                        // scale pivot to 1
                        if(temp[pivot][pivot] == 0){
                            return -1;
                        }else{
                            scale(temp, pivot, 1.0 / temp[pivot][pivot]);
                        }

                        // Turn other numbers in column to 0
                        for(int j = 0; j < temp.size(); j++){
                            if(temp[j][i] != 0 && j != i){
                                addUp(temp, j, i, -temp[j][i]);
                            }
                        }
                    }
                }
            }
        }
    }

    cout << "Result:" << endl;
    print(M);

    return 0;
}