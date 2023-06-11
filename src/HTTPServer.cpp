#include "HTTPServer.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

#ifndef NDEBUG
#include <thread>
#endif

PipelinedHTTPRequest::PipelinedHTTPRequest(std::string &&string)
    : reqString(std::move(string)) {
  std::string_view request{reqString};
  size_t currentReq = 0;
  do {
    std::string_view header =
        request.substr(currentReq, request.find("\r\n\r\n"));
    // There has to be a better way to give writable access to a substring
    // without copying T-T
    HTTPRequest &newReq =
        requests.emplace_back(reqString, currentReq, header.size());
    if (newReq.headers.contains("content-length")) {
      newReq.body = request.substr(
          currentReq + header.size() + 4,
          currentReq + header.size() + 4 +
              std::atoi(newReq.headers["content-length"].data()));
    }
    currentReq += header.size() + 4 + newReq.body.size();
  } while (currentReq < request.size() && request[currentReq] != '\r');
}

HTTPRequest::HTTPRequest(std::string &headerString, size_t offset,
                         size_t size) {
  std::string_view header(headerString);
  header = header.substr(offset, size);
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
  while (end != header.size()) {
    start = end + 2;
    end = header.find(':', start);
    std::string_view headerName = header.substr(start, end - start);
    // Force the header names to be lower case to make searching them later
    // easier since they are case-insensitive
    std::transform(headerString.begin() + offset + start,
                   headerString.begin() + offset + end,
                   headerString.begin() + offset + start,
                   [](char c) { return std::tolower(c); });

    start = end + 2;
    end = header.find("\r\n", start);
    if (end == std::string_view::npos)
      end = header.size();
    std::string_view value = header.substr(start, end - start);

    headers[headerName] = value;
  }
}

std::string HTTPResponse::toString() const {
  string_rope response =
      "HTTP/1.1 " + std::to_string(status) + " " + statusText + "\r\n";
  for (auto const &header : headers) {
    response += header.first + ": " + header.second + "\r\n";
  }
  if (!body.empty())
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
  response += "\r\n";
  response += body;
  return response.toString();
};

void sendResponse(const HTTPResponse &res, const ServerClient &client) {
  std::array<char, 65535> buffArray;
  auto buffer = buffArray.data();
  size_t pos = 0;

  std::memcpy(buffer, "HTTP/1.1 ", 9);
  std::string status{(char)('0' + res.status / 100),
                     (char)('0' + (res.status % 100) / 10),
                     (char)('0' + (res.status % 10))};
  mempcpy(buffer + 9, status.c_str(), status.size());
  pos = 9 + status.size();
  buffer[pos++] = ' ';
  std::memcpy(buffer + pos, res.statusText.c_str(), res.statusText.size());
  pos += res.statusText.size();
  buffer[pos++] = '\r';
  buffer[pos++] = '\n';
  for (auto const &header : res.headers) {
    if (buffArray.max_size() - pos <=
        header.first.size() + header.second.size() + 6) {
      client.snd(buffer, pos);
      pos = 0;
    }
    std::memcpy(buffer + pos, header.first.c_str(), header.first.size());
    pos += header.first.size();
    buffer[pos++] = ':';
    buffer[pos++] = ' ';
    std::memcpy(buffer + pos, header.second.c_str(), header.second.size());
    pos += header.second.size();
    buffer[pos++] = '\r';
    buffer[pos++] = '\n';
  }
  buffer[pos++] = '\r';
  buffer[pos++] = '\n';
  if (buffArray.max_size() - pos <= res.body.size()) {
    client.snd(buffer, pos);
    client.snd(res.body.toString());
  } else {
    for (auto segment : res.body.rope) {
      std::memcpy(buffer + pos, segment.c_str(), segment.size());
      pos += segment.size();
    }
    client.snd(buffer, pos);
  }
}

HTTPServer::HTTPServer(int port, std::vector<Handler> handlers)
    : server(port), handlers(handlers) {}
