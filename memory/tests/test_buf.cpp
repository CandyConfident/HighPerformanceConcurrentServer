#include <stdio.h>
#include <assert.h>

#include "data_buf.h"
#include "log.h"

int main()
{
    Logger::get_instance()->init(NULL);

    FILE *fp = fopen("./test_buf.txt", "w+t");
    if(fp == NULL)
    {
        LOG_ERROR("open file failed\n");
        return -1;
    }
    int fd = fileno(fp);

    const char *content = "hello";
    fputs(content, fp);
    rewind(fp);
    LOG_INFO("content of file: %s", content);

    InputBuffer ib;
    OutputBuffer ob;

    int r_cnt = ib.read_from_fd(fd);
    LOG_INFO("read %d bytes from file by InputBuffer\n", r_cnt);
    const char *r_data = ib.get_from_buf();
    LOG_INFO("data get from InputBuffer: %s", r_data);
    ib.clear();

    const char * w_data = "world";
    ob.write2buf(w_data, strlen(w_data));
    int w_cnt= ob.write2fd(fd);
    LOG_INFO("write %d bytes to file by OutputBuffer\n", w_cnt);
    LOG_INFO("data written to file by OutputBuffer: %s", w_data);

    rewind(fp);
    r_cnt = ib.read_from_fd(fd);
    LOG_INFO("read %d bytes from file by InputBuffer\n", r_cnt);
    r_data = ib.get_from_buf();
    LOG_INFO("data get from buf: %s", r_data);    

    fclose(fp);

    return 0;
}