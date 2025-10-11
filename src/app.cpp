#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>
#include <ftxui/dom/elements.hpp>

#include "app.hpp"
#include "utils/ui.hpp"
#include "utils/time.hpp"

Data* data;
std::condition_variable data_cv;
std::mutex data_mutex;

std::time_t now_tt;
struct tm now_c;

void get_departures(const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL, int NUM_LINES) {
  while (true) {
    // Clearing Error
    {
      std::lock_guard<std::mutex> guard(data_mutex);
      delete data->error;
      data->error = nullptr;
    }

    try {
      api::api_response res = api::get(api_config, request);

      if (res.error) {
        std::lock_guard<std::mutex> guard(data_mutex);

        fmt::print("{}", res.error);
        data->error = new Error({
          .message = fmt::format("Fetching failed - {}", res.error)
        });

      } else {
        std::lock_guard<std::mutex> guard(data_mutex);

        json fetched_departures = res.data["Departure"];

        int i = 0;
        for (json& departure : fetched_departures) {
          if (i >= NUM_LINES) break;

          json bg_color = departure["ProductAtStop"]["icon"]["backgroundColor"];
          json fg_color = departure["ProductAtStop"]["icon"]["foregroundColor"];

          data->departures[i] = {
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

            data->departures[i].is_realtime = true;
            data->departures[i].dmin = std::floor(std::difftime(std::mktime(&rt_tm), now_tt) / 60);
            data->departures[i].drt = std::floor(std::difftime(std::mktime(&rt_tm), std::mktime(&tm)) / 60);
            oss << std::put_time(&rt_tm, "%H:%M");

          } else {
            data->departures[i].dmin = std::floor(std::difftime(std::mktime(&tm), now_tt) / 60);
            oss << std::put_time(&tm, "%H:%M");
          }

          data->departures[i].time = oss.str();

          i++;
        }

        std::stable_sort(
          data->departures,
          data->departures + NUM_LINES,
          [](Departure a, Departure b) { return a.dmin < b.dmin; }
        );
      }
    } catch(...) {
      data->error = new Error({
        .message = "An error ocurred when fetching the data."
      });
    }

    data_cv.notify_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
  }
}

void display(ScreenData* screen_data, int NUM_LINES) {
  while (true) {
    // grab current time
    now_tt = time(nullptr);
    now_c = *localtime(&now_tt);

    std::ostringstream oss;
    oss.imbue(std::locale("de_DE.utf-8"));
    oss << std::put_time(&now_c, "%H:%M:%S");

    screen_data->time = oss.str();

    refresh_screen();

    // Wait one second for the data mutex signal
    std::unique_lock<std::mutex> lk(data_mutex);
    data_cv.wait_for(lk, std::chrono::seconds(1));

    if (data->error) {
      screen_data->error_message = data->error->message;

    } else {
      screen_data->error_message = "";

      std::vector<Elements> lines;

      for (int i = 0; i < NUM_LINES; i++) {
        Departure departure = data->departures[i];

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

        if (!is_soon && i-1 >= 0 && data->departures[i-1].dmin <= 10) {
          add_separator_line(&lines);
        }

        add_line(&lines, name, direction, departure_time_element, departure.is_cancelled);
      }

      screen_data->departures = lines;
    }
  }
}

void app(const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL, int NUM_LINES) {
  ScreenData* screen_data = init_ui();

  data = new Data({
    .departures = new Departure[NUM_LINES],
    .error = nullptr
  });

  std::thread fetcher(get_departures, api_config, request, REFRESH_INTERVAL, NUM_LINES), displayer(display, screen_data, NUM_LINES);
  fetcher.join();
  displayer.join();

  delete[] data->departures;
  if (data->error) {
    delete data->error;
  }
  delete data;
}
