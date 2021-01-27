#include "shell.h"
#include "command.h"
#include "usefulFunctions.h"
#include "typeDefs.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h> // for waitpid
void shellInputLoop() {
	struct Shell* shell = malloc(sizeof(struct Shell));
	initShell(shell);
	while (shell->isRunning) {
		char* shellArg = getUserStringInput(": ", 0);
		handleShellArgument(shellArg,shell);
		checkForZombies(shell);
	}

	freeShell(shell);

}


void initShell(struct Shell* shell) {
	
	shell->isRunning = true;
	shell->pid = getpid();
	int pidLength = getLengthOfNumber(shell->pid);
	shell->pidString = malloc(sizeof(char) * (pidLength + 1));
	sprintf(shell->pidString, "%d", shell->pid);

	shell->cwd = getcwd(NULL, 0);
	
	char* homeString = getenv("HOME");
	shell->homeDirectory = malloc(strlen(homeString) + 1);
	strcpy(shell->homeDirectory, homeString);

	shell->status = 0;
	shell->signalTerminated = 0;
	shell->lastExitedByStatus = true;
	shell->lastExitedBySignal = false;
	

}

void freeShell(struct Shell* shell) {
	assert(shell);
	if (shell->pidString) {
		free(shell->pidString);
	}
	if (shell->cwd) {
		free(shell->cwd);
	}
	if (shell->homeDirectory) {
		free(shell->homeDirectory);
	}

	free(shell);
	
}


int handleShellArgument(char* shellArg, struct Shell* shell) {

	struct Command* command = parseCommand(shellArg, shell);
	if (command->isComment) {
		freeCommand(command);
		return 0;
	}
	int status = isBasicCommand(command,shell);
	
	//gets set to 1 if it's not a basic command
	if (status == 1) {
		if (command->background_execute) {
			handleAdvancedCommandBackground(command, shell);
		}
		else {
			handleAdvancedCommand(command, shell);
		}
	
	}

	freeCommand(command);
	return status;
}
void checkForZombies(struct Shell* shell) {
	int childStatus = 0;
	pid_t childID = waitpid(-1, &childStatus, WNOHANG);
	if (childID) {
		handleStatusSignal(childStatus, shell);
		printf("background pid %d is done: ", childID);
		printStatus(shell);
	}
}



void printStatus(struct Shell* shell) {
	if (shell->lastExitedByStatus) {
		printf("%s %d\n", EXIT_STATUS_STRING, shell->status);
	}
	else if (shell->lastExitedBySignal) {
		printf("%s %d\n", TERMINATED_STRING, shell->signalTerminated);
	}
}
