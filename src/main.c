#include "sfish.h"
#include "debug.h"
/*
 * As in previous hws the main function must be in its own file!
 */


int main(int argc, char const *argv[], char* envp[]){

	struct sigaction sigact;

	sigact.sa_flags = SA_SIGINFO; // replaces default
	sigact.sa_sigaction = sigchld_handler; //override to own handler
    sigaction(SIGCHLD, &sigact, NULL); //child dies, sigaction acts to reciept of sigchld

	if (signal(SIGALRM, sigalrm_handler) == SIG_ERR) {
		perror("SIGALRM error");
	}
	if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR) {
		perror("SIGUSR2 error");
	}
	if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
		perror("SIGSTP error");
	}

	char cwd[1024];
	char str[1024];
	if ( (getcwd(cwd, sizeof(cwd)) != NULL) )
	{
		snprintf(str, sizeof(str), "<bikong> : <%s> $ ", cwd);
	}

    /* DO NOT MODIFY THIS. If you do you will get a ZERO. */
    rl_catch_signals = 0;
    /* This is disable readline's default signal handlers, since you are going to install your own.*/
    char *cmd;
    while((cmd = readline(str)) != NULL) {
    	if (*cmd == '\0')
    	{
    		continue;
    	}
    	// freeAll();
    	fflush(stdin);
		fflush(stdout);
        if (builtIns(cmd, envp) == 1 && strcmp(cmd, "") != 0 )
        {
        	break;
        }

        if ( (getcwd(cwd, sizeof(cwd)) != NULL) )
		{
			snprintf(str, sizeof(str), "<bikong> : <%s> $ ", cwd);
		}

        /* All your debug print statements should use the macros found in debu.h */
        /* Use the `make debug` target in the makefile to run with these enabled. */

        // info("Length of command entered: %ld\n", strlen(cmd));

        /* You WILL lose points if your shell prints out garbage values. */
    }

    /* Don't forget to free allocated memory, and close file descriptors. */
    free(cmd);

    return EXIT_SUCCESS;
}
