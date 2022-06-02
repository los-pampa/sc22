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
        fibNumThreads = fibNumThreads >> 1;
        fibSizeThreadSpace = fibNumThreads;
        fibThreadSpace = (short int*) malloc(fibNumThreads*sizeof(short int));
	j = 2;
	for(i = 0; i< fibSizeThreadSpace; i++){
		fibThreadSpace[i] = j;
		j = j+2;
	}


        /*for(i = 1; i <= fibNumThreads >> 1; i++){
                fibThreadSpace[i-1] = i << 1;
        }

        j = fibNumThreads;
        for(i = fibNumThreads >> 1; i < fibNumThreads; i++){
                fibThreadSpace[i] = j << 1;
                j--;
        } 
        fibSizeThreadSpace -= (fibSizeThreadSpace >> 1) -1; */
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

	fib_detect_packages();
	fib_detect_cpu();
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
                //fib_start_rapl_sysfs();
                return fibThreadSpace[fibKernels[fibIdCurrRegion].fibCurIndex];
        } else {
                fibKernels[fibIdCurrRegion].fibBestResult = fibKernels[fibIdCurrRegion].fibMinValue;
                fibKernels[fibIdCurrRegion].fibBestThread = fibThreadSpace[fibKernels[fibIdCurrRegion].fibMinIndex]; /* Get the best thread */
                fibKernels[fibIdCurrRegion].fibGlState = FIB_GL_STATE_END;
        }
        /* jana */
        if (fibKernels[fibIdCurrRegion].fibResultMAXthreads < fibKernels[fibIdCurrRegion].fibBestResult){
                fibKernels[fibIdCurrRegion].fibBestResult = fibKernels[fibIdCurrRegion].fibResultMAXthreads;
                fibKernels[fibIdCurrRegion].fibBestThread = fibThreadSpace[fibSizeThreadSpace-1]; // depois preciso mudar isso...
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
		
	}/*else{
		//find best
		fibKernels[fibIdCurrRegion].fibBest = -1;
		fibKernels[fibIdCurrRegion].fibSecond = -1;
		double min = 9999999.000;
		double secmin = 9999999.000;
		for(int i = 0; i < FIB_MAX_CPUS; i++){
			if(fibKernels[fibIdCurrRegion].fibThreadsResult[i] < min){
				min = fibKernels[fibIdCurrRegion].fibThreadsResult[i];
				fibKernels[fibIdCurrRegion].fibBest = i;
			}
		}
		for(int i = 0; i < FIB_MAX_CPUS; i++){
			if(fibKernels[fibIdCurrRegion].fibBest != i){
				if(fibKernels[fibIdCurrRegion].fibThreadsResult[i] < secmin){
					secmin = fibKernels[fibIdCurrRegion].fibThreadsResult[i];
					fibKernels[fibIdCurrRegion].fibSecond = i;
				}
			}
		}
		fibKernels[fibIdCurrRegion].fibDiff = secmin/min;
	}*/
	int newNumberThreads = fib_get_num_cpus();
//	printf("%d != %d\n", globalNumThreads, newNumberThreads);
	if(globalNumThreads != newNumberThreads){
		fib_prepare_space();
                fib_print_space();
                fib_init_parameter(fibSizeThreadSpace, fibKernels[fibIdCurrRegion].fibOptimize);
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
        
        for (i = 0; i < fibTotalKernels; i++) {
		printf("Region %d: ", i);
		printf("1st(%d) ", fibKernels[i].fibBest);
		printf("2nd(%d) ", fibKernels[i].fibSecond);
		printf("Diff(%.5f)\n", fibKernels[i].fibDiff);
        }

        fib_free_space();
}

/* Function used by the Intel RAPL to detect the CPU Architecture*/
void fib_detect_cpu()
{
        FILE *fff;
        int family, model = -1;
        char buffer[BUFSIZ], *result;
        char vendor[BUFSIZ];
        fff = fopen("/proc/cpuinfo", "r");
        while (1)
        {
                result = fgets(buffer, BUFSIZ, fff);
                if (result == NULL)
                        break;
                if (!strncmp(result, "vendor_id", 8))
                {
                        sscanf(result, "%*s%*s%s", vendor);
                        if (strncmp(vendor, "GenuineIntel", 12))
                        {
                                printf("%s not an Intel chip\n", vendor);
                        }
                }
                if (!strncmp(result, "cpu family", 10))
                {
                        sscanf(result, "%*s%*s%*s%d", &family);
                        if (family != 6)
                        {
                                printf("Wrong CPU family %d\n", family);
                        }
                }
                if (!strncmp(result, "model", 5))
                {
                        sscanf(result, "%*s%*s%d", &model);
                }
        }
        fclose(fff);
}

