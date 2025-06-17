#include <wil/app.hpp>
#include <wil/appimpl.hpp>
#include <wil/ecs.hpp>
#include <wil/render.hpp>
#include <wil/log.hpp>

using namespace wil::algebra;

WIL_SCENE_CLASS(GameScene)
{
public:

	std::vector<wil::CommandBuffer> cmdbufs;
	wil::Registry registry;

	wil::Entity e4;

	GameScene(wil::Device &device) : wil::Scene(device)
	{
		SubscribeEvent([this](auto &ev){OnKeyPressed(ev);}, wil::KEY_EVENT);

		for (uint32_t i = 0; i < wil::GetApp().GetFramesInFlight(); ++i)
			cmdbufs.emplace_back(device);
		registry.RegisterSystem<wil::RenderSystem>(device);

		wil::TransformComponent tc = {
			{-2, 0, 0},
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

		wil::LightComponent lc = {
			{1.f, 1.f, 1.f},
			0.22f,
			0.20f
		};

		tc.position = {0.f, 3.f, 0.f};
		tc.size = {1.f,1.f,1.f};

		auto e3 = registry.CreateEntity();
		registry.AddComponents(e3, tc, lc);

		tc.position = {5.f, 1.f, 0.f};
		lc.color = {1.f, 0.f, 0.f};

		e4 = registry.CreateEntity();
		registry.AddComponents(e4, tc, lc);

	}

	bool Update(wil::FrameData &frame) override
	{
		auto &sync = GetDrawPresentSynchronizer(frame.index);
		sync.AcquireImageIndex(&frame.image_index);

		Fvec3 light_pos = {5 * cos(frame.app_time), 1.f, 5 * sin(frame.app_time)};
		registry.GetComponent<wil::TransformComponent>(e4).position = light_pos;

		std::vector<wil::CommandBuffer*> cbs;
		cbs.emplace_back(&cmdbufs[frame.index]);
		registry.GetSystem<wil::RenderSystem>().Render(cmdbufs[frame.index], frame);
		wil::GetApp().GetDevice().GetGraphicsQueue().WaitIdle();
		sync.SubmitDraw(cbs);
		sync.PresentToScreen(frame.image_index);
		return true;
	}

	void OnKeyPressed(wil::WindowEvent &ev)
	{
		if (ev.ke.down && (ev.ke.mods & wil::KEYMOD_SHIFT) && ev.ke.code == wil::KEY_W)
			WIL_LOGINFO("Crazy");
	}
};

class Sandbox : public wil::App
{
public:

	void OnInit(wil::AppInitCtx &ctx) override
	{
		// ctx.window.size = {1600, 900};
		// ctx.window.title = "My App";
		ctx.window.size = WIL_MONITOR_SIZE;
		ctx.window.monitor = 0;

		ctx.start_scene = ctx.NewScene<GameScene>();
	}

};

WIL_IMPLEMENT_APP(Sandbox);
