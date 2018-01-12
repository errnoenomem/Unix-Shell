#include "sfish.h"

char previousDir[MAX_PATH_LENGTH];
char currentDir[MAX_PATH_LENGTH];
char** environ;
char** args;
char* fixedPath;
int seconds = 0;
// static process backgrounds[MAX_BACKGROUNDS];
// static int background_num = 0;

int builtIns(char* cmd, char** envp) // builtIns
{
	if(environ == NULL)
	{
		environ = malloc(500);
		environ = envp;
	}
	if ( strcmp(cmd, "") == 0 || strcmp(cmd, " ") == 0 || strcmp(cmd, "  ") == 0 || strcmp(cmd, "   ") == 0 ||cmd == NULL)
	{
		return 0;
	}
	if (cmd == '\0' || strcmp(cmd, "exit") == 0) // if the parsed string is null or exit
	{
		return 1; //return 1 for exit
	}
	if ( strcmp(cmd, "help") == 0 ) //help
	{
		help();
	}
	else if ( (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == ' ') || (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == '\0') ) // cd variation
	{
		cd(cmd);
	}
	else if ( cmd[0] == 'p' && cmd[1] == 'w' &&cmd[2] == 'd' ) // pwd
	{
		if (cmd[4] == '>')
		{
			pwd(cmd, 1);
		}
		else
		{
			pwd(cmd, 0);
		}
	}
	else if ( cmd[0] == 'a' && cmd[1] == 'l' && cmd[2] == 'a' && cmd[3] == 'r' && cmd[4] == 'm' && cmd[5] == ' ' && isdigit(cmd[6]) != 0)
	{
		sig_alarm(cmd);
	}
	else if ( strcmp(cmd, "jobs") == 0 )
	{

	}
	else
	{
		exec(cmd);
	}
	return 0;
}

void sig_alarm(char* cmd)
{
	pid_t pid;
	args = getTokenArray(cmd, " ");
	for(int i = 0; i < strlen(args[1]); i++)
	{
		if(isdigit(args[1][i]) == 0)
		{
			perror("invalid format");
			return;
		}
	}
	if ((seconds = atoi(args[1])) != 0) {
		sleep(seconds);
		pid = getpid();
		if (kill(pid, SIGALRM) == -1) {
			perror("kill()");
		}
	}
}

void help()
{
	printf("%s\n", "help");
	printf("%s\n", "exit");
	printf("%s\n", "cd [-] [.] [..]");
	printf("%s\n", "pwd");
	printf("%s\n", "alarm [n]");
	return;
}
void cd(char* cmd)
{
	setCurrentDir(); // set currentDir to the current working dir
	if (strcmp(cmd, "cd -") == 0) //if you want to use cd -,
	{
		if (previousDir[0] != '\0') //if previousDir is not null
		{
			chdir(previousDir); //changedir to previous dir
			setPreviousDir(); //set the currdir as previous dir
		}
		else
		{
			perror("No previous directory");
		}
		return;
	}
	else if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "cd ") == 0) // "cd" and "cd "
	{
		chdir(getenv("HOME")); //chdir to home environment
		setPreviousDir();
	}
	else if (cmd[3] != '\0') // if you want to chdir to a different directory (works with "." and "..")
	{
		cdPath(cmd);
		return;
	}
}

void cdPath(char* cmd)
{
	int i = 3;
	while ( cmd[i] != '\0')
	{
		i++;
	}
	char *path = strncpy(cmd, cmd + 3, i);

	if ( chdir(path) != 0 ) //if you failed to move directories, print error and previousDir will remain the same
	{
		printf("bash: %s: file or directory does not exist\n", cmd);
	}
	else //if not fail, set previous dir to the current dir as the "new" previous dir
	{
		setPreviousDir();
	}
}

