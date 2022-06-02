#include "fib.h"


#define FIB_INITIAL_VALUES(region, prev1, prev2, low, up, size, state, index0, index1, last_ind)({ \
        fibKernels[region].fibPrev1 = prev1; \
        fibKernels[region].fibPrev2 = prev2; \
        fibKernels[region].fibLower = low; \
        fibKernels[region].fibUpper = up; \
        fibKernels[region].fibSpaceSize = size; \
        fibKernels[region].fibState = state; \
        fibKernels[region].fibIndex[0] = index0; \
        fibKernels[region].fibIndex[1] = index1; \
        fibKernels[region].fibLastIndex = last_ind; \
        fibKernels[region].fibCurIndex = (size - 1); \
})



short int 		*fibThreadSpace;
unsigned short int 	fibSizeThreadSpace;
int 			fibNumThreads;
int 			globalNumThreads;
int 			fibHack = 0;

/* Prepare space of numthreads */

int fib_get_num_cpus(){
        FILE *cpus = fopen("/sys/fs/cgroup/cpuset/cpuset.cpus", "rt");
        char info[50], *token;
	int cont = 0, i=0, ini=0, end=0, total=0;
        fscanf(cpus, "%s", info);
	fclose(cpus);
        for(int i =0; i < strlen(info); i++){
//                printf("%c ", info[i]);
                if(info[i] == '-')
                        cont++;
        }
        do{
                if(i == 0)
                        token = strtok(info, "-");
                else
                        token = strtok(NULL, "-");
                ini = atoi(token);
                token = strtok(NULL, ",");
                end = atoi(token);
                total += (end-ini)+1;
                i++;
        }while(i < cont);
	return total;
}


void fib_prepare_space(){
        int i, j;
        fibNumThreads = fib_get_num_cpus();
	globalNumThreads = fibNumThreads;
        fibSizeThreadSpace = fibNumThreads/2;
        fibThreadSpace = (short int*) malloc(fibNumThreads*sizeof(short int));
	j = 2;
	for(i = 0; i< fibSizeThreadSpace; i++){
		fibThreadSpace[i] = j;
		j = j+2;
	}
}


void fib_free_space(){
        free(fibThreadSpace);
}


void fib_print_space(){
        int i;
        printf("Fib Thread Space: ");
        for(i = 0; i < fibSizeThreadSpace; i++){
                printf("%d ", fibThreadSpace[i]);
        }
        printf("\n");
}

/* Initialize the fibonacci parameters accordingly with the space size */
inline void fib_init(short int region, unsigned short int space_size){
        while (fibKernels[region].fibUpper < space_size){
                fibKernels[region].fibPrev2 = fibKernels[region].fibPrev1;
                fibKernels[region].fibPrev1 = fibKernels[region].fibUpper;
                fibKernels[region].fibUpper = fibKernels[region].fibPrev2 + fibKernels[region].fibPrev1;
        }
        fibKernels[region].fibUpper -= 1;
}

void fib_init_parameter(unsigned short int space_size, int optimization){
        for(int i = 0; i < FIB_MAX_KERNEL; i++){
                FIB_INITIAL_VALUES(i, 1, 0, 0, 1, space_size, FIB_STATE_WARMUP, -1, -1, -1);
                fibKernels[i].fibGlState = FIB_GL_STATE_THREAD;
		fibKernels[i].fibOptimize = optimization;
		for(int j=0; j<FIB_MAX_CPUS+1; j++){
			fibKernels[i].fibThreadsResult[j] = 9999999999.0;
		}
                fib_init(i, space_size);
        }
}


void fib_init_environment(int optimization){

	if(optimization != -1){
	        fib_prepare_space();
        	fib_print_space();
	        fib_init_parameter(fibSizeThreadSpace, optimization);
	}
	
	fibIdCurrRegion = FIB_MAX_KERNEL - 1;
        fib_start_energy();
        fibInitGlobalTime = omp_get_wtime();

}


/**************************FIB***************************/

/* Reduce a level of the fibonacci numbers (ex. prev1 = 8, prev2 = 5 => prev1 = 5, prev2 = 3) */
#define FIB_REDUCE(region)({\
      unsigned short int temp = fibKernels[region].fibPrev1;\
      fibKernels[region].fibPrev1 = fibKernels[region].fibPrev2;\
      fibKernels[region].fibPrev2 = temp - fibKernels[region].fibPrev2;\
    })

/* Util macro to get the minimum value */
#define FIB_MIN(X, Y) (((X) <= (Y)) ? (X) : (Y))

/* Update values after the region execution */
#define FIB_UPDATE_VALUES(region, result)({\
      unsigned short int pos = fibKernels[region].fibPosIndex;\
      unsigned short int index = fibKernels[region].fibCurIndex;\
      fibKernels[region].fibIndexResult[pos] = result;\
      fibKernels[region].fibIndex[pos] = index;\
    })

