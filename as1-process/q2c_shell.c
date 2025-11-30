// cpsc-351-ostep/as1/q2c.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    
    char  input[100];
    pid_t pid;

    // infinite loop
    while (1){
	
	// OUTPUT
	printf("cmd> ");
	fflush(stdout);
	
	// INPUT  
	if (fgets(input, sizeof(input), stdin) == NULL){
	   break;
	}
	input[strcspn(input, "\n")] = '\0'; // strip whitespace
	
	// exit parent process on "exit" 
	if (strcmp(input, "exit") == 0){
	    break;
	}
	
	// create child process
	pid = fork(); 
	
	// fork() error check
	if (pid < 0){

	    perror("fork");
	    exit(-1); // return value indicates failure
	}
	else if (pid == 0){
	    execlp(input, input, (char*)NULL); 
	    perror("execlp");
	    _exit(-1);
	 
	 // execcvp() // multiple args
	 // execlp();
	} 
	else {

	    // wait() error check
	    if (wait(NULL) == -1) {
		perror("wait");
		exit(-1);
	    }
	}

    } // END - infinite loop

    return 0;
}

