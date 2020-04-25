// Author: Nat Tuck
// CS3650 starter code

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "barrier.h"

barrier*
make_barrier(int nn)
{
    barrier* bb = malloc(sizeof(barrier));
    assert(bb != 0);

    // TODO: More setup?
    int rv = pthread_mutex_init(&(bb->mutex), 0);
    if(rv != 0) {
	    perror("mutex_init(bb->mutex)");
	    abort();
    }
    rv = pthread_cond_init(&(bb->cond), 0);
    if(rv != 0) {
	    perror("cond_init(bb->cond)");
	    abort();
    }
    bb->count = nn;
    bb->seen = 0;
    return bb;
}

void
barrier_wait(barrier* bb)
{
    // TODO: something?
    int rv = pthread_mutex_lock(&(bb->mutex));
    if(rv != 0) {
	    perror("pthread_mutex_lock(bb->mutex)");
	    abort();
    }
    bb->seen += 1;
    int seen = bb->seen;
    

    if(seen >= bb->count) {
	    pthread_cond_broadcast(&(bb->cond)); 
    }
    else {
	    pthread_cond_wait(&(bb->cond),&(bb->mutex));
    }
    pthread_mutex_unlock(&(bb->mutex));
}

void
free_barrier(barrier* bb)
{
    // TODO: More cleanup?
    free(bb);
}

