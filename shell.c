#include "shell.h"
#include "usefulFunctions.h"
#include "typeDefs.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
void shellInputLoop() {
	bool running = true;
	int status = 0;
	while (running) {
		char* shellArg = getUserStringInput(": ", 0);
		status = handleShellArgument(shellArg);
		free(shellArg);
		if (status == -1) {
			running = false;
		}
	}

}



int handleShellArgument(char* shellArg) {
	//exit case
	if (strlen(shellArg) >= strlen(EXITCOMMAND)) {
		if (strncmp(shellArg, EXITCOMMAND, strlen(EXITCOMMAND)) == 0) {					//test to see if it's a valid commmand
			if (shellArg[strlen(EXITCOMMAND)] == ' ' || shellArg[strlen(EXITCOMMAND)] == 0) {
				return -1;
			}
		}
	}

	return 0;



}