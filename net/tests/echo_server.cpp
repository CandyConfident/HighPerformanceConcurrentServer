#include "tcp_server.h"
#include "event_loop.h"
#include "pr.h"
#include "log.h"

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const char *ip, uint16_t port) :
            es_loop(loop), es_server(loop, ip, port) 
    {
        es_server.set_connected_cb([this](const TcpConnSP& conn){ this->echo_conneted_cb(conn); });
        es_server.set_message_cb([this](const TcpConnSP& conn, InputBuffer* ibuf){ this->echo_message_cb(conn, ibuf); });
        es_server.set_close_cb([this](){ this->echo_close_cb(); });
    };

    ~EchoServer() {};

    void start(int thread_num) { es_server.set_thread_num(thread_num); es_server.start(); }

    void set_tcp_cn_timeout_ms(int ms) { es_server.set_tcp_conn_timeout_ms(ms); }

private:
    void echo_conneted_cb(const TcpConnSP& conn) {
        PR_INFO("one connected! peer addr is %s, local socket fd is %d\n", conn->get_peer_addr(), conn->get_fd());
    }

    void echo_message_cb(const TcpConnSP& conn, InputBuffer* ibuf) {
        
        const char *msg = ibuf->get_from_buf();
        string msg_str(msg, msg+ibuf->length());
        ibuf->pop(ibuf->length());
        ibuf->adjust();
    
        PR_INFO("socket fd %d recv message:%s", conn->get_fd(), msg_str.c_str());

        conn->send(msg_str.c_str(), msg_str.length());
    }

    void echo_close_cb() { PR_INFO("one connection closed in echo server!\n"); }

    TcpServer es_server;
    EventLoop *es_loop;
};


int main()
{   
    Logger::get_instance()->init(NULL);

    EventLoop base_loop;
    EchoServer server(&base_loop, "127.0.0.1", 8888);
    server.set_tcp_cn_timeout_ms(8000);
    server.start(2);
    base_loop.loop();
    
    return 0;
}