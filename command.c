#include "command.h"
#include "usefulFunctions.h"
#include <assert.h>
#include <sys/types.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include "typeDefs.h"
#include <string.h>
#include <sys/wait.h> // for waitpid
#include "shell.h"
struct Command* parseCommand(char* command, struct Shell* shell) {
	assert(command);
	char* expandedCommand = expandDollarSigns(command,shell);
	char* traveler = expandedCommand;
	struct Command* newCommand = malloc(sizeof(struct Command));
	initializeCommand(newCommand);
	assert(newCommand);
	newCommand->isComment = false;
	if (checkForComment(expandedCommand)) {
		newCommand->isComment = true;
	}
	else {
		setCommand(newCommand, &traveler);
		setArgs(newCommand, &traveler);
		setInputFile(newCommand, &traveler);
		setOutputFile(newCommand, &traveler);
		setBackgroundExecute(newCommand, &traveler);
	}



	free(expandedCommand);

	return newCommand;
}


void initializeCommand(struct Command* command) {
	command->command = NULL;
	command->args = NULL;
	command->amountOfArgs = 0;
	command->input_file = NULL;
	command->output_file = NULL;
	command->background_execute = false;
	command->isComment = false;
}

bool checkForComment(char* newCommand) {
	if (strlen(newCommand) < strlen(COMMENTINDICATOR)) {
		return false;
	}
	if (strncmp(newCommand, COMMENTINDICATOR, strlen(COMMENTINDICATOR)) == 0) {
		return true;
	}
	else {
		return false;
	}

}

void freeCommand(struct Command* command) {
	if (command->command) {
		free(command->command);
	}
	for (int i = 0; i < command->amountOfArgs; i++) {
		free((command->args)[i]);
	}
	if (command->args) {
		free(command->args);
	}
	if (command->input_file) {
		free(command->input_file);
	}
	if (command->output_file) {
		free(command->output_file);
	}


	free(command);
}



void setCommand(struct Command* command, char** currentLocationInString) {
	movePastWhiteSpace(currentLocationInString);
	
	if (**currentLocationInString == 0) {
		command = NULL;
		return;
	}
	char* commandString = readOneWord(currentLocationInString);

	command->command = malloc(strlen(commandString) + 1);
	strcpy(command->command, commandString);
	
	
}


void setArgs(struct Command* command, char** currentLocationInString) {
	movePastWhiteSpace(currentLocationInString);
	command->args = NULL;
	command->amountOfArgs = 0;
	if (**currentLocationInString == 0) {
		return;
	}

	char* argString = NULL;
	//loop until we hit one of the next commands.
	while (**currentLocationInString != IN_FILE_INDICATOR && **currentLocationInString != OUT_FILE_INDICATOR && **currentLocationInString != BACKGROUND_INDICATOR && **currentLocationInString != 0) {
		
		argString = readOneWord(currentLocationInString);
		(command->amountOfArgs)++;
		command->args = realloc(command->args, (sizeof(char*) * command->amountOfArgs));
		(command->args)[command->amountOfArgs - 1] = malloc((strlen(argString) + 1) * sizeof(char));
		strcpy((command->args)[command->amountOfArgs - 1], argString);		//invalid write here? idk why
		movePastWhiteSpace(currentLocationInString);
	}

}

void setInputFile(struct Command* command, char** currentLocationInString){
	movePastWhiteSpace(currentLocationInString);
	if (**currentLocationInString == 0 || **currentLocationInString != IN_FILE_INDICATOR) {
		command->input_file = NULL;
		return;
	}
	(*currentLocationInString += 1);
	movePastWhiteSpace(currentLocationInString);
	if (**currentLocationInString == 0){
		command->input_file = NULL;
		return;
	}
	else {
		char* inputFileString = readOneWord(currentLocationInString);
		command->input_file = malloc(strlen(inputFileString) + 1);
		strcpy(command->input_file, inputFileString);
	}	
}

char* readOneWord(char** currentLocationInString) {

	int lengthLeft = strlen(*currentLocationInString);
	char* saveptr = NULL;
	char* commandString = strtok_r(*currentLocationInString, " ", &saveptr);
	if (strlen(commandString) == lengthLeft) {
		(*currentLocationInString) += strlen(commandString);	//to get to null
	}
	else {
		(*currentLocationInString) += strlen(commandString) + 1;	//to get to next starting point
	}
	return commandString;
}


void setOutputFile(struct Command* command, char** currentLocationInString) {
	movePastWhiteSpace(currentLocationInString);
	if (**currentLocationInString == 0 || **currentLocationInString != OUT_FILE_INDICATOR) {
		command->output_file = NULL;
		return;
	}
	(*currentLocationInString += 1);
	movePastWhiteSpace(currentLocationInString);
	if (**currentLocationInString == 0) {
		command->output_file = NULL;
		return;
	}
	else {
		char* outputFileString = readOneWord(currentLocationInString);
		command->output_file = malloc(strlen(outputFileString) + 1);
		strcpy(command->output_file, outputFileString);
	}
}
void setBackgroundExecute(struct Command* command, char** currentLocationInString) {
	movePastWhiteSpace(currentLocationInString);
	if (**currentLocationInString == 0) {
		command->background_execute = false;
		return;
	}
	if (**currentLocationInString == BACKGROUND_INDICATOR) {
		(*currentLocationInString) += 1;
		if (**currentLocationInString == ' ' || **currentLocationInString == 0) {
			command->background_execute = true;
			
		}
	}
	else {
		command->background_execute = false;
	}
	return;
}




