#pragma once

#include "display.hpp"
#include "device.hpp"
#include "layer.hpp"
#include <string>
#include <unordered_map>

namespace wil {

struct AppInitCtx
{
	WindowCtor window = {};
	bool close_button_op = true;
	uint32_t frames_in_flight = 2;

	template<class T>
	void NewLayer() requires std::is_base_of_v<Layer, T> {
		T *t = new T();
		(*layers_)[t->GetName()] = t;
	}

private:
	std::unordered_map<std::string, Layer*> *layers_;
	friend class App;
};

class App
{
public:

	App();

	WIL_DELETE_COPY_AND_REASSIGNMENT(App);

	static App *Instance();

	virtual void OnInit(AppInitCtx &ctx) {}

	virtual void OnWindowEvent(WindowEvent &ev) {}

	Window &GetWindow() { return *window_; }

	Device &GetDevice() { return *device_; }

	uint32_t GetFramesInFlight() const { return frames_in_flight_; }

private:

	AppInitCtx CreateAppInitCtx_();

	Window *window_;
	Device *device_;
	bool active_;
	uint32_t frames_in_flight_;

	std::unordered_map<std::string, Layer*> layers_;

	friend void appimpl(App *app, int argc, char **argv);

};

}
