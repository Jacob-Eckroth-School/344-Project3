#ifndef SIGNALS
#define SIGNALS


void initializeParentSignalHandler();

void initializeChildForegroundSignalHandler();


void exitChild(int failureStatus);
void initializeChildSignalHandler();
void toggleBackground(int);
#endif