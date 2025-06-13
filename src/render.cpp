#include <wil/render.hpp>
#include <wil/app.hpp>
#include <wil/transform.hpp>
#include <wil/log.hpp>

namespace wil {

RenderSystem::RenderSystem(Registry& registry, Device &device)
	: System(registry), registry_(registry), device_(device)
{
	registry.RegisterEntityView<TransformComponent, ModelComponent>(objects_);
	registry.RegisterEntityView<LightComponent>(lights_);

	CreatePipelines_(device);
	CreateDescriptorSetsAndUniforms_(device);
}

void RenderSystem::CreatePipelines_(Device &device)
{
	PipelineCtor octor;
	octor.device = &device;

	octor.vertex_shader = "../shaders/3d.vert.spv";
	octor.fragment_shader = "../shaders/3d.frag.spv";

	octor.vertex_layout.push_back(wilvrta(0, ObjectVertex, pos));
	octor.vertex_layout.push_back(wilvrta(1, ObjectVertex, texcoord));
	octor.vertex_layout.push_back(wilvrta(2, ObjectVertex, normal));
	octor.vertex_stride = sizeof(ObjectVertex);

	octor.push_constant_stage = wil::VERTEX_SHADER;
	octor.push_constant_size = sizeof(ObjectPushConstant);

	octor.descriptor_set_layouts.resize(2);
	octor.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER | FRAGMENT_SHADER);
	octor.descriptor_set_layouts[0].Add(1, UNIFORM_BUFFER, FRAGMENT_SHADER);
	octor.descriptor_set_layouts[1].Add(0, COMBINED_IMAGE_SAMPLER, FRAGMENT_SHADER);

	object_pipeline_ = std::make_unique<Pipeline>(octor);

	PipelineCtor lctor;
	lctor.device = &device;

	lctor.vertex_shader = "../shaders/light3d.vert.spv";
	lctor.fragment_shader = "../shaders/light3d.frag.spv";

	lctor.vertex_layout.push_back(wilvrta(0, LightVertex, pos));
	lctor.vertex_stride = sizeof(LightVertex);

	lctor.push_constant_stage = VERTEX_SHADER | FRAGMENT_SHADER;
	lctor.push_constant_size = sizeof(LightPushConstant);

	lctor.descriptor_set_layouts.resize(1);
	lctor.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER);

	light_pipeline_ = std::make_unique<Pipeline>(lctor);
}

void RenderSystem::CreateDescriptorSetsAndUniforms_(Device &device)
{
	uint32_t fif = GetApp().GetFramesInFlight();

	object_pool_ = std::make_unique<wil::DescriptorPool>(*object_pipeline_, std::vector<uint32_t>{fif, 100});
	light_pool_ = std::make_unique<wil::DescriptorPool>(*light_pipeline_, std::vector{fif});

	object_0_sets.resize(fif);
	light_0_sets.resize(fif);

	object_pool_->AllocateSets(0, object_0_sets.data(), fif);
	light_pool_->AllocateSets(0, light_0_sets.data(), fif);

	object_1_sets.reserve(fif);

	object_0_0_uniforms.reserve(fif);
	object_0_1_uniforms.reserve(fif);
	light_0_0_uniforms.reserve(fif);

	for (uint32_t i = 0; i < fif; ++i)
	{
		object_0_0_uniforms.emplace_back(device, sizeof(ObjectUniform_0_0));
		object_0_1_uniforms.emplace_back(device, sizeof(ObjectUniform_0_1));
		object_0_sets[i].BindUniform(0, object_0_0_uniforms[i]);
		object_0_sets[i].BindUniform(1, object_0_1_uniforms[i]);

		light_0_0_uniforms.emplace_back(device, sizeof(LightUniform_0_0));
		light_0_sets[i].BindUniform(0, light_0_0_uniforms[i]);
	}
}

