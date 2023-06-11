#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sys/resource.h>

#include "HTTPServer.h"
#include "string_rope.h"

string_rope homePage() {
  return R"htmlString(
			<h1>Meowwwwwwwwwwwwwwwwwwwwwwwwwww <3</h1>
			<img src="meow-big.png"/>
	)htmlString";
}

string_rope app(std::string iconImage) {
  string_rope toRet;
  toRet += R"htmlString(
		<html>
			<head>
				<link rel="icon" href=")htmlString";
  toRet += iconImage;
  toRet += R"htmlString(">
			</head>
			<body>)htmlString";
  toRet += homePage();
  toRet += R"htmlString(
			</body>
		</html>)htmlString";
  return toRet;
}

int main(int argc, char **argv) {
  // Set number of files to max to allow as many connections as possible
  struct rlimit rlim;
  getrlimit(RLIMIT_NOFILE, &rlim);
  rlim.rlim_cur = rlim.rlim_max;
  setrlimit(RLIMIT_NOFILE, &rlim);

  std::unordered_map<std::string, std::string> fileCache;

  HTTPServer server(
      8080,
      {[&](const HTTPRequest &request, HTTPResponse &response) {
         if (fileCache.contains(request.path)) {
           response.body = fileCache[request.path];
           response.status = 200;
           response.statusText = "Static file caches babey!";
           response.headers["Content-Type"] = "image/png";
           return true;
         } else {
           std::filesystem::path staticPath("static" + request.path);
           if (request.path != "/" && std::filesystem::exists(staticPath)) {
             std::ifstream file(staticPath);
             std::string body;
             body.reserve(file.rdbuf()->in_avail());
             body.append(std::istreambuf_iterator(file),
                         std::istreambuf_iterator<char>());
             fileCache[request.path] = body;

             response.body = std::move(body);
             response.status = 200;
             response.statusText = "Static file exists "
                                   "matey!";
             response.headers["Content-Type"] = "image/png";
             return true;
           }
           return false;
         }
       },
       [](const HTTPRequest &request, HTTPResponse &response) {
         if (request.path == "/") {
           response.status = 200;
           response.statusText = "All good homey <3";
           response.headers["Content-Type"] = "text/html";
           response.body = app("/meow.png");
         } else {
           response.status = 404;
           response.statusText = "Dunno how to handle this sorry <3";
         }
         return true;
       }});
  server.serve();
}
