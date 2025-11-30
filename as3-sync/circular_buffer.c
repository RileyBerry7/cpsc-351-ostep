#include <stdio.h>
#define BUFFER_SIZE 10

int main(int argc, char argv[]){

    printf("Hello Cirular Buffer\n");

    typedef struct { int empty;} item; // Anon-Struct

    item buffer[BUFFER_SIZE];

    int counter = 0;
    int in      = 0;
    int out     = 0;
    
    return 0;
}