/* Function used by the Intel RAPL to detect the number of cores and CPU sockets*/
void fib_detect_packages()
{
        char filename[FIB_STRING_BUFFER];
        FILE *fff;
        int package;
        int i;
        for (i = 0; i < MAX_PACKAGES; i++)
                package_map[i] = -1;
        for (i = 0; i < MAX_CPUS; i++)
        {
                sprintf(filename, "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", i);
                fff = fopen(filename, "r");
                if (fff == NULL)
                        break;
                fscanf(fff, "%d", &package);
                fclose(fff);
                if (package_map[package] == -1)
                {
                        fib_total_packages++;
                        package_map[package] = i;
                }
        }
        fib_total_cores = i;
}

void fib_start_energy(){
	 int i, j;
        FILE *fff;
        for (j = 0; j < fib_total_packages; j++)
        {
                i = 0;
                sprintf(packname[j], "/sys/class/powercap/intel-rapl/intel-rapl:%d", j);
                sprintf(tempfile, "%s/name", packname[j]);
                fff = fopen(tempfile, "r");
                if (fff == NULL)
                {
                        fprintf(stderr, "\tCould not open %s\n", tempfile);
                        exit(0);
                }
                fscanf(fff, "%s", event_names[j][i]);
                valid[j][i] = 1;
                fclose(fff);
                sprintf(filenames[j][i], "%s/energy_uj", packname[j]);

                /* Handle subdomains */
                for (i = 1; i < NUM_RAPL_DOMAINS; i++)
                {
                        sprintf(tempfile, "%s/intel-rapl:%d:%d/name", packname[j], j, i - 1);
                        fff = fopen(tempfile, "r");
                        if (fff == NULL)
                        {
                                //fprintf(stderr,"\tCould not open %s\n",tempfile);
                                valid[j][i] = 0;
                                continue;
                        }
                        valid[j][i] = 1;
                        fscanf(fff, "%s", event_names[j][i]);
                        fclose(fff);
                        sprintf(filenames[j][i], "%s/intel-rapl:%d:%d/energy_uj", packname[j], j, i - 1);
                }
        }
        /* Gather before values */
        for (j = 0; j < fib_total_packages; j++)
        {
                for (i = 0; i < NUM_RAPL_DOMAINS; i++)
                {
                        if (valid[j][i])
                        {
                                fff = fopen(filenames[j][i], "r");
                                if (fff == NULL)
                                {
                                        fprintf(stderr, "\tError opening %s!\n", filenames[j][i]);
                                }
                                else
                                {
                                        fscanf(fff, "%lld", &fibKernels[fibIdCurrRegion].kernelBefore[j][i]);
                                        fclose(fff);
                                }
                        }
                }
        }
}



double fib_end_energy(){
        int i, j;
        FILE *fff;
        double total = 0;
        for (j = 0; j < fib_total_packages; j++)
        {
                for (i = 0; i < NUM_RAPL_DOMAINS; i++)
                {
                        if (valid[j][i])
                        {
                                fff = fopen(filenames[j][i], "r");
                                if (fff == NULL)
                                {
                                        fprintf(stderr, "\tError opening %s!\n", filenames[j][i]);
                                }
                                else
                                {
                                        fscanf(fff, "%lld", &fibKernels[fibIdCurrRegion].kernelAfter[j][i]);
                                        fclose(fff);
                                }
                        }
                }
        }
        for (j = 0; j < fib_total_packages; j++)
        {
                for (i = 0; i < NUM_RAPL_DOMAINS; i++)
                {
                        if (valid[j][i])
                        {
                                if (strcmp(event_names[j][i], "core") != 0 && strcmp(event_names[j][i], "uncore") != 0)
                                {
                                        total += (((double)fibKernels[fibIdCurrRegion].kernelAfter[j][i] - (double)fibKernels[fibIdCurrRegion].kernelBefore[j][i]) / 1000000.0);
                                }
                        }
                }
        }
        return total;
}





