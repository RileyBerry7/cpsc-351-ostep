#include "stdlib.h"
#include "stdio.h"
#include "pthread.h"
#include "semaphore.h"

int main(int argc, char *argv[]) {
    printf("Hello Semaphores!\n");
    
    // wait
    sem_lock(s){
        while (s <= 0) ; // busy wait
        s--;
    }

    signal(s){
        s++;
    }

    return 0;
}

/*
* With semaphores, mutual exclusion becomes beautifually simple.
*
* wait(mutex);
* [Critical Section]
* singal(mutex);
*/
