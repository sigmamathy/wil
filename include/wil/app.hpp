#pragma once

#include "display.hpp"
#include "device.hpp"

namespace wil {

struct AppInitCtx
{
	WindowCtor window = {};
	bool close_button_op = true;
};

class App
{
public:

	App();

	virtual void OnInit(AppInitCtx &ctx) {}

	virtual void OnWindowEvent(WindowEvent &ev) {}

	Window &GetWindow() { return *window_; }

private:

	Window *window_;
	Device *device_;
	bool active_;

	friend void appimpl(App *app, int argc, char **argv);

};

}
