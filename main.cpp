#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

void print(vector< vector<double> > &M){
    for(int i = 0; i < M.size(); i++){
        for(int j = 0; j < M[i].size(); j++){
            cout << M[i][j] << " ";
        }
        cout << endl;
    }
}

void printMore(vector<double> &M){
    for(int i = 0; i < M.size(); i++){
        cout << M.at(i) << endl;
    }
}

void change(vector< vector<double> > &M, int r1, int r2){
    for(int i = 0; i < M[r1].size(); i++){
        swap(M[r1][i],M[r2][i]);
    }
    // cout << "After changing rows " << r1 << " and " << r2 << ":" << endl;
    // print(M);
}

void scale(vector< vector<double> > &M, int r1, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = s * M[r1][i];
    }
    // cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    // print(M);
}

void addUp(vector< vector<double> > &M, int r1, int r2, double s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
    // cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    // print(M);
}

double gje(vector< vector<double> > &M1, vector<double> &M2){

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
            cout << "No solution" << endl;
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
    for (int i = 0; i < M1.size(); ++i) {
        M2.push_back(M1.at(i).at(M1.at(0).size() - 1));
    }
    cout << "Result:" << endl;
    printMore(M2);

    return 0;
}

int main() {
    vector< vector<double> > M1(1);

    int matrixAttributes[1][3] = {{3, 4, (3 * 4)}};

    double A[3][4] = {{0.0 ,1.0 ,-3.0, -5.0}, {2.0, 3.0, -1.0, 7.0}, {4.0, 5.0, -2.0, 10.0}};
    
    // creating the 2d part of the vector and saving the values of A
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            M1.at(0).push_back(A[i][j]);
        }
    }

    for (int i = 0; i < M1.size(); ++i) {
        int numberOfElements = matrixAttributes[i][2];
        int rowLength = matrixAttributes[i][1];
        int colLength = matrixAttributes[i][0];

        vector< vector<double> > temp;
        vector<double> B;

        for (int j = 0; j < colLength; ++j) {
            temp.push_back(vector<double>(rowLength));
        }
        
        for (int k = 0; k < colLength; ++k) {
            for (int j = 0; j < rowLength; ++j) {
                temp.at(k).at(j) = M1.at(k).at(j);
            }
        }

        gje(temp, B);
    }

    return 0;
}