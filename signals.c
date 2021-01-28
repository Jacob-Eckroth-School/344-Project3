#include "signals.h"
#include <signal.h>
#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h> 
#include <stdlib.h>
#include <fcntl.h>
#include "globals.h"
#include "typeDefs.h"
void toggleBackground(int sigNumber) {

	background_enabled = !background_enabled;
	if (background_enabled) {
		write(STDOUT_FILENO, "\nExiting foregroud-only mode\n:", 31);
	}
	else {
		write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n:", 52);

	}

}


void initializeParentSignalHandler() {
	struct sigaction SIGINT_action = { { 0 } };
	SIGINT_action.sa_handler = SIG_IGN;
	// Block all catchable signals while handle_SIGINT is running
	sigaddset(&SIGINT_action.sa_mask,SIGINT);
	// No flags set
	SIGINT_action.sa_flags = SA_RESTART;

	sigaction(SIGINT, &SIGINT_action, NULL);
	
	

	struct sigaction SIGTSTP_action = { { 0 } };
	SIGTSTP_action.sa_handler = toggleBackground;
	// Block all catchable signals while handle_SIGINT is running
	sigaddset(&SIGTSTP_action.sa_mask,SIGTSTP);
	// No flags set
	SIGTSTP_action.sa_flags = SA_RESTART;

	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
	
}

void initializeChildForegroundSignalHandler() {

	struct sigaction SIGINT_action = { { 0 } };
	SIGINT_action.sa_handler = SIG_DFL;
	// Block all catchable signals while handle_SIGINT is running
	sigaddset(&SIGINT_action.sa_mask,SIGINT);
	// No flags set
	SIGINT_action.sa_flags = 0;

	sigaction(SIGINT, &SIGINT_action, NULL);
}

void initializeChildSignalHandler() {
	struct sigaction SIGTSTP_action = { { 0 } };
	SIGTSTP_action.sa_handler = SIG_IGN;
	// Block all catchable signals while handle_SIGINT is running
	sigaddset(&SIGTSTP_action.sa_mask,SIGTSTP);
	// No flags set
	SIGTSTP_action.sa_flags = 0;

	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}


void exitChild(int failureStatus) {
	fprintf(stdout,"failure status is: %d\n", failureStatus);
	_exit(failureStatus);
}