void pwd(char* cmd, int oRed)
{
	pid_t pid;
	char cwd[1024];
	char* path;
	if ( (getcwd(cwd, sizeof(cwd)) == NULL) )
	{
		return;
	}
	if(oRed)
	{
		args = getTokenArray(cmd, " ");
		if (args[2] == NULL)
		{
			perror("invalid format");
		}
		path = getPathFromArgs(args, ">");
		if ( (pid = fork()) == -1 )
		{
			perror("fail to fork");
		}
		else if (pid == 0)
		{
			int fd;
			if ( (fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU) ) == -1)
			{
				printf("%s: %s\n", path, "fail to open file descriptor");
				return;
			}
			if ( dup2(fd, STDOUT_FILENO) == -1)
			{
				perror("fail to dup");
				return;
			}
			close(fd);
			printf("%s\n", cwd);
			exit(1);
		}
		else
		{
			int status;
			wait(&status);
		}
	}
	else
	{
		printf("%s\n", cwd);
	}
}

void setPreviousDir()
{
	if ( currentDir[0] != '\0')
	{
		strcpy(previousDir, currentDir);
	}
}

void setCurrentDir()
{
	if ( (getcwd(currentDir, sizeof(currentDir)) == NULL) )
	{
		return;
	}
}

char** getTokenArray(char* cmd, char* delim)
{
	char** args = malloc(400);
	int i = 0;
	char* token = strtok (cmd, delim);
	while (token != NULL) //puts every string separated by space into args array
	{
		args[i++] = token;
		token = strtok(NULL, delim);
	}
	args[i] = NULL; //last arg is NULL

	i = 0;

	return args;
}

char* searchPath(char* basename)
{
	char* fixedPath = malloc(400);
	char* PATH = getenv("PATH"); // path
	char PATH_TO_TOKE[500]; // fake path to not edit real path
	strcpy(PATH_TO_TOKE, PATH);

	char* token = strtok(PATH_TO_TOKE, ":"); //toke

	while(token != NULL)
	{
		strcpy(fixedPath, token); //fixed path gets value of token
		strcat(fixedPath, "/"); // cat a forward slash to the end
		strcat(fixedPath, basename); //cat the first argument onto the path
		if (checkValidPath(fixedPath) == 0) //check whether or not valid path, if it is - break
		{
			break;
		}
		token = strtok(NULL, ":"); // toke some more for next val
	}
	return fixedPath;
}

int checkValidPath(char* path)
{
	struct stat buf;
	if (stat(path, &buf) == -1)
	{
		return -1;
	}
	return 0;
}

void freeAll()
{
	if (args != NULL)
	{
		free(args);
	}
	if (fixedPath != NULL)
	{
		free(fixedPath);
	}
}

char* getPathFromArgs(char** args, char* symbol)
{
	int i = 0;
	while( strcmp(args[i], symbol) != 0)
	{
		i++;
	}
	if(args[i+1] != NULL)
	{
		return args[i+1];
	}
	else
	{
		perror("invalid format");
		return NULL;
	}
	return NULL;
}

void exec(char* cmd)
{
	if (strchr(cmd, '>') != NULL || strchr(cmd, '<') != NULL) // check for redirection
	{
		redirection(cmd);
		return;
	}

	if (strchr(cmd, '|') != NULL)
	{
		pipe_redirection(cmd);
		return;
	}

	args = getTokenArray(cmd, " ");

	if (strchr(args[0], '/') == NULL) //if no forward slash entered by user
	{
		fixedPath = searchPath(args[0]);
		args[0] = fixedPath; //once determined path (wrong or right), args[0] gets the fixed path
	}
	else //slash indicating file
	{
		if (checkValidPath(args[0]) == -1) //test if it exists
		{
			printf("%s: %s\n", basename(args[0]),"file or directory does not exist");
			return;
		}
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fail to fork");
	    return;
	}
	else if (pid > 0)
	{
	    int status;
	    waitpid(pid, &status, 0);
	}
	else
	{
		if (checkValidPath(args[0]) == -1) //if bad no slash command
		{
			printf("%s: command not found\n", basename(args[0]));
		}
		else //else execute
		{
			execve(args[0], args, NULL);
			exit(1);
		}
	}
}

