#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

void print(vector<vector<float>> &M){
    for(int i = 0; i < M.size(); i++){
        for(int j = 0; j < M[i].size(); j++){
            cout << M[i][j] << " ";
        }
        cout << endl;
    }
}

void printMore(vector<vector<float>> &M){
    for(int i = 0; i < M.size(); i++){
        for(int j = 0; j < M[i].size(); j++){
            cout << M[i][j] << " ";
        }
        cout << endl;
    }
}

void change(vector<vector<float>> &M, int r1, int r2){
    for(int i = 0; i < M[r1].size(); i++){
        swap(M[r1][i],M[r2][i]);
    }
    cout << "After changing rows " << r1 << " and " << r2 << ":" << endl;
    printMore(M);
}

void scale(vector<vector<float>> &M, int r1, float s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = s * M[r1][i];
    }
    cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    printMore(M);
}

void addUp(vector<vector<float>> &M, int r1, int r2, float s){
    for(int i = 0; i < M[r1].size(); i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
    cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    printMore(M);
}

float gje(vector<vector<float>> &M1, vector<float> &M2){
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
    printMore(M1);

    return 0;
}

int main() {

    vector<vector<float>> M = {{0.0 ,1.0 ,-3.0}, {2.0, 3.0, -1.0}, {4.0, 5.0, -2.0}};
    vector<float> B = {-5.0, 7.0, 10.0};

    gje(M, B);

    return 0;
}