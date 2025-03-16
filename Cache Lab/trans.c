/* 20220302 jihyunk */

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_32_32(int M, int N, int A[N][M], int B[M][N]); 
void transpose_64_64(int M, int N, int A[N][M], int B[M][N]); 
void transpose_61_67(int M, int N, int A[N][M], int B[M][N]); 


/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{

    if (N == 32){
        transpose_32_32(M, N, A, B); 
    }
    else if (N == 64){
        transpose_64_64(M, N, A, B); 
    }else{
        transpose_61_67(M, N, A, B); 
    }

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 32 × 32 */
char trans_3232_desc[] = "A 32 X 32 transpose";
void transpose_32_32(int M, int N, int A[N][M], int B[M][N])
{
    int col, row, i;
    int t1, t2, t3, t4, t5, t6, t7, t8;
    for (col = 0; col < N; col += 8) {
        for (row = 0; row < M; row += 8) {
            for (i = col; i < col + 8; i++) 
            {
                t1 = A[i][row];
                t2 = A[i][row + 1];
                t3 = A[i][row + 2];
                t4 = A[i][row + 3];
                t5 = A[i][row + 4];
                t6 = A[i][row + 5];
                t7 = A[i][row + 6];
                t8 = A[i][row + 7];

                B[row][i] = t1;
                B[row + 1][i] = t2;
                B[row + 2][i] = t3;
                B[row + 3][i] = t4;
                B[row + 4][i] = t5;
                B[row + 5][i] = t6;
                B[row + 6][i] = t7;
                B[row + 7][i] = t8;
            }
        }
    }
}

/* 64 × 64 */
char trans_6464_desc[] = "A 64 X 64 transpose";
void transpose_64_64(int M, int N, int A[N][M], int B[M][N])
{
    int col, row, i, j;
    int t1, t2, t3, t4, t5, t6, t7, t8;
    for (col = 0; col < N; col += 8) {
        for (row = 0; row < M; row += 8) {
            for (i = col; i < col + 4; i++) {
                
                t1 = A[i][row];
                t2 = A[i][row + 1];
                t3 = A[i][row + 2];
                t4 = A[i][row + 3];
                B[row][i] = t1;
                B[row + 1][i] = t2;
                B[row + 2][i] = t3;
                B[row + 3][i] = t4;
                
                t5 = A[i][row + 4];
                t6 = A[i][row + 5];
                t7 = A[i][row + 6];
                t8 = A[i][row + 7];
                B[row][i + 4] = t5;
                B[row + 1][i + 4] = t6;
                B[row + 2][i + 4] = t7;
                B[row + 3][i + 4] = t8;
            }
            for (j = row; j < row + 4; j++) {
                t1 = A[col + 4][j];
                t2 = A[col + 5][j];
                t3 = A[col + 6][j];
                t4 = A[col + 7][j];
                
                t5 = B[j][col + 4];
                t6 = B[j][col + 5];
                t7 = B[j][col + 6];
                t8 = B[j][col + 7];
                
                B[j][col + 4] = t1;
                B[j][col + 5] = t2;
                B[j][col + 6] = t3;
                B[j][col + 7] = t4;
               
                B[j + 4][col] = t5;
                B[j + 4][col + 1] = t6;
                B[j + 4][col + 2] = t7;
                B[j + 4][col + 3] = t8;

                t1 = A[col + 4][j + 4];
                t2 = A[col + 5][j + 4];
                t3 = A[col + 6][j + 4];
                t4 = A[col + 7][j + 4];
                B[j + 4][col + 4] = t1;
                B[j + 4][col + 5] = t2;
                B[j + 4][col + 6] = t3;
                B[j + 4][col + 7] = t4;
            }
        }
    } 
}


/* 61 × 67 */
char trans_6167_desc[] = "A 61 X 67 transpose";
void transpose_61_67(int M, int N, int A[N][M], int B[M][N])
{
    int col, row, i, j;
    for (col = 0; col < N; col += 8) {
        for (row = 0; row < M; row += 8) {
            for (j = row; j < row + 8 && j < M; j++) {
                for (i = col; i < col + 8 && i < N; i++) 
                    B[j][i] = A[i][j];
            }
        }
    }   
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(transpose_32_32, trans_3232_desc);
    registerTransFunction(transpose_64_64, trans_6464_desc);
    registerTransFunction(transpose_61_67, trans_6167_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

