// semaphore.c

//#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h> // NULL macro

#define QUEUE_NAME "/my_queue"

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
//                  MY SEMAPHORE IMPLEMENTATION
//==============================================================================
typedef struct {
    int   value;     // Resources
    pid_t list[100]; // Waiting processes
    int   count;     // Length of waitlist

} semaphore;

//===================== FUNCTION PROTOTYPES ====================================
void block();
void wake_up();
int  sem_wait(semaphore* s);
int  sem_post(semaphore* s);

//======================== CONFIGURATION =======================================
#define RESOURCES 2


//==============================================================================
//                                      MAIN
//==============================================================================
int main(int argc, char *argv[]) {
    
    // POSIX Message Queue Attributes
    struct mq_attr attr;
    attr.mq_flags   = 0;    // Blocking queue
    attr.mq_maxmsg  = 1;    // Maximum number of messages in the queue
    attr.mq_msgsize = 1;    // Maximum message size in bytes
    attr.mq_curmsgs = 0;    // Current number of messages (set to 0 initially)
    
    printf("Hello world!\n");
    semaphore my_s = {RESOURCES, {NULL}, 0};
    
    // Fork 3 Children
    for (int i = 0; i < 3; i++) {
        printf("[CHILD %d] Created\n", i+1);
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        // ============================
        // CHILD 1
        // ============================
        if (pid == 0 && i == 0) {
            //printf("[CHILD 1] PID=%d, Parent=%d\n", getpid(), getppid());
            sem_wait(&my_s);
            printf("[CHILD 1] Waiting 10 Sedonds...\n");
            sleep(10);
            exit(0);
        }

        // ============================
        // CHILD 2
        // ============================
        if (pid == 0 && i == 1) {
            sem_wait(&my_s);
            printf("[CHILD 2] Waiting 10 Sedonds...\n");
            sleep(10);
            exit(0);
        }

        // ============================
        // CHILD 3
        // ============================
        if (pid == 0 && i == 2) {
            sem_wait(&my_s);
            printf("[CHILD 3] Waiting 10 Sedonds...\n");
            sleep(10);
            exit(0);
        } 
    }   // END - Fork Loop

    // ============================
    // PARENT: after all forks
    // ============================
    printf("[PARENT] Waiting for children...\n");
    
    // Wait for all 3 children to finish
    for (int i = 0; i < 3; i++) {
        wait(NULL);
        printf("[CHILD %d] Exiting.\n", i+1);
    }

    printf("[PARENT] Exiting...\n");
    return 0;
}
//_____________________________________________________________________________
// END - Main




//=============================================================================
//                                  BLOCK
//=============================================================================
void block() {
    extern struct mq_attr attr;
    
    bool   buffer;
    ssize_t bytes_read;
    mqd_t   mq;

    // Message Queue Open
    mq = mq_open(QUEUE_NAME, O_RDONLY | O_CREAT); 
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
    printf("Message received: %d\n", buffer);
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
    if (s->value <= 0) {
        
        // Add Process to waitlist
        s->list[s->count];
        s->count++;
        printf("Process added to waitlist\n");
        
        // Block until a resource becomes available
        printf("\tProcess blocking\n");
        block();
        printf("\tProcess woken up\n");

        // Pop from the waitlist
        s->list[s->count] = NULL;
        s->count--;
    } else {
        printf("\tProcess was served immediately\n");
    }
    printf("\tCurrently holding 1 resource\n");
    return 0;
}


//=============================================================================
//                                 WAKE_UP 
//=============================================================================
void wake_up() {

    struct mq_attr attr;
    mqd_t   mq;

    // Message Queue Open / Create
    mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
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
    printf("Message sent: %d\n", message);
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
    if (s->value <= 0) {
        // Pop a process from the waiting list
        // p s->list[s->count]
        // s->count--;
        wake_up();
    }
}
