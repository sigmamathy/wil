#include "wil/appimpl.hpp"
#include "wil/algebra.hpp"
#include "wil/scene.hpp"
#include "wil/layer.hpp"
#include "wil/buffer.hpp"

static std::array vertices = {
	wil::Vertex3D{wil::Fvec2(-0.5f, 0.5f)},
	wil::Vertex3D{wil::Fvec2(0.5f, 0.5f)},
	wil::Vertex3D{wil::Fvec2(0.5f, -0.5f)},
	wil::Vertex3D{wil::Fvec2(-0.5f, -0.5f)},
};

static std::array<unsigned, 6> indices = {
	0, 1, 2,
	0, 2, 3,
};

class MyLayer : public wil::Layer3D
{
public:

	std::unique_ptr<wil::VertexBuffer> vb;
	std::unique_ptr<wil::IndexBuffer> ib;

	void OnInit(wil::Device &device) override {
		vb = std::make_unique<wil::VertexBuffer>(device, vertices.size() * sizeof(wil::Vertex3D));
		vb->MapData(vertices.data());
		ib = std::make_unique<wil::IndexBuffer>(device, indices.size() * sizeof(unsigned));
		ib->MapData(indices.data());
	}

	void OnRender(wil::CommandBuffer &cb, uint32_t index) override
	{
		cb.Reset();

		cb.RecordDraw(index, [this](wil::CmdDraw &cmd){
			cmd.BindPipeline(GetPipeline());
			cmd.BindVertexBuffer(*vb);
			cmd.BindIndexBuffer(*ib);
			auto size = wil::App::Instance()->GetWindow().GetFramebufferSize();
			cmd.SetViewport({0, 0}, size);
			cmd.SetScissor({0, 0}, size);
			cmd.DrawIndexed(6, 1);
		});
	}

	std::string GetName() const override { return "MyLayer"; }
};

class MyScene : public wil::Scene
{
public:

	std::vector<std::string> SetupLayers() const override {
		return {
			"MyLayer"
		};
	}

	std::string GetName() const override { return "MyScene"; }
};

class MyApp : public wil::App
{
public:
	void OnInit(wil::AppInitCtx &ctx) override
	{
		ctx.window.size = {1600, 900};
		ctx.window.title = "MyApp";
		ctx.NewLayer<MyLayer>();
		ctx.start_scene = ctx.NewScene<MyScene>();
	}
};

int main(int argc, char **argv) {
	MyApp app;
	wil::appimpl(&app, argc, argv);
	return 0;
}
