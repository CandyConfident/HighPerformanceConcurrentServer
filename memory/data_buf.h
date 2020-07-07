#ifndef __DATA_BUF_H__
#define __DATA_NIF_H__

#include "chunk.h"
#include "mem_pool.h"

class BufferBase {
public:
    BufferBase();
    ~BufferBase();

    const int length() const;
    void pop(int len);
    void clear();

protected:
    Chunk *data_buf{ nullptr };
};

class InputBuffer : public BufferBase 
{
public:
    int read_from_fd(int fd);

    const char *get_from_buf() const;

    void adjust();
};

class OutputBuffer : public BufferBase 
{
public:
    int write2buf(const char *data, int len);

    int write2fd(int fd);
};

#endif