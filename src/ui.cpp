#include <thread>
#include <chrono>
#include <fmt/format.h>

#include "ui.hpp"

void departure_list(Screen screen, const api::api_request request, int REFRESH_INTERVAL) {
  while (true) {
    api::api_response res = api::get(request);

    if (res.error) {
      fmt::print("{}", res.error);
      continue;
    }

    json departures = res.data["Departure"];

    bool has_horizontal_separator = false;
    Elements columns[3];

    for (json& departure : departures) {
      json departure_bg_color = departure["ProductAtStop"]["icon"]["backgroundColor"];
      json departure_fg_color = departure["ProductAtStop"]["icon"]["foregroundColor"];

      columns[0].push_back(
        bgcolor(
          Color::RGB(departure_bg_color["r"], departure_bg_color["g"], departure_bg_color["b"]),
          hbox(
            separatorEmpty(),
            color(
              Color::RGB(departure_fg_color["r"], departure_fg_color["g"], departure_fg_color["b"]),
              text((std::string) departure["name"])
            ),
            separatorEmpty()
          )
        )
      );

      columns[1].push_back(text((std::string) departure["direction"]));

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

      Element departure_time_element;

      if (dmin <= 0) {
        departure_time_element = bold(text("now\n"));

      } else if (dmin <= 10) {
        departure_time_element = bold(text(fmt::format("{}\n", dmin)));

      } else {
        departure_time_element = bold(text(fmt::format("{}:{}\n", tm.tm_hour - 1, tm.tm_min)));
      }

      if (is_realtime) {
        departure_time_element = hbox(
          separatorEmpty(),
          departure_time_element | flex,
          text(" *"),
          separatorEmpty()
        );

      } else {
        departure_time_element = hbox(
          separatorEmpty(),
          departure_time_element,
          separatorEmpty()
        );
      }

      columns[2].push_back(departure_time_element);
    }

    Element document = hbox({
      vbox(columns[0]),
      separatorEmpty(),
      separatorEmpty(),
      vbox(columns[1]) | flex,
      separatorEmpty(),
      vbox(columns[2])
    });

    screen.Clear();

    Render(screen, document);
    screen.Print();

    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
  }
}

Screen init_ui() {
  Screen screen = Screen::Create(Dimension::Full(), Dimension::Full());

  return screen;
}

void start_ui(Screen screen, const api::api_request request, int REFRESH_INTERVAL) {
  std::thread t(departure_list, screen, request, REFRESH_INTERVAL);
  t.join();
}
