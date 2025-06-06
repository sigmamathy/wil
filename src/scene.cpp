#include <wil/scene.hpp>
#include <wil/app.hpp>

namespace wil {

Scene::Scene(Device &device, const std::vector<std::string> &layers)
{
	layers_.reserve(layers.size());
	for (auto l : layers)
		layers_.emplace_back(App::Instance()->GetLayer(l));

	uint32_t fif = App::Instance()->GetFramesInFlight();
	syncs_.reserve(fif);
	for (uint32_t i = 0; i < fif; ++i)
		syncs_.emplace_back(new DrawPresentSynchronizer(device, layers_.size()));
}

Scene::~Scene()
{
	for (auto sync: syncs_)
		delete sync;
}

bool Scene::Render(uint32_t frame)
{
	uint32_t index;
	if (!syncs_[frame]->AcquireImageIndex(&index))
		return false;
	std::vector<CommandBuffer*> cbs;
	cbs.reserve(layers_.size());
	for (auto layer : layers_)
		cbs.push_back(&layer->Render(frame, index));
	App::Instance()->GetDevice().GetGraphicsQueue().WaitIdle();
	syncs_[frame]->SubmitDraw(cbs);
	return syncs_[frame]->PresentToScreen(index);
}

}
