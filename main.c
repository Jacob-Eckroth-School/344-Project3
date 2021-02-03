/*
Name: Jacob Eckroth
Date: 1/25/2021
Project Name: Assignment 3: smallsh
Description: This program runs a shell. There are 3 written in commands. CD, Status, and Exit.
**			All other commands are handled by the exec family of functions. Supports redirecting input/output once.
**			Supports background execution of commands.
**			General Syntax: command [arg1 arg2 ...] [< input_file] [> output_file] [&]
**			Comments can be made on lines starting with '#'
*/

#include <stdio.h>
#include "usefulFunctions.h"
#include "shell.h"
#include "command.h"
#include <sys/types.h>
#include <unistd.h> 
int main() {

	shellInputLoop();
	return 0;
}