void RenderSystem::Render(CommandBuffer &cb, FrameData &frame)
{
	Fvec3 camera_pos = {0.f, 3.f, -4.f};
	Fvec3 camera_ori = {0.f, -0.5f, 1.f};

	Fmat4 proj = PerspectiveProjection(90.f*3.14f/180.f, 16.f/9, .1f, 100.f);
	Fmat4 cam = LookAtView(camera_pos, camera_ori);

	ObjectUniform_0_0 obj00 = { cam, proj, camera_pos };
	object_0_0_uniforms[frame.index].Update(&obj00);

	LightUniform_0_0 light00 = { cam, proj };
	light_0_0_uniforms[frame.index].Update(&light00);

	Fvec3 light_pos = {2 * std::cos(frame.app_time), -1.f, 2 * std::sin(frame.app_time)};
	Fvec3 light_color = {1.f, (std::sin(frame.app_time * 0.7f) + 0.5f) / 2, 0.7f};

	// update later
	ObjectUniform_0_1 obj01 = { light_pos, light_color };
	object_0_1_uniforms[frame.index].Update(&obj01);

	// for (Entity e : lights_.set) {
	// 	auto light
	// }

	cb.RecordDraw(frame.image_index, [&, this, frame](wil::CmdDraw &cmd)
	{
		auto size = wil::GetApp().GetWindow().GetFramebufferSize();
		cmd.SetViewport({0, 0}, size);
		cmd.SetScissor({0, 0}, size);

		cmd.BindPipeline(*object_pipeline_);

		for (Entity e : objects_.set)
		{
			auto [tc, mc] = registry_.GetComponents<TransformComponent, ModelComponent>(e);

			ObjectPushConstant push;
			push.model = TranslateModel(tc.position) * ScaleModel(tc.size);
			cmd.PushConstant(*object_pipeline_, &push);

			if (!models_.count(mc.path))
			{
				constexpr auto f = [](void *data, Fvec3 pos, Fvec2 texcoord, Fvec3 normal) {
					ObjectVertex v;
					v.pos = pos;
					v.texcoord = texcoord;
					v.normal = normal;
					std::memcpy(data, &v, sizeof(ObjectVertex));
				};
				models_.emplace(mc.path, Model(device_, mc.path, sizeof(ObjectVertex), f));
				auto &m = models_.at(mc.path);
				auto start_index = mc.texture_index = object_1_sets.size();
				object_1_sets.resize(start_index + m.GetTextureCount());
				object_pool_->AllocateSets(1, object_1_sets.data() + start_index, m.GetTextureCount());
				for (uint32_t i = 0; i < m.GetTextureCount(); ++i)
					object_1_sets[start_index + i].BindTexture(0, m.GetTextures()[i]);
			}

			Model &m = models_.at(mc.path);

			for (int i = 0; i < m.GetMeshes().size(); ++i)
			{
				const wil::Mesh &mesh = m.GetMeshes()[i];
				wil::DescriptorSet sets[] = { object_0_sets[frame.index], object_1_sets[mesh.material_index + mc.texture_index] };

				cmd.BindDescriptorSets(*object_pipeline_, 0, sets, 2);
				cmd.BindVertexBuffer(mesh.vertex_buffer);

				if (mesh.index_buffer) {
					cmd.BindIndexBuffer(mesh.index_buffer.value());
					cmd.DrawIndexed(mesh.draw_count, 1);
				} else {
					cmd.Draw(mesh.draw_count, 1);
				}
			}
		}
		// cmd.BindPipeline(*light_pipeline_);
		// cmd.BindDescriptorSets(*light_pipeline_, 0, &light_uniform_sets[frame.index], 1);
		// LightPushConstant3D push;
		// push.model = wil::TranslateModel(light.pos);
		// push.light_color = light.color & 1.f;
		//
		// cmd.PushConstant(*light_pipeline_, &push);
		// cmd.BindVertexBuffer(vb);
		// cmd.BindIndexBuffer(ib);
		// cmd.DrawIndexed(indices.size(), 1);
	});
	
}

}
