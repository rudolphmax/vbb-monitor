#include <thread>
#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>
#include <ftxui/dom/elements.hpp>

#include "app.hpp"
#include "utils/ui.hpp"
#include "utils/time.hpp"

std::time_t now_tt;
struct tm now_c;

Departure* get_departures(const api::api_config api_config, const api::api_request request, int NUM_LINES) {
  api::api_response res = api::get(api_config, request);

  // TODO: error handling
  // if (res.error) {
  //   fmt::print("{}", res.error);
  //   continue;
  // }

  json fetched_departures = res.data["Departure"];
  static Departure* departures;

  departures = (Departure*) calloc(NUM_LINES, sizeof(Departure));

  int i = 0;
  for (json& departure : fetched_departures) {
    if (i >= NUM_LINES) break;

    json bg_color = departure["ProductAtStop"]["icon"]["backgroundColor"];
    json fg_color = departure["ProductAtStop"]["icon"]["foregroundColor"];

    departures[i] = {
      .name = departure["name"],
      .bg_color = Color::RGB(bg_color["r"], bg_color["g"], bg_color["b"]),
      .fg_color = Color::RGB(fg_color["r"], fg_color["g"], fg_color["b"]),
      .direction = departure["direction"],
      .time = "",
      .is_realtime = false,
      .is_cancelled = departure.contains("cancelled") && departure["cancelled"],
      .dmin = 0,
      .drt = 0
    };

    std::tm tm = {};
    std::tm rt_tm = {};

    std::ostringstream oss;
    oss.imbue(std::locale("de_DE.utf-8"));

    parse_time(&tm, departure["date"], departure["time"], now_c.tm_isdst);

    if (departure.contains("rtTime") && departure.contains("rtDate")) {
      parse_time(&rt_tm, departure["rtDate"], departure["rtTime"], now_c.tm_isdst);

      departures[i].is_realtime = true;
      departures[i].dmin = std::floor(std::difftime(std::mktime(&rt_tm), now_tt) / 60);
      departures[i].drt = std::floor(std::difftime(std::mktime(&rt_tm), std::mktime(&tm)) / 60);
      oss << std::put_time(&rt_tm, "%H:%M");

    } else {
      departures[i].dmin = std::floor(std::difftime(std::mktime(&tm), now_tt) / 60);
      oss << std::put_time(&tm, "%H:%M");
    }

    departures[i].time = oss.str();

    i++;
  }

  std::stable_sort(
    departures,
    departures + NUM_LINES,
    [](Departure a, Departure b) { return a.dmin < b.dmin; }
  );

  return departures;
}

void app(const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL, int NUM_LINES) {
  while (true) {
    now_tt = time(nullptr);
    now_c = *localtime(&now_tt);

    Departure* departures = get_departures(api_config, request, NUM_LINES);

    std::vector<Elements> lines;

    for (int i = 0; i < NUM_LINES; i++) {
      Departure departure = departures[i];

      Element departure_time_element;

      bool is_soon = departure.dmin <= 10;

      if (departure.dmin <= 0) {
        departure_time_element = time_element(
          "now\n",
          "",
          departure.is_realtime,
          true
        );

      } else if (is_soon) {
        departure_time_element = time_element(
          fmt::format("{}\n", departure.dmin),
          get_delay(departure.drt),
          departure.is_realtime,
          true
        );

      } else {
        departure_time_element = time_element(
          departure.time,
          get_delay(departure.drt),
          departure.is_realtime
        );
      }

      auto name = text_element(
        departure.name,
        departure.bg_color,
        departure.fg_color,
        true,
        true,
        is_soon
      );

      auto direction = text_element((std::string) departure.direction);

      if (!is_soon && i-1 >= 0 && departures[i-1].dmin <= 10) {
        add_separator_line(&lines);
      }

      add_line(&lines, name, direction, departure_time_element, departure.is_cancelled);
    }

    refresh_screen(lines, now_c);

    free(departures);
    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
  }
}

void start(const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL, int NUM_LINES) {
  init_ui();

  std::thread t(app, api_config, request, REFRESH_INTERVAL, NUM_LINES);
  t.join();
}
