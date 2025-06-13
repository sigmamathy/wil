#pragma once

#include "app.hpp"

namespace wil {

void appimpl(App *app, int argc, char **argv);

#define WIL_IMPLEMENT_APP(c) int main(int argc, char **argv) {\
	static_assert(std::is_base_of_v<::wil::App, c>, #c " must be derived from wil::App");\
	c app; ::wil::appimpl(&app, argc, argv); return 0; }

}
