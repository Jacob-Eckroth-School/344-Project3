#include "command.h"
#include "usefulFunctions.h"
#include <assert.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include "typeDefs.h"
#include "globals.h"
#include <string.h>
#include <sys/wait.h> // for waitpid
#include "shell.h"
#include "signals.h"
#include <sys/stat.h>
#include <fcntl.h>

/*
** Description: Prases a string and returns a command struct with all the data from the string
** Prerequisites: command and shell are allocated
** Updated/Returned: Returns an allocated command struct with all the data from the command in it
*/
struct Command* parseCommand(char* command, struct Shell* shell) {
	assert(command);
	char* expandedCommand = expandDollarSigns(command,shell);
	char* traveler = expandedCommand;
	struct Command* newCommand = malloc(sizeof(struct Command));
	initializeCommand(newCommand);
	assert(newCommand);
	//fills in all data for command, based on a char* traveler that travels through the expandedCommand string
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

/*
** Description: Initializes all the values for a new Command struct
** Prerequisites: command is allocated
** Updated/Returned: Initial state is set for the command.
*/
void initializeCommand(struct Command* command) {
	assert(command);
	command->command = NULL;
	command->args = NULL;
	command->amountOfArgs = 0;
	command->input_file = NULL;
	command->output_file = NULL;
	command->background_execute = false;
	command->isComment = false;
}

/*
** Description: Checks if the string passed to it begins with the COMMENTINDICATOR macro
** Prerequisites: newCommand is allocated
** Updated/Returned: True if the string begins as a comment, false otherwise.
*/
bool checkForComment(char* newCommand) {
	assert(newCommand);
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


/*
** Description: Frees all data associated with a command struct
** Prerequisites: command is allocateed
** Updated/Returned: All data with associated with command is freed
*/
void freeCommand(struct Command* command) {
	assert(command);
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


/*
** Description: Initializes the "command" part of the command struct
** Prerequisites: command is allocated
** Updated/Returned: currentLocationInString moved to one after the initial command, command is set within command
*/
void setCommand(struct Command* command, char** currentLocationInString) {
	assert(command);
	movePastWhiteSpace(currentLocationInString);
	
	//if there is no command i.e. we're at the end of the string
	if (**currentLocationInString == 0) {
		command = NULL;
		return;
	}
	
	//reads the command
	char* commandString = readOneWord(currentLocationInString);

	command->command = malloc(strlen(commandString) + 1);
	strcpy(command->command, commandString);
	
}


/*
** Description: Initializes the args part of the command
** Prerequisites: command is allocated
** Updated/Returned: currentLocationInString moved to one after the args, if there's args they are allocated within command
*/
void setArgs(struct Command* command, char** currentLocationInString) {
	assert(command);
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

		//increases the size of args to hold the new argument
		command->args = realloc(command->args, (sizeof(char*) * command->amountOfArgs));
		(command->args)[command->amountOfArgs - 1] = malloc((strlen(argString) + 1) * sizeof(char));
		strcpy((command->args)[command->amountOfArgs - 1], argString);		

		//moves to the next arg
		movePastWhiteSpace(currentLocationInString);
	}

}



/*
** Description: Initializes inputFile part of the command
** Prerequisites: command is allocated
** Updated/Returned: currentLocationInString moved to one after the input file, sets input_file in command struct
*/
void setInputFile(struct Command* command, char** currentLocationInString){
	assert(command);
	movePastWhiteSpace(currentLocationInString);

	//if the first symbol isn't the file indicator
	if (**currentLocationInString == 0 || **currentLocationInString != IN_FILE_INDICATOR) {
		command->input_file = NULL;
		return;
	}
	(*currentLocationInString += 1);
	movePastWhiteSpace(currentLocationInString);
	//if there's no file name
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

/*
** Description: Initializes outputFile part of the command
** Prerequisites: command is allocated
** Updated/Returned: currentLocationInString moved to one after the output file, sets output_file in command struct
*/
void setOutputFile(struct Command* command, char** currentLocationInString) {
	assert(command);
	movePastWhiteSpace(currentLocationInString);

	//if the next symbol isn't the out file indicator
	if (**currentLocationInString == 0 || **currentLocationInString != OUT_FILE_INDICATOR) {
		command->output_file = NULL;
		return;
	}
	(*currentLocationInString += 1);
	movePastWhiteSpace(currentLocationInString);

	//if there's no out file name
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

/*
** Description: Sets whether the command should execute in the background or not
** Prerequisites: command is allocated
** Updated/Returned: currentLocationInString moved to one after the output file, updates background execute depending on whether the last character is a '&' or not
*/
void setBackgroundExecute(struct Command* command, char** currentLocationInString) {
	if (!background_enabled) {
		command->background_execute = false;
		return;
	}
	movePastWhiteSpace(currentLocationInString);
	

	if (**currentLocationInString == 0) {
		command->background_execute = false;
		return;
	}
	//if we're at the &
	if (**currentLocationInString == BACKGROUND_INDICATOR) {
		(*currentLocationInString) += 1;

		//if there's nothing after the &
		if (**currentLocationInString == ' ' || **currentLocationInString == 0 || **currentLocationInString == '\t') {
			command->background_execute = true;

		}
	}
	else {
		command->background_execute = false;
	}
	return;
}



/*
** Description: Returns a char* to a string, either up to null or to a space. 
** Prerequisites: currentLocation is pointing to something, hopefully a string
** Updated/Returned: currentLocationString is updated along the string, and returns a pointer to the first word
*/
char* readOneWord(char** currentLocationInString) {

	int lengthLeft = strlen(*currentLocationInString);
	char* saveptr = NULL;
	char* commandString = strtok_r(*currentLocationInString, " \t", &saveptr);
	if (strlen(commandString) == lengthLeft) {		//if we end on the null terminator
		(*currentLocationInString) += strlen(commandString);	//to get to null
	}
	else {		//if we end on a space
		(*currentLocationInString) += strlen(commandString) + 1;	//to get to next starting point
	}
	return commandString;
}






void movePastWhiteSpace(char** currentLocationInString) {
	while ((**currentLocationInString == ' ' || **currentLocationInString == '\t') && **currentLocationInString != 0) {	//iterates through whitespace to first character
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
	if (!command->command) {
		return 0;
	}

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
	int files;
	switch (spawnPid) {
	case -1:
		perror("fork() failed!\n");
		exit(1);
		break;
	case 0:
		//this is the child
		initializeChildSignalHandler();
		initializeChildForegroundSignalHandler();
		files = handleFiles(command);

		if (files < 0) {
			exit(EXIT_FAILURE);
		}

	
		execvp(command->command, newArgs);
		perror(command->command);
		_exit(EXIT_FAILURE);
		break;
	default:
		//this is the parent
		
		spawnPid = waitpid(spawnPid, &childStatus, 0);
		
		if (DEBUG)
			printf("process is finished, status is: %d", childStatus);
		handleStatusSignal(childStatus, shell,false);
	}
	
	freeNewArgs(command, newArgs);
}


void handleAdvancedCommandBackground(struct Command* command, struct Shell* shell) {
	char** newArgs = createArgsForExec(command);
	pid_t spawnPid = 0;
	
	spawnPid = fork();
	int files;
	(shell->backgroundProcessesRunning)++;
	switch (spawnPid) {
	case -1:
		perror("fork() failed!\n");
		exit(1);
		break;
	case 0:
		//this is the child
		setDefaultStreams();
		initializeChildSignalHandler();
		files = handleFiles(command);
		if (files < 0) {
			exit(EXIT_FAILURE);
		}
		execvp(command->command, newArgs);
		perror(command->command);
		_exit(EXIT_FAILURE);
		break;
	}
	printf("background id is %d\n", spawnPid);

	freeNewArgs(command, newArgs);
}

void setDefaultStreams() {
	int input = open("/dev/null", O_RDONLY);
	int output = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT,FILE_PERMISSIONS);
	dup2(input, 0);
	dup2(output, 1);
}

int handleFiles(struct Command* command) {
	if (!(command->input_file) && !(command->output_file)) {
		return 1;
	}
	if (command->input_file) {
		int inputFile = open(command->input_file, O_RDONLY,FILE_PERMISSIONS);
		if (inputFile == -1) {
			printf("cannot open %s for input\n", command->input_file);
			return -1;
		}
		dup2( inputFile,0);
	}
	if (command->output_file) {
		if (DEBUG)
			printf("assigning output file\n");
		int output_file = open(command->output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (output_file == -1) {
			printf("cannot open %s for output\n", command->output_file);
			return -1;
		}
		dup2(output_file,1);

	}
	return 0;


}

void handleStatusSignal(int status, struct Shell* shell,bool background) {
	if (WIFEXITED(status) != 0) {
		if (DEBUG) {
			printf("exited normally\n");
		}
		shell->status = WEXITSTATUS(status);
		shell->lastExitedByStatus = true;
		shell->lastExitedBySignal = false;
		
	}
	else {
		if (DEBUG)
			printf("exited with signal\n");
		shell->status = WTERMSIG(status);
		shell->lastExitedBySignal = true;
		shell->lastExitedByStatus = false;
		if (DEBUG)
			printf("Status: %d\n", shell->status);
		if (!background) {
			printStatus(shell);
		}
		
		
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