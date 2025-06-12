#include <wil/app.hpp>
#include <wil/appimpl.hpp>
#include <wil/ecs.hpp>

class GameScene : public wil::Scene
{
public:

	wil::Registry registry;

	GameScene(wil::Device &device) : wil::Scene(device) {
		// registry.RegisterSystem<wil::RenderSystem>();
	}

	bool Update(wil::FrameData &frame) override {
		// auto &sync = GetDrawPresentSync(frame.index);
		// sync.AcquireImageIndex(&frame.image_index);
		// sync.GraphicsWaitIdle();
		// sync.SubmitDraw(registry.GetSystem<wil::RenderSystem>().Render(frame));
		// sync.PresentToScreen();
		return true;
	}
};

class Sandbox : public wil::App
{
public:

	void Init(wil::AppInitCtx &ctx) {
		ctx.window.size = {1600, 900};

	}

};
