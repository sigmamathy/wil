#include "wil/appimpl.hpp"
#include "wil/algebra.hpp"
#include "wil/scene.hpp"
#include "wil/layer.hpp"
#include "wil/buffer.hpp"
#include "wil/transform.hpp"
#include "wil/descriptor.hpp"
#include "wil/model.hpp"
#include "wil/log.hpp"

#include <GLFW/glfw3.h>
#include <cstring>

using wil::Fvec2;
using wil::Fvec3;

static std::vector<wil::LightVertex3D> vertices = {
	{{-0.5f, -0.5f, -0.5f}},
    {{0.5f, -0.5f, -0.5f}},
    {{0.5f, 0.5f, -0.5f}},
    {{-0.5f, 0.5f, -0.5f}},

	{{-0.5f, -0.5f, 0.5f}},
    {{0.5f, -0.5f, 0.5f}},
    {{0.5f, 0.5f, 0.5f}},
    {{-0.5f, 0.5f, 0.5f}},
};

static std::vector<unsigned> indices = {
	0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
	0, 3, 7, 0, 4, 7,
	1, 2, 5, 2, 5, 6,
	0, 1, 4, 1, 4, 5,
	2, 3, 6, 3, 6, 7,
};

class MyLayer : public wil::Layer3D
{
public:

	std::unique_ptr<wil::VertexBuffer> vb;
	std::unique_ptr<wil::IndexBuffer> ib;
	std::unique_ptr<wil::DescriptorPool> light_pool;
	std::vector<wil::DescriptorSet> light_uniform_sets;

	std::unique_ptr<wil::Model> model;

	std::unique_ptr<wil::DescriptorPool> pool;

	std::vector<wil::DescriptorSet> uniform_sets;
	std::vector<wil::DescriptorSet> tex_sets;

	std::vector<std::unique_ptr<wil::UniformBuffer>> uniforms;
	// std::unique_ptr<wil::Texture> texture;

	MyLayer(wil::Device &device) : wil::Layer3D(device)
	{
		uint32_t fif = wil::App::Instance()->GetFramesInFlight();

		vb = std::make_unique<wil::VertexBuffer>(device, vertices.size() * sizeof(wil::LightVertex3D));
		ib = std::make_unique<wil::IndexBuffer>(device, indices.size() * sizeof(unsigned));
		vb->MapData(vertices.data());
		ib->MapData(indices.data());

		light_pool = std::make_unique<wil::DescriptorPool>(GetLightPipeline(), std::vector{fif});
		light_uniform_sets = light_pool->AllocateSets(0, fif);

		model = std::make_unique<wil::Model>(device, "../../tests/Duck.glb", sizeof(wil::Vertex3D), [](void *data, Fvec3 pos, Fvec2 texcoord, Fvec3 normal){
			wil::Vertex3D v;
			v.pos = pos;
			v.texcoord = texcoord;
			v.normal = normal;
			std::memcpy(data, &v, sizeof(wil::Vertex3D));
		});

		pool = std::make_unique<wil::DescriptorPool>(GetPipeline(),
				std::vector<uint32_t>{fif, static_cast<uint32_t>(model->GetTextureCount())});

		uniform_sets = pool->AllocateSets(0, fif);
		tex_sets = pool->AllocateSets(1, model->GetTextureCount());

		// texture = std::make_unique<wil::Texture>(device, "../../tests/texture.jpg");
		for (int i = 0; i < model->GetTextureCount(); ++i)
			tex_sets[i].BindTexture(0, *model->GetTextures()[i]);

		for (int i = 0; i < fif; ++i) {
			uniforms.emplace_back(new wil::UniformBuffer(device, sizeof(wil::MVP3D)));
			uniform_sets[i].BindUniform(0, *uniforms[i]);
			light_uniform_sets[i].BindUniform(0, *uniforms[i]);
		}

	}

	wil::CommandBuffer &Render(uint32_t frame, uint32_t index) override
	{
		auto &cb = GetCommandBuffer(frame);
		cb.Reset();

		wil::MVP3D mvp;
		mvp.proj = wil::PerspectiveProjection(2.0944f, 16.f/9, .1f, 100.f);
		mvp.view = wil::LookAtView(Fvec3(0.0f, -1.f, -2.f), Fvec3(0.0f, 0.f, 1.f));

		uniforms[frame]->Update(&mvp);

		cb.RecordDraw(index, [this, frame, &mvp](wil::CmdDraw &cmd)
		{
			cmd.BindPipeline(GetPipeline());
			auto size = wil::App::Instance()->GetWindow().GetFramebufferSize();
			cmd.SetViewport({0, 0}, size);
			cmd.SetScissor({0, 0}, size);

			wil::Fmat4 mod = 
					wil::RotateModel(glfwGetTime(), wil::Fvec3(0.f, 1.f, 0.f))
					* wil::ScaleModel(1.f/120 * wil::Fvec3(1.f, -1.f, 1.f));

			cmd.PushConstant(GetPipeline(), &mod);

			for (int i = 0; i < model->GetMeshes().size(); ++i)
			{
				const wil::Mesh &m = model->GetMeshes()[i];
				wil::DescriptorSet sets[] = { uniform_sets[frame], tex_sets[m.material_index] };

				cmd.BindDescriptorSets(GetPipeline(), 0, sets, 2);
				cmd.BindVertexBuffer(*m.vertex_buffer);

				if (m.index_buffer) {
					cmd.BindIndexBuffer(*m.index_buffer);
					cmd.DrawIndexed(m.draw_count, 1);
				} else {
					cmd.Draw(m.draw_count, 1);
				}
			}

			cmd.BindPipeline(GetLightPipeline());
			cmd.BindDescriptorSets(GetLightPipeline(), 0, &light_uniform_sets[frame], 1);
			wil::LightPushConstant3D push;
			push.model = wil::RotateModel(-glfwGetTime(), Fvec3(0.f, 1.f, 0.f))
				* wil::TranslateModel({4.f,-1.f,0});
			push.light_color = {1.f, 1.f, 1.f, 1.f};

			cmd.PushConstant(GetLightPipeline(), &push);
			cmd.BindVertexBuffer(*vb);
			cmd.BindIndexBuffer(*ib);
			cmd.DrawIndexed(indices.size(), 1);

		});

		return cb;
	}

	std::string GetName() const override { return "MyLayer"; }
};

class MyScene : public wil::Scene
{
public:

	inline static const std::vector<std::string> LAYERS = {
		"MyLayer"
	};

	MyScene(wil::Device &dev) : wil::Scene(dev, LAYERS) {}

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
		ctx.NewScene<MyScene>();

		ctx.start_scene = "MyScene";
	}
};

WIL_IMPLEMENT_APP(MyApp)

