#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <functional>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

class EventLoop;
class TcpServer;

class Acceptor
{
public:
    Acceptor(TcpServer* server, EventLoop* loop, const char *ip, uint16_t port);
    ~Acceptor();

  bool is_listenning() const { return ac_listening; }
  void listen();

 private:
    void do_accept();

    TcpServer *ac_server;
    int ac_listen_fd;
    EventLoop *ac_loop;
    bool ac_listening;
    int ac_idle_fd;
    sockaddr_in ac_server_addr;
};

#endif