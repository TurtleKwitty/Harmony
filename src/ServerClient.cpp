#include "ServerClient.h"
#include <netinet/in.h>
#include <optional>
#include <unistd.h>

#include <iostream>

ServerClient::ServerClient(int connection_fd) : connection_fd(connection_fd) {}
ServerClient::ServerClient(ServerClient &&client)
    : connection_fd(client.connection_fd) {
  client.connection_fd = -1;
}

ServerClient::~ServerClient() { close(connection_fd); }

std::string ServerClient::rec() {
  std::string request;
  char buffer[1024] = {0};

  int valread = read(connection_fd, buffer, sizeof(buffer));
  std::cout << "Read " << valread << std::endl;
  do {
    if (valread > 0)
      request.append(buffer, valread);
  } while (valread == sizeof(buffer));

  return request;
}

void ServerClient::snd(const std::string &data) {
  send(connection_fd, data.c_str(), data.size(), 0);
  char EOT = 4;
  send(connection_fd, &EOT, 1, 0);
}
