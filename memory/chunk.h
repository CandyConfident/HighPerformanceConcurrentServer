#ifndef __CHUNK_H__
#define __CHUNK_H__

struct Chunk{

    explicit Chunk(int size);

    ~Chunk();

    void clear();

    void adjust();

    void copy(const Chunk *other);

    void pop(int len);

    // api for debug
    [[deprecated("chunk debug api deprecated!")]]
    void print_data();

    int capacity{ 0 };
    int length{ 0 };
    int head{ 0 };
    char *data{ nullptr };
    Chunk *next{ nullptr };
};

#endif