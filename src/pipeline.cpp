#include <wil/pipeline.hpp>
#include <wil/log.hpp>
#include <wil/app.hpp>
#include <wil/descriptor.hpp>

#include <fstream>
#include <array>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

void DescriptorSetLayout::Add(uint32_t binding, DescriptorType type, ShaderType stage)
{
	bindings_.emplace_back(binding, type, stage);
	++descriptor_count_[type];
}

static VkShaderModule CreateShaderModule_(VkDevice device, char const* path)
{
    std::ifstream ifs(path, std::ios::ate | std::ios::binary);
	if (!ifs.is_open())
		WIL_LOGERROR("Unable to open file {}", path);
    size_t fsize = ifs.tellg();
    std::vector<char> buffer(fsize);
    ifs.seekg(0);
    ifs.read(buffer.data(), static_cast<std::streamsize>(fsize));
    ifs.close();

    VkShaderModuleCreateInfo shader_ci{};
    shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_ci.codeSize = buffer.size();
    shader_ci.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule module;
    if (vkCreateShaderModule(device, &shader_ci, nullptr, &module) != VK_SUCCESS)
		WIL_LOGERROR("Unable to create shader module with file {}", path);
    return module;
}

static VkDescriptorType GetVkDescriptorType_(DescriptorType type) {
	switch (type) {
		case UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}
	__builtin_unreachable();
}

