#include "signals.h"
#include <signal.h>
#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h> 
#include <stdlib.h>
#include <fcntl.h>
#include "globals.h"
#include "typeDefs.h"



/*
** Description: Toggles whether commands running in the background is enabled or not. Takes in an int because it's a signal handler.
** Prerequisites: None
** Updated/Returned: Background_enabled global bool is flipped, prints out what the new state is.
*/
void toggleBackground(int sigNumber) {

	background_enabled = !background_enabled;
	if (background_enabled) {
		write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 30); fflush(stdout);
	}
	else {
		write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 51); fflush(stdout);
	}

	//we need to reprompt if it's not a foreground process.
	if (!foreground_executing) {
		write(STDOUT_FILENO, ":", 2); fflush(stdout);
	}

}

/*
** Description: Initializes the signal handlers for the parent shell.
** Prerequisites: None
** Updated/Returned: ^C is ignored, ^Z toggles the background.
*/
void initializeParentSignalHandler() {
	struct sigaction SIGINT_action = { { 0 } };
	SIGINT_action.sa_handler = SIG_IGN;
	// Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGINT_action.sa_mask);
	// No flags set
	SIGINT_action.sa_flags = SA_RESTART;

	sigaction(SIGINT, &SIGINT_action, NULL);



	struct sigaction SIGTSTP_action = { { 0 } };
	SIGTSTP_action.sa_handler = toggleBackground;
	// Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGTSTP_action.sa_mask);
	// No flags set
	SIGTSTP_action.sa_flags = SA_RESTART;

	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

}
/*
** Description: Initializes the signal handler for a foreground child process
** Prerequisites: None
** Updated/Returned: ^C executes default behavior.
*/
void initializeChildForegroundSignalHandler() {

	struct sigaction SIGINT_action = { { 0 } };
	SIGINT_action.sa_handler = SIG_DFL;
	// Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGINT_action.sa_mask);
	// No flags set
	SIGINT_action.sa_flags = SA_RESTART;

	sigaction(SIGINT, &SIGINT_action, NULL);
}

/*
** Description: Initializes the signal handler for any child process
** Prerequisites: None
** Updated/Returned: ^Z is ignored
*/
void initializeChildSignalHandler() {
	struct sigaction SIGTSTP_action = { { 0 } };
	SIGTSTP_action.sa_handler = SIG_IGN;
	// Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGTSTP_action.sa_mask);
	// No flags set
	SIGTSTP_action.sa_flags = SA_RESTART;

	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}


