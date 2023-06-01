#ifndef HTTPHeader_h
#define HTTPHeader_h

#include <string>
#include <string_view>
#include <vector>

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

class HTTPHeader {
public:
  HTTPHeader(std::string requestString);

  HTTPRequestType requestType;
  // FIXME: This shold be a string_view but there's a big when the path is /
  // where it magically becomes ^E on return of constructor for no reason
  std::string path;
  std::vector<std::pair<std::string_view, std::string_view>> headers;

  const std::string requestString;
};

#endif
