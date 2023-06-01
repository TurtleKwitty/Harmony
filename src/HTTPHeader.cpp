#include "HTTPHeader.h"

HTTPHeader::HTTPHeader(std::string requestString)
    : requestString(requestString) {
  std::string_view request{requestString};

  std::string_view::size_type start = 0;
  std::string_view::size_type end = request.find(' ');
  {
    std::string_view typeView = request.substr(start, end - start);
    if (typeView == "GET")
      requestType = HTTPRequestType::GET;
    else if (typeView == "HEAD")
      requestType = HTTPRequestType::HEAD;
    else if (typeView == "POST")
      requestType = HTTPRequestType::POST;
    else if (typeView == "PUT")
      requestType = HTTPRequestType::PUT;
    else if (typeView == "DELETE")
      requestType = HTTPRequestType::DELETE;
    else if (typeView == "CONNECT")
      requestType = HTTPRequestType::CONNECT;
    else if (typeView == "OPTIONS")
      requestType = HTTPRequestType::OPTIONS;
    else if (typeView == "TRACE")
      requestType = HTTPRequestType::TRACE;
    else if (typeView == "PATCH")
      requestType = HTTPRequestType::PATCH;
  }

  start = end + 1;
  end = request.find(' ', start);
  path = request.substr(start, end - start);

  end = request.find('\n', end);
  while (true) {
    start = end + 1;
    end = request.find(':', start);
    if (end == std::string_view::npos)
      break;

    std::string_view header = request.substr(start, end - start);
    start = end + 2;
    end = request.find('\n', start);
    std::string_view value = request.substr(start, end - start);

    headers.push_back({header, value});
  }
}
