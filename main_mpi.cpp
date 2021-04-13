#include <iostream>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

int mpiRank, mpiSize, data, N;

void print(vector<double> &B);
void printMore(vector< vector<double> > &M);
void change(vector< vector<double> > &M, int r1, int r2);
void scale(vector<double> &M, int r1, double s);
void scale(vector< vector<double> > &M, int r1, double s);
void addUp(vector< vector<double> > &M, int r1, int r2, double s);
double gje(vector< vector<double> > &M, vector<double> &B, int numberOfProcs);

int main(int argc,char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &mpiRank); 
    MPI_Comm_size(MCW, &mpiSize); 

    // Creating an array matrix to pass into a 2d vector
    double A[3][4] = {{0.0 ,1.0 ,-3.0, -5.0}, {2.0, 3.0, -1.0, 7.0}, {4.0, 5.0, -2.0, 10.0}};

    vector< vector<double> > M(3);
    vector<double> B(3);

    // creating the 2d part of the vector and saving the values of A
    for (int i = 0; i < 3; ++i) {
        M.at(i) = vector<double>(4);
        for (int j = 0; j < 4; ++j) {
            M.at(i).at(j) = A[i][j];
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
        cout << B.at(i) << " " << endl;
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

void printArray(double B[]){
    for(int i = 0; i < sizeof(B) / sizeof(B[0]); i++){
        cout << B[i] << " " << endl;
    }
}

// helper function that swaps rows of matrix
void change(vector< vector<double> > &M, int r1, int r2){
    for(int i = 0; i < M[r1].size(); i++){
        swap(M.at(r1).at(i),M.at(r2).at(i));
    }
    cout << "After changing rows " << r1 << " and " << r2 << ":" << endl;
    printMore(M);
}

// helper function that scales rows to make the pivot 1
void scale(double M[], int r1, double s, int col){
    for(int i = r1 * col; i < i + col; i++){
        M[i] = s * M[i];
    }
    cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    printArray(M);
}
void scale(vector< vector<double> > &M, int r1, double s){
    for(int i = 0; i < M.at(r1).size(); i++){
        M.at(r1).at(i) = s * M.at(r1).at(i);
    }
    cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    printMore(M);
}

// helper function to make other rows in pivot column 0
void addUp(double M[], int r1, int r2, double s, int col){
    int inc = 0;
    for(int i = r1 * col; i < i + col; i++, inc++){
        M[i] = M[i] + (s * M[r2 * col]);
    }
    cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    printArray(M);
}
void addUp(vector< vector<double> > &M, int r1, int r2, double s){
    for(int i = 0; i < M.at(r1).size(); i++){
        M.at(r1).at(i) = M.at(r1).at(i) + (s * M.at(r2).at(i));
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
    vector<double> temp(4);
    // temp[0] = vector<double>(M.at(0).size());
    bool done = false; // used to tell worker threads to stop
    int rowCount = 0; // used to know how many rows have been used and when we run out
    int rowLength = M.at(0).size();
    int colLength = M.size();
    int numberOfElements = rowLength * colLength;
    cout << "Number of elements: " << numberOfElements << endl;
    double data[numberOfElements];

    for (int i = 0; i < numberOfElements; i++) {
        data[i] = 0.0;
    }

    cout << "Print out data array with this number of elements: " << (sizeof(data) / sizeof(data[0])) << endl;
    for(int i = 0; i < sizeof(data) / sizeof(data[0]); i++){
        cout << B[i] << " " << endl;
    }

    if (mpiRank == 0) {
        // if rank == 0, perform swaps before sending rows to other processes
        for (int i = 0; i < M.size(); ++i) {
            if(M.at(i).at(i) == 0){
                for(int j = 0; j < M.size(); j++){
                    if(M.at(j).at(i) != 0){
                        change(M, i, j);
                    }
                }
            }
        }

        int count = 0;
        for (int i = 0; i < colLength; ++i) {
            for (int j = 0; j < rowLength; ++j, count++) {
                data[count] = M.at(i).at(j); 
            }
        }

        // sending a row to each process
        for (int i = 0; i < (numberOfProcs - 1) && i < M.size(); i++, rowCount++) {
            MPI_Send(data, (rowLength * colLength), MPI_DOUBLE, (i + 1), rowCount, MCW);
            cout << "Master sending row " << i << " to processor " << (i + 1) << endl;
        }

        // while loop runs until there are no more rows to send out
        while(rowCount < M.size()) {
            int myFlag;
            double received;
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
                    MPI_Send(&epithet, 1, MPI_INT, myStatus.MPI_SOURCE, 101, MCW);
                } else {
                    MPI_Send(data, (rowLength * colLength), MPI_DOUBLE, myStatus.MPI_SOURCE, rowCount, MCW);
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
                if (myStatus.MPI_TAG == 101) {
                    done = true;
                } else {
                    MPI_Recv(data, (rowLength * colLength), MPI_DOUBLE, 0, MPI_ANY_TAG, MCW, &myStatus);
                    int pivot = myStatus.MPI_TAG;

                    // scale pivot to 1
                    if(data[rowLength * pivot] == 0){
                        cout << "There is no solution" << endl;
                        return -1;
                    }else{
                        scale(data, pivot, 1.0 / data[rowLength * pivot], colLength);
                    }

                    int loopCount = 0;
                    for(int i = pivot; loopCount < colLength; i += rowLength){
                        // Turn other numbers in column to 0
                        if(data[i] != 0 && i != (pivot * colLength)) {
                            addUp(data, i, pivot, -data[i], colLength);
                        }
                    }
                    MPI_Send(&data[pivot * colLength + colLength], 1, MPI_DOUBLE, 0, pivot, MCW);
                }
            }
        }
    }

    // print the results of the function
    cout << "Result:" << endl;
    print(B);

    return 0;
}