#ifndef SFISH_H
#define SFISH_H
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>


#define MAX_PATH_LENGTH 1024

int builtIns(char* cmd, char** envp);
void help();
void cd(char* cmd);
void cdPath(char* cmd);
void pwd();
void setPreviousDir();
void setCurrentDir();
void ls();
char** getTokenArray(char*cmd, char* delim);
char* searchPath(char* basename);
int checkValidPath(char* path);
void exec(char* cmd);
void redirection(char* cmd);
void execRedirection(char** args, char* program, char* file, char* file2, int iRed, int oRed, int ogRed, char* special);
void freeAll();
char* getPathFromArgs(char** args, char* symbol);
void pipe_redirection(char* cmd);
void execPipeRedirection(char** args2, char** args3, char** args4);
void sig_alarm(char* cmd);
void sigchld_handler(int sig, siginfo_t *si, void *context);
void sigalrm_handler(int signo);
void sigusr2_handler(int signo);
void sigchld_handler2(int sig);

#endif
