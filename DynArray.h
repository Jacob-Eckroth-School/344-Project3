#ifndef DYNARRAY
#define DYNARRAY

struct DynArray {
	int size;
	int capacity;
	int* arr;
};


void initArray(struct DynArray*);
void addToArray(struct DynArray*, int);
void removeFromArray(struct DynArray*, int);


void printArrayContents(struct DynArray*);

void freeArray(struct DynArray*);
void resizeArray(struct DynArray*);


#endif