// p3_thread_sync.c

// CPSC 351 - Assignment 3 (Part II: Thread Synchronization (Dining Philosophers))
// -------------------------------------------------------------------------------
// Build:
//      gcc -pthread p2_thread_sync.c -o p2
// Run:
//      ./p2
// -------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


// ============================================================================
//                                CONFIGURATION
// ============================================================================
#define N 5
#define LEFT(i)  ((i + N - 1) % N)
#define RIGHT(i) ((i + 1) % N)

typedef enum { THINKING, HUNGRY, EATING } state_t;

state_t state[N];                                  // Shared state array
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // One mutex
pthread_cond_t cond[N];                            // Shared condition array

// ============================================================================
//                            MONITOR FUNCTIONS
// ============================================================================

// Checks if a philosopher can eat
void test(int i) {
    if (state[i]        == HUNGRY &&
        state[LEFT(i)]  != EATING &&
        state[RIGHT(i)] != EATING) 
    {     
        state[i] = EATING;
        pthread_cond_signal(&cond[i]);
    }
}


// ============================================================================
//                          PHILOSOPHER THREAD FUNCTION
// ----------------------------------------------------------------------------
// Called by philosopher when hungry
// ============================================================================
void pickup_chopsticks(int i) {
    pthread_mutex_lock(&mutex);

    printf("Philosopher %d is hungry and trying to pick up chopsticks...\n", i);
    state[i] = HUNGRY;

    test(i);

    while (state[i] != EATING)
        pthread_cond_wait(&cond[i], &mutex);

    printf("Philosopher %d is eating...\n", i);

    pthread_mutex_unlock(&mutex);
}


// ============================================================================
//                          PHILOSOPHER THREAD FUNCTION
// ----------------------------------------------------------------------------
// Called by philosopher when done eating 
// ============================================================================
void putdown_chopsticks(int i) {
    
    pthread_mutex_lock(&mutex);

    printf("Philosopher %d has finished eating and is putting down chopsticks...\n", i);
    state[i] = THINKING;
    
    // Notify neighbors
    test(LEFT(i));
    test(RIGHT(i));

    pthread_mutex_unlock(&mutex);
}

// ============================================================================
//                          PHILOSOPHER THREAD FUNCTION
// ============================================================================
void* philosopher(void* arg) {
    int id = *(int*)arg;
    
    // Philosopher loop
    while (1) {
        printf("Philosopher %d is thinking...\n", id);
        sleep(1);               // thinks
        pickup_chopsticks(id);  // tries to eat
        sleep(1);               // eats
        putdown_chopsticks(id); // done eating
    }
}


// ============================================================================
//                                     MAIN
// ============================================================================
int main() {
    pthread_t threads[N];
    int ids[N];
    
    // Initialize states
    for (int i   = 0; i < N; i++) {
        ids[i]   = i;
        state[i] = THINKING;
        pthread_cond_init(&cond[i], NULL);
    }

    // Create philosophers threads
    for (int i = 0; i < N; i++)
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
    
    // Join threads
    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    return 0;
}

