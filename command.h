#ifndef COMMAND
#define COMMAND
#include <stdbool.h>
#include "shell.h"
struct Command {
	char* command;
	char** args;
	int amountOfArgs;
	char* input_file;
	char* output_file;
	bool background_execute;
	bool isComment;
};


struct Command* parseCommand(char*,struct Shell*);

char* expandDollarSigns(char*,struct Shell* shell);

int isBasicCommand(struct Command*, struct Shell*);

void printCommand(struct Command*);

int calculateNewSize(char* command, struct Shell* shell);

bool startOfDoubleDollar(char*);

void setCommand(struct Command*, char**);
void setArgs(struct Command*, char**);
void setInputFile(struct Command*, char**);
void setOutputFile(struct Command*, char**);
void setBackgroundExecute(struct Command*, char**);


void movePastWhiteSpace(char**);

void freeCommand(struct Command*);


char* readOneWord(char**);


bool checkForComment(char* command);

void initializeCommand(struct Command*);

void handleAdvancedCommand(struct Command*, struct Shell*);


char** createArgsForExec(struct Command*);
void freeNewArgs(struct Command*,char**);

void handleStatusSignal(int, struct Shell*);
#endif