void HTTPServer::serve() {
#ifndef NDEBUG
  auto lastUpdateTime = std::chrono::system_clock::now();
  std::chrono::high_resolution_clock::duration totalRequestTimeThisPass;
  auto numRequestsThisPass = 0;
  auto numRequestsThisTick = 0;
  std::vector<std::chrono::high_resolution_clock::duration>
      responseDurationsSinceLastTick;
  std::vector<std::chrono::high_resolution_clock::duration>
      responseOnlyDurationsSinceLastTick;
  auto timedOut = 0;
  std::array<std::chrono::high_resolution_clock::duration, 5>
      responseDurationRingBuffer;
  auto curr = 0;
  auto size = 0;
#endif
  while (true) {
#ifndef NDEBUG
    auto passStart = std::chrono::high_resolution_clock::now();
#endif
    auto clientIter = clients.begin();
    while (clientIter != clients.end()) {
#ifndef NDEBUG
      auto responseStart = std::chrono::high_resolution_clock::now();
#endif
      try {
        auto &client = clientIter->client;
        std::string reqString = client.rec();
        if (reqString.size() == 0) {
          if (std::chrono::system_clock::now() - clientIter->lastReqTime >=
              std::chrono::seconds(3)) {
            clients.erase(clientIter++);
#ifndef NDEBUG
            timedOut++;
#endif
            continue;
          } else {
            clientIter++;
            continue;
          }
        } else {
          clientIter->lastReqTime = std::chrono::system_clock::now();
        }
        PipelinedHTTPRequest reqs(std::move(reqString));
        for (auto const &req : reqs.requests) {
          HTTPResponse res;

          for (auto const &handler : handlers) {
            if (handler(req, res)) {
              break;
            }
          }

          res.headers["Content-Length"] = std::to_string(res.body.size());
          res.headers["Connection"] = "keep-alive";

          sendResponse(res, client);
        }
#ifndef NDEBUG
        numRequestsThisPass += reqs.requests.size();
#endif

        if (reqs.requests.back().headers["connection"] == "close") {
          clients.erase(clientIter++);
#ifndef NDEBUG
          timedOut++;
#endif
        } else {
          clientIter++;
        }
      } catch (std::exception exception) {
        // Remove from clients if close errors (they closed
        // connection being expected error)
        clients.erase(clientIter++);
        std::cout << "Exception: " << exception.what() << std::endl;
#ifndef NDEBUG
        timedOut++;
#endif
      }
#ifndef NDEBUG
      auto responseEnd = std::chrono::high_resolution_clock::now();
      totalRequestTimeThisPass += (responseEnd - responseStart);
#endif
    }

    while (server.handle([&](ServerClient client) {
      clients.emplace_back(std::move(client), std::chrono::system_clock::now());
    })) {
    }

#ifndef NDEBUG
    auto passEnd = std::chrono::high_resolution_clock::now();
    responseDurationsSinceLastTick.push_back(
        (passEnd - passStart) /
        (numRequestsThisPass ? numRequestsThisPass : 1));

    if (numRequestsThisPass)
      responseOnlyDurationsSinceLastTick.push_back(totalRequestTimeThisPass /
                                                   numRequestsThisPass);
    totalRequestTimeThisPass = std::chrono::seconds(0);
    numRequestsThisTick += numRequestsThisPass;

    auto now = std::chrono::system_clock::now();
    if ((now - lastUpdateTime) >= std::chrono::seconds(1)) {
      std::chrono::high_resolution_clock::duration averageResponseTime;
      for (auto const &responseDuration : responseDurationsSinceLastTick) {
        averageResponseTime += responseDuration;
      }
      averageResponseTime /= responseDurationsSinceLastTick.size()
                                 ? responseDurationsSinceLastTick.size()
                                 : 1;

      responseDurationsSinceLastTick.clear();
      lastUpdateTime = now;

      responseDurationRingBuffer[curr] = averageResponseTime;
      curr = ++curr % responseDurationRingBuffer.max_size();
      if (size < responseDurationRingBuffer.max_size())
        size++;

      std::chrono::high_resolution_clock::duration averageResponseTimeLong;
      for (auto i = responseDurationRingBuffer.begin();
           i < responseDurationRingBuffer.begin() + size; i++) {
        averageResponseTimeLong += *i;
      }
      averageResponseTimeLong /= responseDurationRingBuffer.size()
                                     ? responseDurationRingBuffer.size()
                                     : 1;

      std::chrono::high_resolution_clock::duration responseOnlyTimeAvg =
          std::chrono::nanoseconds(0);
      for (auto const &responseOnlyTime : responseOnlyDurationsSinceLastTick) {
        responseOnlyTimeAvg += responseOnlyTime;
      }
      responseOnlyTimeAvg /= responseOnlyDurationsSinceLastTick.size()
                                 ? responseOnlyDurationsSinceLastTick.size()
                                 : 1;
      responseOnlyDurationsSinceLastTick.clear();

      std::cout << "Clients: " << clients.size()
                << " Requests: " << numRequestsThisTick
                << " (Dead: " << timedOut
                << ") Avg Response Time:[ 1sec: " << responseOnlyTimeAvg
                << " / " << averageResponseTime << " ( "
                << responseOnlyTimeAvg / averageResponseTime << "), "
                << responseDurationRingBuffer.size()
                << "s: " << averageResponseTimeLong << " ]" << std::endl;
      timedOut = 0;
      numRequestsThisTick = 0;
    }
    if (!numRequestsThisPass)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    numRequestsThisPass = 0;
#endif
  }
}
