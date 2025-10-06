#include <algorithm>
#include <cstddef>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <thread>
#include <fmt/format.h>

#include "utils.hpp"
#include "ui.hpp"

std::string get_delay(int drt) {
  if (drt == 0) return "";

  return fmt::format("{:+}", drt);
}

Element cell_element(Elements content, Color bg_color = Color::Black, Color fg_color = Color::White, bool is_wide = false, bool is_tall = false) {
  return (
    bgcolor(
      bg_color,
      color(
        fg_color,
        vbox(
          is_tall ? text(" ") : emptyElement(),
          hbox(
            is_wide ? text(" ") : emptyElement(),
            text(" "),
            content,
            text(" "),
            is_wide ? text(" ") : emptyElement()
          ),
          is_tall ? text(" ") : emptyElement()
        )
      )
    )
  );
}

Element text_element(std::string content, Color bg_color = Color::Black, Color fg_color = Color::White, bool is_bold = false, bool is_wide = false, bool is_tall = false) {
  return cell_element({ is_bold ? text(content) | bold : text(content)}, bg_color, fg_color, is_wide, is_tall);
}

Element time_element(std::string time, std::string delay = "", bool is_realtime = false, bool is_soon = false) {
  return cell_element(
    {
      is_realtime ? text("* ") : text("  "),
      (is_soon ? bold(text(time)) : text(time)) | flex,
      text(" "),
      delay.empty() ? text("    ") : color(Color::Red, text(fmt::format("{:4}", delay.substr(0, 4)))),
    },
    Color::Black, Color::White,
    false,
    is_soon
  );
}

void add_separator_line(std::vector<Elements>* lines) {
  (*lines).push_back({separatorLight(), separatorCharacter("─"), separatorLight() | flex, separatorCharacter("─"), separatorLight()});
}

void add_line(std::vector<Elements>* lines, Element name, Element direction, Element time, bool is_cancelled = false) {
  (*lines).push_back({
    name,
    separatorEmpty(),
    vbox(
      filler(),
      is_cancelled ? (direction | strikethrough | dim) : direction,
      filler()
    ) | flex,
    separatorEmpty(),
    vbox(
      filler(),
      is_cancelled ? color(Color::Red, text("cancelled ")) : time,
      filler()
    )
  });
}

Element get_document(std::vector<Elements> lines, std::string timestring) {
  return vbox({
    bgcolor(
      Color::White,
      color(
        Color::Black,
        hbox({
          filler(),
          text(timestring),
          filler(),
        })
      )
    ),
    gridbox(lines)
  });
}

void refresh_screen(Screen& screen, std::vector<Elements> lines, tm current_time) {
  screen.Clear();

  std::ostringstream oss;
  oss.imbue(std::locale("de_DE.utf-8"));
  oss << std::put_time(&current_time, "%H:%M");

  Render(screen, get_document(lines, oss.str()));
  screen.Print();
}

typedef struct Departure {
  std::string name;
  Color bg_color;
  Color fg_color;
  std::string direction;
  std::string time;
  bool is_realtime;
  bool is_cancelled;
  int dmin;
  int drt;
} Departure;

void departure_list(Screen screen, const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL, int NUM_LINES) {
  while (true) {
    std::time_t now_tt = time(nullptr);
    struct tm now_c = *localtime(&now_tt);

    api::api_response res = api::get(api_config, request);

    if (res.error) {
      fmt::print("{}", res.error);
      continue;
    }

    json fetched_departures = res.data["Departure"];
    Departure departures[NUM_LINES];

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

      if (!is_soon && i-1 > 0 && departures[i-1].dmin <= 10) {
        add_separator_line(&lines);
      }

      add_line(&lines, name, direction, departure_time_element, departure.is_cancelled);
    }

    refresh_screen(screen, lines, now_c);

    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
  }
}

Screen init_ui() {
  Screen screen = Screen::Create(Dimension::Full(), Dimension::Full());

  return screen;
}

void start_ui(Screen screen, const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL, int NUM_LINES) {
  std::thread t(departure_list, screen, api_config, request, REFRESH_INTERVAL, NUM_LINES);
  t.join();
}
