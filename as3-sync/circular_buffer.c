#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 10

int main(int argc, char argv[]){

    typedef struct { int index;} item; // Anon-Struct

    item* buffer[BUFFER_SIZE] = {NULL};
    int counter = 0;
    int in      = 0;
    int out     = 0;
    
    // Producer Function
    void produce(item **arr, int size, int *in){

        item *new_item = malloc(sizeof(item));
        new_item->index = *in;
        arr[*in]    = new_item;
        printf("Produced index: %d\n", new_item->index);
        (*in)++;
    }

    // Consumer Function
    void consume(item **arr, int size, int *out){
        if (arr[*out] != NULL) {
            item *old_item = arr[*out];
            arr[*out] = NULL;
            printf("Consumed index: %d\n", old_item->index);
            (*out)++;
            free(old_item);
        } else {
            printf("Nothing to consume.\n");
        } 
    }

    // Draw Buffer Function
    void draw_buffer(item **arr, int size){
        for (int i = 0; i < size; i++){
            if ( arr[i] == NULL){
                printf("[ ]");
            } else {
                printf("[*]");
            }
        }
    }
    
    char input[10] = "r";

    // Main Loop
    while (strcmp(input, "x") != 0){
        
        system("clear");
        draw_buffer(buffer, BUFFER_SIZE);
        strcpy(input, "r");
        printf("\n\n\t[c] Consume\n\t[p] Produce\n\t[x] Exit\n");
        printf("\t: ");
        scanf("%s", input);

        // Produce Action
        if (strcmp(input, "p") == 0) {
            produce(buffer, BUFFER_SIZE, &in);

        // Consume Action
        } else if (strcmp(input, "c") == 0) {
            consume(buffer, BUFFER_SIZE, &out);
        }
    }



    return 0;
}
