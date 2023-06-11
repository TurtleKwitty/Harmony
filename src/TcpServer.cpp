#include "TcpServer.h"

#include <asm-generic/errno.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

TcpServer::TcpServer(int port) {
  int opt = 1;

  if ((server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 256) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
}

bool TcpServer::handle(std::function<void(ServerClient)> handler) {
  int new_socket;

  if ((new_socket = accept4(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen, SOCK_NONBLOCK)) < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return false;
    perror("accept");
    exit(EXIT_FAILURE);
  }

  handler(new_socket);

  return true;
}
TcpServer::~TcpServer() { shutdown(server_fd, SHUT_RDWR); }
