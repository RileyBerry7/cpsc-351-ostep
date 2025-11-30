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
    char header[100] = " --- Welcome to my Circular Buffer ---";
    
    int get_next(int index, int size){
        if (index+1 < size){
            return index+1;
        } else {
            return 0;
        }
    }

    // Producer Function
    const char* produce(item **arr, int size, int *in){
            
        static char buf1[100];

        if (arr[*in] == NULL) {
            item *new_item = malloc(sizeof(item));
            new_item->index = *in;
            arr[*in]    = new_item;
            *in = get_next(*in, size);

            sprintf(buf1, "Produced index: %d", new_item->index);
        
        } else {
            sprintf(buf1, "Cannot produce.(Buffer is full)");
        }
        return buf1;
    }   

    // Consumer Function
    const char* consume(item **arr, int size, int *out){

        static char buf2[100];
        
        if (arr[*out] != NULL) {
            item *old_item = arr[*out];
            arr[*out] = NULL;
            sprintf(buf2,"Consumed index: %d", old_item->index);
            *out = get_next(*out, size);
            free(old_item);
        } else {
            sprintf(buf2, "Nothing to consume.(Buffer is empty)");
        } 
        return buf2;
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
        printf("%s\n\n", header);
        draw_buffer(buffer, BUFFER_SIZE);
        strcpy(input, "r");
        printf("\n\n\t[c] Consume\n\t[p] Produce\n\t[x] Exit\n");
        printf("\t: ");
        scanf("%s", input);

        // Produce Action
        if (strcmp(input, "p") == 0) {
            strcpy(header, produce(buffer, BUFFER_SIZE, &in));

        // Consume Action
        } else if (strcmp(input, "c") == 0) {
            strcpy(header, consume(buffer, BUFFER_SIZE, &out));
        }
    }

    return 0;
}


