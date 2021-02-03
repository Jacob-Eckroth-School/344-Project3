#include "DynArray.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include "command.h"

/*
** Description: Initializes a DynArray struct
** Prerequisites: array is allocated
** Updated/Returned: Initial capacity of array set to 5, size set to 0.
*/
void initArray(struct DynArray* array) {
	assert(array);
	array->size = 0;
	array->capacity = 5;
	array->arr = malloc(sizeof(int) * array->capacity);
}


/*
** Description: Adds a int to an array
** Prerequisites: array is allocated
** Updated/Returned: Array has newPID added to the end of it.
*/
void addToArray(struct DynArray* array, int newPID) {
	assert(array);
	(array->size)++;
	if (array->size == array->capacity) {
		resizeArray(array);
	}
	(array->arr)[array->size] = newPID;
	assert(array);
}

/*
** Description: Removes int from array
** Prerequisites: array is allocated
** Updated/Returned: Int is removed from array.
*/
void removeFromArray(struct DynArray* array, int PID) {
	assert(array);
	for (int i = 0; i < array->size; i++) {
		if ((array->arr)[i] == PID) {
			//now we iterate Through until the second to last item
		
			for (int j = i; j < array->size - 1; j++) {
				(array->arr)[j] = (array->arr)[j + 1];
			}	
			break;
		}
	}
	(array->size)--;
	assert(array);
}



/*
** Description: Prints all contents of an array for debugging
** Prerequisites: array is allocated
** Updated/Returned: array contents are printed.
*/
void printArrayContents(struct DynArray* array) {
	assert(array);
	printf("Array contents: "); fflush(stdout);
	for (int i = 0; i < array->size; i++) {
		printf("%d ", array->arr[i]); fflush(stdout);
	}
	printf("\n"); fflush(stdout);



}

/*
** Description: Frees a dynamic array
** Prerequisites: array is allocated
** Updated/Returned: array and all memory associated with it is freed
*/
void freeArray(struct DynArray* array) {
	assert(array);
	if (array->arr) {
		free(array->arr);
	}
	free(array);
}


/*
** Description: Resizes an array to have twice the memory allowed to it.
** Prerequisites: array is allocated
** Updated/Returned: Memory is copied over, capacity of array is doubled.
*/
void resizeArray(struct DynArray* array) {
	assert(array);
	array->capacity *= 2;

	array->arr = realloc(array->arr, (array->capacity) * sizeof(int));
}