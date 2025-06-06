#include "wil/appimpl.hpp"
#include "wil/algebra.hpp"
#include "wil/scene.hpp"
#include "wil/layer.hpp"
#include "wil/buffer.hpp"
#include "wil/transform.hpp"

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
		wil::Layer3D::OnInit(device);
		vb = std::make_unique<wil::VertexBuffer>(device, vertices.size() * sizeof(wil::Vertex3D));
		vb->MapData(vertices.data());
		ib = std::make_unique<wil::IndexBuffer>(device, indices.size() * sizeof(unsigned));
		ib->MapData(indices.data());
	}

	wil::CommandBuffer &Render(uint32_t frame, uint32_t index) override
	{
		auto &cb = GetCommandBuffer(frame);
		cb.Reset();

		wil::MVP3D mvp;
		mvp.proj = wil::Transpose(wil::PerspectiveProjection(2.0944f, 16.f/9, .1f, 100.f));
		mvp.view = wil::Transpose(wil::LookAtView(wil::Fvec3(0.5f, 0.f, -1.f), wil::Fvec3(-0.2f, 0.f, 1.f)));
		mvp.model = wil::Transpose(wil::RotateModel(2.f));

		GetDescriptorSet(frame, 0).GetUniform(0).Update(&mvp);

		cb.RecordDraw(index, [this, frame](wil::CmdDraw &cmd){
			cmd.BindPipeline(GetPipeline());
			cmd.BindDescriptorSet(GetPipeline(), GetDescriptorSet(frame, 0));
			cmd.BindVertexBuffer(*vb);
			cmd.BindIndexBuffer(*ib);
			auto size = wil::App::Instance()->GetWindow().GetFramebufferSize();
			cmd.SetViewport({0, 0}, size);
			cmd.SetScissor({0, 0}, size);
			cmd.DrawIndexed(6, 1);
		});

		return cb;
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
		ctx.window.resizable = true;
		ctx.NewLayer<MyLayer>();
		ctx.start_scene = ctx.NewScene<MyScene>();
	}
};

int main(int argc, char **argv) {
	MyApp app;
	wil::appimpl(&app, argc, argv);
	return 0;
}
