#include "opt-synchprobs.h"
#include <types.h>  /* required by lib.h */
#include <lib.h>    /* for kprintf */
#include <synch.h>  /* for P(), V(), sem_* */
#include <thread.h> /* for thread_fork() */
#include <test.h>

#include "dining_driver.h"

/*
 * Declare any data structures you might need to synchronise 
 * your forks here.
 */

#define LEFT(d) (phil_num+NUM_PHILOSOPHERS-d)%NUM_PHILOSOPHERS
#define RIGHT(d) (phil_num+d)%NUM_PHILOSOPHERS

/* semaphore declaration */
static struct semaphore *mutex;
static struct semaphore **philosopher_semaphores;

/* declare state array (T = Thinking, H = Hungry, E = Eating) */
char * state;

/*
 * Take forks ensures mutually exclusive access to two forks
 * associated with the philosopher.
 * 
 * The left fork number = phil_num
 * The right fork number = (phil_num + 1) % NUM_PHILOSPHERS
 */

void take_forks(unsigned long phil_num)
{
    P(mutex);
    state[phil_num] = 'H';
    /* test whether the current philosopher can enter the eating state */
    if (state[phil_num] == 'H' && state[LEFT(1)] != 'E' && state[RIGHT(1)] != 'E') {
        state[phil_num] = 'E';
        V(philosopher_semaphores[phil_num]);
    }
    V(mutex);
    P(philosopher_semaphores[phil_num]);
}


/*
 * Put forks releases the mutually exclusive access to the
 * philosophers forks.
 */

void put_forks(unsigned long phil_num)
{
    P(mutex);
    state[phil_num] = 'T';

    /* test whether the left philosopher can enter the eating state */
    if (state[LEFT(1)] == 'H' && state[LEFT(2)] != 'E' && state[phil_num] != 'E') {
        state[LEFT(1)] = 'E';
        V(philosopher_semaphores[LEFT(1)]);
    }
    /* test whether the right philosopher can enter the eating state */
    if (state[RIGHT(1)] == 'H' && state[RIGHT(2)] != 'E' && state[phil_num] != 'E') {
        state[RIGHT(1)] = 'E';
        V(philosopher_semaphores[RIGHT(1)]);
    }
    V(mutex);
}


/* 
 * Create gets called before the philosopher threads get started.
 * Insert any initialisation code you require here.
 */

void create_forks()
{
    /* allocate memory for arrays */
    philosopher_semaphores = kmalloc(sizeof(struct semaphore *) * NUM_PHILOSOPHERS);
    if (philosopher_semaphores == NULL)
        panic("Cannot kmalloc philosopher_semaphores array.");
    state = kmalloc(sizeof(char) * NUM_PHILOSOPHERS);
    if (state == NULL)
        panic("Cannot kmalloc state array.");

    /* create semaphores */
    mutex = sem_create("mutex", 1);
    int phil_num;
    for (phil_num = 0; phil_num < NUM_PHILOSOPHERS; phil_num++)
        philosopher_semaphores[phil_num] = sem_create("philosopher", 0);
        /* initialise each philosopher to state T (thinking) */
        state[phil_num] = 'T';
}


/*
 * Destroy gets called when the system is shutting down.
 * You should clean up whatever you allocated in create_forks()
 */

void destroy_forks()
{
    /* destory all semaphores */
    sem_destroy(mutex);
    int i;
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
        sem_destroy(philosopher_semaphores[i]);

    /* free allocated memory for variables */
    kfree(philosopher_semaphores);
    kfree(state);
}
