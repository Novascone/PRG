#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;
#define n 3

void print(float M[][n]){
    for(int i = 0; i <n; i++){
        for(int j = 0; j < n; j++){
            cout<<M[i][j]<<" ";
        }
        cout<<endl;
    }
}

void printMore(float M[][n+1]){
    for(int i = 0; i <n; i++){
        for(int j = 0; j < n + 1; j++){
            cout<<M[i][j]<<" ";
        }
        cout<<endl;
    }
}

void change(float M[][4], int r1, int r2){
    for(int i = 0; i < 4; i++){
        swap(M[r1][i],M[r2][i]);
    }
    cout << "After changing rows " << r1 << " and " << r2 << ":" << endl;
    printMore(M);
}

void scale(float M[][n + 1],int r1, float s){
    for(int i = 0; i < n + 1; i++){
        M[r1][i] = s * M[r1][i];
    }
    cout << "After scaling row " << r1 << " by " << s << ":" << endl;
    printMore(M);
}

void addUp(float M[][4], int r1, int r2, float s){
    for(int i = 0; i < 4; i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
    cout << "After addUp of row " << r1 << " with row " << r2 << " and " << s << ":" <<  endl;
    printMore(M);
}

float gje(float A[][n], float B[n]){
    float X[n][n + 1];
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n + 1; j++){
            if(j == n){
                X[i][j] = B[i];
            }else{
                X[i][j] = A[i][j];
            }
            cout << X[i][j] << " ";
        }
        cout << endl;
    }

    // Looping through X
    for(int i = 0; i < n; i++){
        // Get a non-zero pivot
        if(X[i][i] == 0){
            for(int j = 0; j < n; j ++){
                if(X[j][i] != 0){
                    change(X,i,j);
                }
            }
        }

        // scale pivot to 1
        if(X[i][i] == 0){
            return -1;
        }else{
            scale(X, i, 1.0 / X[i][i]);
        }

        // Turn other numbers in column to 0
        for(int j = 0; j < n; j++){
            if(X[j][i] != 0 && j != i){
                addUp(X,j,i, -X[j][i]);
            }
        }
    }

    cout << "Result:" << endl;
    printMore(X);

    return 0;
}

int main() {
    float M[3][3] = {{0.0 ,1.0 ,-3.0}, {2.0, 3.0, -1.0}, {4.0, 5.0, -2.0}};
    float B[3] = {-5.0, 7.0, 10.0};

    // cout<<"unswapped"<<endl; 
    // print(M);
    
    // change(M,0,2);

    // cout<<"swapped"<<endl; 
    // print(M);
    
    // cout<<"scale"<<endl; 
    // scale(M,2,4);
    // print(M);

    // cout<<"add"<<endl; 
    // addUp(M,0,1,5);
    // print(M);

    gje(M,B);

    return 0;
}