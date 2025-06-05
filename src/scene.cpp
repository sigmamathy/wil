#include <wil/scene.hpp>
#include <wil/app.hpp>

namespace wil {

void Scene::Init(Device &device)
{
	auto lns = SetupLayers();
	layers_.reserve(lns.size());
	for (auto l : lns)
		layers_.emplace_back(App::Instance()->GetLayer(l));

	uint32_t fif = App::Instance()->GetFramesInFlight();
	syncs_.reserve(fif);
	for (uint32_t i = 0; i < fif; ++i)
		syncs_.emplace_back(new DrawPresentSynchronizer(device, layers_.size()));

	OnInit(device);
}

void Scene::Free()
{
	for (auto sync: syncs_)
		delete sync;
}

void Scene::Render(uint32_t frame)
{
	uint32_t index = syncs_[frame]->AcquireImageIndex();
	std::vector<CommandBuffer*> cbs;
	cbs.reserve(layers_.size());
	for (auto layer : layers_)
		cbs.push_back(&layer->Render(frame, index));
	App::Instance()->GetDevice().GetGraphicsQueue().WaitIdle();
	syncs_[frame]->SubmitDraw(cbs);
	syncs_[frame]->PresentToScreen(index);	
}

}
