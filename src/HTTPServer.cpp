#include "HTTPServer.h"

#include <iostream>
#include <string>
#include <string_view>

HTTPRequest::HTTPRequest(std::string requestString)
    : requestString(requestString) {
  std::string_view request{requestString};

  {
    std::string_view header = request.substr(0, request.find("\r\n\r\n"));

    std::string_view::size_type start = 0;
    std::string_view::size_type end = header.find(' ');
    {
      std::string_view typeView = header.substr(start, end - start);
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
    end = header.find(' ', start);
    path = header.substr(start, end - start);

    end = header.find("\r\n", end);
    while (true) {
      start = end + 2;
      end = header.find(':', start);
      std::string_view headerName = header.substr(start, end - start);

      start = end + 2;
      end = header.find("\r\n", start);
      std::string_view value = header.substr(start, end - start);

      headers[headerName] = value;

      if (end == std::string_view::npos)
        break;
    }
  }

  if (headers.contains("Content-Type")) {
    body = request.substr(request.find("\r\n\r\n") + 4);
    int size = std::stoi(headers["Content-Length"].data());
    if (body.size() != size)
      std::cerr << "Length of request body does not match Content-Length header"
                << std::endl;
  }
}

std::string HTTPResponse::toString() {
  std::string response =
      "HTTP/1.1 " + std::to_string(status) + " " + statusText + "\r\n";
  for (auto header : headers) {
    response += header.first + ": " + header.second + "\r\n";
  }
  if (body.size())
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
  response += "\r\n";
  response += body;
  return response;
};
