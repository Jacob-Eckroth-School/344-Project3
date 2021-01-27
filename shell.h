
#ifndef SHELL
#define SHELL
#include <stdbool.h>

#include <sys/types.h>
#include <unistd.h> 

struct Shell {
	bool isRunning;
	pid_t pid;
	char* pidString;
	char* cwd;
	char* homeDirectory;
	int status;
	int signalTerminated;
	bool lastExitedByStatus;
	bool lastExitedBySignal;
};


void shellInputLoop();

int handleShellArgument(char* argument,struct Shell*);




void initShell(struct Shell*);

void freeShell(struct Shell*);

void printStatus();


#endif


