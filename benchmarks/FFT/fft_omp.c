/******************************************************************************/
/* 
  Author:
    Original C version by Wesley Petersen.
    This C version by John Burkardt.
  Reference:
    Wesley Petersen, Peter Arbenz, 
    Introduction to Parallel Computing - A practical guide with examples in C,
    Oxford University Press,
    ISBN: 0-19-851576-6,
    LC: QA76.58.P47.
*/

# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <time.h>
# include <float.h>
# include <omp.h>

#define MAX 27 // standard
//#define MAX 19 // small

void ccopy(int n, double x[], double y[]){
    int i;
    for(i=0; i<n; i++){
        y[i*2+0] = x[i*2+0];
        y[i*2+1] = x[i*2+1];
    }
    return;
}

double ggl(double *seed){
    double d2 = 0.2147483647e10, t, value;
    t = *seed;
    t = fmod ( 16807.0 * t, d2 );
    *seed = t;
    value = ( t - 1.0 ) / ( d2 - 1.0 );
    return value;
}


void cffti(int n, double w[]){
    double arg, aw;
    int i, n2;
    const double pi = 3.141592653589793;
    n2 = n / 2;
    aw = 2.0 * pi / ( ( double ) n );
    #pragma omp parallel shared(aw, n, w) private(arg, i)
    #pragma omp for nowait
    for(i=0; i<n2; i++){
        arg = aw * ( ( double ) i );
        w[i*2+0] = cos ( arg );
        w[i*2+1] = sin ( arg );
    }
    return;
}

void step(int n, int mj, double a[], double b[], double c[], double d[], double w[], double sgn){
    double ambr, ambu, wjw[2];
    int j, ja, jb, jc, jd, jw, k, lj, mj2;

    mj2 = 2 * mj;
    lj  = n / mj2;

    #pragma omp parallel shared(a, b, c, d, lj, mj, mj2, sgn, w) private(ambr, ambu, j, ja, jb, jc, jw, k, wjw, jd)
    #pragma omp for nowait
    for (j=0; j<lj; j++){
        jw = j * mj;
        ja  = jw;
        jb  = ja;
        jc  = j * mj2;
        jd  = jc;
        wjw[0] = w[jw*2+0]; 
        wjw[1] = w[jw*2+1];
        if(sgn<0.0){
            wjw[1] = - wjw[1];
        }
        for(k=0; k<mj; k++){
            c[(jc+k)*2+0] = a[(ja+k)*2+0] + b[(jb+k)*2+0];
            c[(jc+k)*2+1] = a[(ja+k)*2+1] + b[(jb+k)*2+1];

            ambr = a[(ja+k)*2+0] - b[(jb+k)*2+0];
            ambu = a[(ja+k)*2+1] - b[(jb+k)*2+1];

            d[(jd+k)*2+0] = wjw[0] * ambr - wjw[1] * ambu;
            d[(jd+k)*2+1] = wjw[1] * ambr + wjw[0] * ambu;
        }
    }
    return;
}

void cfft2(int n, double x[], double y[], double w[], double sgn){
    int j, m, mj, tgle;
    m = ( int ) ( log ( ( double ) n ) / log ( 1.99 ) );
    mj   = 1;
    tgle = 1;
    step ( n, mj, &x[0*2+0], &x[(n/2)*2+0], &y[0*2+0], &y[mj*2+0], w, sgn );
    if(n == 2){
        return;
    }
    for(j=0; j<m-2; j++){
        mj = mj * 2;
        if(tgle){
            step(n, mj, &y[0*2+0], &y[(n/2)*2+0], &x[0*2+0], &x[mj*2+0], w, sgn);
            tgle = 0;
        }else{
            step(n, mj, &x[0*2+0], &x[(n/2)*2+0], &y[0*2+0], &y[mj*2+0], w, sgn);
            tgle = 1;
        }
    }
    if(tgle){
        ccopy(n, y, x);
    }

    mj = n / 2;
    step(n, mj, &x[0*2+0], &x[(n/2)*2+0], &y[0*2+0], &y[mj*2+0], w, sgn);
}

int main (int argc, char **argv){
    double ctime, ctime1, ctime2, error, flops, fnm1, mflops;
    int first, i, icase, it, ln2, n;
    static double seed;
    double sgn, *w, *x, *y, *z, z0, z1;
    int nits = 10000;


    printf ( "\n" );
    printf ( "FFT_SERIAL\n" );
    printf ( "  C version\n" );
    printf ( "\n" );
    printf ( "  Demonstrate an implementation of the Fast Fourier Transform\n" );
    printf ( "  of a complex data vector.\n" );
    /*
    Prepare for tests.
    */
    printf ( "\n" );
    printf ( "  Accuracy check:\n" );
    printf ( "\n" );
    printf ( "    FFT ( FFT ( X(1:N) ) ) == N * X(1:N)\n" );
    printf ( "\n" );
    printf ( "             N      NITS    Error\n" );
    printf ( "\n" );

    seed  = 331.0;
    n = 1;
    double t1 = omp_get_wtime();
    for ( ln2 = 1; ln2 <= MAX; ln2++ ){
        n = 2 * n;
        w = ( double * ) malloc (     n * sizeof ( double ) );
        x = ( double * ) malloc ( 2 * n * sizeof ( double ) );
        y = ( double * ) malloc ( 2 * n * sizeof ( double ) );
        z = ( double * ) malloc ( 2 * n * sizeof ( double ) );

        first = 1;

        for ( icase = 0; icase < 2; icase++ ){
            if ( first ){
                for ( i = 0; i < 2 * n; i = i + 2 ){
                    z0 = ggl ( &seed );
                    z1 = ggl ( &seed );
                    x[i] = z0;
                    z[i] = z0;
                    x[i+1] = z1;
                    z[i+1] = z1;
                }
            }else{
                #pragma omp parallel shared(n, x, z) private(i, z0, z1)
                #pragma omp for nowait
                for ( i = 0; i < 2 * n; i = i + 2 ){
                    z0 = 0.0;              /* real part of array */
                    z1 = 0.0;              /* imaginary part of array */
                    x[i] = z0;
                    z[i] = z0;           /* copy of initial real data */
                    x[i+1] = z1;
                    z[i+1] = z1;         /* copy of initial imag. data */
                }
            }
            cffti ( n, w );
            if ( first ){
                sgn = + 1.0;
                cfft2 ( n, x, y, w, sgn );
                sgn = - 1.0;
                cfft2 ( n, y, x, w, sgn );
                fnm1 = 1.0 / ( double ) n;
                error = 0.0;
                for ( i = 0; i < 2 * n; i = i + 2 ){
                    error = error + pow ( z[i]   - fnm1 * x[i], 2 ) + pow ( z[i+1] - fnm1 * x[i+1], 2 );
                }
                error = sqrt ( fnm1 * error );
                printf ( "  %12d  %8d  %12e\n", n, nits, error );
                first = 0;
            }else{
                for ( it = 0; it < nits; it++ ){
                    sgn = + 1.0;
                    cfft2 ( n, x, y, w, sgn );
                    sgn = - 1.0;
                    cfft2 ( n, y, x, w, sgn );
                }
            }
        }
        if((ln2%4) == 0){
            nits = nits/10;
        }
        if(nits<1){
            nits=1;
        }
        free ( w );
        free ( x );
        free ( y );
        free ( z );
    }
    double t2 = omp_get_wtime() - t1;
    printf("Execution Time: %.15f\n", t2);
    return 0;
}
