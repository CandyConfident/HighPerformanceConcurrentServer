#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "event_loop.h"
#include "../log/pr.h"
#include "../log/log.h"

using namespace std;

EventLoop::EventLoop() : el_epoller(new Epoll()) {
    if(el_evfd = { eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC) }; el_evfd < 0)
    {
        PR_ERROR("fail to create event_fd\n");
        exit(1);
    }
LOG_INFO("create one eventloop, event fd is %d\n", el_evfd);
    el_epoller->epoll_add(el_evfd, EPOLLIN /*| EPOLLET*/, [this](){ this->evfd_read(); });
}

EventLoop::~EventLoop() {
    close(el_evfd);
}

void EventLoop::evfd_wakeup() {
    uint64_t one = 1;
    if(auto n = write(el_evfd, &one, sizeof one); n != sizeof one)
    {
        PR_ERROR("write %ld bytes to event_fd instead of 8\n", n);
    }
}

void EventLoop::evfd_read() {
    uint64_t one = 1;
    if(auto n = read(el_evfd, &one, sizeof one); n != sizeof one)
    {
        PR_ERROR("read %ld bytes from event_fd instead of 8\n", n);
    }
}

void EventLoop::add_task(Task&& cb) {
LOG_INFO("eventloop, add one task\n");
    if (is_in_loop_thread())
    {
        cb();
    }
    else
    {
        lock_guard<mutex> lock(el_mutex);
        el_task_funcs.emplace_back(move(cb));       
    }

    if (!is_in_loop_thread() || el_dealing_task_funcs) { evfd_wakeup(); }
}

void EventLoop::loop() {
    el_quit = false;
    while (!el_quit) {
        auto cnt = el_epoller->poll();
LOG_INFO("eventloop, tid %lld, loop once, epoll event cnt %d\n", tid_to_ll(this_thread::get_id()), cnt);
        execute_task_funcs();
    }
}

void EventLoop::execute_task_funcs() {
    std::vector<Task> functors;
    el_dealing_task_funcs = true;

    {
        lock_guard<mutex> lock(el_mutex);
        functors.swap(el_task_funcs);
    }

    for (size_t i = 0; i < functors.size(); ++i) functors[i]();
    el_dealing_task_funcs = false;
}

void EventLoop::quit() {
    el_quit = true;
    if (!is_in_loop_thread()) {
        evfd_wakeup();
    }
}