// also used in cmdbuf.cpp
VkShaderStageFlags GetVkShaderStageFlag_(ShaderType type) {
	switch (type) {
		case VERTEX_SHADER: return VK_SHADER_STAGE_VERTEX_BIT;
		case FRAGMENT_SHADER: return VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	__builtin_unreachable();
}

Pipeline::Pipeline(const PipelineCtor &ctor) : device_(*ctor.device), descriptor_set_layouts_(ctor.descriptor_set_layouts)
{
	VkDevice device = static_cast<VkDevice>(ctor.device->GetVkDevicePtr_());

	VkShaderModule vert = CreateShaderModule_(device, ctor.shaders[VERTEX_SHADER]);
	VkShaderModule frag = CreateShaderModule_(device, ctor.shaders[FRAGMENT_SHADER]);

	VkPipelineShaderStageCreateInfo shader_stages_ci[] = { {}, {} };

	shader_stages_ci[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages_ci[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages_ci[0].module = vert;
	shader_stages_ci[0].pName = "main";

	shader_stages_ci[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages_ci[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages_ci[1].module = frag;
	shader_stages_ci[1].pName = "main";

	VkVertexInputBindingDescription binding_desc{};
	binding_desc.binding = 0;
	binding_desc.stride = ctor.vertex_stride;
	binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertex_input_ci{};
	vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (!ctor.vertex_layout.empty()) {
		vertex_input_ci.vertexBindingDescriptionCount = 1;
		vertex_input_ci.vertexAttributeDescriptionCount = ctor.vertex_layout.size();
		vertex_input_ci.pVertexBindingDescriptions = &binding_desc;
		vertex_input_ci.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription*)ctor.vertex_layout.data();
	}

	VkPipelineInputAssemblyStateCreateInfo input_assembly_ci{};
	input_assembly_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_ci.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_ci{};
	viewport_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_ci.viewportCount = 1;
	viewport_ci.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer_ci{};
	rasterizer_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_ci.depthClampEnable = VK_FALSE;
	rasterizer_ci.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_ci.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_ci.lineWidth = 1.0f;
	rasterizer_ci.cullMode = VK_CULL_MODE_NONE;
	rasterizer_ci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer_ci.depthBiasEnable = VK_FALSE;
	rasterizer_ci.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer_ci.depthBiasClamp = 0.0f; // Optional
	rasterizer_ci.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling_ci{};
	multisampling_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling_ci.sampleShadingEnable = VK_FALSE;
	multisampling_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling_ci.minSampleShading = 1.0f; // Optional
	multisampling_ci.pSampleMask = nullptr; // Optional
	multisampling_ci.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling_ci.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState blend_func{};
	blend_func.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blend_func.blendEnable = VK_TRUE;
	blend_func.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blend_func.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_func.colorBlendOp = VK_BLEND_OP_ADD;
	blend_func.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_func.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blend_func.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo blending_ci{};
	blending_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blending_ci.logicOpEnable = VK_FALSE;
	blending_ci.logicOp = VK_LOGIC_OP_COPY; // Optional
	blending_ci.attachmentCount = 1;
	blending_ci.pAttachments = &blend_func;
	blending_ci.blendConstants[0] = 0.0f; // Optional
	blending_ci.blendConstants[1] = 0.0f; // Optional
	blending_ci.blendConstants[2] = 0.0f; // Optional
	blending_ci.blendConstants[3] = 0.0f; // Optional

	constexpr std::array dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
	dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_ci.dynamicStateCount = dynamic_states.size();
	dynamic_state_ci.pDynamicStates = dynamic_states.data();

	std::vector<VkDescriptorSetLayout> dslptr;
	dslptr.reserve(descriptor_set_layouts_.size());

	for (auto &dsl : descriptor_set_layouts_)
	{
		VkDescriptorSetLayoutCreateInfo layout_ci{};
		layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_ci.bindingCount = dsl.bindings_.size();

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.reserve(dsl.bindings_.size());
		for (auto bind : dsl.bindings_) {
			bindings.emplace_back(
					bind.binding,
					GetVkDescriptorType_(bind.type),
					1,
					GetVkShaderStageFlag_(bind.stage),
					nullptr);
		}

		layout_ci.pBindings = bindings.data();

		VkDescriptorSetLayout layout;
		if (vkCreateDescriptorSetLayout(device, &layout_ci, nullptr, &layout) != VK_SUCCESS)
			WIL_LOGERROR("Unable to create descriptor set layout");
		dsl.descriptor_set_layout_ptr_ = layout;
		dslptr.push_back(layout);
	}

	push_constant_stage_ = ctor.push_constant_stage;
	push_constant_size_ = ctor.push_constant_size;

	VkPushConstantRange pc_range;
	pc_range.stageFlags = GetVkShaderStageFlag_(ctor.push_constant_stage);
	pc_range.offset = 0;
	pc_range.size = ctor.push_constant_size;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = ctor.depth_test;
	depthStencil.depthWriteEnable = ctor.depth_test;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkPipelineLayoutCreateInfo pipeline_layout_ci{};
	pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_ci.pushConstantRangeCount = ctor.push_constant_size ? 1 : 0;
	pipeline_layout_ci.pPushConstantRanges = ctor.push_constant_size ? &pc_range : nullptr;
	pipeline_layout_ci.setLayoutCount = dslptr.size();
	pipeline_layout_ci.pSetLayouts = dslptr.size() ? dslptr.data() : nullptr;

	if (vkCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, reinterpret_cast<VkPipelineLayout*>(&layout_ptr_)) != VK_SUCCESS)
		WIL_LOGERROR("Unable to create pipeline layout");

	VkGraphicsPipelineCreateInfo pipeline_ci{};
	pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_ci.stageCount = 2;
	pipeline_ci.pStages = shader_stages_ci;
	pipeline_ci.pVertexInputState = &vertex_input_ci;
	pipeline_ci.pInputAssemblyState = &input_assembly_ci;
	pipeline_ci.pViewportState = &viewport_ci;
	pipeline_ci.pRasterizationState = &rasterizer_ci;
	pipeline_ci.pMultisampleState = &multisampling_ci;
	pipeline_ci.pColorBlendState = &blending_ci;
	pipeline_ci.pDynamicState = &dynamic_state_ci;
	pipeline_ci.pDepthStencilState = &depthStencil;
	pipeline_ci.layout = static_cast<VkPipelineLayout>(layout_ptr_);
	pipeline_ci.renderPass = static_cast<VkRenderPass>(ctor.device->GetVkRenderPassPtr_());
	pipeline_ci.subpass = 0;
	pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;

	 if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, reinterpret_cast<VkPipeline*>(&pipeline_ptr_)) != VK_SUCCESS)
		 WIL_LOGERROR("Unable to create render pipeline");

	vkDestroyShaderModule(device, vert, nullptr);
	vkDestroyShaderModule(device, frag, nullptr);
}

Pipeline::~Pipeline()
{
	auto device = static_cast<VkDevice>(device_.GetVkDevicePtr_());

	for (auto l : descriptor_set_layouts_)
		vkDestroyDescriptorSetLayout(device, static_cast<VkDescriptorSetLayout>(l.descriptor_set_layout_ptr_), nullptr);
	vkDestroyPipelineLayout(device, static_cast<VkPipelineLayout>(layout_ptr_), nullptr);
	vkDestroyPipeline(device, static_cast<VkPipeline>(pipeline_ptr_), nullptr);
}

template<> uint32_t getvkattribformat_<float>() { return VK_FORMAT_R32_SFLOAT; }
template<> uint32_t getvkattribformat_<Fvec2>() { return VK_FORMAT_R32G32_SFLOAT; }
template<> uint32_t getvkattribformat_<Fvec3>() { return VK_FORMAT_R32G32B32_SFLOAT; }
template<> uint32_t getvkattribformat_<Fvec4>() { return VK_FORMAT_R32G32B32A32_SFLOAT; }

template<> uint32_t getvkattribformat_<int>() { return VK_FORMAT_R32_SINT; }
template<> uint32_t getvkattribformat_<Ivec2>() { return VK_FORMAT_R32G32_SINT; }
template<> uint32_t getvkattribformat_<Ivec3>() { return VK_FORMAT_R32G32B32_SINT; }
template<> uint32_t getvkattribformat_<Ivec4>() { return VK_FORMAT_R32G32B32A32_SINT; }

template<> uint32_t getvkattribformat_<unsigned>() { return VK_FORMAT_R32_UINT; }
template<> uint32_t getvkattribformat_<Uvec2>() { return VK_FORMAT_R32G32_UINT; }
template<> uint32_t getvkattribformat_<Uvec3>() { return VK_FORMAT_R32G32B32_UINT; }
template<> uint32_t getvkattribformat_<Uvec4>() { return VK_FORMAT_R32G32B32A32_UINT; }

}
