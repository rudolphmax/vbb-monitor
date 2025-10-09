#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>

using namespace ftxui;

Screen* init_ui();

void draw_departure_screen(std::vector<Elements> lines, tm current_time);

void draw_error_screen(std::string message);

std::string get_delay(int drt);

Element cell_element(Elements content, Color bg_color = Color::Black, Color fg_color = Color::White, bool is_wide = false, bool is_tall = false);

Element text_element(std::string content, Color bg_color = Color::Black, Color fg_color = Color::White, bool is_bold = false, bool is_wide = false, bool is_tall = false);

Element time_element(std::string time, std::string delay = "", bool is_realtime = false, bool is_soon = false);

void add_separator_line(std::vector<Elements>* lines);

void add_line(std::vector<Elements>* lines, Element name, Element direction, Element time, bool is_cancelled = false);
