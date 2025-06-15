#include <wil/app.hpp>
#include <wil/appimpl.hpp>
#include <wil/ecs.hpp>
#include <wil/render.hpp>
#include <wil/log.hpp>

WIL_SCENE_CLASS(GameScene)
{
public:

	std::vector<wil::CommandBuffer> cmdbufs;
	wil::Registry registry;

	GameScene(wil::Device &device) : wil::Scene(device)
	{
		for (uint32_t i = 0; i < wil::GetApp().GetFramesInFlight(); ++i)
			cmdbufs.emplace_back(device);
		registry.RegisterSystem<wil::RenderSystem>(device);

		wil::TransformComponent tc = {
			{0, 0, 0},
			{0.01f, 0.01f, 0.01f},
			{0, 1, 0},
			0.f
		};

		wil::ModelComponent mc = {
			"../../tests/Duck.glb"
		};

		auto e1 = registry.CreateEntity();
		registry.AddComponents(e1, tc, mc);

		tc.position = {2, 0, 0};

		auto e2 = registry.CreateEntity();
		registry.AddComponents(e2, tc, mc);
	}

	bool Update(wil::FrameData &frame) override
	{
		auto &sync = GetDrawPresentSynchronizer(frame.index);
		sync.AcquireImageIndex(&frame.image_index);

		std::vector<wil::CommandBuffer*> cbs;
		cbs.emplace_back(&cmdbufs[frame.index]);
		registry.GetSystem<wil::RenderSystem>().Render(cmdbufs[frame.index], frame);
		wil::GetApp().GetDevice().GetGraphicsQueue().WaitIdle();
		sync.SubmitDraw(cbs);
		sync.PresentToScreen(frame.image_index);
		return true;
	}
};

class Sandbox : public wil::App
{
public:

	void OnInit(wil::AppInitCtx &ctx) override
	{
		ctx.window.size = {1600, 900};
		ctx.window.title = "My App";
		ctx.start_scene = ctx.NewScene<GameScene>();
	}

};

WIL_IMPLEMENT_APP(Sandbox);
