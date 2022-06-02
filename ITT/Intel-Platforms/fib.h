#include "libgomp.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <omp.h>
#include <string.h>


/* define RAPL Environment */

#define CPU_SANDYBRIDGE         42
#define CPU_SANDYBRIDGE_EP      45
#define CPU_IVYBRIDGE           58
#define CPU_IVYBRIDGE_EP        62
#define CPU_HASWELL             60      
#define CPU_HASWELL_EP          63
#define CPU_HASWELL_1           69
#define CPU_BROADWELL           61      
#define CPU_BROADWELL_EP        79
#define CPU_BROADWELL_DE        86
#define CPU_SKYLAKE             78      
#define CPU_SKYLAKE_1           94
#define NUM_RAPL_DOMAINS        4
#define MAX_CPUS                128 
#define MAX_PACKAGES            4 


#define FIB_STRING_BUFFER 1024
#define FIB_MAX_CPUS                128 //1024
#define FIB_MAX_PACKAGES            4 //16


/* define FIB environment */

#define FIB_MAX_KERNEL          61


/* global variables */
static int package_map[FIB_MAX_PACKAGES];
char packname[FIB_MAX_PACKAGES][256];
char tempfile[256];
char valid[FIB_MAX_PACKAGES][NUM_RAPL_DOMAINS];
char rapl_domain_names[NUM_RAPL_DOMAINS][30] = {"energy-cores", "energy-cpu", "energy-pkg", "energy-ram"};
char event_names[FIB_MAX_PACKAGES][NUM_RAPL_DOMAINS][256];
char filenames[FIB_MAX_PACKAGES][NUM_RAPL_DOMAINS][256];
short int fibTotalKernels = 0;
short int fibIdCurrRegion = 0;
unsigned long int fibIdKernels[FIB_MAX_KERNEL];
double fibInitGlobalTime = 0.0;
int fib_total_packages=0;
int fib_total_cores=0;


/* FIB STATES */

#define FIB_STATE_MAX            99
#define FIB_STATE_WARMUP        100
#define FIB_STATE_LOOP          101
#define FIB_STATE_LAST_STEP     102
#define FIB_STATE_END           103

#define FIB_GL_STATE_THREAD 	200
#define FIB_GL_STATE_END	202
#define FIB_GL_OPT_TIME		301
#define FIB_GL_OPT_EDP		303



typedef struct{
        /* Fib variables */
        unsigned short int fibPrev1, fibPrev2;
        unsigned short int fibLower, fibUpper, fibSpaceSize;
        short int fibState, fibIndex[2], fibCurIndex, fibLastIndex, fibPosIndex, fibMinIndex;
        double fibIndexResult[2];
        double fibLastIndexResult;

        /* Global Variables */
        short int fibGlState, fibBestThread, fibOptimize;
        double fibInitResult, fibBestResult, fibMinValue, fibResultMAXthreads;
	long long kernelBefore[FIB_MAX_PACKAGES][NUM_RAPL_DOMAINS];
	long long kernelAfter[FIB_MAX_PACKAGES][NUM_RAPL_DOMAINS];
	double fibThreadsResult[FIB_MAX_CPUS+1];
	short int fibBest, fibSecond;
       	double	fibDiff;

} typeFibFrame;

typeFibFrame fibKernels[FIB_MAX_KERNEL];

