#include <string.h>
#include <time.h>
#include <chrono>
#include <stdarg.h>
#include <stdexcept>
#include "log.h"

using namespace std;

Logger::log_level g_log_level = Logger::LOG_LEVEL_INFO;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "[ERROR]",
  "[WARN ]",
  "[INFO ]",
  "[DEBUG]",
};

struct my_time
{
    int year;
    char month;
    char day;
    char hour;
    char minute;
    char second; 
};

static my_time get_current_sys_time()
{
    auto tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    my_time t = { ptm->tm_year + 1900, static_cast<char>(ptm->tm_mon + 1), static_cast<char>(ptm->tm_mday),
            static_cast<char>(ptm->tm_hour), static_cast<char>(ptm->tm_min), static_cast<char>(ptm->tm_sec)};
    return t;
}

Logger::Logger()
{
    PR_DEBUG("log class constructed\n");
    l_count = 0;
}

Logger::~Logger()
{
    if(l_asyncw_thread)
    {
        is_thread_stop = true;
        if(l_asyncw_thread->joinable())
        {
            l_buffer_queue->notify();
            l_asyncw_thread->join();
        }
        delete l_asyncw_thread;
    }

    lock_guard<mutex> lck (l_mutex);
    if (l_fp != NULL)
    {
        fclose(l_fp);
    }

    if(l_buf)
    {
        delete [] l_buf;
    }

    if(l_buffer_queue)
    {
        delete l_buffer_queue;
    }
}

 bool Logger::init(const char *file_name, int buffer_queue_size, 
                    Logger::log_level level, int buffer_size, int split_lines)
{
    if(l_inited)
    {
        PR_WARN("Logger has been initialized, do not try again!\n");
        return false;
    }

    if(!file_name)
    {
        l_is_stdout = true;
    }

    if( !l_is_stdout && strlen(file_name)>=128 )
    {
        PR_ERROR("file name must be less than 128 bytes!\n");
        // exit(-1);
        throw invalid_argument("file name must be less than 128 bytes!");;
    }

    set_log_level(level);

    if (buffer_queue_size >= 1)
    {
        l_is_async = true;
        l_buffer_queue = new buffer_queue<string>(buffer_queue_size);
        l_asyncw_thread = new thread(&Logger::async_flush);
    }
    
    l_buf_size = buffer_size;
    l_buf = new char[l_buf_size];
    memset(l_buf, '\0', l_buf_size);
    l_split_lines = split_lines;

    my_time tm = get_current_sys_time();
    l_today = tm.day;
 
    if(l_is_stdout)
    {
        l_inited = true;
        l_fp = stdout;
        PR_DEBUG("succeed in using stdout as log output\n");
        PR_DEBUG("log init finished!\n");
        return true;
    }

    const char *p = strrchr(file_name, '/');
    char log_file_fullname[268] = {0};

    if (p == NULL)
    {
        PR_ERROR("log file name should behind '/'\n");
        return false;
    }
    else
    {
        strcpy(l_file_name, p + 1);
        strncpy(l_dir_name, file_name, p - file_name + 1);
        snprintf(log_file_fullname, 267, "%s%04d_%02d_%02d_%s", l_dir_name, 
            tm.year, tm.month, tm.day, l_file_name);
    
        l_fp = fopen(log_file_fullname, "a");
    }

    if (l_fp == NULL)
    {
        PR_ERROR("open %s failed!\n", log_file_fullname);
        return false;
    }

    l_inited = true;
    PR_DEBUG("succeed in using file %s as log output\n", log_file_fullname);
    PR_DEBUG("log init finished!\n");

    return true;
}

void Logger::write_log(const char* file_name, const char* tn_callbackname, int line_no, log_level level, const char *format, ...)
{
    my_time my_tm = get_current_sys_time();

    {
        lock_guard<mutex> lck (l_mutex);
        l_count++;

        if (l_today != my_tm.day || l_count % l_split_lines == 0)
        {
            PR_DEBUG("start to create a new log file\n");
            char new_file_name[301] = {0};
            fflush(l_fp);
            fclose(l_fp);
            char prefix[24] = {0};
        
            snprintf(prefix, 23, "%04d_%02d_%02d_", my_tm.year, my_tm.month, my_tm.day);
        
            if (l_today != my_tm.day)
            {
                snprintf(new_file_name, 300, "%s%s%s", l_dir_name, prefix, l_file_name);
                l_today = my_tm.day;
                l_count = 0;
            }
            else
            {
                snprintf(new_file_name, 300, "%s%s%s.%lld", l_dir_name, prefix, l_file_name, l_count / l_split_lines);
            }
            l_fp = fopen(new_file_name, "a");
        }
    }

    va_list valst;
    va_start(valst, format);

    string log_str;
    {
        lock_guard<mutex> lck (l_mutex);;

        int n = snprintf(l_buf, 300, "%04d-%02d-%02d %02d:%02d:%02d %s [%s:%s:%d] ",
                        my_tm.year, my_tm.month, my_tm.day,
                            my_tm.hour, my_tm.minute, my_tm.second, LogLevelName[level],
                            file_name, tn_callbackname, line_no);
        
        int m = vsnprintf(l_buf + n, l_buf_size - 1, format, valst);
        l_buf[n + m] = '\n';
        l_buf[n + m + 1] = '\0';
        log_str = l_buf;
    }
    va_end(valst);
    

    if (l_is_async)
    {
        while (!l_buffer_queue->push(log_str) && !is_thread_stop)   //FIXME: use tm_condvar replacing busy loop
        {               
        }
        
    }
    else
    {
        lock_guard<mutex> lck (l_mutex);
        fputs(log_str.c_str(), l_fp);   
    }
}

void Logger::flush(void)
{
    lock_guard<mutex> lck (l_mutex);
    fflush(l_fp);
}
