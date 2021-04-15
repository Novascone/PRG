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
    int matrixAttributes[3][3] = {{3, 4, (3 * 4)}, {12, 13, (12 * 13)}, {4, 5, (4 * 5)}}; // number of col, rows, and total elements per matrix
    vector<double> B; // used to store results

    bool done = false; // used to tell worker threads to stop
    int rowLength;
    int colLength;
    int numberOfElements;
    int matrixCount = 0;
    int retCount = 0;
    
    if (mpiRank == 0) {
        vector< vector<double> > M1(3);
        vector<double> sendTemp;

        for (int i = 0; i < M1.size(); ++i) {
            M1.at(i) = vector<double>();
        }
    
        // Creating an array matrix to pass into a 2d vector
        double A[3][4] = {{0.0 ,1.0 ,-3.0, -5.0}, {2.0, 3.0, -1.0, 7.0}, {4.0, 5.0, -2.0, 10.0}};
        // double C[3][4] = {{1.0 ,1.0 ,1.0, 3.0}, {1.0, 2.0, 3.0, 0.0}, {1.0, 3.0, 2.0, 3.0}};
        // double C[3][4] = {{2, -1, 3, 4}, {3, 0, 2, 5}, {-2, 1, 4, 6}};
        double D[4][5] = {{73, 136, 173, 112, 3}, {61, 165, 146, 14, 1}, {137, 43, 183, 73, 10}, {196, 40, 144, 31, 1}};
        double C[12][13] = {{44.,  5., 38., 30., 39., 41., 93., 71., 86., 12., 72., 92., 14.},
                            {77., 34., 59., 13., 82.,  6., 34.,  9., 51., 32., 44.,  7., 23.},
                            {66., 25., 84., 12., 39., 27., 50., 28., 74., 23., 56., 97., 24.},
                            {96., 35., 94., 85., 93., 35., 55., 69., 64.,  9.,  9., 40., 72.},
                            {43., 84., 78., 98., 28., 83., 17., 67., 81., 59.,  7., 99., 63.},
                            {76., 56., 22., 86., 62., 60., 67., 44., 12., 72., 28., 17., 89.},
                            {89., 86., 85.,  5., 91., 82., 38., 37.,  3., 84.,  5., 21., 40.},
                            {54.,  9., 64., 92., 93., 90., 55., 13., 67., 99., 31., 23., 39.},
                            {26., 74., 86., 86., 12., 62., 77., 81.,  0., 74.,  2., 28., 19.},
                            { 2., 60., 79., 91., 63., 49., 23., 57.,  0., 51., 29., 15., 81.},
                            {12., 42., 55., 85., 72., 22., 57.,  1., 17., 95., 64., 11., 24.},
                            {72., 73., 57., 43., 92., 91., 74., 61., 18., 30.,  2., 33., 62.}};

        // creating the 2d part of the vector and saving the values of A
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(0).push_back(A[i][j]);
            }
        }

        for (int i = 0; i < 12; ++i) {
            for (int j = 0; j < 13; ++j) {
                M1.at(1).push_back(C[i][j]);
            }
        }

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 5; ++j) {
                M1.at(2).push_back(D[i][j]);
            }
        }

        for (int i = 0; i < M1.size() && i < (mpiSize - 1); ++i, ++matrixCount) {
            for (int j = 0; j < matrixAttributes[i][2]; ++j) {
                sendTemp.push_back(M1.at(i).at(j));
            }

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
                cout << "Return count: " << retCount << endl;

                // checking to see if there are more rows to send
                // if there aren't, we send a kill tag to other processors
                if(retCount == (M1.size() - 1)) {
                    for (int i = 1; i < mpiSize; ++i) {
                        MPI_Send(&epithet, 1, MPI_INT, i, 101, MCW);
                    }
                    done = true;
                } else {
                    sendTemp.clear();

                    for (int i = 0; i < matrixAttributes[matrixCount][2]; ++i) {
                        sendTemp.push_back(M1.at(matrixCount).at(i));
                    }
                    MPI_Send(&sendTemp[0], matrixAttributes[matrixCount][2], MPI_DOUBLE, myStatus.MPI_SOURCE, matrixCount, MCW);
                    cout << "Master sending matrix " << matrixCount << " to processor " << myStatus.MPI_SOURCE << endl;
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
            cout << "My flag flag: " << myFlag << endl;
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
    // cout << "After changing rows " << r1 << " and " << r2 << ":" << endl;
    // printMore(M);
}

// helper function that scales rows to make the pivot 1
void scale(vector< vector<double> > &M, int r1, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = s * M[r1][i];
    }
    // cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    // printMore(M);
}

// helper function to make other rows in pivot column 0
void addUp(vector< vector<double> > &M, int r1, int r2, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
    // cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    // printMore(M);
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

    for (int i = 0; i < M.size(); ++i) {
        B.push_back(M.at(i).at(M.at(0).size() - 1));
    }
    cout << "Result:" << endl;
    print(B);

    return 0;
}