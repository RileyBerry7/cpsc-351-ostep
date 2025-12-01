#include <stdio.h>
#include <pthread.h>

int counter = 0;
// mutex
pthread_mutex_t lock;

void* increment(void* arg)
{
    for(int i = 0; i < 100000; i++)
    {
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main()
{
    pthread_t threads[10];

    // make 10 threads
    for(int i = 0; i < 10; i++)
        pthread_create(&threads[i], NULL, increment, NULL);

    // wait for threads to finish
    for(int i = 0; i < 10; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&lock);
    printf("Final counter with mutex: %d\n", counter);

    return 0;
}