/* Verifies if the new position needs to be executed or can be reused */
inline int FIB_NEED_EVALUATION(short int region, unsigned short int index, unsigned short int index_pos) {
  if (fibKernels[region].fibIndex[index_pos] == index) {
    return 0;
  } else if (fibKernels[region].fibLastIndex == index) {
    fibKernels[region].fibIndexResult[index_pos] = fibKernels[region].fibLastIndexResult;
    return 0;
  } else {
    return 1;
  }
}

/* Update index to be executed */
#define FIB_UPDATE_INDEX(region, index, index_pos)({\
      fibKernels[region].fibCurIndex = index;\
      fibKernels[region].fibPosIndex = index_pos;\
    })

/* Update indexes that has already executed */
#define FIB_UPDATE_LAST_INDEX(region, index, index_result)({\
      fibKernels[region].fibMinValue = index_result;\
      fibKernels[region].fibLastIndex = index;\
      fibKernels[region].fibLastIndexResult = index_result;\
      fibKernels[region].fibMinIndex = index;\
    })


void fib_search(short int region, double result)
{
        switch(fibKernels[region].fibState) {
                case FIB_STATE_WARMUP:
                        fibKernels[region].fibResultMAXthreads = 0;
                        fibKernels[region].fibState = FIB_STATE_MAX;
                        break;
                case FIB_STATE_MAX:
                        fibKernels[region].fibResultMAXthreads = result;
                        fibKernels[region].fibCurIndex = fibKernels[region].fibPrev2 + fibKernels[region].fibLower - 1;
                        fibKernels[region].fibPosIndex = 0;
                        fibKernels[region].fibState = FIB_STATE_LOOP;
                        break;
                case FIB_STATE_LOOP:
                        FIB_UPDATE_VALUES(region, result);
                        while (1) {
                                unsigned short int index0 = fibKernels[region].fibPrev2 + fibKernels[region].fibLower - 1;
                                unsigned short int index1 = FIB_MIN(fibKernels[region].fibPrev1 + fibKernels[region].fibLower - 1, fibKernels[region].fibSpaceSize - 1);
                                if (index0 < index1) {
                                        int need_eval0, need_eval1;
                                        need_eval0 = FIB_NEED_EVALUATION(region, index0, 0);
                                        if (need_eval0) {
                                                FIB_UPDATE_INDEX(region, index0, 0);
                                                break;
                                        }
                                        need_eval1 = FIB_NEED_EVALUATION(region, index1, 1);
                                        if (need_eval1) {
                                                FIB_UPDATE_INDEX(region, index1, 1);
                                                break;
                                        }
                                        if (fibKernels[region].fibIndexResult[0] < fibKernels[region].fibIndexResult[1]) {
                                                fibKernels[region].fibUpper = index1 - 1;
                                                FIB_UPDATE_LAST_INDEX(region, index0, fibKernels[region].fibIndexResult[0]);
                                        } else {
                                                fibKernels[region].fibLower = index0 + 1;
                                                FIB_UPDATE_LAST_INDEX(region, index1, fibKernels[region].fibIndexResult[1]);
                                        }
                                }
                                FIB_REDUCE(region);
                                if (fibKernels[region].fibLower + 1 >= FIB_MIN(fibKernels[region].fibUpper, fibKernels[region].fibSpaceSize - 1)) {
                                        unsigned short int index0 = fibKernels[region].fibLower;
                                        unsigned short int index1 = FIB_MIN(fibKernels[region].fibUpper, fibKernels[region].fibSpaceSize - 1);
                                        unsigned short int need_eval0 = FIB_NEED_EVALUATION(region, index0, 0);
                                        unsigned short int need_eval1 = FIB_NEED_EVALUATION(region, index1, 1);
                                        fibKernels[region].fibState = FIB_STATE_LAST_STEP;
                                        if (need_eval0) {
                                                FIB_UPDATE_INDEX(region, index0, 0);
                                        } else if (need_eval1) {
                                                FIB_UPDATE_INDEX(region, index1, 1);
                                        } else {
                                                fibKernels[region].fibState = FIB_STATE_END;
                                        }
                                        break;
                                }
                        }
                        break;
                case FIB_STATE_LAST_STEP:
                        FIB_UPDATE_VALUES(region, result);
                        if (fibKernels[region].fibIndexResult[0] < fibKernels[region].fibIndexResult[1]) {
                                FIB_UPDATE_LAST_INDEX(region, fibKernels[region].fibLower, fibKernels[region].fibIndexResult[0]);
                        } else {
                                FIB_UPDATE_LAST_INDEX(region, FIB_MIN(fibKernels[region].fibUpper, fibKernels[region].fibSpaceSize - 1),
                                        fibKernels[region].fibIndexResult[1]);
                        }
                        fibKernels[region].fibState = FIB_STATE_END;
                        break;
        }
}

/********************************************************/

inline int fib_find_region(uintptr_t ptr_region)
{
        int i, id_current_region = -1;

        for (i = 0; i < fibTotalKernels; i++) {
                if (fibIdKernels[i] == ptr_region) {
                        id_current_region = i;
                        break;
                }
        }
        return id_current_region;
}

inline int fib_add_region(uintptr_t ptr_region)
{
        int new_region;

        fibIdKernels[fibTotalKernels] = ptr_region;
        new_region = fibTotalKernels;
        fibTotalKernels++;

        return new_region;
}

