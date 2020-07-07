#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <netinet/in.h>
#include <vector>
#include <chrono>
#include <mutex>

#include "tcp_conn.h"
#include "../timer/timer.h"
#include "../log/log.h"

class EventLoop;
class Threadpool;
class Acceptor;

class TcpServer
{ 
public:
    typedef TcpConnection::ConnectionCallback ConnectionCallback;
    typedef TcpConnection::CloseCallback CloseCallback;
    typedef TcpConnection::MessageCallback MessageCallback;

    friend class Acceptor;
    friend class TcpConnection;

    TcpServer(EventLoop* loop, const char *ip, uint16_t port); 

    ~TcpServer();

    void set_thread_num(int t_num) { ts_thread_num = t_num; }
    EventLoop* get_next_loop();

    void start();
    void do_clean(const TcpConnSP& tcp_conn);

    void set_tcp_conn_timeout_ms(int ms) { ts_tcp_conn_timout_ms = ms; }
    void set_connected_cb(const ConnectionCallback& cb) { ts_connected_cb = cb; }
    void set_message_cb(const MessageCallback& cb) { ts_msg_cb = cb; }
    void set_close_cb(const CloseCallback& cb) { ts_close_cb = cb; }

private:
    void add_new_tcp_conn(const TcpConnSP& tcp_conn) { 
        auto timer_id = ts_timer.run_after(ts_tcp_conn_timout_ms, false, [tcp_conn]{
LOG_INFO("tcp conn timeout!\n");
                            tcp_conn->active_close();
                        });
        tcp_conn->set_timer_id(timer_id);
        ts_tcp_connections.emplace_back(tcp_conn); 
    }

    void update_conn_timeout_time(const TcpConnSP& tcp_conn) {
        ts_timer.cancel(tcp_conn->get_timer_id());
        add_new_tcp_conn(tcp_conn);
    }

    const char *ip;
    uint16_t port;
    unique_ptr<Acceptor> ts_acceptor;

    EventLoop *ts_acceptor_loop;
    vector<EventLoop*> ts_conn_loops;
    unique_ptr<Threadpool> ts_thread_pool;
    int ts_thread_num{ 1 };
    int ts_next_loop{ -1 };

    Timer ts_timer;
    int ts_tcp_conn_timout_ms { 6000 };

    mutex ts_mutex;
    vector<TcpConnSP> ts_tcp_connections;

    bool ts_started;

    ConnectionCallback ts_connected_cb;
    MessageCallback ts_msg_cb;
    MessageCallback ts_message_cb;
    CloseCallback ts_close_cb;
}; 

#endif