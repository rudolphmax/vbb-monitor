#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>
#include <thread>

#include "api.hpp"

void departureList(const api::api_request request, int REFRESH_INTERVAL) {
  while (true) {
    api::api_response res = api::get(request);

    if (res.error) {
      fmt::print("{}", res.error);
      continue;
    }

    json departures = res.data["Departure"];

    for (json& departure : departures) {
      fmt::print("{} {} ", (std::string) departure["name"], (std::string) departure["direction"]);

      std::stringstream timestring;
      bool is_realtime = false;

      if (departure.contains("rtTime") && departure.contains("rtDate")) {
        timestring << (std::string) departure["rtDate"] << " " << (std::string) departure["rtTime"];
        is_realtime = true;
      } else {
        timestring << (std::string) departure["date"] << "" << (std::string) departure["time"];
      }

      std::tm tm = {};
      timestring >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

      std::time_t now_tt = time(nullptr);
      struct tm now_c = *localtime(&now_tt);

      int dmin = std::floor(std::difftime(std::mktime(&tm), now_tt) / 60 - 60);

      if (dmin <= 0) {
        fmt::print("now{}\n", is_realtime ? " *" : "");
        continue;

      } else if (dmin <= 10) {
        fmt::print("{}{}\n", dmin, is_realtime ? " *" : "");
        continue;
      }

      fmt::print("{}:{}\n", tm.tm_hour, tm.tm_min);
    }

    fmt::print("\n\n");

    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
  }
}

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

  std::thread t(departureList, request, REFRESH_INTERVAL);
  t.join();

  return EXIT_SUCCESS;
}
