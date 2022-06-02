#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>

#define N 8192
#define ITER_MAX 5000


int main(int argc, char** argv){
    int i, j;
    int iter = 0;
    const double pi  = 2.0f * asinf(1.0f);
    double *A = malloc(sizeof(double)*N*N);
    double *Anew = malloc(sizeof(double)*N*N);
    double *y0 = malloc(sizeof(double)*N);
    double *aux;
    memset(A, 0, N * N * sizeof(double));
    for(i = 0; i < N; i++){
        A[i]   = 0.f;
        A[((N-1)*N)+i] = 0.f;
        y0[i] = sinf(pi * i / (N-1));
        A[(i*N)] = y0[i];
        A[(i*N)+N-1] = y0[i]*expf(-pi);
        Anew[i] = 0.f;
        Anew[((N-1)*N)+i] = 0.f;
        Anew[i*N] = y0[i];
        Anew[(i*N)+N-1] = y0[i]*expf(-pi);
    }

        for(iter=0;iter<ITER_MAX;iter++){
            #pragma omp parallel private(i, j)
            {
             #pragma omp for
              for(j = 1; j < N-1; j++){
                for(i = 1; i < N-1; i++ ){
                    Anew[(j*N)+i] = 0.25f * ( A[(j*N)+i+1] + A[(j*N)+i-1]+ A[((j-1)*N)+i] + A[((j+1)*N)+i]);
                }
            }
           #pragma omp master
           {
                aux = A;
                A = Anew;
                Anew = aux;
           }
        }
   // 		printf("get_num_procs_fora_região = %d\n", omp_get_num_procs());
    }
        if(A[100] == 0.45)
                printf("error\n");
//    FILE *out = fopen("out_omp.txt", "w");
//    int size = N*N;
//    fwrite(A, sizeof(double), size, out);
//    fclose(out);
    return 0;
}
