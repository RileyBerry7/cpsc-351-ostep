// figure 5.4 "explaining pipes and intrepocedural communication"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
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
		
		// redirect output to .output file
		printf("  closing stdout");
		printf("  opening .output");
		close(STDOUT_FILENO);
		open("./p4.output", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
		
		// collect arguments
		printf("  populating: args_list\n");
		char *args_list[3];
		args_list[0] = strdup("wc");	       // program name "wc"
		args_list[1] = strdup("figure_5_3.c"); // input file name
		args_list[2] = NULL;		       // end of array marker
		
		// execute "wc"
		printf("  calling: execvp(program=\"wc\", args_list)\n");
		printf("  ...\n");
		execvp(args_list[0], args_list);	    // runs word count
		
		// --- Unreachable ---
		printf("[This should not be reachable]");
		printf("  ...\n");
		printf("  terminating: becoming 'zombie'\n\n");

	// ------ Parent Process ------
	} else {
		printf("Parent Process (pid:%d) (child: %d)\n", getpid(), pid);
		printf("  ...\n");
		printf("  calling: wait()\n\n");
		int reaped_pid = wait(NULL);
		
		printf("\n[reaping child: %d]\n", reaped_pid);
		printf("Parent Process (pid:%d) (child: %d)\n", getpid(), pid);
		printf("  ...\n");
		printf("  terminating: living children are now 'orphans'\n\n");
	}
	
	return 0;
}


