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
#include "signals.h"
#include "DynArray.h"


/*
** Description: Runs the shell input loop until the user exits
** Prerequisites: None
** Updated/Returned: None
*/
void shellInputLoop() {
	struct Shell* shell = malloc(sizeof(struct Shell));
	initShell(shell);
	shell->backgroundPIDs = malloc(sizeof(struct DynArray));
	initArray(shell->backgroundPIDs);
	initializeParentSignalHandler();
	//infinite loop until user enters exit
	while (shell->isRunning) {

		if (DEBUG)
			printf("waiting for user input\n");
		char* shellArg = getUserStringInput(": ", 0);
		if (DEBUG) printf("got user input\n");
		handleShellArgument(shellArg, shell);
		checkForZombies(shell);		//checks for any finished background proccesses
	}
	killAllProcesses(shell);
	
	freeShell(shell);
}


/*
** Description: Initializes all the shell variables
** Prerequisites: Shell memory is allocated
** Updated/Returned: All memory within shell is malloced and initialized.
*/
void initShell(struct Shell* shell) {
	assert(shell);
	shell->isRunning = true;
	shell->status = 0;
	shell->backgroundProcessesRunning = 0;
	shell->signalTerminated = 0;
	shell->lastExitedByStatus = true;
	shell->lastExitedBySignal = false;
	shell->lastExitedSignalPrintStatus = false;
	shell->lastExitedStatusPrintStatus = true;
	shell->printStatus = 0;

	//initializing shell pid and current working directory
	shell->pid = getpid();
	int pidLength = getLengthOfNumber(shell->pid);
	shell->pidString = malloc(sizeof(char) * (pidLength + 1));
	sprintf(shell->pidString, "%d", shell->pid);
	shell->cwd = getcwd(NULL, 0);

	//initializing shell home directory
	char* homeString = getenv("HOME");
	shell->homeDirectory = malloc(strlen(homeString) + 1);
	strcpy(shell->homeDirectory, homeString);


}

/*
** Description: Frees all memory associated with a shell pointer
** Prerequisites: Shell memory is allocated
** Updated/Returned: All memory within shell is freed, include the memory the pointer points to.
*/
void freeShell(struct Shell* shell) {
	assert(shell);
	if (shell->backgroundPIDs) {
		freeArray(shell->backgroundPIDs);
	}
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

/*
** Description: Handles the argument entered by the user
** Prerequisites: shellArg and shell are allocated
** Updated/Returned: Handles and parses argument sent to shell.
*/
void handleShellArgument(char* shellArg, struct Shell* shell) {
	if (DEBUG)
		printf("Handling shell argument \n");
	assert(shell);
	if (shellArg == NULL) {
		if (DEBUG)
			printf("no shell argument\n");
		return;
	}
	//if it's an empty string
	if (*shellArg == 0) {
		return;
	}

	struct Command* command = parseCommand(shellArg, shell);
	
	//no need to handle the command if it's a comment
	if (command->isComment) {
		freeCommand(command);
		return;
	}

	//tests if the command is one of the basic commands: cd, status, or exit
	int status = isBasicCommand(command, shell);

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
	return;
}

/*
** Description: Checks to see if any child processes have ended
** Prerequisites: shell is allocated
** Updated/Returned: Deals with a single ended child process, and handles the status signal.
*/
void checkForZombies(struct Shell* shell) {
	assert(shell);
	int childStatus = 0;

	//Potentially loops through every background process to see if it's ended
	for (int i = 0; i < shell->backgroundProcessesRunning; i++) {
		pid_t childID = waitpid(-1, &childStatus, WNOHANG);

		//if it's ended print the status
		if (childID > 0) {
			handleStatusSignal(childStatus, shell, true);
			printf("background pid %d is done: ", childID); fflush(stdout);
			printStatus(shell);
			removeFromArray(shell->backgroundPIDs, childID);
			(shell->backgroundProcessesRunning)--;
		}
		else {		//if it doesn't catch any, then no processes have ended so we can stop iterating
			break;
		}
	}
}



/*
** Description: Prints the shell status
** Prerequisites: shell is allocated
** Updated/Returned: None
*/
void printStatus(struct Shell* shell) {
	assert(shell);
	if (shell->lastExitedStatusPrintStatus) {
		printf("%s %d\n", EXIT_STATUS_STRING, shell->printStatus); fflush(stdout);
	}
	else if (shell->lastExitedSignalPrintStatus) {
		printf("%s %d\n", TERMINATED_STRING, shell->printStatus); fflush(stdout);
	}
	//resetting print statuses to the shell statuses. 
	shell->printStatus = shell->status;
	shell->lastExitedSignalPrintStatus = shell->lastExitedBySignal;
	shell->lastExitedStatusPrintStatus = shell->lastExitedByStatus;
}


/*
** Description: Kills all background processes running in a shell
** Prerequisites: shell is allocated
** Updated/Returned: All background processes started by shell are ended or dealt with.
*/
void killAllProcesses(struct Shell* shell) {
	assert(shell);
	assert(shell->backgroundPIDs);

	for (int i = 0; i < shell->backgroundPIDs->size; i++) {
		
		int childStatus;
		int PID = (shell->backgroundPIDs->arr)[i];
		pid_t childID = waitpid(PID, &childStatus, WNOHANG);
		if (childID > 0) {			//if the child ended by itself
			handleStatusSignal(childStatus, shell, true);
			printf("background pid %d is done: ", PID); fflush(stdout);
		}
		else {		//KILL THE CHILD
			kill(PID, SIGKILL);
			shell->status = 9;
			printf("background pid %d has been terminated: ", PID); fflush(stdout);
		}
		printStatus(shell);
	}
}
