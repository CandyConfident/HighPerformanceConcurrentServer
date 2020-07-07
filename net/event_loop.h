#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <sys/eventfd.h>

#include "epoll.h"

using namespace std;

class EventLoop {
public:
    typedef std::function<void()> Task;

    EventLoop();

    ~EventLoop();

    void loop();
    void quit();

    void add_task(Task&& cb);

    void add_to_poller(int fd, int event, const Epoll::EventCallback& cb) {
        el_epoller->epoll_add(fd, event, cb);
    }

    void del_from_poller(int fd, int event) {
        el_epoller->epoll_del(fd, event);
    }

    void del_from_poller(int fd) {
        el_epoller->epoll_del(fd);
    }

    bool is_in_loop_thread() const { return el_tid == this_thread::get_id(); }

private:
    shared_ptr<Epoll> el_epoller;
    bool el_quit{ false };
    
    const thread::id el_tid{ this_thread::get_id() };
    mutex el_mutex;

    int el_evfd;
    std::vector<Task> el_task_funcs;
    bool el_dealing_task_funcs{ false };

    void evfd_wakeup();
    void evfd_read();
    void execute_task_funcs();
};

#endif