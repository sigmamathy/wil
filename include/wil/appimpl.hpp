#pragma once

#include "app.hpp"

namespace wil {

void appimpl(App *app, int argc, char **argv);

#define WIL_IMPLEMENT_APP(c) int main(int argc, char **argv) { c app; wil::appimpl(&app, argc, argv); return 0; }

}
