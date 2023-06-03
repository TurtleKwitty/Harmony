#ifndef HTTPHeader_h
#define HTTPHeader_h

#include <string>
#include <string_view>
#include <unordered_map>

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
  HTTPRequest(std::string requestString);

  HTTPRequestType requestType;
  // FIXME: This shold be a string_view but there's a big when the path is /
  // where it magically becomes ^E on return of constructor for no reason
  std::string path;
  std::unordered_map<std::string_view, std::string_view> headers;
  std::string_view body;

  const std::string requestString;
};

class HTTPResponse {
public:
  int status;
  std::string statusText;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  std::string toString();
};

#endif
