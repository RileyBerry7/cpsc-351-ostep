// figure 5.4 "explaining pipes and intrepocedural communication"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char argv[]){
	
	printf("\n[Parent Process (pid:%d)]\n", getpid());
	printf("  calling: fork()\n\n");

	pid_t pid = fork();
	
	// ------ Fork Failed -------
	if (pid < 0) {
		fprintf(stderr, "fork failed\n");
		exit(-1);
	
	// ------ Child Process ------
	} else if (pid == 0) {
		printf("[Child Process (pid:%d)]\n", getpid());

		// redirect output to .output file
		printf("  closing stdout\n");
		printf("  opening .output\n");
		printf("  calling exec()\n");
		close(STDOUT_FILENO);
		open("./p4.output", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
		
		// collect arguments
		char *args_list[3];
		args_list[0] = strdup("wc");	       // program name "wc"
		args_list[1] = strdup("figure_5_3.c"); // input file name
		args_list[2] = NULL;		       // end of array marker
		
		// execute "wc"
		//fflush(stdout);
		execvp(args_list[0], args_list);	    // runs word count
		
		// --- Unreachable ---

	// ------ Parent Process ------
	} else {
		printf("[Parent Post-Fork]\n");
		printf("  calling: wait()\n\n");
		int rv = wait(NULL);
		if (rv == -1) {
			printf("  wait failed");
		} else {
			printf("\n  reaping zombie: %d\n", rv);
			printf("  wait completed");
		}
		printf("  terminating parent\n");
	}
	
	return 0;
}


