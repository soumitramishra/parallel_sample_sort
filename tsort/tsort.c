#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "float_vec.h"
#include "barrier.h"
#include "utils.h"
#include "float.h"

////pthread_mutex_t mutex;


typedef struct sort_struct {
	long pnum;
	float* data;
       	long size;
        floats* samps; 
        long* sizes;
        barrier* bb;
} sort_struct;
int comparator(const void *a, const void *b) {
	if ((*(float*)a >*(float*)b)) {
		return 1;
	}
	
	else if ((*(float*)a <*(float*)b)) {
		return -1;
	}
	return 0;
}

void
qsort_floats(floats* xs)
{
    // TODO: man 3 qsort ?
    qsort(xs->data, xs->size, sizeof(float), comparator);

}

floats*
sample(float* data, long size, int P)
{
    // TODO: Randomly sample 3*(P-1) items from the input data.
    int no_of_items = 3*(P-1);
    floats *items = make_floats(0);

    for(int i = 0; i< no_of_items; i++) {
	    long random_index = rand()%size;
	    floats_push(items, data[random_index]);
    }
    qsort_floats(items);


    floats *samples = make_floats(0);
    floats_push(samples,0);
    for(int i = 1; i< no_of_items; i+=3) {
	floats_push(samples, items->data[i]);
	  
    }
    floats_push(samples, FLT_MAX);
    free_floats(items);
    return samples;
}

void*
sort_worker(void* arg)
{
    sort_struct* sort_data = (sort_struct*) arg;
    int pnum = sort_data->pnum;
    float* data = sort_data->data;
    long size = sort_data->size;
    floats* samps = sort_data->samps;
    long* sizes = sort_data->sizes;
    barrier* bb = sort_data->bb;
    floats* xs = make_floats(0);
    // TODO: Copy the data for our partition into a locally allocated array.
    for(int i = 0; i<size; i++) {
	    if(data[i] >= samps->data[pnum] && data[i] < samps->data[pnum+1]) {
		    floats_push(xs, data[i]);
	    }
    }

    sizes[pnum] = xs->size;
    printf("%d: start %.04f, count %ld\n", pnum, samps->data[pnum], xs->size);
    // TODO: Sort the local array.
    qsort_floats(xs);
    // TODO: Using the shared sizes array, determine where the local
    // output goes in the global data array.
    
    barrier_wait(bb);
    int start = 0;
    for(int i = 0; i<pnum; i++) {
	    start += sizes[i];
    }
    int end = start + sizes[pnum];

    // TODO: Copy the local array to the appropriate place in the global array.
    int j = 0;
    for(int i = start; i< end; i++) {
	    data[i] = xs->data[j++];
    }
    free_floats(xs);
    free(sort_data);
    // TODO: Make sure this function doesn't have any data races.
    return 0;
}

void
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
    // TODO: Spawn P processes running sort_worker
    pthread_t threads[P];
    for(int p = 0; p<P; p++) {
	    sort_struct *sort_data = malloc(sizeof(sort_struct));
	    sort_data->pnum = p;
	    sort_data->data =  data;
	    sort_data->size = size;
	    sort_data->samps = samps;
	    sort_data-> sizes = sizes; 
	    sort_data->bb = bb;
	    pthread_create(&(threads[p]),0,sort_worker,sort_data);
    }
    for(int i = 0; i<P; i++) {
	    pthread_join(threads[i],0);
    }
    // TODO: Once all P processes have been started, wait for them all to finish.
}

void
sample_sort(float* data, long size, int P, long* sizes, barrier* bb)
{
    // TODO: Sequentially sample the input data.
    floats *samples;
    samples = sample(data, size, P);

    // TODO: Sort the input data using the sampled array to allocate work
    // between parallel processes.
    run_sort_workers(data, size, P, samples, sizes, bb);
    free_floats(samples);
}

int
main(int argc, char* argv[])
{
    if (argc != 3) {
        printf("Usage:\n");
        printf("\t%s P data.dat\n", argv[0]);
        return 1;
    }

    const int P = atoi(argv[1]);
    const char* fname = argv[2];

    seed_rng();

    int fd = open(fname, O_RDWR);
    check_rv(fd);

    //Run filestat
    struct stat input_file_stats;
    int fstat_result = fstat(fd, &input_file_stats);
    check_rv(fstat_result);
    // Get the file size using fstat
    int size = input_file_stats.st_size;
    // Read file using mmap
    void *xs = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    // Read the count - First 8 bytes of xs
    long *count_pointer;
    count_pointer = (long*)xs;
    long count;
    count = count_pointer[0];
    float *data;
    data = (float*)(xs+8);
        
    long sizes_bytes = P * sizeof(long);
    
    long* sizes = malloc(sizes_bytes); // TODO: This should be shared memory.
    
    //long* sizes = mmap(0, sizes_bytes, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    check_rv((long)sizes);
    barrier* bb = make_barrier(P);

    sample_sort(data, count, P, sizes, bb);
    free_barrier(bb);
    // TODO: Clean up resources.
    int rv = munmap(xs, size);
    free(sizes);
    check_rv(rv);
    return 0;
}

