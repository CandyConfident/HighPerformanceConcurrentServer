#ifndef __TCP_CONN_H__
#define __TCP_CONN_H__

#include <memory>
#include <any>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>

#include "../memory/data_buf.h"

using namespace std;

class TcpConnection;

typedef shared_ptr<TcpConnection> TcpConnSP;

class EventLoop;
class TcpServer;

class TcpConnection : public enable_shared_from_this<TcpConnection>
{
public:
    typedef function<void(const TcpConnSP&)> ConnectionCallback;
    typedef function<void()> CloseCallback;
    typedef function<void(const TcpConnSP&, InputBuffer*)> MessageCallback;

    TcpConnection(TcpServer *server, EventLoop* loop, int sockfd, struct sockaddr_in& addr, socklen_t& len);
    ~TcpConnection();

    EventLoop* getLoop() const { return tc_loop; }

    void add_task();

    void set_context(const any& context) { tc_context = context; }
    auto get_context() { return &tc_context; }

    const char* get_peer_addr() { return inet_ntoa(tc_peer_addr.sin_addr);}
    auto get_fd() { return tc_fd; }

    bool send(const char *data, int len);

    void set_connected_cb(const ConnectionCallback& cb) { tc_connected_cb = cb; }
    void set_message_cb(const MessageCallback& cb) { tc_message_cb = cb; }
    void set_close_cb(const CloseCallback& cb) { tc_close_cb = cb; }

    void connected();  
    void active_close() { do_close(); }

    void set_timer_id(int id) {tc_timer_id = id; }
    int get_timer_id() { return tc_timer_id; }

private:
    inline void set_sockfd(int& fd);
    void do_read();
    void do_write();
    void do_close();

    TcpServer* tc_server;
    EventLoop* tc_loop;
    int tc_fd;
    int tc_timer_id{ -1 };

    struct sockaddr_in tc_peer_addr;
    socklen_t tc_peer_addrlen;

    OutputBuffer tc_obuf;
    InputBuffer tc_ibuf;

    any tc_context;

    ConnectionCallback tc_connected_cb;
    MessageCallback tc_message_cb;
    CloseCallback tc_close_cb;
};

#endif