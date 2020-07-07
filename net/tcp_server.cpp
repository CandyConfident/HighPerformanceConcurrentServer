#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../log/pr.h"
#include "../log/log.h"
#include "../threadpool/threadpool.h"
#include "acceptor.h"
#include "tcp_server.h"
#include "event_loop.h"

TcpServer::TcpServer(EventLoop* loop, const char *ip, uint16_t port) {    

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {       // SIGHUP: when terminal closing
        PR_ERROR("ignore SIGHUP signal error\n");
    }
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {     // SIGPIPE: write a closed client
        PR_ERROR("ignore SIGPIPE signal error\n");
    }

    ts_acceptor_loop = loop;
    this->ip = ip;
    this->port = port;
    ts_message_cb = [this](const TcpConnSP& conn, InputBuffer* ibuf) {
                        update_conn_timeout_time(conn);
                        if(ts_msg_cb)
                        {
                            ts_msg_cb(conn, ibuf);
                        }
                    };
    ts_acceptor = make_unique<Acceptor>(this, loop, ip, port);
}

void TcpServer::start() {
    if (!ts_started)
    {
        ts_timer.run();
        ts_started = true;
LOG_INFO("tcp server create thread pool, thread num is %d\n", ts_thread_num);
        if(ts_thread_pool = make_unique<Threadpool>(ts_thread_num); ts_thread_pool==nullptr) {
            PR_ERROR("tcp_server failed to create thread_pool\n");
            exit(1);
        }
        for(int i=0; i<ts_thread_num; i++)
        {
            ts_conn_loops.emplace_back(new EventLoop());
            EventLoop* ev = ts_conn_loops[i];
LOG_INFO("tcp server add loop_task to thread pool\n");
            ts_thread_pool->post_task([&ev]() { ev->loop(); });
        }
    }

    if (!ts_acceptor->is_listenning())
    {
LOG_INFO("tcp server add listen task to accpetor eventloop\n");
        ts_acceptor_loop->add_task([this](){ this->ts_acceptor->listen(); });
    }
}

EventLoop* TcpServer::get_next_loop() {
    int size;
    if(size = ts_conn_loops.size(); size==0) { return nullptr; }

    ++ts_next_loop;
    ts_next_loop = ts_next_loop % size;
    return ts_conn_loops[ts_next_loop]; 
}

void TcpServer::do_clean(const TcpConnSP& tcp_conn) {
    lock_guard<mutex> lck(mutex);
    for(auto i=ts_tcp_connections.begin(), e=ts_tcp_connections.end(); i!=e; ++i) {
        if(tcp_conn==*i) {
LOG_INFO("tcpserver do clean, erase tcp_conn\n");
            ts_tcp_connections.erase(i);
            break;
        }
    }
}

TcpServer::~TcpServer() {

}