void redirection(char* cmd)
{
	char* args2[100];
	char* inputPath = NULL;
	char* outputPath = NULL;
	int iRed = 0;
	int oRed = 0;
	int ogRed = 0;
	char num[10];
	int i = 0;
	int pos;
	char* special = NULL;

	args = getTokenArray(cmd, " ");
	if (args[2] == NULL)
	{
		perror("invalid format");
		return;
	}
	if (strchr(args[0], '/') == NULL) //if no forward slash entered by user
	{
		fixedPath = searchPath(args[0]);
		args[0] = fixedPath; //once determined path (wrong or right), args[0] gets the fixed path
	}
	else //slash indicating file
	{
		if (checkValidPath(args[0]) == -1) //test if it exists
		{
			printf("%s: %s\n", basename(args[0]),"file or directory does not exist");
			return;
		}
	}

	if (checkValidPath(args[0]) == 0)
	{
		while (args[i] != NULL)
		{
			if ( strcmp(args[i], "<") == 0)
			{
				iRed = 1;
			}
			if ( strcmp(args[i], ">") == 0)
			{
				oRed = 1;
			}
			if( strcmp(args[i], "1>") == 0 || strcmp(args[i], "2>") == 0 )
			{
				pos = i;
				num[0] = args[i][0];
				ogRed = atoi(num);
				iRed = 0;
				oRed = 0;
			}
			if ( strcmp(args[i], "&>" ) == 0 )
			{
				oRed = 0;
				iRed = 0;
				special = "&>";
			}
			if ( strcmp(args[i], ">>" ) == 0 )
			{
				oRed = 0;
				iRed = 0;
				special = ">>";
			}
			i++;
		}

		if (iRed == 1)  //set if input
		{
			inputPath = getPathFromArgs(args, "<");
			if (checkValidPath(inputPath) == -1)
			{
				printf("%s: %s\n", basename(inputPath),"file or directory does not exist");
				return;
			}
			i = 0;
			while( strcmp(args[i], "<") != 0)
			{
				args2[i] = args[i]; //augment here need to change later
				i++;
			}
			args2[i] = NULL;
		}

		if (iRed == 1 && oRed == 1) //if both get output path and redirect
		{
			outputPath = getPathFromArgs(args, ">");
			execRedirection(args2, args[0], inputPath, outputPath, iRed, oRed, ogRed, special);
		}
		else if (oRed == 1 && iRed == 0) //if output only
		{
			outputPath = getPathFromArgs(args, ">");
			i = 0;
			while( strcmp(args[i], ">") != 0 )
			{
				args2[i] = args[i]; //augment here need to change later
				i++;
			}
			args2[i] = NULL;
			execRedirection(args2, args[0], inputPath, outputPath, iRed, oRed, ogRed, special);
		}
		else if (ogRed != 0 && iRed == 0 && oRed == 0)
		{
			outputPath = getPathFromArgs(args, args[pos]);
			i = 0;
			while( strcmp(args[i], args[pos] ) != 0)
			{
				args2[i] = args[i]; //augment here need to change later
				i++;
			}
			args2[i] = NULL;
			execRedirection(args2, args[0], inputPath, outputPath, iRed, oRed, ogRed, special);

		}
		else if (special != NULL && iRed == 0 && oRed == 0)
		{
			outputPath = getPathFromArgs(args, special);
			i = 0;
			while( strcmp(args[i], special ) != 0)
			{
				args2[i] = args[i]; //augment here need to change later
				i++;
			}
			args2[i] = NULL;
			execRedirection(args2, args[0], inputPath, outputPath, iRed, oRed, ogRed, special);

		}
		else // else must be input only , redirect
		{
			execRedirection(args2, args[0], inputPath, outputPath, iRed, oRed, ogRed, special);
		}
	}
	else
	{
		printf("%s: %s\n", basename(args[0]),"file or directory does not exist");
		return;
	}
}

