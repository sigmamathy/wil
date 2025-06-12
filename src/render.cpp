#include <wil/render.hpp>
#include <wil/app.hpp>
#include <wil/transform.hpp>

namespace wil {

RenderSystem::RenderSystem(Registry& registry, Device &device)
	: System(registry), registry_(registry)
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

	object_0_sets = object_pool_->AllocateSets(0, fif);
	light_0_sets = light_pool_->AllocateSets(0, fif);

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
	Fvec3 camera_pos = {0.f, -2.f, -2.f};
	Fvec3 camera_ori = {0.f, 0.5f, 1.f};

	Fmat4 proj = PerspectiveProjection(2.0944f, 16.f/9, .1f, 100.f);
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
			auto [transform, model] = registry_.GetComponents<TransformComponent, ModelComponent>(e);

			ObjectPushConstant push;
			push.model = TranslateModel(transform.position) * ScaleModel(transform.size);
			cmd.PushConstant(*object_pipeline_, &push);

		}
		//
		//
		// for (int i = 0; i < model->GetMeshes().size(); ++i)
		// {
		// 	const wil::Mesh &m = model->GetMeshes()[i];
		// 	wil::DescriptorSet sets[] = { uniform_sets[frame.index], tex_sets[m.material_index] };
		//
		// 	cmd.BindDescriptorSets(*pipeline_, 0, sets, 2);
		// 	cmd.BindVertexBuffer(m.vertex_buffer);
		//
		// 	if (m.index_buffer) {
		// 		cmd.BindIndexBuffer(m.index_buffer.value());
		// 		cmd.DrawIndexed(m.draw_count, 1);
		// 	} else {
		// 		cmd.Draw(m.draw_count, 1);
		// 	}
		// }
		//
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
