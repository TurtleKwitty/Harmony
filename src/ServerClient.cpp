#include "ServerClient.h"
#include <sys/socket.h>
#include <unistd.h>

ServerClient::ServerClient(int connection_fd) : connection_fd(connection_fd) {}
ServerClient::ServerClient(const ServerClient &client)
    : connection_fd(client.connection_fd) {}
ServerClient::ServerClient(ServerClient &&client)
    : connection_fd(client.connection_fd) {
  client.connection_fd = -1;
}

ServerClient::~ServerClient() { close(connection_fd); }

std::string ServerClient::rec() {
  std::string request;
  char buffer[1024] = {0};

  int valread = recv(connection_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
  do {
    if (valread > 0)
      request.append(buffer, valread);
  } while (valread == sizeof(buffer));

  return request;
}

void ServerClient::snd(const std::string &data) const {
  send(connection_fd, data.c_str(), data.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
}

void ServerClient::snd(const char *data, size_t length) const {
  send(connection_fd, data, length, MSG_DONTWAIT | MSG_NOSIGNAL);
}
