#include <assert.h>
#include <memory.h>
#include <stdio.h>

#include "chunk.h"

Chunk::Chunk(int size) : capacity(size), data(new char[size])
{
    assert(data);
}

Chunk::~Chunk()
{
    if( data )
    {
        delete [] data;
    }
}

void Chunk::clear() {
    length = head = 0;
}

void Chunk::adjust() {
    if (head != 0) {
        if (length != 0) {
            memmove(data, data+head, length);
        }
        head = 0;
    }
}

void Chunk::copy(const Chunk *other) {
    memcpy(data, other->data + other->head, other->length);
    head = 0;
    length = other->length;
}

void Chunk::pop(int len) {
    length -= len;
    head += len;
}

void Chunk::print_data()
{
    for(int i=head; i<length; i++)
    {
        printf("%c", data[i]);
    }
    printf("\n");
}