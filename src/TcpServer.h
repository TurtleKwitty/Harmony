#ifndef TcpServer_h
#define TcpServer_h

#include <functional>
#include <netinet/in.h>

#include "ServerClient.h"

class TcpServer {
public:
  TcpServer(int port);
  bool handle(std::function<void(ServerClient)> handler);
  ~TcpServer();

private:
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  int server_fd;
};

#endif