void execRedirection(char** args, char* program, char* file, char* file2, int iRed, int oRed, int ogRed, char* special)
{
	pid_t pid = fork();

	if (pid == -1)
	{
		perror("fail to fork");
		return;
	}
	else if (pid > 0)
	{
		int status;
		waitpid(pid, &status, 0);
	}
	else
	{
		if (iRed == 1)
		{
			int fd;
			if ( (fd = open(file, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
			{
				printf("%s: %s\n", file, "fail to open file descriptor");
				return;
			}
			if ( (dup2(fd, STDIN_FILENO)) == -1 )
			{
				perror("fail to dup");
				return;
			}
			close(fd);
		}
		if (oRed == 1)
		{
			int fd2;
			if ( (fd2 = open(file2, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
			{
				printf("%s: %s\n", file2, "fail to open file descriptor");
				return;
			}
			if ( dup2(fd2, STDOUT_FILENO) == -1)
			{
				perror("fail to dup");
				return;
			}
			close(fd2);
		}
		else if (ogRed != 0 && oRed == 0 && iRed == 0)
		{
			int fd;
			if ( (fd = open(file2, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
			{
				printf("%s: %s\n", file2, "fail to open file descriptor");
				return;
			}
			if ( dup2(fd, ogRed) == -1)
			{
				perror("fail to dup");
				return;
			}
			close(fd);
		}
		else if (special != NULL  && oRed == 0 && iRed == 0)
		{
			int fd;
			if ( strcmp(special, "&>") == 0 )
			{
				if ( (fd = open(file2, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
				{
					printf("%s: %s\n", file2, "fail to open file descriptor");
					return;
				}
				if ( dup2(fd, STDOUT_FILENO) == -1)
				{
					perror("fail to dup");
					return;
				}
				if ( dup2(fd, STDERR_FILENO) == -1)
				{
					perror("fail to dup");
					return;
				}
			}
			if ( strcmp(special, ">>") == 0)
			{
				if ( (fd = open(file2, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) == -1)
				{
					printf("%s: %s\n", file2, "fail to open file descriptor");
					return;
				}
				if ( dup2(fd, STDOUT_FILENO) == -1)
				{
					perror("fail to dup");
					return;
				}
			}
			close(fd);
		}
		if (checkValidPath(program) == 0)
		{
			execve(program, args, environ);
			exit(1);
		}
		else
		{
			printf("%s: %s\n", basename(program),"file or directory does not exist");
			return;
		}
	}
}

void pipe_redirection(char* cmd)
{
	args = getTokenArray(cmd, " ");
	if (args[2] == NULL)
	{
		perror("invalid format");
		return;
	}
	char* args2[100];
	char* args3[100];
	char* args4[100];
	args2[0] = NULL;
	args3[0] = NULL;
	args4[0] = NULL;
	int i = 0;
	int j = 0;

	while( strcmp(args[j], "|") != 0)
	{
		args2[i++] = args[j++];
	}
	args2[i] = NULL;
	i = 0;
	j++;

	if ( args[j] != NULL )
	{
		while (args[j] != NULL && strcmp(args[j], "|") != 0 )
		{
			args3[i++] = args[j++];
		}
		args3[i] = NULL;
		i = 0;
		j++;

		if (args[j] != NULL)
		{
			i = 0;
			while( args[j] != NULL )
			{
				args4[i++] = args[j++];
			}
		}

		if (args4[0] != NULL)
		{
			args4[i] = NULL;
		}
	}
	else
	{
		perror("invalid format");
		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (strchr(args2[0], '/') == NULL)
	{
		fixedPath = searchPath(args2[0]);
		args2[0] = fixedPath;
	}
	if (checkValidPath(args2[0]) == -1)
	{
		printf("%s: %s\n", basename(args2[0]),"file or directory does not exist");
		return;
	}

	if (strchr(args3[0], '/') == NULL)
	{
		fixedPath = searchPath(args3[0]);
		args3[0] = fixedPath;
	}
	if (checkValidPath(args3[0]) == -1)
	{
		printf("%s: %s\n", basename(args3[0]),"file or directory does not exist");
		return;
	}
	if (args4[0] != NULL)
	{
		if (strchr(args4[0], '/') == NULL)
		{
			fixedPath = searchPath(args4[0]);
			args4[0] = fixedPath;
		}
		if (checkValidPath(args4[0]) == -1)
		{
			printf("%s: %s\n", basename(args4[0]),"file or directory does not exist");
			return;
		}
	}

	execPipeRedirection(args2, args3, args4);

	return;
}

void execPipeRedirection(char** args2, char** args3, char** args4)
{
	int status;
	int fd[2];

	if (pipe(fd) == -1)
	{
		perror("fail to pipe");
		return;
	}

	pid_t pid = fork();

	if (args4[0] != NULL) // if 3 pipes
	{
		int fd2[2];

		if (pipe(fd2) == -1)
		{
			perror("fail to pipe");
			return;
		}

		if (pid == -1)
		{
			perror("fail to fork");
			return;
		}
		else if (pid == 0)
		{
			if ( dup2(fd[0], STDIN_FILENO) == -1)
				{
					perror("fail to dup");
					return;
				}
	            close(fd[0]);
	            close(fd[1]);
	            execve(args4[0], args4, environ);
	            exit(3);
		}
		usleep(25000);
		if ( (pid = fork()) == -1)
		{
			perror("fail to fork");
			return;
		}
		else if (pid == 0)
		{
			if ( dup2(fd2[0], STDIN_FILENO) == -1 )
	        {
				perror("fail to dup");
	            return;
	        }
	        if ( dup2(fd[1], STDOUT_FILENO) == -1 )
	        {
				perror("fail to dup");
	            return;
	        }
	            close(fd2[0]);
	            close(fd2[1]);
				execve(args3[0], args3, environ);
	            exit(2);
		}
		usleep(25000);
		if ( (pid = fork()) == -1)
		{
			perror("fail to fork");
			return;
		}
		else if (pid == 0)
		{
			if ( dup2(fd2[1], STDOUT_FILENO) == -1 )
			{
				perror("fail to dup");
				return;
			}
	        close(fd2[0]);
	        close(fd2[1]);
			execve(args2[0], args2, environ);
	        exit(1);
		}
		else
		{
			wait(&status);
		}
	}
	else //2 pipes
	{
		if (pid == -1)
		{
			perror("fail to fork");
			return;
		}
		else if (pid == 0)
		{
			close(fd[1]);
			if ( dup2(fd[0], STDIN_FILENO) == -1)
			{
				perror("fail to dup");
				return;
			}
			close(fd[0]);
			execve(args3[0], args3, environ);
			exit(2);

		}
		usleep(25000);
		if ( (pid = fork()) == -1)
		{
			perror("fail to fork");
		}
		else if (pid == 0)
		{
			close(fd[0]);
			if ( dup2(fd[1], STDOUT_FILENO) == -1)
			{
				perror("fail to dup");
				return;
			}
			close(fd[1]);
			execve(args2[0], args2, environ);
			exit(1);
		}
		else
		{
			wait(&status);
		}
	}
	usleep(50000);
	fflush(stdout);
	fflush(stdin);
}

void sigalrm_handler(int signo)
{
    printf("Your %d second timer has finished!\n", seconds);
}
void sigchld_handler(int sig, siginfo_t *si, void *context)
{
	usleep(50000);
	clock_t time = (si->si_stime + si->si_utime) * 10; //CENTISECONDS?
    printf("Child with PID %d has died. It spent %ld milliseconds utilizing the CPU.\n", si->si_pid, time);
}
void sigchld_handler2(int sig)
{
	usleep(50000);
	printf("sad");
	//clock_t time = (si->si_stime + si->si_utime) * 10; //CENTISECONDS?
    //printf("Child with PID %d has died. It spent %ld milliseconds utilizing the CPU.\n", si->si_pid, time);
}
void sigusr2_handler(int signo)
{
    printf("Well that was easy.\n");
}

