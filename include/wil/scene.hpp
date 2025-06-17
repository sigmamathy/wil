#pragma once

#include "core.hpp"
#include "drawsync.hpp"
#include <vector>

namespace wil {

template<class T>
struct SceneInfo;

#define WIL_SCENE_CLASS(c) class c;\
		template<> struct wil::SceneInfo<c>{ static constexpr std::string_view name = #c; };\
		class c : public ::wil::Scene

class Scene
{
public:

	Scene(Device &device);

	virtual ~Scene();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Scene);

	DrawPresentSynchronizer &GetDrawPresentSynchronizer(uint32_t frame_index) {
		return *syncs_[frame_index];
	}

	void HandleEvent(WindowEvent &ev);

	void SubscribeEvent(const WindowEventHandler &handler, WindowEventType filter) {
		event_subscriptions_.emplace_back(std::make_pair(filter, handler));
	}

	virtual bool Update(struct FrameData &frame) = 0;

private:

	std::vector<DrawPresentSynchronizer*> syncs_;
	std::vector<std::pair<WindowEventType, WindowEventHandler>> event_subscriptions_;

};

}
