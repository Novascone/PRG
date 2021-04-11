#include <iostream>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

void print(vector< vector<double> > &M);
void change(vector< vector<double> > &M, int r1, int r2);
void scale(vector< vector<double> > &M, int r1, double s);
void addUp(vector< vector<double> > &M, int r1, int r2, double s);
double gje(vector< vector<double> > &M1, vector<double> &M2);

int main(int argc,char *argv[]) {
    int rank, size;
    int data;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 

    int A[3][3] = {{0.0 ,1.0 ,-3.0}, {2.0, 3.0, -1.0}, {4.0, 5.0, -2.0}};
    int C[] = {-5.0, 7.0, 10.0};

    vector< vector<double> > M(3);
    vector<double> B;

    for (int i = 0; i < 3; ++i) {
        M[i] = vector<double>(3);
        for (int j = 0; j < 3; ++j) {
            M[i][j] = A[i][j];
        }
    }
    
    for (int i = 0; i < 3; ++i) {
        B.push_back(C[i]);
    }


    gje(M, B);

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

double gje(vector< vector<double> > &M1, vector<double> &M2){
    for(int i = 0; i < M1.size(); i++){
        M1[i].push_back(M2[i]);
    }

    // Looping through X
    for(int i = 0; i < M1.size(); i++){
        // Get a non-zero pivot
        if(M1[i][i] == 0){
            for(int j = 0; j < M1.size(); j++){
                if(M1[j][i] != 0){
                    change(M1, i, j);
                }
            }
        }

        // scale pivot to 1
        if(M1[i][i] == 0){
            return -1;
        }else{
            scale(M1, i, 1.0 / M1[i][i]);
        }

        // Turn other numbers in column to 0
        for(int j = 0; j < M1.size(); j++){
            if(M1[j][i] != 0 && j != i){
                addUp(M1, j, i, -M1[j][i]);
            }
        }
    }

    cout << "Result:" << endl;
    print(M1);

    return 0;
}