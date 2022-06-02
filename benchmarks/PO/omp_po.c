/*OpenMP Code -- Poisson
To compile: gcc omp_po.c -o omp_po -O3 -lm -fopenmp
To run:
$ export OMP_NUM_THREADS=#threads
$ ./omp_po
To change the input size, modify the line #define N*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <omp.h>

//#include <papi.h>

//#define PAPI_EVENT_SIZE 2

//int PAPI_EVENTS[] = {PAPI_TOT_CYC, PAPI_TOT_INS};

//#define ENERGY 1
#define N 768//280 //default 768
//#define N 384
#define tolerance 0.000001
//#define tolerance 0.001


double u_exact ( double x, double y ){
    double pi = 3.141592653589793;
    double value;

    value = sin ( pi * x * y );

    return value;
}

double uxxyy_exact ( double x, double y ){
    double pi = 3.141592653589793;
    double value;

    value = - pi * pi * ( x * x + y * y ) * sin ( pi * x * y );

    return value;
}

double r8mat_rms ( double *a ){
    int i,j;
    double v;

    v = 0.0;
//    omp_set_num_threads(32);
    #pragma omp parallel for private(i, j) reduction(+:v)
    for ( i = 0; i < N; i++ ){
        for ( j = 0; j < N; j++ ){
             v = v + a[(i*N)+j] * a[(i*N)+j];
        }
    }
    v = sqrt ( v / ( double ) ( N * N )  );
    return v;
}

void rhs ( double *f){
    double fnorm;
    int i,j;
    double x, y;
    #pragma omp parallel for private(i, j, x, y)
    for ( i = 0; i < N; i++ ){
        x = ( double ) ( i ) / ( double ) ( N - 1 );
        for ( j = 0; j < N; j++ ){
	    y = ( double ) ( j ) / ( double ) ( N - 1 );
            if ( i == 0 || i == N - 1 || j == 0 || j == N - 1 ){
                f[(i*N)+j] = u_exact ( x, y );
            }else{
                f[(i*N)+j] = - uxxyy_exact ( x, y );
            }
        }
    }
    fnorm = r8mat_rms ( f );
    return;
}

void sweep ( double dx, double dy, double *f, int itold, int itnew, double *u, double *unew ){
    int i;
    int it;
    int j;
  //  omp_set_num_threads(32);
    #pragma omp parallel private(it, i, j)
    {
        for ( it = itold + 1; it <= itnew; it++ ){
            #pragma omp for
            for ( i= 0; i < N; i++ ){
                for ( j = 0; j < N; j++ ){
                    u[(i*N)+j] = unew[(i*N)+j];
                }
            }
            #pragma omp for
            for ( i = 0; i < N; i++ ){
                for ( j = 0; j < N; j++ ){
                    if ( i == 0 || j == 0 || i == N - 1 || j == N - 1 ){
                        unew[(i*N)+j] = f[(i*N)+j];
                    }else{ 
                        unew[(i*N)+j] = 0.25 * ( u[((i-1)*N)+j] + u[(i*N)+j+1] + u[(i*N)+j-1] + u[((i+1)*N)+j] + f[(i*N)+j] * dx * dy );
                    }
                }
            }
        }
    }
}

void init(double *f, double *unew, double *uexact, double *udiff){
    int i, j;
    double x, y, error;
    rhs(f);
    for(i=0;i<N;i++){
        for (j=0;j<N;j++){
            if(i==0||i==N-1||j==0||j==N-1){
                unew[(i*N)+j] = f[(i*N)+j];
            }else{
                unew[(i*N)+j] = 0.0;
            }
        }
    }
 
    double unew_norm = r8mat_rms(unew);
 
    for(i=0;i<N;i++){
        y = (double)j/(double)(N-1);
        for(j=0;j<N;j++){
            x = (double)i/(double)(N-1);
            uexact[(i*N)+j] = u_exact(x,y);
        }
    }
    double u_norm = r8mat_rms(uexact);

    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
            udiff[(i*N)+j] = unew[(i*N)+j] - uexact[(i*N)+j];
        }
    }
    error = r8mat_rms(udiff);
    printf("Initialization:\n");
    printf("unew_norm = %g\n", unew_norm);
    printf("u_norm = %g\n", u_norm);
    printf("error = %g\n", error);
    printf("-------------------------------\n\n");
}


void poisson(double dx, double dy, double *f, double *u, double *unew, double *uexact, double *udiff){
    int i, j, itnew = 0, itold, converged = 0;
    double u_norm, unew_norm, diff, error;
    for( ; ; ){
        itold = itnew;
        itnew = itold + 500;
    
        sweep (dx, dy, f, itold, itnew, u, unew );
    
        u_norm = unew_norm;
        unew_norm = r8mat_rms(unew);
  //    omp_set_num_threads(32);  
      #pragma omp parallel for private(i, j)
        for(i=0;i<N;i++){
            for(j=0;j<N;j++){
                udiff[(i*N)+j] = unew[(i*N)+j] - u[(i*N)+j];
            }
        }
        diff = r8mat_rms(udiff);
//	omp_set_num_threads(32);
        #pragma omp parallel for private(i, j)
        for(i=0;i<N;i++){
            for(j=0;j<N;j++){
                udiff[(i*N)+j] = unew[(i*N)+j] - uexact[(i*N)+j];
            }
        }
        error = r8mat_rms(udiff);

        if(diff <= tolerance){
            converged = 1;
            printf("Converged after %d iterations\n", itnew);
            printf("unew_norm: %g\n", unew_norm);
            printf("diff: %g\n", diff);
            printf("tolerance: %g\n", tolerance);
            printf("error: %g\n", error);
            break;
        }
    }
}

int main ( int argc, char **argv){

    double dx, dy;
    double *f = malloc(sizeof(double)*N*N);
    double *u = malloc(sizeof(double)*N*N);
    double *udiff = malloc(sizeof(double)*N*N);
    double *uexact = malloc(sizeof(double)*N*N);
    double *unew = malloc(sizeof(double)*N*N);

    //long long papi_values[PAPI_EVENT_SIZE] = {0, 0};
    
    dx = 1.0 / ( double ) ( N - 1 );
    dy = 1.0 / ( double ) ( N - 1 );

    init(f, unew, uexact, udiff);

    //PAPI_start_counters(PAPI_EVENTS, PAPI_EVENT_SIZE);
    poisson(dx, dy, f, u, unew, uexact, udiff);
    //PAPI_read_counters(papi_values, PAPI_EVENT_SIZE);
    //printf("Cycles: %lld\n", papi_values[0]);
    //printf("Instructions: %lld\n", papi_values[1]);

    return 0;
}


