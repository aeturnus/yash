#ifndef __VVECTOR_H__
#define __VVECTOR_H__

//VVector: void vector

struct vvector_str;
typedef struct vvector_str VVector;

VVector* VVector_new(int length);            // null initializes a vector of length
void VVector_delete(VVector* vector);        // uses lite
void VVector_deleteLite(VVector* vector);    // free vector structure but not internal contents
void VVector_deleteFull(VVector* vector);    // free vector and all internal contents

void VVector_push(VVector* vector, void* ptr);   // Get value from index
void* VVector_pop(VVector* vector);   // Get value from index

void* VVector_get(VVector* vector, int index);   // Get value from index

void VVector_realloc(VVector* vector, int newSize); //Reallocates the vector to a new size. Will only make it bigger.

void** VVector_toArray(VVector* vector);    //Returns an array with the values;

int VVector_length(VVector* vector);    //reports length of the vector
int VVector_length(VVector* vector);    //reports size of the vector

#endif  //__VVECTOR_H__

