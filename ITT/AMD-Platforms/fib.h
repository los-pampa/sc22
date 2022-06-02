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


/* define RAPL Environment */

#define AMD_MSR_PWR_UNIT 0xC0010299
#define AMD_MSR_CORE_ENERGY 0xC001029A
#define AMD_MSR_PACKAGE_ENERGY 0xC001029B
#define AMD_TIME_UNIT_MASK 0xF0000
#define AMD_ENERGY_UNIT_MASK 0x1F00
#define AMD_POWER_UNIT_MASK 0xF
#define FIB_STRING_BUFFER 1024
#define FIB_MAX_CPUS                128 //1024
#define FIB_MAX_PACKAGES            4 //16


/* define FIB environment */

#define FIB_MAX_KERNEL          61


/* global variables */
short int fibTotalKernels = 0;
short int fibIdCurrRegion = 0;
unsigned long int fibIdKernels[FIB_MAX_KERNEL];
double fibInitGlobalTime = 0.0;


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
	long long kernelBefore[FIB_MAX_PACKAGES];
	long long kernelAfter[FIB_MAX_PACKAGES];
	double fibThreadsResult[FIB_MAX_CPUS+1];
	short int fibBest, fibSecond;
       	double	fibDiff;

} typeFibFrame;

typeFibFrame fibKernels[FIB_MAX_KERNEL];

