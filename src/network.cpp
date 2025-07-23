#include <_string.h>
#include <cstdlib>
#include <cstddef>
#include <fmt/format.h>

#include "network.hpp"

net::io_context ioc;
tcp::resolver resolver(ioc);

const char* network::get(const char* host, const char* port, const char* target, http::response<http::dynamic_body> *response) {
  try {
    beast::tcp_stream stream(ioc);

    auto const lookup_res = resolver.resolve(host, port);
    stream.connect(lookup_res);

    http::request<http::string_body> req{ http::verb::get, target, 11 };
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::read(stream, buffer, *response);

    beast::error_code ec;
    auto shutdown_res = stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  } catch (std::exception &e) {
    return e.what();
  }

  return NULL;
}
