#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

int mpiRank, mpiSize;

void print(vector<double> &B, int row);
void printMore(vector< vector<double> > &M);
void change(vector< vector<double> > &M, int r1, int r2);
void scale(vector <vector<double> > &M, int r1, double s);
void addUp(vector <vector<double> > &M, int r1, int r2, double s);
double gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs);

int main(int argc,char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &mpiRank); 
    MPI_Comm_size(MCW, &mpiSize);
    MPI_Status myStatus; 
    int matrixAttributes[1][3] = {{3, 4, (3 * 4)}}; // number of col, rows, and total elements per matrix

    vector<double> B; // used to store results

    bool done = false; // used to tell worker threads to stop
    int rowLength;
    int colLength;
    int numberOfElements;
    int matrixCount = 0; // Used to check how many matrices have been sent out
    int retCount = 0; // Used to count the returns from worker processors
    
    if (mpiRank == 0) {
        //  We create a 2d vector to house all matrices
        vector< vector<double> > M1(1);
        // We send the matrices through vector double for easy sending.
        // There is a better way but we just couldn't understand it
        vector<double> sendTemp;

        for (int i = 0; i < M1.size(); ++i) {
            M1.at(i) = vector<double>();
        }
    
        // Creating an array matrix to pass into a 2d vector
        double A[3][4] = {{0.0 ,1.0 ,-3.0, -5.0}, {2.0, 3.0, -1.0, 7.0}, {4.0, 5.0, -2.0, 10.0}};

        // creating the 2d part of the vector and saving the values of A
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(0).push_back(A[i][j]);
            }
        }

        // Sending the matrices out to each process
        for (int i = 0; i < M1.size() && i < (mpiSize - 1); ++i, ++matrixCount) {
            for (int j = 0; j < matrixAttributes[i][2]; ++j) {
                sendTemp.push_back(M1.at(i).at(j));
            }
            cout << "Master sending matrix " << matrixCount << " to processor " << i << endl;
            MPI_Send(&sendTemp[0], matrixAttributes[i][2], MPI_DOUBLE, (i + 1), i, MCW);
        }

        while (!done) {
            int myFlag;
            int data;
            int epithet = -1;
            // probing to ensure there is a message to receive
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
            if (myFlag) {
                // once received, we save the received double to B which houses the results
                // using MPI_TAG to know pivot/row of result
                MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &myStatus);
                retCount++;

                // checking to see if there are more rows to send
                // if there aren't, we send a kill tag to other processors
                if(retCount == (M1.size() - 1)) {
                    // sending a kill message to each worker
                    for (int i = 1; i < mpiSize; ++i) {
                        MPI_Send(&epithet, 1, MPI_INT, i, 101, MCW);
                    }
                    done = true;
                } else {
                    sendTemp.clear();

                    for (int i = 0; i < matrixAttributes[matrixCount][2]; ++i) {
                        sendTemp.push_back(M1.at(matrixCount).at(i));
                    }
                    cout << "Master sending matrix " << matrixCount << " to processor " << myStatus.MPI_SOURCE << endl;
                    MPI_Send(&sendTemp[0], matrixAttributes[matrixCount][2], MPI_DOUBLE, myStatus.MPI_SOURCE, matrixCount, MCW);
                    matrixCount++;
                }
            }
        } 
    } else {
        while(!done) {
        int myFlag;
        vector<double> temp;
        vector< vector<double> > recTemp;
        // probing to ensure the is a message to receive
        MPI_Iprobe(0, MPI_ANY_TAG, MCW, &myFlag, &myStatus);
        if (myFlag) {
            // if kill message is received, we get out of the loop
            if (myStatus.MPI_TAG == 101) {
                cout << "No more matrices for processor " << mpiRank << endl;
                done = true;
            } else {
                numberOfElements = matrixAttributes[myStatus.MPI_TAG][2];
                rowLength = matrixAttributes[myStatus.MPI_TAG][1];
                colLength = matrixAttributes[myStatus.MPI_TAG][0];

                // temp used for storing the passed vectors when using MPI_Recv
                for (int i = 0; i < numberOfElements; ++i) {
                    temp.push_back(0.0);
                }
                for (int i = 0; i < colLength; ++i) {
                    recTemp.push_back(vector<double>(rowLength));
                }

                MPI_Recv(&temp[0], numberOfElements, MPI_DOUBLE, 0, MPI_ANY_TAG, MCW, &myStatus);

                int tempCounter = 0;
                for (int i = 0; i < colLength; ++i) {
                    for (int j = 0; j < rowLength; ++j, tempCounter++) {
                        recTemp.at(i).at(j) = temp.at(tempCounter);
                    }
                }

                gje(recTemp, B, mpiSize);

                MPI_Send(&numberOfElements, 1, MPI_INT, 0, myStatus.MPI_TAG, MCW);
                cout << "Processor " << mpiRank << " is checking with Rank 0 for another matrix to solve..." << endl;
                }
            }
        }
        cout << mpiRank << " is dead" << endl;
    }

    MPI_Finalize();
}

// Used to print the final result of the elimination
void print(vector<double> &B){
    for(int i = 0; i < B.size(); i++){
        cout << B.at(i) << endl;
    }
}

// used to print a 2d vector
void printMore(vector< vector<double> > &M){
    for(int i = 0; i < M.size(); i++){
        for(int j = 0; j < M.at(0).size(); j++){
            cout << M.at(i).at(j) << " ";
        }
        cout << endl;
    }
}

// helper function that swaps rows of matrix
void change(vector< vector<double> > &M, int r1, int r2){
    for(int i = 0; i < M[r1].size(); i++){
        swap(M.at(r1).at(i),M.at(r2).at(i));
    }
}

// helper function that scales rows to make the pivot 1
void scale(vector< vector<double> > &M, int r1, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = s * M[r1][i];
    }
}

// helper function to make other rows in pivot column 0
void addUp(vector< vector<double> > &M, int r1, int r2, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
}

// The gaussian-jordan elimination function that utilizes helper functions to get results
double gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs){

    // Looping through X
    for(int i = 0; i < M.size(); i++){
        // Get a non-zero pivot
        if(M[i][i] == 0){
            for(int j = 0; j < M.size(); j++){
                if(M[j][i] != 0){
                    change(M, i, j);
                }
            }
        }

        // scale pivot to 1
        if(M[i][i] == 0){
            cout << "No Solution" << endl;
            return -1;
        }else{
            scale(M, i, 1.0 / M[i][i]);
        }

        // Turn other numbers in column to 0
        for(int j = 0; j < M.size(); j++){
            if(M[j][i] != 0 && j != i){
                addUp(M, j, i, -M[j][i]);
            }
        }
    }

    B.clear();

    // This pushes the solution to a B vector that houses them
    for (int i = 0; i < M.size(); ++i) {
        B.push_back(M.at(i).at(M.at(0).size() - 1));
    }
    cout << "Result:" << endl;
    print(B);

    return 0;
}