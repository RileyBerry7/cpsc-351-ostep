// semaphore.c

//#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h> // NULL macro
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

//==============================================================================
// SEMAPHORES
//------------------------------------------------------------------------------
// This is roughly how POSIX semaphores and pthread mutexes behave under the hood.
//==============================================================================

//sem_t s; // This is a POSIX semaphore in C.
//&s;      // Address-of '&' operator returns a pointer to the object.

// s = int : # of resources available
//           so 5 means that 5 resources are available.
//           and -3 means that 3 processes are still waiting for a resource.

//==============================================================================
//              CUSTOM SEMAPHORE IMPLEMENTATION
//==============================================================================
typedef struct {
    atomic_int  value;     // Resources
    pid_t       list[100]; // Waiting processes
    atomic_int  count;     // Length of waitlist

} semaphore;

//===================== FUNCTION PROTOTYPES ====================================
void* thread_1(void* arg);
void* thread_2(void* arg);
void* thread_3(void* arg);
void  block();
void  wake_up();
int   sem_wait(semaphore* s);
int   sem_post(semaphore* s);

//======================== CONFIGURATION =======================================

#define QUEUE_NAME "/my_queue"
#define RESOURCES     2
#define WAITLIST_SIZE 100
#define THREAD_COUNT  3

struct mq_attr attr = {
    .mq_flags   = 0,    // Blocking queue
    .mq_maxmsg  = 1,    // Maximum number of messages in the queue
    .mq_msgsize = 1,    // Maximum message size in bytes
    .mq_curmsgs = 0     // Current number of messages (automatically set by OS, often ignored here)
};

//==============================================================================
//                                      MAIN
//==============================================================================
int main(int argc, char *argv[]) {
    
    printf("Hello world!\n");
   
    mq_unlink(QUEUE_NAME);

    // Create Semaphore
    semaphore* my_s = malloc(sizeof(semaphore));
    
    // Initialize Semaphore
    my_s->value = RESOURCES;
    memset(my_s->list, 0, sizeof(my_s->list));
    my_s->count = 0;
    
    // Create Threads
    pthread_t threads[THREAD_COUNT];
    pthread_create(&threads[0], NULL, thread_1, my_s);
    pthread_create(&threads[1], NULL, thread_2, my_s);
    pthread_create(&threads[2], NULL, thread_3, my_s);

    
    // Wait for each thread to complete its work
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
        // The main thread pauses here until threads[i] finishes
    }

    free(my_s);

    printf("[MAIN] Exiting...\n");
    return 0;
}
//_____________________________________________________________________________
// END - Main


//=============================================================================
//                                  THREAD 1
//=============================================================================
void* thread_1(void* arg) {
    semaphore* my_s = arg; // create alias

    printf("[THREAD 1] Created\n");
    sem_wait(my_s);
    
    printf("[THREAD 1] Holding for 10 Seconds...\n");
    sleep(10);
    
    printf("[THREAD 1] Releasing resource\n");
    sem_post(my_s);

    printf("[THREAD 1] Exiting\n");
    pthread_exit(NULL);
}


//=============================================================================
//                                  THREAD 2
//=============================================================================
void* thread_2(void* arg) {
    semaphore* my_s = arg; // create alias

    printf("[THREAD 2] Created\n");
    sem_wait(my_s);
    
    printf("[THREAD 2] Holding for 10 Seconds...\n");
    sleep(10);
    
    printf("[THREAD 2] Releasing resource\n");
    sem_post(my_s);

    printf("[THREAD 2] Exiting\n");
    pthread_exit(NULL);
}


//=============================================================================
//                                  THREAD 3
//=============================================================================
void* thread_3(void* arg) {
    semaphore* my_s = arg; // create alias

    printf("[THREAD 3] Created\n");
    sem_wait(my_s);
    
    printf("[THREAD 3] Holding for 10 Seconds...\n");
    sleep(10);
    
    printf("[THREAD 3] Releasing resource\n");
    sem_post(my_s);

    printf("[THREAD 3] Exiting\n");
    pthread_exit(NULL);
}

//=============================================================================
//                                  BLOCK
//=============================================================================
void block() {
    
    bool   buffer;
    ssize_t bytes_read;
    mqd_t   mq;

    // Message Queue Open
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0666, &attr); 
    if (mq == (mqd_t)-1) {
        perror("mq_open for receiving");
        exit(EXIT_FAILURE);
    }

    // Message Queue Receive                        <- Blocks until receive
    bytes_read = mq_receive(mq, (const char*)&buffer, 1, NULL);
    if (bytes_read == -1) {
        perror("mq_receive");
        exit(EXIT_FAILURE);
    }
    printf("\tMessage received: %d\n", buffer);
    mq_close(mq);
}

//==============================================================================
//                                      SEM_WAIT
//------------------------------------------------------------------------------
// Decrement Semaphore, possibly blocks.
//  Parameters: 1. sem_t* s;
//      
//      Calling process holds 1 resource, or blocks if none are avaliable.
//==============================================================================
int sem_wait(semaphore* s) {
   

    printf("\t%d resources are avaliable.\n", s->value);

    s->value--; // Remove 1 resource <- held in current process
    
    // If No Resources Are Available
    if (s->value < 0) {
        
        // Add Process to waitlist
        s->list[s->count];
        s->count++;
        printf("\tAdded to waitlist\n");
        
        // Block until a resource becomes available
        printf("\tBlocked\n");
        block();
        printf("\tWoken up\n");

        // Pop from the waitlist
        s->list[s->count] = NULL;
        s->count--;
    } else {
        printf("\tServed immediately\n");
    }
    printf("\tHolding 1 resource\n");
    return 0;
}


//=============================================================================
//                                 WAKE_UP 
//=============================================================================
void wake_up() {

    mqd_t   mq;

    // Message Queue Open / Create
    mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // Send Message to Message Queue
    bool message = true;
    if (mq_send(mq, (const char*)&message, 1, 0) == -1) { // (mq, msg, siz, prio)
        perror("mq_send");
        exit(EXIT_FAILURE);
    }
    printf("\tMessage sent: %d\n", message);
    mq_close(mq);
}   

//==============================================================================
//                                      SEM_POST
//------------------------------------------------------------------------------
// Increment Semaphore (Release Resource).
//==============================================================================
int sem_post(semaphore* s) {
    
    s->value++;

    // If Waitinglist is full
    if (s->count > 0) {
        wake_up();
    }
}
