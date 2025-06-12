#include <wil/render.hpp>

namespace wil {

RenderSystem::RenderSystem(Registry& registry, Device &device)
	: System(registry), registry_(registry)
{
	CreatePipelines_(device);
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

}
