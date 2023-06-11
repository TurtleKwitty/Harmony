#ifndef HTTPHeader_h
#define HTTPHeader_h

#include <chrono>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>

#include "TcpServer.h"

enum class HTTPRequestType {
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE,
  PATCH
};

class HTTPRequest {
public:
  HTTPRequest(std::string &header, size_t offset, size_t size);

  HTTPRequestType requestType;
  // FIXME: This shold be a string_view but there's a big when the path is /
  // where it magically becomes ^E on return of constructor for no reason
  std::string path;
  std::unordered_map<std::string_view, std::string_view> headers;
  std::string_view body;
};

class PipelinedHTTPRequest {
public:
  PipelinedHTTPRequest(std::string &&requestString);
  std::string reqString;
  std::vector<HTTPRequest> requests;
};

class HTTPResponse {
public:
  int status;
  std::string statusText;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  std::string toString() const;
};

class HTTPServer {
  using Handler = std::function<bool(const HTTPRequest &, HTTPResponse &)>;
  struct ClientInfo {
    ServerClient client;
    std::chrono::system_clock::time_point lastReqTime;
    ClientInfo &operator=(const ClientInfo &in) {
      client = in.client;
      lastReqTime = in.lastReqTime;
      return *this;
    }
  };

public:
  HTTPServer(int port, std::vector<Handler> handlers);
  void serve();
  TcpServer server;
  std::list<ClientInfo> clients;
  std::vector<Handler> handlers;
};
#endif
