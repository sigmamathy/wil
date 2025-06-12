#include <wil/render.hpp>
#include <wil/app.hpp>

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

}

}
