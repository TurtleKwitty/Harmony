#ifndef ServerClient_h
#define ServerClient_h

#include <optional>
#include <string>

class ServerClient {
public:
  ServerClient(int connection_fd);
  ServerClient(const ServerClient &client);
  ServerClient(ServerClient &&client);
  ~ServerClient();

  ServerClient &operator=(const ServerClient &in) = default;

  std::string rec();
  void snd(const std::string &data) const;
  void snd(const char *data, size_t length) const;

private:
  int connection_fd;
};

#endif
