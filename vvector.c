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

void VVector_delete(VVector* this)
{
    VVector_deleteLite(this);
}

void VVector_deleteLite(VVector* this)
{
    free(this->array);   //free the array
    free(this);          //free the pointer
}

void VVector_deleteFull(VVector* this)
{
    int length = this->length;
    for(int i = 0; i < length; i++)
    {
        if(this->array[i] != 0)
            free(this->array[i]);
    }
    VVector_deleteLite(this);
}

//Will double size
void VVector_realloc(VVector* this, int size)
{
    //Break if we're making it even smaller
    if(size <= this->length)
        return;
    void** oldArr = this->array;
    int oldSize = this->size;
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

    this->array = newArr;    //Set the new array
    this->size = newSize;    //Set new size
    free(oldArr); //Free the old array
}

void VVector_push(VVector* this, void* ptr)
{
    if(this->length == this->size)
    {
        VVector_realloc(this,this->size*2);    //amortize doubling
    }
    this->array[this->length] = ptr;  //set the value
    this->length++;                  //increment the length
}

void* VVector_pop(VVector* this)
{
    if(this->length > 0)
    {
        this->length -= 1;
        return this->array[this->length]; //get the last value; lol already decremented for this
    }
    return 0x0;
}

void* VVector_get(VVector* this, int index)
{
    if(index < this->length)
        return this->array[index];
    return 0x0;
}

const void * const * VVector_toArray(VVector* this)
{
    return this->array;
}

void** VVector_toArray_cpy(VVector* this)
{
    int length = this->length;
    void** output = malloc(sizeof(void*) * length);
    void** array = this->array;
    for(int i = 0; i < length; i++)
    {
        output[i] = array[i];
    }
    return output;
}


int VVector_length(VVector* this)
{
    return this->length;
}
int VVector_size(VVector* this)
{
    return this->size;
}
