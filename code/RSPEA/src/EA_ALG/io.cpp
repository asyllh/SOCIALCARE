/*--------------/
EA_ALG
io.h
UoS
01/02/2022
/--------------*/

#include "io.h"


void PrintSolMatrix(struct Instance* ip){

    for(int i = 0; i < ip->nNurses; ++i){
        for(int j = 0; j < ip->nJobs; ++j){
            if(ip->solMatrix[i][j] > -1){
                printf("%2d\t", ip->solMatrix[i][j]);
            }
            else{
                printf(" .\t", ip->solMatrix[i][j]);
            }
        }
        printf("\t\\\\\n");
    }

}// End of PrintSolMatrix function

void PrintIntMatrix(std::vector<std::vector<int> >& matrix, int nRows, int nCols){

    for(int i = 0; i < nRows; ++i){
        for(int j = 0; j < nCols; ++j){
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }

}// End of PrintIntMatrix function


void PrintAllNurseRoutes(struct Instance* ip){

    for(int i = 0; i < ip->nNurses; ++i){
        for(int j = 0; j < ip->nJobs; ++j){
            if(ip->allNurseRoutes[i][j] > -1){
                printf("%2d\t", ip->allNurseRoutes[i][j]);
            }
            else{
                printf(" .\t", ip->allNurseRoutes[i][j]);
            }
        }
        printf("\t\\\\\n");
    }

}// End of PrintAllNurseRoutes function

int CopyIntMatrix(std::vector<std::vector<int> >& source, std::vector<std::vector<int> >& destination, int nRows, int nCols){
    for(int i = 0; i < nRows; ++i)
        for(int j = 0; j < nCols; ++j)
            destination[i][j] = source[i][j];
    return 0;
}// End CopyIntMatrix