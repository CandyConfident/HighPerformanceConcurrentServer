#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "../log/pr.h"
#include "../log/log.h"
#include "event_loop.h"
#include "tcp_server.h"
#include "acceptor.h"

using namespace std;

Acceptor::Acceptor(TcpServer* server, EventLoop* loop, const char *ip, uint16_t port)
    : ac_server(server),
      ac_loop(loop),
      ac_listening(false),
      ac_idle_fd(open("/dev/null", O_RDONLY | O_CLOEXEC)),
      ac_listen_fd(socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP))
{
LOG_INFO("create one acceptor, listen fd is %d\n", ac_listen_fd);
    assert(ac_listen_fd >= 0);
    assert(ac_idle_fd >= 0);

    int op = 1;
    if (setsockopt(ac_listen_fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0) {
        PR_ERROR("set listen socket SO_REUSEADDR failed!\n");
    }

    memset(&ac_server_addr, 0, sizeof(ac_server_addr));
    ac_server_addr.sin_family = AF_INET;
    inet_aton(ip, &ac_server_addr.sin_addr);
    ac_server_addr.sin_port = htons(port);

LOG_INFO("acceptor bind, ip is %s, port is %d\n", ip, (int)port);
    if (::bind(ac_listen_fd, (const struct sockaddr*)&ac_server_addr, sizeof(ac_server_addr)) < 0) {
        PR_ERROR("bind server port error!\n");
        exit(1);
    }
}

Acceptor::~Acceptor()
{
    close(ac_listen_fd);
    close(ac_idle_fd);
}

void Acceptor::listen()
{
LOG_INFO("acceptor execute listen, listen fd is %d\n", ac_listen_fd);
    if (::listen(ac_listen_fd, 1000) == -1) {
        PR_ERROR("server listen error\n");
        exit(1);
    }
    ac_listening = true;
    ac_loop->add_to_poller(ac_listen_fd, EPOLLIN, [this](){ this->do_accept(); });
}

void Acceptor::do_accept()
{
    int connfd;
    struct sockaddr_in conn_addr;
    socklen_t conn_addrlen = sizeof conn_addr;
     while(true) {
        if (connfd = accept(ac_listen_fd, (struct sockaddr*)&conn_addr, &conn_addrlen); connfd == -1) {
            if (errno == EINTR) {
                PR_ERROR("accept fail, errno=EINTR, continue\n");
                continue;
            }
            else if (errno == EMFILE) {
                PR_WARN("accept fail, errno=EMFILE, use idle fd\n");
                close(ac_idle_fd);
                ac_idle_fd = accept(ac_listen_fd, NULL, NULL);
                close(ac_idle_fd);
                ac_idle_fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
            }
            else if (errno == EAGAIN) {
                PR_ERROR("accept fail, errno=EAGAIN, break\n");
                break;
            }
            else {
                PR_ERROR("accept fail, error no:%d, error str:%s\n", errno,strerror(errno));
                continue;
            }
        }
        else {
LOG_INFO("accepted one connection, sock fd is %d\n", connfd);
            EventLoop* sub_loop = ac_server->get_next_loop();
            
            TcpConnSP conn = make_shared<TcpConnection>(ac_server, sub_loop, connfd, conn_addr, conn_addrlen);
            conn->set_connected_cb(ac_server->ts_connected_cb);
            conn->set_message_cb(ac_server->ts_message_cb);
            conn->set_close_cb(ac_server->ts_close_cb);
            conn->add_task();

            lock_guard<mutex> lck(ac_server->ts_mutex);
            ac_server->add_new_tcp_conn(conn);     
        }
    }
}

