#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>

#include "api.hpp"
#include "ui.hpp"

int main() {
  int REFRESH_INTERVAL;

  if (std::getenv("VBBMON_REFRESH_INTERVAL")) {
    REFRESH_INTERVAL = std::stoi(std::getenv("VBBMON_REFRESH_INTERVAL"));
  } else {
    REFRESH_INTERVAL = 25000;
  }

  const char* ACCESS_ID = std::getenv("VBBMON_ACCESS_ID");
  const char* STOP_ID = std::getenv("VBBMON_STOP_ID");

  if (!ACCESS_ID) {
    fmt::print("No Access Id provided, set VBBMON_ACCESS_ID.\n");
    return EXIT_FAILURE;
  }

  if (!STOP_ID) {
    fmt::print("No Stop Id provided, set VBBMON_STOP_ID.\n");
    return EXIT_FAILURE;
  }

  api::api_request request = {
    .path = "/departureBoard",
    .api_params = {
      { "accessId", ACCESS_ID },
      { "id", STOP_ID },
      { "maxJourneys", "10" },
      { "format", "json" }
    }
  };

  auto screen = init_ui();
  start_ui(screen, request, REFRESH_INTERVAL);

  return EXIT_SUCCESS;
}
