#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <sys/epoll.h>
#include <functional>
#include <unordered_map>
#include <set>
#include <vector>

using namespace std;

class Epoll {
public:
    
    typedef function<void()> EventCallback;

    struct io_event 
    {
        int event;
        EventCallback read_callback;
        EventCallback write_callback;
    };

    Epoll();

    ~Epoll();

    void epoll_add(int fd, int event, const EventCallback& cb);

    void epoll_del(int fd, int event);

    void epoll_del(int fd);

    int poll();

    int get_epoll_fd() { return ep_epoll_fd; }

    void get_listen_fds(set<int> &fd_set) {
        fd_set = ep_listen_fds;
    }

 private:
    void execute_cbs(int event_count);

    static const int MAXFDS = 100000;
    int ep_epoll_fd;
    set<int> ep_listen_fds;

    typedef unordered_map<int, io_event>::iterator ep_event_map_iter;
    unordered_map<int, io_event> ep_event_map;
    vector<epoll_event> ep_events;
};

#endif