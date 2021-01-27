#include "usefulFunctions.h"
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "typeDefs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
 
/*
** Description: checks to see if a string has a certain suffix
** Prerequisites: suffix and the string to check against are allocated
** Updated/Returned: True if the string has the suffix, false if it doesn't 
*/
bool checkSuffix(char* suffix, char* stringToCheckAgainst) {
	assert(suffix); 
	assert(stringToCheckAgainst);

	//if the string is longer than the actual string yaknow.
	if (strlen(stringToCheckAgainst) < strlen(suffix)) {
		return false;
	}

	int suffixLength = strlen(suffix);
	int stringLength = strlen(stringToCheckAgainst);
	//looping through the strings backwards
	for (int i = 0; i < strlen(suffix); i++) {
		//-1 because 0 based indexing -i to loop through the last characters of each string
		if (stringToCheckAgainst[stringLength - 1 - i] != suffix[suffixLength - 1 - i]) {
			return false;
		}
	}
	return true;
}


/*
** Description: Gets the length of a number, i.e 1000 returns 4, 0 returns 1, 20 returns 2
** Prerequisites: None
** Updated/Returned: Returns the length of a number as if it were a string
*/
int getLengthOfNumber(int number) {
	return (number == 0) ? 1 : floor(log10(abs(number))) + 1;
}

/*
** Description: Gets string input from a user, with a size of bufsize
** Prerequisites: Prompt is allocated
** Updated/Returned: Returns a pointer to the user entered string
*/
char* getUserStringInput(char* prompt, int bufSize) {
	assert(prompt);
	size_t bufsize = bufSize;
	char* input = NULL;

	//if it's not zero, then we need to allocate it ourself. Otherwise getline will malloc it for us
    if(bufSize!=0){
		input = (char*)malloc(bufsize * (sizeof(char)));
	}

	printf("%s",prompt);
	getline(&input, &bufsize, stdin);
	if(DEBUG){
		printf("Full Dec of inputted string: ");
		for(int i = 0; i < strlen(input); i++){
			printf("%d ",input[i]);
		}
		printf("\n");
	}
	//if there's a newline character left over in the user input
	if (input[strlen(input) - 1] == 10) {
		input[strlen(input) - 1] = 0; 
	}

	return input;
}


/*
** Description: Tests if a file is a file or a directory
** Prerequisites: fileName is allocated
** Updated/Returned: Returns non-zero if file is regular file, zero otherwise
*/
int is_regular_file(const char* fileName) {
	struct stat path_stat;
	stat(fileName, &path_stat);
	return S_ISREG(path_stat.st_mode);
}
