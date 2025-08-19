#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>

#include "api.hpp"

using namespace ftxui;

void departure_list(Screen screen, const api::api_request request, int REFRESH_INTERVAL);

Screen init_ui();

void start_ui(Screen screen, const api::api_request request, int REFRESH_INTERVAL);
