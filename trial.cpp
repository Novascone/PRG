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
    int matrixAttributes[25][3] = {{3, 4, (3 * 4)}, {3, 4, (3 * 4)}, {3, 4, (3 * 4)}, {4, 5, (4 * 5)},
                                {10, 11, (10 * 11)}, {8, 9, (8 * 9)}, {3, 4, (3 *4)}, {6, 7, (6 * 7)},
                                {8, 9, (8 * 9)}, {10, 11, (10 * 11)}, {3, 4, (3 * 4)}, {4, 5, (4 * 5)},
                                {4, 5, (4 * 5)}, {10, 11, (10 * 11)}, {9, 10, (9 * 10)}, {5, 6, (5 * 6)},
                                {6, 7, (6 * 7)}, {8, 9, (8 * 9)}, {8, 9, (8 * 9)}, {3, 4, (3 * 4)},
                                {8, 9, (8 * 9)}, {7, 8, (7 * 8)}, {5, 6, (5 * 6)}, {8, 9, (8 * 9)},
                                {3, 4, (3 * 4)}}; // number of col, rows, and total elements per matrix

    vector<double> B; // used to store results

    bool done = false; // used to tell worker threads to stop
    int rowLength;
    int colLength;
    int numberOfElements;
    int matrixCount = 0;
    int retCount = 0;
    
    if (mpiRank == 0) {
        vector< vector<double> > M1(25);
        vector<double> sendTemp;

        for (int i = 0; i < M1.size(); ++i) {
            M1.at(i) = vector<double>();
        }
    
        // Creating an array matrix to pass into a 2d vector
        double A[3][4] = {{0.0 ,1.0 ,-3.0, -5.0}, {2.0, 3.0, -1.0, 7.0}, {4.0, 5.0, -2.0, 10.0}};
        double C[3][4] = {{1.0 ,1.0 ,1.0, 3.0}, {1.0, 2.0, 3.0, 0.0}, {1.0, 3.0, 2.0, 3.0}};
        double D[3][4] = {{2, -1, 3, 4}, {3, 0, 2, 5}, {-2, 1, 4, 6}};
        double E[4][5] = {{73, 136, 173, 112, 3}, {61, 165, 146, 14, 1}, {137, 43, 183, 73, 10}, {196, 40, 144, 31, 1}};
        double F[10][11] = {{44.,  5., 38., 30., 39., 41., 93., 71., 86., 12., 72.},
                            {77., 34., 59., 13., 82.,  6., 34.,  9., 51., 32., 44.},
                            {66., 25., 84., 12., 39., 27., 50., 28., 74., 23., 56.},
                            {96., 35., 94., 85., 93., 35., 55., 69., 64.,  9.,  9.},
                            {43., 84., 78., 98., 28., 83., 17., 67., 81., 59.,  7.},
                            {76., 56., 22., 86., 62., 60., 67., 44., 12., 72., 28.},
                            {89., 86., 85.,  5., 91., 82., 38., 37.,  3., 84.,  5.},
                            {54.,  9., 64., 92., 93., 90., 55., 13., 67., 99., 31.},
                            {26., 74., 86., 86., 12., 62., 77., 81.,  0., 74.,  2.},
                            { 2., 60., 79., 91., 63., 49., 23., 57.,  0., 51., 29.}};
        double G[8][9] = {{63., 84., 44., 97., 70., 25., 18., 28., 44.},
                            {75., 55.,  5., 17.,  9., 19., 47.,  8., 66.},
                            {41., 82., 24., 91., 93., 47.,  4., 15., 95.},
                            {56., 49., 19., 29., 38., 18., 79., 30., 23.},
                            {62., 53., 38., 93., 68.,  7., 54., 35., 41.},
                            { 3., 47., 28., 47., 83., 70., 73., 33., 32.},
                            {63.,  4., 87., 74., 40.,  9., 65., 17., 49.},
                            {12., 76., 21., 32., 26., 10., 25., 94., 64.}};
        double H[3][4] = {{47., 11., 21., 12.},
                            {42., 40., 10.,  4.},
                            {74., 68., 25., 47.}};
        double I[6][7] = {{73., 23., 55., 65., 83., 34.,  2.},
                            {97., 90., 23., 26., 36., 79., 94.},
                            {67., 15.,  9., 15., 18., 98., 84.},
                            {86., 95., 68., 68., 30., 34., 60.},
                            {84., 76., 94., 44., 14.,  6., 15.},
                            {64., 37., 27., 69., 25., 57., 14.}};
        double J[8][9] = {{53., 71., 60., 45., 69., 14., 19., 69., 86.},
                            {88., 79., 53., 57.,  2., 52., 88., 94., 19.},
                            {81., 48., 36.,  3., 64., 41., 43.,  2., 16.},
                            {90., 47., 57., 12., 13., 29., 91.,  7., 70.},
                            {77.,  4.,  4., 84., 93., 23., 48., 52., 82.},
                            {30., 50., 18., 84., 92., 83.,  9., 83., 69.},
                            {87., 82., 67., 91., 31., 57., 61., 76., 61.},
                            {84., 23., 76., 63., 73., 29.,  8., 78.,  4.}};
        double K[10][11] = {{70., 48., 20., 74., 55., 18., 43., 55., 96., 87., 40.},
                            {39., 45., 79., 89., 90., 28., 74., 17., 53., 73., 86.},
                            {42., 13., 18., 84., 40., 33., 25., 37., 61., 59., 61.},
                            {45., 51., 62., 56.,  3., 20., 24., 91., 59., 84., 31.},
                            {45., 51., 33., 96., 29., 65., 56., 63., 86., 65., 30.},
                            {81., 28., 72., 95.,  4., 98., 88., 40., 34., 26.,  4.},
                            {21., 26., 58.,  7., 83., 79., 23.,  2., 10., 41., 51.},
                            {55., 29., 77., 53., 26., 61., 25., 52., 25., 72., 17.},
                            {73., 87., 57., 65., 28., 82., 73.,  9., 23., 84., 91.},
                            {41., 86., 95., 67., 59., 83., 62., 15., 56., 59., 63.}};
        double L[3][4] = {{19., 20., 33., 81.},
                            {92., 14., 64., 36.},
                            {43., 90.,  4.,  1.}};
        double N[4][5] = {{49., 90., 85.,  1.,  0.},
                            {18., 34., 29., 97.,  0.},
                            {86., 52., 27., 18.,  6.},
                            {63., 55., 62., 71., 26.}};
        double O[4][5] = {{59., 85., 12., 34., 24.},
                            {23., 27., 70., 11., 73.},
                            {26., 47., 91., 53., 42.},
                            {74., 26., 90., 79., 88.}};
        double P[10][11] = {{41., 73.,  5., 33., 84., 32., 16., 86., 94.,  7., 91.},
                            {66., 27., 85., 59., 34., 24., 21., 10., 72., 60., 59.},
                            {90., 32., 71., 34., 73., 19.,  3.,  2., 24., 77., 77.},
                            {42., 52., 28.,  5., 70., 12., 17.,  9., 74., 51.,  3.},
                            { 5., 97., 48., 11., 81., 80.,  1., 25., 71., 25., 30.},
                            {89., 23., 33., 76., 98., 86., 76., 97., 66., 94., 64.},
                            {58.,  3., 15., 99., 24., 75., 69., 94.,  0., 84., 93.},
                            {21., 70., 62., 41., 29.,  6., 22., 34., 78., 51., 14.},
                            {25., 86., 87.,  0., 52., 48., 78., 61., 57., 55., 31.},
                            {97., 19., 34., 13., 47., 58., 51., 44., 52., 24., 53.}};
        double Q[9][10] = {{85., 38., 87., 31., 40., 63., 87., 23.,  2., 30.},
                            {76., 42.,  6., 77., 48., 62., 26., 24., 84., 77.},
                            {38., 69., 68., 22., 30., 43., 39., 56., 43., 71.},
                            {99., 38., 86., 72., 94., 77., 23., 11., 34.,  7.},
                            {83.,  4., 32., 45., 24., 38., 61., 65., 99., 30.},
                            {11., 91., 17., 56.,  9., 93., 59.,  3., 97., 53.},
                            {31., 71., 57., 73., 20., 83., 18.,  4.,  3., 65.},
                            {74., 67., 97., 47., 88., 74., 55., 16., 22., 45.},
                            { 7., 21., 79., 64., 69., 75., 58., 16., 47., 19.}};
        double R[5][6] = {{32., 12., 79.,  0., 37., 31.},
                            {63., 92., 70., 34., 62., 19.},
                            { 8., 87., 83., 10., 53., 59.},
                            {11., 90., 53., 35., 54., 31.},
                            {72., 39., 59., 35., 76., 92.}};
        
        double S[6][7] = {{26., 85., 24., 76., 50., 54., 80.},
                            {65., 24., 64., 11.,  3., 20., 96.},
                            {36., 22., 75., 87.,  7., 64., 74.},
                            {22., 43., 22., 48., 21., 14., 93.},
                            {30., 14., 37., 26., 23., 95., 24.},
                            {68., 24., 27., 67., 36., 77., 80.}};
        
        double T[8][9] = {{23., 64., 44.,  7., 22., 69., 20., 70., 21.},
                            {84., 44., 37., 39., 26., 34., 19., 64., 46.},
                            {72., 47., 18., 31.,  7., 27., 91.,  2., 17.},
                            {23., 67., 63., 11., 87., 17., 55., 10., 81.},
                            {76., 53., 65., 18., 90., 54., 76., 66., 83.},
                            {19., 64., 98.,  4., 34., 48., 13.,  8., 89.},
                            {20., 35., 79., 46.,  5., 79., 57., 64.,  4.},
                            {23., 67., 68., 81., 96., 36., 61., 11., 22.}};
        
        double U[8][9] = {{91.,  8., 88., 59., 11., 83., 17., 98., 93.},
                            {65.,  7., 82., 68.,  6., 42., 40., 22., 19.},
                            {58., 18., 94., 60., 53., 70., 27., 43., 26.},
                            {99., 99., 96., 17., 48., 81., 56., 38., 65.},
                            {80., 45., 12., 65., 59., 67., 78.,  7., 33.},
                            {35., 89., 20.,  0., 36., 77., 20., 86., 41.},
                            {81., 58., 86.,  4., 19., 25.,  3., 58.,  4.},
                            {44., 17., 62., 53., 50., 22., 73., 26., 87.}};
        
        double V[3][4] = {{10., 43.,  8., 21.},
                            {71., 14., 75., 45.},
                            { 6., 64., 85.,  3.}};
        
        double W[8][9] = {{74., 33., 31., 43., 54., 42., 34., 28., 41.},
                            {13., 51., 57., 41., 40., 20., 65., 29., 76.},
                            {46., 79.,  8., 43., 29.,  6., 94., 23., 97.},
                            {64., 53., 48., 76., 83., 63., 21., 64., 37.},
                            { 3., 98., 66., 33., 75., 74., 30., 80., 23.},
                            {90., 46., 76., 80., 90.,  5., 40.,  8., 35.},
                            {60., 89., 86., 53., 19., 31., 19., 43., 97.},
                            {70., 31., 94., 17., 80., 79., 40., 83., 25.}};
        
        double X[7][8] = {{61., 55., 45., 92., 83., 42., 44., 63.},
                            {31., 34., 47., 81.,  1., 37., 34., 70.},
                            {32., 96., 85., 89., 85., 75., 63., 51.},
                            {40., 21., 67., 80.,  7.,  1., 20., 61.},
                            {60., 76., 97., 11., 42., 80.,  6., 25.},
                            {92., 33., 33., 12., 84., 10., 26., 20.},
                            {20., 31., 41., 57., 69., 62., 18., 66.}};
        
        double Y[5][6] = {{47., 72., 49., 56., 91., 64.},
                            {98., 72.,  6., 61., 88., 15.},
                            {66., 21., 55., 38., 61., 64.},
                            {27., 68., 50., 75., 79., 32.},
                            {81., 74.,  8., 57.,  8.,  5.}};
        
        double Z[8][9] = {{84., 67., 35., 71., 75., 69., 18., 82., 80.},
                            {39., 65., 63., 55., 33., 69., 59., 75., 63.},
                            {35., 83., 46., 56., 20., 33., 20., 48., 94.},
                            { 8., 45., 48., 15., 19.,  8., 87., 10.,  6.},
                            {32., 64., 58., 12., 78., 23., 69., 78., 63.},
                            {71.,  6., 58., 61., 69., 48., 75., 60., 56.},
                            {84., 90.,  7., 77., 70., 62., 29., 21., 27.},
                            {91.,  5., 56., 96., 72., 29., 57., 25., 38.}};
        
        double AA[3][4] = {{18., 73., 92., 63.},
                            {83., 90., 70., 72.},
                            {30., 20., 37., 41.}};

        // creating the 2d part of the vector and saving the values of A
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(0).push_back(A[i][j]);
            }
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(1).push_back(C[i][j]);
            }
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(2).push_back(D[i][j]);
            }
        }

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 5; ++j) {
                M1.at(3).push_back(E[i][j]);
            }
        }

        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 11; ++j) {
                M1.at(4).push_back(F[i][j]);
            }
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 9; ++j) {
                M1.at(5).push_back(G[i][j]);
            }
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(6).push_back(H[i][j]);
            }
        }

        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 7; ++j) {
                M1.at(7).push_back(I[i][j]);
            }
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 9; ++j) {
                M1.at(8).push_back(J[i][j]);
            }
        }

        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 11; ++j) {
                M1.at(9).push_back(K[i][j]);
            }
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(10).push_back(L[i][j]);
            }
        }

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 5; ++j) {
                M1.at(11).push_back(N[i][j]);
            }
        }

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 5; ++j) {
                M1.at(12).push_back(O[i][j]);
            }
        }

        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 11; ++j) {
                M1.at(13).push_back(P[i][j]);
            }
        }

        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 10; ++j) {
                M1.at(14).push_back(Q[i][j]);
            }
        }

        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 6; ++j) {
                M1.at(15).push_back(R[i][j]);
            }
        }

        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 7; ++j) {
                M1.at(16).push_back(S[i][j]);
            }
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 9; ++j) {
                M1.at(17).push_back(T[i][j]);
            }
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 9; ++j) {
                M1.at(18).push_back(U[i][j]);
            }
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(19).push_back(V[i][j]);
            }
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 9; ++j) {
                M1.at(20).push_back(W[i][j]);
            }
        }

        for (int i = 0; i < 7; ++i) {
            for (int j = 0; j < 8; ++j) {
                M1.at(21).push_back(X[i][j]);
            }
        }

        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 6; ++j) {
                M1.at(22).push_back(Y[i][j]);
            }
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 9; ++j) {
                M1.at(23).push_back(Z[i][j]);
            }
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                M1.at(24).push_back(AA[i][j]);
            }
        }

        for (int i = 0; i < 25 && i < (mpiSize - 1); ++i, ++matrixCount) {
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
                if(retCount == 24) {
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