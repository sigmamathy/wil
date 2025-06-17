#include <wil/app.hpp>
#include <wil/appimpl.hpp>
#include <wil/ecs.hpp>
#include <wil/render.hpp>
#include <wil/log.hpp>

using namespace wil::algebra;
using enum wil::WindowEventType;
using enum wil::KeyCode;
using enum wil::MouseButton;

WIL_SCENE_CLASS(GameScene)
{
public:

	std::vector<wil::CommandBuffer> cmdbufs;
	wil::Registry registry;

	wil::Entity e4;

	GameScene(wil::Device &device) : wil::Scene(device)
	{
		SubscribeEvent([this](auto &ev){OnInput(ev);},
				KEY_EVENT | MOUSE_EVENT | CURSOR_EVENT);

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

		auto &win = wil::GetApp().GetWindow();
		auto &cam = registry.GetSystem<wil::RenderSystem>().GetCamera();

		if (win.IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
		{
			float val = frame.elapsed * 5.f;
			if (win.IsKeyPressed(KEY_W)) cam.MoveStraightNoUp(val);
			if (win.IsKeyPressed(KEY_S)) cam.MoveStraightNoUp(-val);
			if (win.IsKeyPressed(KEY_D)) cam.MoveSideway(val);
			if (win.IsKeyPressed(KEY_A)) cam.MoveSideway(-val);
			if (win.IsKeyPressed(KEY_SPACE)) cam.MoveUp(val);
			if (win.IsKeyPressed(KEY_LEFT_CONTROL)) cam.MoveUp(-val);
		}

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

	void OnInput(wil::WindowEvent &ev)
	{
		auto &win = wil::GetApp().GetWindow();
		auto &cam = registry.GetSystem<wil::RenderSystem>().GetCamera();

		if (ev.type == MOUSE_EVENT && ev.me.button == MOUSE_BUTTON_MIDDLE) {
			win.SetCursorEnable(!win.IsCursorEnabled());
		}

		if (ev.type == CURSOR_EVENT && win.IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
		{
			cam.h_angle += 0.002f * ev.ce.delta.x;
			cam.v_angle -= 0.002f * ev.ce.delta.y;
			if (cam.v_angle > 1.57f) cam.v_angle = 1.57f;
			if (cam.v_angle < -1.57f) cam.v_angle = -1.57f;
		}
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

		ctx.res_directory = "../../res";
		ctx.start_scene = ctx.NewScene<GameScene>();
	}

};

WIL_IMPLEMENT_APP(Sandbox);
