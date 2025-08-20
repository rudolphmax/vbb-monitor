#include <ftxui/dom/node.hpp>
#include <thread>
#include <chrono>
#include <fmt/format.h>

#include "ui.hpp"

Element cell_element(Elements content, Color bg_color = Color::Black, Color fg_color = Color::White) {
  return (
    bgcolor(
      bg_color,
      color(
        fg_color,
        hbox(
          separatorEmpty(),
          content,
          separatorEmpty()
        )
      )
    )
  );
}

Element text_element(std::string content, Color bg_color = Color::Black, Color fg_color = Color::White) {
  return cell_element({ text(content) }, bg_color, fg_color);
}

Element time_element(std::string time, bool is_realtime = false, bool is_soon = false) {
  return cell_element({
    is_soon ? bold(text(time)) : text(time) | flex,
    is_realtime ? text(" *") : text(" "),
  });
}

void add_line(Elements columns[3], Element name, Element direction, Element time) {
  columns[0].push_back(name);
  columns[1].push_back(direction);
  columns[2].push_back(time);
}

Element get_document(Elements columns[3]) {
  return (
    hbox({
      vbox(columns[0]),
      separatorEmpty(),
      separatorEmpty(),
      vbox(columns[1]) | flex,
      separatorEmpty(),
      vbox(columns[2])
    })
  );
}

void refresh_screen(Screen& screen, Elements columns[3]) {
  screen.Clear();

  Render(screen, get_document(columns));
  screen.Print();
}

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

      auto name = text_element(
        departure["name"],
        Color::RGB(departure_bg_color["r"], departure_bg_color["g"], departure_bg_color["b"]),
        Color::RGB(departure_fg_color["r"], departure_fg_color["g"], departure_fg_color["b"])
      );

      auto direction = text_element((std::string) departure["direction"]);

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
        departure_time_element = time_element(
          "now\n",
          is_realtime,
          true
        );

      } else if (dmin <= 10) {
        departure_time_element = time_element(
          fmt::format("{}\n", dmin),
          is_realtime,
          true
        );

      } else {
        departure_time_element = time_element(
          fmt::format("{}:{}\n", tm.tm_hour - 1, tm.tm_min),
          is_realtime
        );
      }

      add_line(columns, name, direction, departure_time_element);
    }

    refresh_screen(screen, columns);

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
