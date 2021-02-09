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
	char* expandedCommand = expandDollarSigns(command, shell);

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
void setInputFile(struct Command* command, char** currentLocationInString) {
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
	if (**currentLocationInString == 0) {
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





/*
** Description: Increments a char* until it is either at the end of a string, or not on a space or a tab
** Prerequisites: CurrentlocationInString is allocated and pointing at something
** Updated/Returned: char* currentLocationInString is pointing at the next non-whitespace character
*/
void movePastWhiteSpace(char** currentLocationInString) {
	while ((**currentLocationInString == ' ' || **currentLocationInString == '\t') && **currentLocationInString != 0) {	//iterates through whitespace to first character
		*currentLocationInString += 1;
	}
}



/*
** Description: Takes in a string and replaces every instance of "$$" within the string with the pid of the shell
** Prerequisites: command is allocated, and shell is allocated
** Updated/Returned: frees the memory pointed to by command, returns the new string with the '$$'s replaced with the pid
*/
char* expandDollarSigns(char* command, struct Shell* shell) {



	assert(command);

	//returns the required string size of the new string, including null terminator.
	int newSize = calculateNewSize(command, shell);

	char* newCommand = malloc(sizeof(char) * newSize);

	char* oldCommandIndexer = command;
	char* newCommandIndexer = newCommand;

	//iterates through the old string until we get to the null terminator.
	while (*oldCommandIndexer) {
		//tests if we're at the start of a double dollar
		if (startOfDoubleDollar(oldCommandIndexer)) {
			oldCommandIndexer += 2;
			for (int i = 0; i < strlen(shell->pidString); i++) {
				*newCommandIndexer = (shell->pidString)[i];
				newCommandIndexer += 1;
			}
		}
		else {	//if we're not at the start of $$ then just iterate through both strings normally.
			*newCommandIndexer = *oldCommandIndexer;
			newCommandIndexer += 1;
			oldCommandIndexer += 1;
		}
	}
	newCommand[newSize - 1] = 0;


	free(command);
	return newCommand;

}



/*
** Description: Returns true if the character being pointed to is a '$' and the next character is also a '$'
** Prerequisites: currentCommandIndexer is allocated.
** Updated/Returned: True if first 2 characters of string are '$$' false otherwise
*/
bool startOfDoubleDollar(char* currentCommandIndexer) {
	assert(currentCommandIndexer);
	//if length of string is <= 1 then there is no way it's the start of a $$
	if (strlen(currentCommandIndexer) <= 1) {
		return false;
	}

	//worst case, currentCommandIndexer[1] = 0 which is totally fine
	if (currentCommandIndexer[0] == '$' && currentCommandIndexer[1] == '$') {
		return true;
	}
	return false;
}


/*
** Description: Calculates the new required byte space for a string with the '$$'s in the original command expanded
**				to be the size of the pid
** Prerequisites: Command is allocated, shell is allocated
** Updated/Returned: Returns the size required for the $$ expanded string
*/
int calculateNewSize(char* command, struct Shell* shell) {
	assert(command);
	assert(shell);
	int initialSize = strlen(command) + 1;			//plus one for null character
	int amountOfDoubleDollarSigns = 0;
	bool firstDollarSignFound = false;

	for (int i = 0; i < strlen(command); i++) {
		if (firstDollarSignFound) {
			if (command[i] == '$') {			//if the previous was a dollar and the current is as well
				amountOfDoubleDollarSigns++;

			}
			firstDollarSignFound = false;		//If the first dollar sign was found, and the next character isn't a dollar sign, then we reset our search
		}
		else if (command[i] == '$') {
			firstDollarSignFound = true;
		}

	}

	int finalSize = initialSize + (strlen(shell->pidString) - 2) * amountOfDoubleDollarSigns;
	return finalSize;
}


/*
** Description: Prints the contents of a command for debugging purposes
** Prerequisites: userCommand is allocated
** Updated/Returned: Prints out information about the contents of userCommand
*/
void printCommand(struct Command* userCommand) {
	assert(userCommand);
	if (userCommand->command) {
		printf("Command: %s\n", userCommand->command); fflush(stdout);
	}
	if (userCommand->args) {
		printf("Arguments: "); fflush(stdout);
	}

	for (int i = 0; i < userCommand->amountOfArgs; i++) {
		printf("%s ", (userCommand->args)[i]);	 fflush(stdout);
	}
	if (userCommand->args) {
		printf("\n"); fflush(stdout);
	}
	if (userCommand->input_file) {
		printf("Input File: %s\n", userCommand->input_file); fflush(stdout);
	}
	if (userCommand->output_file) {
		printf("Output File: %s\n", userCommand->output_file); fflush(stdout);
	}
	if (userCommand->background_execute) {
		printf("Executing in background\n"); fflush(stdout);
	}
	else {
		printf("Executing in foreground\n"); fflush(stdout);
	}


}

/*
** Description: Returns 1 if the command is not one of our basic commands handled by the shell. Returns -1 if the user wants to end, returns 0 otherwise
** Prerequisites: userCommand is allocated
** Updated/Returned: Prints out information about the contents of userCommand
*/
int isBasicCommand(struct Command* command, struct Shell* shell) {
	assert(command);
	if (!command->command) {
		return 0;
	}

	if (strcmp(command->command, EXITCOMMAND) == 0) {
		shell->isRunning = false;
		return -1;				//-1 means end
	}
	//change directory commmand
	else if (strcmp(command->command, CDCOMMAND) == 0) {
		changeDirectory(command, shell);
		return 0;
	}
	// status command.
	else if (strcmp(command->command, STATUSCOMMAND) == 0) {
		printStatus(shell);
		return 0;
	}

	return 1;


}

/*
** Description: Changes directory based on arguments in command struct.
** Prerequisites: Command is allocated, shell is allocated, homeDirectory is defined.
** Updated/Returned: Directory is changed appropriately.
*/
void changeDirectory(struct Command* command, struct Shell* shell) {
	assert(command);
	assert(shell);
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
}



/*
** Description: Handles any nonbasic foreground commands by passing them to exec() functions.
** Prerequisites: Command is allocated, shell is allocated
** Updated/Returned: User-specified command is run.
*/
void handleAdvancedCommand(struct Command* command, struct Shell* shell) {
	assert(command);
	assert(shell);
	//sets up the new args array with the program name at the beginning and NULL at the end
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
		//Sets up child signal handlers.
		files = handleFiles(command);
		initializeChildSignalHandler();
		initializeChildForegroundSignalHandler();
		//deals with redirecting files if there are files to be redirected
		//if this returns less than 0 then there was an error
		if (files < 0) {
			exit(EXIT_FAILURE);
		}
		foreground_executing = true;
		execvp(newArgs[0], newArgs);

		//if we messed up, print why we messed up
		perror(command->command);
		_exit(EXIT_FAILURE);
		break;
	default:
		//this is the parent

		foreground_executing = true;
		spawnPid = waitpid(spawnPid, &childStatus, 0);
		foreground_executing = false;
		if (DEBUG)
			printf("process is finished, status is: %d", childStatus);

		//assigns status signal to shell based on how the process exited.
		handleStatusSignal(childStatus, shell, false);
	}
	//frees the allocated arguments because reasons.
	freeNewArgs(command, newArgs);
}

/*
** Description: Handles any nonbasic background commands by passing them to exec() functions.
** Prerequisites: Command is allocated, shell is allocated
** Updated/Returned: User-specified command is run in the background.
*/
void handleAdvancedCommandBackground(struct Command* command, struct Shell* shell) {
	assert(command);
	assert(shell);
	//sets up the new args array with the program name at the beginning and NULL at the end
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
		//assigns and deals with input/output redirection
		files = handleFiles(command);
		initializeChildSignalHandler();

		if (files < 0) {
			exit(EXIT_FAILURE);
		}
		execvp(newArgs[0], newArgs);
		perror(command->command);
		_exit(EXIT_FAILURE);
		break;
	}
	printf("background pid is %d\n", spawnPid); fflush(stdout);
	addToArray(shell->backgroundPIDs, spawnPid);
	freeNewArgs(command, newArgs);
}

