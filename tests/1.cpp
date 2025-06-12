#include "wil/appimpl.hpp"
#include "wil/algebra.hpp"
#include "wil/scene.hpp"
#include "wil/pipeline.hpp"
#include "wil/buffer.hpp"
#include "wil/transform.hpp"
#include "wil/descriptor.hpp"
#include "wil/model.hpp"
#include "wil/log.hpp"

#include <GLFW/glfw3.h>
#include <cstring>
#include <memory>

using namespace wil::algebra;
using enum wil::ShaderStageBit;
using enum wil::DescriptorType;

struct Vertex3D
{
	Fvec3 pos;
	Fvec2 texcoord;
	Fvec3 normal;
};

struct LightVertex3D
{
	Fvec3 pos;
};

struct LightPushConstant3D
{
	Fmat4 model;
	Fvec4 light_color; 
};

struct LightUniform {
	alignas(16) Fvec3 pos;
	alignas(16) Fvec3 color;
};

struct GlobalData3D
{
	alignas(16) Fmat4 view;
	alignas(16) Fmat4 proj;
	alignas(16) Fvec3 view_pos;
};

static std::vector<LightVertex3D> vertices = {
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

class MyScene : public wil::Scene
{
public:

	std::vector<wil::CommandBuffer> cmd_buffers;

	std::unique_ptr<wil::Pipeline> pipeline_, light_pipeline_;

	wil::VertexBuffer vb;
	wil::IndexBuffer ib;
	std::unique_ptr<wil::DescriptorPool> light_pool;
	std::vector<wil::DescriptorSet> light_uniform_sets;

	std::unique_ptr<wil::Model> model;

	std::unique_ptr<wil::DescriptorPool> pool;

	std::vector<wil::DescriptorSet> uniform_sets;
	std::vector<wil::DescriptorSet> tex_sets;

	std::vector<wil::UniformBuffer> uniforms;
	std::vector<wil::UniformBuffer> light_pos_uniforms;

	void MakePipelines(wil::Device &device)
	{
		for (uint32_t i = 0; i < wil::GetApp().GetFramesInFlight(); ++i)
			cmd_buffers.emplace_back(device);

		wil::PipelineCtor ctor;
		ctor.device = &device;

		ctor.vertex_shader = "../shaders/3d.vert.spv";
		ctor.fragment_shader = "../shaders/3d.frag.spv";

		ctor.vertex_layout.push_back(wilvrta(0, Vertex3D, pos));
		ctor.vertex_layout.push_back(wilvrta(1, Vertex3D, texcoord));
		ctor.vertex_layout.push_back(wilvrta(2, Vertex3D, normal));
		ctor.vertex_stride = sizeof(Vertex3D);

		ctor.push_constant_stage = wil::VERTEX_SHADER;
		ctor.push_constant_size = sizeof(Fmat4);

		ctor.descriptor_set_layouts.resize(2);
		ctor.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER | FRAGMENT_SHADER);
		ctor.descriptor_set_layouts[0].Add(1, UNIFORM_BUFFER, FRAGMENT_SHADER);
		ctor.descriptor_set_layouts[1].Add(0, COMBINED_IMAGE_SAMPLER, FRAGMENT_SHADER);

		pipeline_ = std::make_unique<wil::Pipeline>(ctor);

		wil::PipelineCtor lct;
		lct.device = &device;

		lct.vertex_shader = "../shaders/light3d.vert.spv";
		lct.fragment_shader = "../shaders/light3d.frag.spv";

		lct.vertex_layout.push_back(wilvrta(0, LightVertex3D, pos));
		lct.vertex_stride = sizeof(LightVertex3D);

		lct.push_constant_stage = VERTEX_SHADER | FRAGMENT_SHADER;
		lct.push_constant_size = sizeof(LightPushConstant3D);

		lct.descriptor_set_layouts.resize(1);
		lct.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER);

		light_pipeline_ = std::make_unique<wil::Pipeline>(lct);
	}

	MyScene(wil::Device &device) : wil::Scene(device)
	{
		MakePipelines(device);

		uint32_t fif = wil::GetApp().GetFramesInFlight();

		vb = wil::VertexBuffer(device, vertices.size() * sizeof(LightVertex3D));
		ib = wil::IndexBuffer(device, indices.size() * sizeof(unsigned));
		vb.MapData(vertices.data());
		ib.MapData(indices.data());

		light_pool = std::make_unique<wil::DescriptorPool>(*light_pipeline_, std::vector{fif});
		light_uniform_sets = light_pool->AllocateSets(0, fif);

		model = std::make_unique<wil::Model>(device, "../../tests/Duck.glb", sizeof(Vertex3D), [](void *data, Fvec3 pos, Fvec2 texcoord, Fvec3 normal){
			Vertex3D v;
			v.pos = pos;
			v.texcoord = texcoord;
			v.normal = normal;
			std::memcpy(data, &v, sizeof(Vertex3D));
		});

		pool = std::make_unique<wil::DescriptorPool>(*pipeline_,
				std::vector<uint32_t>{fif, static_cast<uint32_t>(model->GetTextureCount())});

		uniform_sets = pool->AllocateSets(0, fif);
		tex_sets = pool->AllocateSets(1, model->GetTextureCount());

		// texture = std::make_unique<wil::Texture>(device, "../../tests/texture.jpg");
		for (int i = 0; i < model->GetTextureCount(); ++i)
			tex_sets[i].BindTexture(0, model->GetTextures()[i]);

		uniforms.reserve(fif);
		light_pos_uniforms.reserve(fif);

		for (int i = 0; i < fif; ++i) {
			uniforms.emplace_back(wil::UniformBuffer(device, sizeof(GlobalData3D)));
			uniform_sets[i].BindUniform(0, uniforms[i]);
			light_uniform_sets[i].BindUniform(0, uniforms[i]);

			light_pos_uniforms.emplace_back(wil::UniformBuffer(device, sizeof(LightUniform)));
			uniform_sets[i].BindUniform(1, light_pos_uniforms[i]);
		}
	}

	bool Update(wil::FrameData &frame) override
	{
		auto &sync = GetDrawPresentSynchronizer(frame.index);
		sync.AcquireImageIndex(&frame.image_index);

		std::vector<wil::CommandBuffer*> cmdbuf;
		cmdbuf.emplace_back(&cmd_buffers[frame.index]);
		Render(cmd_buffers[frame.index], frame);

		wil::GetApp().GetDevice().GetGraphicsQueue().WaitIdle();
		sync.SubmitDraw(cmdbuf);
		sync.PresentToScreen(frame.image_index);
		return true;
	}

	void Render(wil::CommandBuffer &cb, wil::FrameData &frame)
	{
		cb.Reset();

		GlobalData3D mvp;
		mvp.proj = wil::PerspectiveProjection(2.0944f, 16.f/9, .1f, 100.f);
		mvp.view_pos = Fvec3(0.0f, -2.f, -2.f);
		mvp.view = wil::LookAtView(mvp.view_pos, Fvec3(0.0f, 0.5f, 1.f));

		uniforms[frame.index].Update(&mvp);

		LightUniform light;
		light.pos = Fvec3(2 * std::cos(glfwGetTime()), -1.f, 2 * std::sin(glfwGetTime()));
		light.color = Fvec3(1.f, (std::sin(glfwGetTime() * 0.7f) + 0.5f) / 2, 0.7f);

		light_pos_uniforms[frame.index].Update(&light);

		cb.RecordDraw(frame.image_index, [this, frame, &mvp, &light](wil::CmdDraw &cmd)
		{
			cmd.BindPipeline(*pipeline_);
			auto size = wil::GetApp().GetWindow().GetFramebufferSize();
			cmd.SetViewport({0, 0}, size);
			cmd.SetScissor({0, 0}, size);

			wil::Fmat4 mod = 
					wil::RotateModel(glfwGetTime() * 2, wil::Fvec3(0.f, 1.f, 0.f))
					* wil::ScaleModel(1.f/120 * wil::Fvec3(1.f, -1.f, 1.f));

			cmd.PushConstant(*pipeline_, &mod);

			for (int i = 0; i < model->GetMeshes().size(); ++i)
			{
				const wil::Mesh &m = model->GetMeshes()[i];
				wil::DescriptorSet sets[] = { uniform_sets[frame.index], tex_sets[m.material_index] };

				cmd.BindDescriptorSets(*pipeline_, 0, sets, 2);
				cmd.BindVertexBuffer(m.vertex_buffer);

				if (m.index_buffer) {
					cmd.BindIndexBuffer(m.index_buffer.value());
					cmd.DrawIndexed(m.draw_count, 1);
				} else {
					cmd.Draw(m.draw_count, 1);
				}
			}

			cmd.BindPipeline(*light_pipeline_);
			cmd.BindDescriptorSets(*light_pipeline_, 0, &light_uniform_sets[frame.index], 1);
			LightPushConstant3D push;
			push.model = wil::TranslateModel(light.pos);
			push.light_color = light.color & 1.f;

			cmd.PushConstant(*light_pipeline_, &push);
			cmd.BindVertexBuffer(vb);
			cmd.BindIndexBuffer(ib);
			cmd.DrawIndexed(indices.size(), 1);
		});
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

		ctx.NewScene<MyScene>();

		ctx.start_scene = "MyScene";
	}
};

WIL_IMPLEMENT_APP(MyApp)

