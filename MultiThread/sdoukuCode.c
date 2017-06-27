#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>

typedef struct
{
    int **arr;
} number_arr;

typedef enum {
    false, true
} bool;

typedef struct sdokuGrid {
    int row;
    int col;
} sdokuGrid;

void* runner_row(void *data);
void* runner_col(void *data);
void* runner_grid(void *data);

int number[9][9];
int rowindex, rowNumber, colNumber;

int main() {
    int res = 0;
    int result = 0;
    
    pthread_t threadd[11];
    
    freopen("/input.txt", "r", stdin);
    
    int ii, jj, i;
    
    for (ii = 0; ii < 9; ii++) {
        for (jj = 0; jj < 9; jj++) {
            scanf("%d", &number[ii][jj]);
        }
    }
    
    int col_thread;
    int row_thread;
    int nine_thread;
    
    col_thread = pthread_create(&threadd[0], NULL, runner_col, (void*)number);
    row_thread = pthread_create(&threadd[1], NULL, runner_row, (void*)number);
    
    int startRow = 0;
    int endRow = 3;
    int startCol = 0;
    int endCol = 3;
    number_arr *gridNum;
    
    sdokuGrid *grid;
    for(i = 3; i < 12; i++){
        grid = (sdokuGrid *)malloc(sizeof(sdokuGrid));
        
        if (i == 2) {
            grid->row = 0;
            grid->col = 0;
            rowNumber = grid->row;
            colNumber = grid->col;
        }
        else {
            colNumber += 3;
            if (colNumber == 9) {
                colNumber = 0;
                rowNumber += 3;
            }
        }
        grid->row = rowNumber;
        grid->col = colNumber;
        
        pthread_create(&threadd[i], NULL, runner_grid, grid);
    }
    
    for(i = 0; i < 11; i++) {
        pthread_join(threadd[i], (void **)&res);
        printf("%d \n",res);
        result += res;
    }
    
    printf("%d", result);
    if(result == 11){
        printf("Valid result !\n");
    }
    else {
        printf("Invalid result !\n");
    }
    
    return 0;
}

void* runner_col(void *data){
    printf("coltrehad created \n");
    
    int correctCol = 0;
    int i;
    for(i=0; i<9; i++){
        int j = 0;
        int correctNum = 1;
        while(correctNum != 9){
            if(correctNum == number[i][j]){
                correctNum++;
                j = 0;
            }
            else {
                j++;
                if(j > 9){
                    break;
                }
            }
        }
        
        if(correctNum == 9){
            correctCol++;
        }
    }
    
    printf("coltrhead finished \n");
    if(correctCol == 9){
        return (void *)1;
    }
    else {
        return (void *)0;
    }
}

void* runner_row(void *data){
    printf("rowthread created \n");
    
    int j = 0;
    int i;
    int correctRow = 0;
    for(i=0; i<9; i++){
        int correctNum = 1;
        while(correctNum != 9){
            if(correctNum == number[j][i]){
                correctNum++;
                j = 0;
            }
            else {
                j++;
                if(j > 9){
                    break;
                }
            }
        }
        
        if(correctNum == 9){
            correctRow++;
        }
        else {
            
        }
    }
    
    printf("rowthread finished \n");
    if(correctRow == 9){
        return (void *)1;
    }
    else {
        return (void *)0;
    }
}

void* runner_grid(void *data){
    printf("gridthread created \n");
    bool check[9]={0, };
    int count = 0;
    int result = 0;
    int i, j;
    
    sdokuGrid* rr = (sdokuGrid*)data;
    int rowNumber = rr->row;
    int colNumber = rr->col;
    
    for (i = rowNumber; i < (rowNumber + 3); i++) {
        for (j = colNumber; j < (colNumber + 3); j++) {
            if( check[number[i][j] - 1] == false) {
                count++;
                check[number[i][j] - 1] = true;
            }
        }
    }
    
    printf("gridthread finished \n");
    
    if(count == 9){
        return (void *)1;
    }
    else {
        return (void *)0;
    }
}
