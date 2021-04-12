#include <iostream>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

int mpiRank, size, data;

void print(vector<double> &B);
void printMore(vector< vector<double> > &M);
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
void print(vector<double> &B){
    for(int i = 0; i < B.size(); i++){
        cout << B[i] << " " << endl;
    }
}

void printMore(vector< vector<double> > &M){
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
    printMore(M);
}

void scale(vector< vector<double> > &M, int r1, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = s * M[r1][i];
    }
    cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    printMore(M);
}

void addUp(vector< vector<double> > &M, int r1, int r2, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
    cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    printMore(M);
}

double gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs){
    MPI_Comm_rank(MCW, &mpiRank); 
    MPI_Comm_size(MCW, &size); 
    MPI_Status myStatus;
    vector< vector<double> > temp(1);

    temp[0] = vector<double>(M[0].size());

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
            MPI_Send(&M[i], M[0].size(), MPI_DOUBLE, (i + 1), rowCount, MCW);
            cout << "Master sending row " << i << " to processor " << (i + 1) << endl;
        }

        while(rowCount < M.size()) {
            int myFlag, received;
            int epithet = -1;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
            if (myFlag) {
                MPI_Recv(&received, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myStatus);
                B[myStatus.MPI_TAG] = received;
                rowCount++;

                if(rowCount == M.size()) {
                    MPI_Send(&epithet, 1, MPI_INT, myStatus.MPI_SOURCE, -1, MCW);
                } else {
                    MPI_Send(&M[rowCount], M[0].size(), MPI_DOUBLE, myStatus.MPI_SOURCE, rowCount, MCW);
                }
            }

        }
    } else {
        while(!done) {
            int myFlag;
            MPI_Iprobe(0, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
            if (myFlag) {
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

    cout << "Result:" << endl;
    print(B);

    return 0;
}