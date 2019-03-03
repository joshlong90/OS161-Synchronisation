/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include <lib.h>
#include <synch.h>
#include "producerconsumer_driver.h"

/* Declare any variables you need here to keep track of and
   synchronise your bounded. A sample declaration of a buffer is shown
   below. It is an array of pointers to items.
   
   You can change this if you choose another implementation. 
   However, you should not have a buffer bigger than BUFFER_SIZE 
*/

data_item_t * item_buffer;
int buffer_pos;

/* semaphore declaration */
static struct semaphore *mutex;
static struct semaphore *empty;
static struct semaphore *full;

/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. */

data_item_t * consumer_receive(void)
{
        data_item_t * item;
        item = (data_item_t *) kmalloc(sizeof(data_item_t));
        if (item == NULL) {
                panic("Cannot kmalloc single item.");
        }
        P(full); // go to sleep if buffer is empty
        P(mutex);
        /* take an item from the buffer */
        KASSERT(buffer_pos > 0);
        *item = item_buffer[--buffer_pos];
        V(mutex);
        V(empty);
        return item;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer. */

void producer_send(data_item_t *item)
{
        P(empty); // go to sleep if buffer is full
        P(mutex);
        /* add an item onto the buffer */
        KASSERT(buffer_pos < BUFFER_SIZE);
        item_buffer[buffer_pos++] = *item;
        V(mutex);
        V(full);
}




/* Perform any initialisation (e.g. of global data) you need
   here. Note: You can panic if any allocation fails during setup */

void producerconsumer_startup(void)
{
        /* initialise mutex with count 1 (mutual exclusion allows for only 1 thread at a time) */
        mutex = sem_create("mutex", 1);
        /* intitialise empty with buffer count BUFFER_SIZE (all buffer slots are empty) */
        empty = sem_create("empty", BUFFER_SIZE);
        /* initialise full with count 0 (no buffer slots are full) */
        full = sem_create("full", 0);
        /* allocate memory for item_buffer */
        item_buffer = kmalloc(sizeof(data_item_t) * BUFFER_SIZE);
        if (item_buffer == NULL) {
                panic("Cannot kmalloc item buffer.");
        }
        /* initialise the position in the buffer to 0 */
        buffer_pos = 0;
}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
        /* destory all semaphores */
        sem_destroy(mutex);
        sem_destroy(empty);
        sem_destroy(full);
        /* free the buffer variable */
        kfree(item_buffer);
}

