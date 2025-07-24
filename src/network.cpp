#include <_string.h>
#include <cstdlib>
#include <cstddef>
#include <fmt/format.h>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "network.hpp"

namespace ssl = net::ssl;

net::io_context ioc;
ssl::context ctx(ssl::context::tlsv13_client);
tcp::resolver resolver(ioc);

const char* network::get(const char* host, const char* port, const char* target, http::response<http::dynamic_body> *response) {
  const char* ROOT_CERT_LOC = std::getenv("ROOT_CERT_BUNDLE_LOCATION");

  if (ROOT_CERT_LOC) {
    ctx.load_verify_file(ROOT_CERT_LOC);
  }

  ctx.set_verify_mode(ssl::verify_peer);

  try {
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (! SSL_set_tlsext_host_name(stream.native_handle(), host)) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    auto const lookup_res = resolver.resolve(host, port);

    beast::get_lowest_layer(stream).connect(lookup_res);

    stream.handshake(ssl::stream_base::client);

    http::request<http::string_body> req{ http::verb::get, target, 11 };
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::read(stream, buffer, *response);

    beast::error_code ec;
    auto shutdown_res = stream.shutdown(ec);

    if(ec == net::error::eof) {
      // Rationale:
      // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
      ec = {};
    }

    if(ec) throw beast::system_error{ec};

  } catch (std::exception &e) {
    return e.what();
  }

  return NULL;
}
