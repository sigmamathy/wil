#pragma once

#include "display.hpp"
#include "device.hpp"
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
	std::string start_scene;
	std::string res_directory;

	template<class T, class... Ts>
	std::string_view NewScene(Ts&&... args) {
		auto f = [...args = std::forward<Ts>(args)](Device &dev)
				-> Scene* { return new T(dev, args...); };
		scenes_.emplace_back(std::make_pair(SceneInfo<T>::name, f));
		return SceneInfo<T>::name;
	}

private:
	std::vector<std::pair<std::string_view, Scene*(*)(Device&)>> scenes_;
	friend void appimpl(class App *app, int argc, char **argv);
};

struct FrameData
{
	uint32_t index;
	uint32_t image_index;
	float elapsed;
	float app_time;
};

class App
{
public:

	App();

	WIL_DELETE_COPY_AND_REASSIGNMENT(App);

	virtual void OnInit(AppInitCtx &ctx) = 0;

	virtual void OnWindowEvent(WindowEvent &ev) {}

	Window &GetWindow() { return *window_; }

	Device &GetDevice() { return *device_; }

	uint32_t GetFramesInFlight() const { return frames_in_flight_; }

private:

	Window *window_;
	Device *device_;
	bool active_;
	uint32_t frames_in_flight_;

	std::unordered_map<std::string, Scene*> scenes_;

	Scene* current_scene_;

	friend void appimpl(App *app, int argc, char **argv);

};

App& GetApp();

std::string GetResource(const std::string &path);

}
