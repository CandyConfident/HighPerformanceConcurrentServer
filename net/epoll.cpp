#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "epoll.h"
#include "../log/log.h"

using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 20000;

Epoll::Epoll() : ep_epoll_fd(epoll_create1(EPOLL_CLOEXEC)), ep_events(EVENTSNUM) {
    assert(ep_epoll_fd > 0);
}

Epoll::~Epoll() {}

void Epoll::epoll_add(int fd, int event, const EventCallback& cb) {
    int final_events;
    int op;

    if (auto ret = ep_event_map.find(fd); ret == ep_event_map.end()) {
        final_events = event;    
        op = EPOLL_CTL_ADD;
    }
    else {
        final_events = ret->second.event | event;
        op = EPOLL_CTL_MOD;
    }

    if (event & EPOLLIN) {
        ep_event_map[fd].read_callback = cb;
    }
    else if (event & EPOLLOUT) {
        ep_event_map[fd].write_callback = cb;
    }
    
    ep_event_map[fd].event = final_events;

    struct epoll_event ev;
    ev.events = final_events;
    ev.data.fd = fd;
    if (epoll_ctl(ep_epoll_fd, op, fd, &ev) == -1) {
        PR_ERROR("epoll ctl error for fd %d\n", fd);
        return;
    }
LOG_INFO("epoll add, fd is %d, event is %d\n", fd, final_events);
    ep_listen_fds.insert(fd);
}


void Epoll::epoll_del(int fd, int event) {

    ep_event_map_iter ret;
    if (ret = ep_event_map.find(fd); ret == ep_event_map.end()) {
        return ;
    }

    int &target_event = ret->second.event;

    if (target_event = target_event & (~event); target_event == 0) {
        this->epoll_del(fd);
    }
    else {
        struct epoll_event ev;
        ev.events = target_event;
        ev.data.fd = fd;
        if (epoll_ctl(ep_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
            PR_ERROR("epoll ctl error for fd %d\n", fd);
            return;
        }
    }
}

void Epoll::epoll_del(int fd) {

    ep_event_map.erase(fd);

    ep_listen_fds.erase(fd);

    epoll_ctl(ep_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

int Epoll::poll() {
    while (true) {
        int event_count =
            epoll_wait(ep_epoll_fd, &*ep_events.begin(), ep_events.size(), EPOLLWAIT_TIME);
        if (event_count < 0)
        {
            PR_ERROR("epoll wait return val <0! error no:%d, error str:%s\n", errno, strerror(errno));
            continue;
        }

        execute_cbs(event_count);
        return event_count;
    }
}

inline void Epoll::execute_cbs(int event_count) {
    for (int i = 0; i < event_count; i++) {
        auto ev_ret = ep_event_map.find(ep_events[i].data.fd);
        assert(ev_ret != ep_event_map.end());

        io_event *ev = &(ev_ret->second);

        if (ep_events[i].events & EPOLLIN) {
LOG_INFO("execute read cb\n");
            if(ev->read_callback) ev->read_callback();
        }
        else if (ep_events[i].events & EPOLLOUT) {
LOG_INFO("execute write cb\n");
            if(ev->write_callback) ev->write_callback();
        }
        else if (ep_events[i].events & (EPOLLHUP|EPOLLERR)) {
            if (ev->read_callback) {
                ev->read_callback();
            }
            else if (ev->write_callback) {
                ev->write_callback();
            }
            else {
                LOG_INFO("get error, delete fd %d from epoll\n", ep_events[i].data.fd);
                epoll_del(ep_events[i].data.fd);
            }
        }
    }
}
