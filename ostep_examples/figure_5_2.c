// figure 5.2 "explaining wait`()"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char argv[]){
	
	printf("\nInitial Process (pid:%d)\n", getpid());
	printf("  ...\n");
	printf("  calling: fork()\n\n");

	pid_t pid = fork();
	
	// ------ Fork Failed -------
	if (pid < 0) {
		fprintf(stderr, "fork failed\n");
		exit(-1);
	
	// ------ Child Process ------
	} else if (pid == 0) {
		printf("Child Process (pid:%d)\n", getpid());
		printf("  ...\n");
		printf("  terminating: becoming 'zombie'\n\n");

	// ------ Parent Process ------
	} else {
		printf("Parent Process (pid:%d) (child: %d)\n", getpid(), pid);
		printf("  ...\n");
		printf("  calling: wait()\n\n");
		int reaped_pid = wait(NULL);
		
		printf("[reaping child: %d]\n", reaped_pid);
		printf("Parent Process (pid:%d) (child: %d)\n", getpid(), pid);
		printf("  ...\n");
		printf("  terminating: living children are now 'orphans'\n\n");
	}
	
	return 0;
}

