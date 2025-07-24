#ifndef NETWORK_H
#define NETWORK_H

#include <boost/beast.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace network {
  const char* get(const char* host, const char* port, const char* target, http::response<http::dynamic_body> *response);
}

#endif