void movePastWhiteSpace(char** currentLocationInString) {
	while (**currentLocationInString == ' ' && **currentLocationInString != 0) {	//iterates through whitespace to first character
		*currentLocationInString += 1;
	}
}




char* expandDollarSigns(char* command, struct Shell* shell) {



	
	assert(command);
	int newSize = calculateNewSize(command,shell);

	char* newCommand = malloc(sizeof(char) * newSize);

	char* oldCommandIndexer = command;
	char* newCommandIndexer = newCommand;

	while (*oldCommandIndexer) {
		if (startOfDoubleDollar(oldCommandIndexer)) {
			oldCommandIndexer += 2;
			for (int i = 0; i < strlen(shell->pidString); i++) {
				*newCommandIndexer = (shell->pidString)[i];
				newCommandIndexer += 1;
			}
		}
		else {
			*newCommandIndexer = *oldCommandIndexer;
			newCommandIndexer += 1;
			oldCommandIndexer += 1;
		}
	}
	newCommand[newSize-1] = 0;
	

	free(command);
	return newCommand;

}




bool startOfDoubleDollar(char* currentCommandIndexer) {
	if (strlen(currentCommandIndexer) <= 1) {	//guaranteeing we're not at the end of the string, so the worst we'll access is a null terminator
		return false;
	}
	if (currentCommandIndexer[0] == '$' && currentCommandIndexer[1] == '$') {
		return true;
	}
	return false;




}



int calculateNewSize(char* command, struct Shell* shell) {
	int initialSize = strlen(command) + 1;			//plus one for null character
	int amountOfDoubleDollarSigns = 0;
	bool firstDollarSignFound = false;

	for (int i = 0; i < strlen(command); i++) {
		if (firstDollarSignFound) {
			if (command[i] == '$') {			//if the previous was a dollar and the current is as well
				amountOfDoubleDollarSigns++;
				
			}
			firstDollarSignFound = false;
		}else if (command[i] == '$') {
			firstDollarSignFound = true;
		}

	}

	int finalSize = initialSize + (strlen(shell->pidString) - 2) * amountOfDoubleDollarSigns;	
	return finalSize;
}

void printCommand(struct Command* userCommand) {
	assert(userCommand);
	if (userCommand->command) {
		printf("Command: %s\n", userCommand->command);
	}
	if(userCommand->args){
		printf("Arguments: ");
	}
	
	for (int i = 0; i < userCommand->amountOfArgs; i++) {
		printf("%s ", (userCommand->args)[i]);	
	}
	if (userCommand->args) {
		printf("\n");
	}
	if (userCommand->input_file) {
		printf("Input File: %s\n", userCommand->input_file);
	}
	if (userCommand->output_file) {
		printf("Output File: %s\n", userCommand->output_file);
	}
	if (userCommand->background_execute) {
		printf("Executing in background\n");
	}
	else {
		printf("Executing in foreground\n");
	}
	

}

int isBasicCommand(struct Command* command, struct Shell* shell) {
	assert(command);
	if (strcmp(command->command, EXITCOMMAND) == 0) {
		shell->isRunning = false;
		return -1;				//-1 means end
	}
	else if (strcmp(command->command, CDCOMMAND) == 0) {
		if (command->amountOfArgs == 0) {
			chdir(shell->homeDirectory);
		}
		else {
			chdir((command->args)[0]);
		}
		if (shell->cwd) {
			free(shell->cwd);
		}
		shell->cwd = getcwd(NULL, 0);
		return 0;
	}
	else if (strcmp(command->command, STATUSCOMMAND) == 0) {
		printStatus(shell);
		return 0;
	}
	return 1;


}





void handleAdvancedCommand(struct Command* command, struct Shell* shell) {
	char** newArgs = createArgsForExec(command);
	

	pid_t spawnPid = 0;
	int childStatus = 0;
	spawnPid = fork();
	switch (spawnPid) {
	case -1:
		perror("fork() failed!\n");
		exit(1);
		break;
	case 0:
		//this is the child

		execvp(command->command, newArgs);
		perror(command->command);
		exit(EXIT_FAILURE);
		break;
	default:
		//this is the parent

		spawnPid = waitpid(spawnPid, &childStatus, 0);
		handleStatusSignal(childStatus, shell);
	}
	
	freeNewArgs(command, newArgs);


}

void handleStatusSignal(int status, struct Shell* shell) {
	if (WIFEXITED(status) != 0) {
		shell->status = WEXITSTATUS(status);
		shell->lastExitedByStatus = true;
		shell->lastExitedBySignal = false;
	}
	else {
		shell->status = WTERMSIG(status);
		shell->lastExitedBySignal = true;
		shell->lastExitedByStatus = false;
	}
}





char** createArgsForExec(struct Command* command) {
	char** newArgs = malloc((command->amountOfArgs + 2) * sizeof(char*));
	int newArgsIndex = 0;
	newArgs[newArgsIndex] = malloc(strlen(command->command) + 1);
	strcpy(newArgs[newArgsIndex], command->command);
	newArgsIndex++;
	for (int i = 0; i < command->amountOfArgs; i++) {
		newArgs[newArgsIndex] = malloc(strlen((command->args)[i]) + 1);
		strcpy(newArgs[newArgsIndex], (command->args)[i]);
		newArgsIndex++;
	}
	newArgs[newArgsIndex] = NULL;
	return newArgs;
}

void freeNewArgs(struct Command* command, char** newArgs) {
	for (int i = 0; i < command->amountOfArgs + 2; i++) {
		if (newArgs[i]) {
			free(newArgs[i]);
		}
	}
	free(newArgs);
}