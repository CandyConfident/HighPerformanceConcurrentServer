#include <sys/ioctl.h>
#include <unistd.h>
#include <memory.h>
#include <assert.h>

#include "data_buf.h"
#include "pr.h"

BufferBase::BufferBase() 
{
}

BufferBase::~BufferBase()
{
    clear();
}

const int BufferBase::length() const 
{
    return data_buf != nullptr ? data_buf->length : 0;
}

void BufferBase::pop(int len) 
{
    assert(data_buf != nullptr && len <= data_buf->length);

    data_buf->pop(len);
    if(data_buf->length == 0) {
        Mempool::get_instance().retrieve(data_buf);
        data_buf = nullptr;
    }
}

void BufferBase::clear()
{
    if (data_buf != nullptr)  {
        Mempool::get_instance().retrieve(data_buf);
        data_buf = nullptr;
    }
}


int InputBuffer::read_from_fd(int fd)
{
    int need_read;
    // FIONREAD: get readable bytes num in kernel buffer
    if (ioctl(fd, FIONREAD, &need_read) == -1) {
        PR_ERROR("ioctl FIONREAD error\n");
        return -1;
    }
    
    if (data_buf == nullptr) {
        data_buf = Mempool::get_instance().alloc_chunk(need_read);
        if (data_buf == nullptr) {
            PR_INFO("no free buf for alloc\n");
            return -1;
        }
    }
    else {
        assert(data_buf->head == 0);
        if (data_buf->capacity - data_buf->length < (int)need_read) {   
            Chunk *new_buf = Mempool::get_instance().alloc_chunk(need_read + data_buf->length);
            if (new_buf == nullptr) {
                PR_INFO("no free buf for alloc\n");
                return -1;
            }
            new_buf->copy(data_buf);
            Mempool::get_instance().retrieve(data_buf);
            data_buf = new_buf;
        }
    }

    int already_read = 0;
    do { 
        if(need_read == 0) {
            already_read = read(fd, data_buf->data + data_buf->length, m4K);
        } else {
            already_read = read(fd, data_buf->data + data_buf->length, need_read);
        }
    } while (already_read == -1 && errno == EINTR);
    if (already_read > 0)  {
        if (need_read != 0) {
            assert(already_read == need_read);
        }
        data_buf->length += already_read;
    }

    return already_read;
}

const char *InputBuffer::get_from_buf() const 
{
    return data_buf != nullptr ? data_buf->data + data_buf->head : nullptr;
}

void InputBuffer::adjust()
{
    if (data_buf != nullptr) {
        data_buf->adjust();
    }
}


int OutputBuffer::write2buf(const char *data, int len)
{
    if (data_buf == nullptr) {
        data_buf = Mempool::get_instance().alloc_chunk(len);
        if (data_buf == nullptr) {
            PR_INFO("no free buf for alloc\n");
            return -1;
        }
    }
    else {
        assert(data_buf->head == 0);
        if (data_buf->capacity - data_buf->length < len) {
            Chunk *new_buf = Mempool::get_instance().alloc_chunk(len + data_buf->length);
            if (new_buf == nullptr) {
                PR_INFO("no free buf for alloc\n");
                return -1;
            }
            new_buf->copy(data_buf);
            Mempool::get_instance().retrieve(data_buf);
            data_buf = new_buf;
        }
    }

    memcpy(data_buf->data + data_buf->length, data, len);
    data_buf->length += len;

    return 0;
}

int OutputBuffer::write2fd(int fd)
{
    assert(data_buf != nullptr && data_buf->head == 0);

    int already_write = 0;

    do { 
        already_write = write(fd, data_buf->data, data_buf->length);
    } while (already_write == -1 && errno == EINTR);

    if (already_write > 0) {
        data_buf->pop(already_write);
        data_buf->adjust();
    }

    if (already_write == -1 && errno == EAGAIN) {
        already_write = 0;
    }

    return already_write;
}