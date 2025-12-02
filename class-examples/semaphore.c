// semaphore.c

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <stdbool.h>

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

void block();
int  sem_wait(sem_t* s);
int  sem_post(sem_t* s);
void wake_up(pid_t p);

//==============================================================================
//                  MY SEMAPHORE IMPLEMENTATION
//==============================================================================
typedef struct {
    int value       = 0;
     pid_t list[100] = {NULL}; // Waiting processes
    int count       = 0;

} semaphore;

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



    semaphore my_s;
    sem_wait(&my_s); // Hold resource
    sem_post(&my_s); // Release resource
    
    //=============================================================================
    //                                  BLOCK
    //=============================================================================
    void block() {
        extern struct mq_attr attr;
        
        bool break_cond = false;
        bool buffer[1];
        ssize_t bytes_read;

        // Message Queue Open
        mq = mq_open(QUEUE_NAME, O_RDONLY | O_CREAT); 
        if (mq == (mqd_t)-1) {
            perror("mq_open for receiving");
            exit(EXIT_FAILURE);
        }

        // Message Queue Receive                        <- Blocks until receive
        bytes_read = mq_receive(mq, buffer, 1, NULL);
        if (bytes_read == -1) {
            perror("mq_receive");
            exit(EXIT_FAILURE);
        }
        printf("Message received: %s\n", buffer);
        mq_close(mq);
    }

    //==============================================================================
    //                                      SEM_WAIT
    //------------------------------------------------------------------------------
    // Decrement Semaphore, possibly blocks.
    //  Parameters: 1. sem_t* s;
    //==============================================================================
    int sem_wait(semaphor* s) {

        s->value--;
        
        // If No Resources Are Available
        if (s->value <= 0) {
            
            // Add Process to waitlist
            s->list[s->count];
            s->count++;
            //wait(NULL);
            block();
        }
        return 0;
    }
    

    //=============================================================================
    //                                 WAKE_UP 
    //=============================================================================
    void wake_up(pid_t p) {

        extern struct mq_attr attr;

        // Message Queue Open / Create
        mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
        if (mq == (mqd_t)-1) {
            perror("mq_open");
            exit(EXIT_FAILURE);

        // Send Singal to process
        bool message = true;
        if (mq_send(mq, &message, 1, 0) == -1) { // (mq, msg, siz, prio)
            perror("mq_send");
            exit(EXIT_FAILURE);
        }
        printf("Message sent: %s\n", message);
        mq_close(mq);
    }   

    //==============================================================================
    //                                      SEM_POST
    //------------------------------------------------------------------------------
    // Increment Semaphore (Release Resource).
    //==============================================================================
    int sem_post(semaphore* s) {
        
        S->value++;

        // If Waitinglist is full
        if (S->value <= 0) {
            // Pop a process from the waiting list
            p s->list[s->count]
            s->count--;
            wake_up(p);
        }
    }
    return 0;
}
