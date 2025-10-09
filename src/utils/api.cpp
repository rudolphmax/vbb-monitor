#include <fmt/base.h>
#include <fmt/format.h>

#include "api.hpp"
#include "network.hpp"

const api::api_response api::get(const api::api_config config, const api::api_request request) {
  std::string target = config.base;

  // Ensuring leading slash before path
  if (!(request.path.substr(0, 1) == "/")) {
    target += std::string("/");
  }

  target += request.path;

  // Adding trailing slash to path before args
  if (!(request.path.substr(request.path.length()-1, 1) == "/")) {
    target += "/";
  }

  for (auto param = request.api_params.begin(); param != request.api_params.end(); ++param) {
    if (param.key().empty()) break;

    if (param == request.api_params.begin()) {
      target += "?";
    } else {
      target += "&";
    }

    target += param.key() + "=" + network::url_encode(param.value());
  }

  boost::beast::http::response<boost::beast::http::dynamic_body> beast_res;
  const char* error = network::get(config.host.c_str(), config.port.c_str(), target.c_str(), &beast_res);

  // std::cout << beast_res.result_int() >= 200 && beast_res.result_int() < 400 << std::endl;

  if (error) return api::api_response({ error, nullptr });

  if (beast_res.result_int() < 200 || beast_res.result_int() >= 400) {
    static auto response_message = std::string{beast_res.reason()};

    return api::api_response({ response_message.c_str(), nullptr });
  }

  try {
    return api::api_response({ nullptr, json::parse(boost::beast::buffers_to_string(beast_res.body().data())) });

  } catch (json::exception) {
    return api::api_response({ "An error ocurred parsing the data.", nullptr });

  } catch (...) {
    return api::api_response({ "An error ocurred fetching the data.", nullptr });
  }
}
