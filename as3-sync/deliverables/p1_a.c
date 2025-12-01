#include <stdio.h>
#include <pthread.h>

int counter = 0;

void* increment(void* arg)
{
    for(int i = 0; i < 100000; i++)
        counter++;

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

    printf("Final counter with no mutex: %d\n", counter);
    return 0;
}