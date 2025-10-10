// figure 5.1 "explaining fork()"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char argv[]){
	
	printf("Initial Process (pid:%d)\n\n", getpid());
	
	pid_t pid = fork();
	
	// ------ Fork Failed -------
	if (pid < 0) {
		fprintf(stderr, "fork failed\n");
		exit(-1);
	
	// ------ Child Process ------
	} else if (pid == 0) {
		printf("Child (pid:%d)\n", getpid());
		printf("...\n\n");

	// ------ Parent Process ------
	} else {
		printf("Parent of %d (pid:%d)\n", pid, getpid());
		printf("...\n\n");
	}
	
	return 0;
}
