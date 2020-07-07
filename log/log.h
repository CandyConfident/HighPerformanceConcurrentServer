#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <assert.h>
#include <atomic>

#include "log_queue.h"
#include "pr.h"

using namespace std;

class Logger
{
public:
    typedef enum 
    {
        LOG_LEVEL_ERROR,
        LOG_LEVEL_WARN,
        LOG_LEVEL_INFO,
        LOG_LEVEL_DEBUG,
        NUM_LOG_LEVELS,
    } log_level;

    static Logger *get_instance()
    {
        static Logger instance;
        return &instance;
    }

    static void async_flush(void)
    {
        Logger::get_instance()->async_write();
    }

    static log_level get_log_level();

    static log_level set_log_level(log_level level);

    // this function is not reentrant, should be called at main thread before subthread started.
    bool init(const char *file_name, int buffer_queue_size = 0, Logger::log_level = Logger::LOG_LEVEL_INFO,
                int buffer_size = 8192, int split_lines = 5000);

    bool is_inited()
    {
        return l_inited;
    }

    void write_log(const char* file_name, const char* tn_callbackname, int line_no, log_level level, const char *format, ...);

    void flush(void);

private:
    Logger();
    Logger(const Logger&);
    ~Logger();
    void *async_write()
    {
        string single_line;
        //@ assuming that no thread pushes elements in, once async writing thread starts, it falls in sleep
        //  and will never be awaken. To deal with that, an interface "notify" is introduced in buffer_queue.
        //  In destruction func, notify is called to wake up this thread and let it stop naturally.
        //@ assuming that pop func returns false under spurious wakeup, the loop will be over as well as the async writing thread,
        //  which is not what we expected. So the variable is_thread_stop is introduced. Only in destruction func will it be setted,
        //  combining with the notify func, the thread can finish normally. Anothor function of is_thread_stop is to terminate the
        // busy loop of pushing operation in write_log func when this class comes to end.
        while(l_buffer_queue->pop(single_line) && !is_thread_stop)
        {
            lock_guard<mutex> lck (l_mutex);
            fputs(single_line.c_str(), l_fp); 
        }   
    }

private:
    char l_dir_name[128]; 
    char l_file_name[128];
    int l_split_lines;
    int l_buf_size;
    long long l_count;
    int l_today;       
    FILE *l_fp;     
    char *l_buf = nullptr;
    bool l_inited = false;
    buffer_queue<string> *l_buffer_queue = nullptr;
    bool l_is_async; 
    bool l_is_stdout = false;
    atomic<bool> is_thread_stop = false;
    mutex l_mutex;              // TODO: add mutexes for different critical resources
    thread *l_asyncw_thread = nullptr;
};


extern Logger::log_level g_log_level;

inline Logger::log_level Logger::get_log_level()
{
    return g_log_level;
}

inline Logger::log_level Logger::set_log_level(Logger::log_level level)
{
    assert(level>=Logger::LOG_LEVEL_ERROR && level<=Logger::LOG_LEVEL_DEBUG);
    Logger::log_level old_level = g_log_level;
    g_log_level = level;
    return old_level;
}

#define LOG_DEBUG(format, ...)              \
    do{                                    \
        if(!Logger::get_instance()->is_inited())                       \
        {                                                               \
            PR_ERROR("logger must be inited before user!\n");           \
        }                                                               \
        if(Logger::LOG_LEVEL_DEBUG <= Logger::get_log_level())            \
        {                                       \
            Logger::get_instance()->write_log(__FILE__, __FUNCTION__,       \
                __LINE__, Logger::LOG_LEVEL_DEBUG, format, ##__VA_ARGS__);   \
            Logger::get_instance()->flush();                                \
        }                                                                   \
    } while(0)

#define LOG_INFO(format, ...)               \
    do{                                   \
        if(!Logger::get_instance()->is_inited())                       \
        {                                                               \
            PR_ERROR("logger must be inited before user!\n");           \
        }                                                               \
        if(Logger::LOG_LEVEL_INFO <= Logger::get_log_level())             \
        {                                   \
            Logger::get_instance()->write_log(__FILE__, __FUNCTION__,       \
                __LINE__, Logger::LOG_LEVEL_INFO, format, ##__VA_ARGS__);   \
            Logger::get_instance()->flush();                                \
        }                                                                  \
    } while(0)

#define LOG_WARN(format, ...)                                               \
    do{                                                                      \
        if(!Logger::get_instance()->is_inited())                       \
        {                                                               \
            PR_ERROR("logger must be inited before user!\n");           \
        }                                                               \
        if(Logger::LOG_LEVEL_WARN <= Logger::get_log_level())              \
        {                                                                    \
            Logger::get_instance()->write_log(__FILE__, __FUNCTION__,       \
                __LINE__, Logger::LOG_LEVEL_WARN, format, ##__VA_ARGS__);   \
            Logger::get_instance()->flush();                                  \
        }                                                                     \
    } while(0)

#define LOG_ERROR(format, ...)                                              \
    do{                                                                      \
        if(!Logger::get_instance()->is_inited())                       \
        {                                                               \
            PR_ERROR("logger must be inited before user!\n");           \
        }                                                               \
        if(Logger::LOG_LEVEL_ERROR <= Logger::get_log_level())             \
        {                                                                   \
            Logger::get_instance()->write_log(__FILE__, __FUNCTION__,       \
                __LINE__, Logger::LOG_LEVEL_ERROR, format, ##__VA_ARGS__);   \
            Logger::get_instance()->flush();                                 \
        }                                                                   \
    } while(0)

#endif
