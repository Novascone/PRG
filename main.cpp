#include<iostream>
#include<algorithm>
#define n 3

void change(int M[][4], int r1, int r2){
    for(int i = 0; i < 4; i++){
        std::swap(M[r1][i],M[r2][i]);
    }
}

void scale(int M[][4],int r1, int s){
    for(int i = 0; i < 4; i++){
        M[r1][i] = s * M[r1][i];
    }
}

void addUp(int M[][4], int r1, int r2, int s){
    for(int i = 0; i < 4; i++){
        M[r1][i] = M[r1][i] + (s * M[r2][i]);
    }
}

void print(int M[][n]){
    for(int i = 0; i <n; i++){
        for(int j = 0; j < n; j++){
            std::cout<<M[i][j];
        }
        std::cout<<std::endl;
    }
}

void printMore(int M[][n+1]){
    for(int i = 0; i <n; i++){
        for(int j = 0; j < n + 1; j++){
            std::cout<<M[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
}

int gje(int A[][n], int B[n]){
    int X[n][n + 1];
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n + 1; j++){
            if(j == n){
                X[i][j] = B[i];
            }else{
                X[i][j] = A[i][j];
            }
        }
    }

    for(int i = 0; i < n - 1; i++){
        if(X[i][i] == 0){
            for(int j = 0; j < n - 1; j ++){
                if(X[j][i] != 0){
                    change(X,i,j);
                }
            }
        }
    
        if(X[i][i] == 0){
            return -1;
        }else{
            scale(X,i,1/X[i][i]);
        }

        for(int j = 0; j < n; j++){
            if(X[j][i] != 0 && j != i){
                addUp(X,j,i, -X[j][i]);
            }
        }
    }


    printMore(X);

    return 0;
}

int main() {
    int M[3][3] = {{0,1,-3}, {2,3,-1},{4,5-2}};
    int B[3] = {-5,7,10};
    // std::cout<<"unswapped"<<std::endl; 
    // print(M);
    
    // change(M,0,2);

    // std::cout<<"swapped"<<std::endl; 
    // print(M);
    
    // std::cout<<"scale"<<std::endl; 
    // scale(M,1,5);
    // print(M);

    // std::cout<<"add"<<std::endl; 
    // addUp(M,0,1,5);
    // print(M);

    gje(M,B);

    return 0;
}