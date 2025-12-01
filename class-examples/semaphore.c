#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* This is roughly how POSIX semaphores and pthread mutexes behave under the hood. */

int main(int argc, char *argv[]) {
    
    wait(S) {
        S->value--;
        if (S->value < 0) {
            add this process to S->list;
            block();
        }
    }

    signal(S) {
        S->value++;
        if (S->value <= 0) {
            remove a process P from S->list;
            wakeup(P);
        }
    }
    return 0;
}