/*
** Description: Uses dup2 to reassign stdin and stdout based on user input in command
** Prerequisites: Command is allocated
** Updated/Returned: Ifuser has specified files, input/output is redirected. If command running in background, input/output set to default /dev/null
*/
int handleFiles(struct Command* command) {

	if (!(command->input_file) && !(command->output_file)) {
		if (command->background_execute) {
			setDefaultInput();
			setDefaultOutput();
		}
		return 1;
	}

	if (command->input_file) {
		int inputFile = open(command->input_file, O_RDONLY);
		if (inputFile < 0) {
			printf("cannot open %s for input\n", command->input_file); fflush(stdout);
			return inputFile;
		}
		dup2(inputFile, 0);
	}
	else if (command->background_execute) {
		//setting default input
		if (DEBUG) printf("assigning default input\n");
		setDefaultInput();
	}
	if (command->output_file) {

		int output_file = open(command->output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (output_file < 0) {
			printf("cannot open %s for output\n", command->output_file); fflush(stdout);
			return output_file;
		}
		dup2(output_file, 1);

	}
	else if (command->background_execute) {
		if (DEBUG) printf("assigning default output\n");
		//setting default output for background processes
		setDefaultOutput();
	}
	return 0;
}

/*
** Description: Sets default input to null
** Prerequisites: None
** Updated/Returned: stdin is reassigned to null
*/
void setDefaultInput() {
	int input = open("/dev/null", O_RDONLY);
	dup2(input, 0);
}

/*
** Description: Sets default output to null
** Prerequisites: None
** Updated/Returned: stdout is reassigned to null
*/
void setDefaultOutput() {
	int output = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	dup2(output, 1);
}

/*
** Description: Updates signal status held in shell based on how the child process exited.
** Prerequisites: shell is allocated
** Updated/Returned: status in shell is updated to hold correct value. Prints status if child process exited from background by signal.
*/
void handleStatusSignal(int status, struct Shell* shell, bool background) {
	assert(shell);

	//if it's in the background we'll only print it, not update the actual status.
	if (background) {
		if (WIFEXITED(status) != 0) {
			shell->printStatus = WEXITSTATUS(status);
			shell->lastExitedStatusPrintStatus = true;
			shell->lastExitedSignalPrintStatus = false;
		}
		else {

			shell->printStatus = WTERMSIG(status);
			shell->lastExitedSignalPrintStatus = true;
			shell->lastExitedStatusPrintStatus = false;
		}
	}else{
		//we want to update the actual status, not just print it if its in the foreground
		if (WIFEXITED(status) != 0) {
			shell->status = shell->printStatus = WEXITSTATUS(status);
			shell->lastExitedByStatus = shell->lastExitedStatusPrintStatus = true;
			shell->lastExitedBySignal = shell->lastExitedSignalPrintStatus = false;

		}
		else {

			shell->status = shell->printStatus = WTERMSIG(status);
			shell->lastExitedBySignal = shell->lastExitedSignalPrintStatus = true;
			shell->lastExitedByStatus = shell->lastExitedStatusPrintStatus = false;
			if (!background) {
				printStatus(shell);
			}


		}
	}
}


/*
** Description: Creates the new list of args for the exec function, with the function name as the first arg, and null as the last one.
** Prerequisites: command is allocated
** Updated/Returned: Returns a string array of args that could be passed to an exec() function.
*/
char** createArgsForExec(struct Command* command) {
	assert(command);
	char** newArgs = malloc((command->amountOfArgs + 2) * sizeof(char*));
	int newArgsIndex = 0;

	//sets the first item to be the command.
	newArgs[newArgsIndex] = malloc(strlen(command->command) + 1);
	strcpy(newArgs[newArgsIndex], command->command);
	newArgsIndex++;

	//assigns all the other commands.
	for (int i = 0; i < command->amountOfArgs; i++) {
		newArgs[newArgsIndex] = malloc(strlen((command->args)[i]) + 1);
		strcpy(newArgs[newArgsIndex], (command->args)[i]);
		newArgsIndex++;
	}
	newArgs[newArgsIndex] = NULL;
	return newArgs;
}

/*
** Description: Frees an string array
** Prerequisites: command is allocated, newArgs is allocated.
** Updated/Returned: frees memory held by newArgs
*/
void freeNewArgs(struct Command* command, char** newArgs) {
	assert(command);
	for (int i = 0; i < command->amountOfArgs + 2; i++) {
		if (newArgs[i]) {		//only free things that exist, i.e. not NULL
			free(newArgs[i]);
		}
	}
	free(newArgs);
}
