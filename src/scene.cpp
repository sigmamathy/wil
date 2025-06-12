#include <wil/scene.hpp>
#include <wil/app.hpp>

namespace wil {

Scene::Scene(Device &device)
{
	uint32_t fif = GetApp().GetFramesInFlight();
	syncs_.reserve(fif);
	for (uint32_t i = 0; i < fif; ++i)
		syncs_.emplace_back(new DrawPresentSynchronizer(device, 1));
}

Scene::~Scene()
{
	for (auto sync: syncs_)
		delete sync;
}

}
