#include <stdlib.h>

#include "vvector.h"

struct vvector_str
{
    int length;     // length of elements
    int size;       // # of total spots
    void** array;   // array of void ptrs
};

VVector* VVector_new(int length)
{
    VVector* vec = malloc(sizeof(VVector));         // Allocate vvector
    vec->length = 0;
    vec->size = length;
    vec->array = malloc(sizeof(void*) * length);    // Allocate array space
    for(int i = 0; i < length; i++)
    {
        vec->array[i] = 0x0;    // Null init
    }
    return vec;
}

void VVector_delete(VVector* vec)
{
    VVector_deleteLite(vec);
}

void VVector_deleteLite(VVector* vec)
{
    free(vec->array);   //free the array
    free(vec);          //free the pointer
}

void VVector_deleteFull(VVector* vec)
{
    int length = vec->length;
    for(int i = 0; i < length; i++)
    {
        if(vec->array[i] != 0)
            free(vec->array[i]);
    }
    VVector_deleteLite(vec);
}

//Will double size
void VVector_realloc(VVector* vec, int size)
{
    //Break if we're making it even smaller
    if(size <= vec->length)
        return;
    void** oldArr = vec->array;
    int oldSize = vec->size;
    int newSize = size;

    void** newArr = malloc(sizeof(void*) * newSize);   // alloc new memory
    int i = 0;
    for(; i < oldSize; i++)
    {
        newArr[i] = oldArr[i];  //set to old void*
    }
    for(; i < newSize; i++)
    {
        newArr[i] = 0x0;    //null init
    }

    vec->array = newArr;    //Set the new array
    vec->size = newSize;    //Set new size
    free(oldArr); //Free the old array
}

void VVector_push(VVector* vec, void* ptr)
{
    if(vec->length == vec->size)
    {
        VVector_realloc(vec,vec->size*2);    //amortize doubling
    }
    vec->array[vec->length] = ptr;  //set the value
    vec->length++;                  //increment the length
}

void* VVector_pop(VVector* vec)
{
    if(vec->length > 0)
    {
        vec->length -= 1;
        return vec->array[vec->length]; //get the last value; lol already decremented for this
    }
    return 0x0;
}

void* VVector_get(VVector* vec, int index)
{
    if(index < vec->length)
        return vec->array[index];
    return 0x0;
}

void** VVector_toArray(VVector* vec)
{
    int length = vec->length;
    void** output = malloc(sizeof(void*) * length);
    void** array = vec->array;
    for(int i = 0; i < length; i++)
    {
        output[i] = array[i];
    }
    return output;
}

int VVector_length(VVector* vec)
{
    return vec->length;
}
int VVector_size(VVector* vec)
{
    return vec->size;
}
