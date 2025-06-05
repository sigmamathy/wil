#pragma once

#include "display.hpp"
#include "device.hpp"
#include "layer.hpp"
#include "scene.hpp"
#include <string>
#include <unordered_map>

namespace wil {

struct AppInitCtx
{
	WindowCtor window = {};
	bool close_button_op = true;
	uint32_t frames_in_flight = 2;
	bool vsync = true;
	Scene *start_scene = nullptr;

	template<class T>
	T *NewLayer() requires std::is_base_of_v<Layer, T> {
		T *t = new T();
		(*layers_)[t->GetName()] = t;
		return t;
	}

	template<class T>
	T *NewScene() requires std::is_base_of_v<Scene, T> {
		T *t = new T();
		(*scenes_)[t->GetName()] = t;
		return t;
	}

private:
	std::unordered_map<std::string, Layer*> *layers_;
	std::unordered_map<std::string, Scene*> *scenes_;
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

	Layer *GetLayer(const std::string &name) { return layers_.at(name); }

private:

	AppInitCtx CreateAppInitCtx_();

	Window *window_;
	Device *device_;
	bool active_;
	uint32_t frames_in_flight_;

	std::unordered_map<std::string, Layer*> layers_;
	std::unordered_map<std::string, Scene*> scenes_;

	Scene* current_scene_;

	friend void appimpl(App *app, int argc, char **argv);

};

}