int fib_resolve_num_threads(uintptr_t ptr_region)
{
        fibIdCurrRegion = fib_find_region(ptr_region);

        /* If a new parallel region is discovered */
        if (fibIdCurrRegion == -1) {
                fibIdCurrRegion = fib_add_region(ptr_region);
        }

        if (fibKernels[fibIdCurrRegion].fibState != FIB_STATE_END) {
                fibKernels[fibIdCurrRegion].fibInitResult = omp_get_wtime();
                return fibThreadSpace[fibKernels[fibIdCurrRegion].fibCurIndex];
        } else {
                fibKernels[fibIdCurrRegion].fibBestResult = fibKernels[fibIdCurrRegion].fibMinValue;
                fibKernels[fibIdCurrRegion].fibBestThread = fibThreadSpace[fibKernels[fibIdCurrRegion].fibMinIndex]; /* Get the best thread */
                fibKernels[fibIdCurrRegion].fibGlState = FIB_GL_STATE_END;
        }
        if (fibKernels[fibIdCurrRegion].fibResultMAXthreads < fibKernels[fibIdCurrRegion].fibBestResult){
                fibKernels[fibIdCurrRegion].fibBestResult = fibKernels[fibIdCurrRegion].fibResultMAXthreads;
                fibKernels[fibIdCurrRegion].fibBestThread = fibThreadSpace[fibSizeThreadSpace-1];
        }
        return fibKernels[fibIdCurrRegion].fibBestThread;
}

void fib_end_parallel_region()
{
        double result = -1.0, time, energy;
	int idThread;
        
        if (fibKernels[fibIdCurrRegion].fibGlState != FIB_GL_STATE_END) {
                time = omp_get_wtime() - fibKernels[fibIdCurrRegion].fibInitResult;
        	if(fibKernels[fibIdCurrRegion].fibOptimize == FIB_GL_OPT_TIME){
			result = time;
		}else{
			if(time < 0.001){
				fibKernels[fibIdCurrRegion].fibOptimize = FIB_GL_OPT_TIME;
				fibKernels[fibIdCurrRegion].fibGlState = FIB_GL_STATE_THREAD;
				FIB_INITIAL_VALUES(fibIdCurrRegion, 1, 0, 0, 1, fibSizeThreadSpace, FIB_STATE_MAX, -1, -1, -1);
				fib_init(fibIdCurrRegion, fibSizeThreadSpace);
				fibKernels[fibIdCurrRegion].fibPosIndex = 0;
				return;
			} else{
				energy = fib_end_energy();
				result = energy * time;
			}
		}
		idThread = fibThreadSpace[fibKernels[fibIdCurrRegion].fibCurIndex];
		fibKernels[fibIdCurrRegion].fibThreadsResult[idThread] = result;
	}

        fib_search(fibIdCurrRegion, result);
}

void fib_destructor()
{
        int i;

        float time = omp_get_wtime() - fibInitGlobalTime;
        fibIdCurrRegion = FIB_MAX_KERNEL - 1;
	double energy = fib_end_energy();
        printf("=== FIB ===\n");
        printf("FIB - Execution Time: %.5f seconds\n", time);
        printf("FIB - Energy: %.8f Joules\n", energy);
        printf("FIB - EDP: %.5f\n", time*energy);
        fib_free_space();
}


void fib_start_energy(){
	char msr_filename[FIB_STRING_BUFFER];
	int fd;
        sprintf(msr_filename, "/dev/cpu/0/msr");
        fd = open(msr_filename, O_RDONLY);
        if ( fd < 0 ) {
                if ( errno == ENXIO ) {
                        fprintf(stderr, "rdmsr: No CPU 0\n");
                        exit(2);
                } else if ( errno == EIO ) {
                        fprintf(stderr, "rdmsr: CPU 0 doesn't support MSRs\n");
                        exit(3);
                } else {
                        perror("rdmsr:open");
                        fprintf(stderr,"Trying to open %s\n",msr_filename);
                        exit(127);
                }
        }
        uint64_t data;
        pread(fd, &data, sizeof data, AMD_MSR_PACKAGE_ENERGY);
        fibKernels[fibIdCurrRegion].kernelBefore[0] = (long long) data;
}


double fib_end_energy(){
        char msr_filename[FIB_STRING_BUFFER];
        int fd;
        sprintf(msr_filename, "/dev/cpu/0/msr");
        fd = open(msr_filename, O_RDONLY);
        uint64_t data;
        pread(fd, &data, sizeof data, AMD_MSR_PWR_UNIT);
        int core_energy_units = (long long) data;
        unsigned int energy_unit = (core_energy_units & AMD_ENERGY_UNIT_MASK) >> 8;
        pread(fd, &data, sizeof data, AMD_MSR_PACKAGE_ENERGY);
        fibKernels[fibIdCurrRegion].kernelAfter[0] = (long long) data;
        double result = (fibKernels[fibIdCurrRegion].kernelAfter[0] - fibKernels[fibIdCurrRegion].kernelBefore[0])*pow(0.5,(float)(energy_unit));
        return result;
}





