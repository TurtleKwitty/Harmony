#ifndef ServerClient_h
#define ServerClient_h

#include <optional>
#include <string>

class ServerClient {
public:
  ServerClient(int connection_fd);
  ServerClient(ServerClient &&client);
  ~ServerClient();

  std::string rec();
  void snd(const std::string &data);

private:
  int connection_fd;
};

#endif
