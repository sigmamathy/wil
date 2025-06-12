#include <wil/cmdbuf.hpp>
#include <wil/log.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

CommandBuffer::CommandBuffer(Device &device) : device_(device)
{
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = static_cast<VkCommandPool>(device.GetVkCommandPoolPtr_());
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

	VkCommandBuffer cb;
    if (vkAllocateCommandBuffers(static_cast<VkDevice>(device.GetVkDevicePtr_()), &info, &cb) != VK_SUCCESS)
		WIL_LOGERROR("Unable to create command buffer");
	buffer_ptr_ = cb;
}

void CommandBuffer::Reset()
{
    vkResetCommandBuffer(static_cast<VkCommandBuffer>(buffer_ptr_), 0);
}

void CommandBuffer::RecordDraw(uint32_t fb_index, const std::function<void(CmdDraw&)> &fn)
{
	auto buffer = static_cast<VkCommandBuffer>(buffer_ptr_);

    VkCommandBufferBeginInfo begin_i{};
    begin_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_i.flags = 0; // Optional
    begin_i.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(buffer, &begin_i) != VK_SUCCESS)
		WIL_LOGERROR("Unable to start recording command buffer");

    VkRenderPassBeginInfo render_pass_i{};
    render_pass_i.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_i.renderPass = static_cast<VkRenderPass>(device_.GetVkRenderPassPtr_());
    render_pass_i.framebuffer = static_cast<VkFramebuffer>(device_.GetVkFramebufferPtr_(fb_index));
    render_pass_i.renderArea.offset = { 0, 0 };
	auto ext = device_.GetSwapchainExtent();
    render_pass_i.renderArea.extent = VkExtent2D{ext.x, ext.y};

	std::array<VkClearValue, 2> clear_color;
	clear_color[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clear_color[1].depthStencil = {1.f, 0};

    render_pass_i.clearValueCount = clear_color.size();
    render_pass_i.pClearValues = clear_color.data();

    vkCmdBeginRenderPass(buffer, &render_pass_i, VK_SUBPASS_CONTENTS_INLINE);

	CmdDraw cmd(*this);
	fn(cmd);

    vkCmdEndRenderPass(buffer);
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
		WIL_LOGERROR("Unable to end recording command buffer");
}

void CmdDraw::SetViewport(Fvec2 pos, Fvec2 size, float min_depth, float max_depth)
{
    VkViewport viewport = {
		pos.x, pos.y,
		size.x, size.y,
		min_depth, max_depth
    };

    vkCmdSetViewport(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), 0, 1, &viewport);
}

void CmdDraw::SetScissor(Ivec2 offset, Uvec2 extent)
{
    VkRect2D scissor = {
        {offset.x, offset.y},
        VkExtent2D{extent.x, extent.y}
    };

    vkCmdSetScissor(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), 0, 1, &scissor);
}

void CmdDraw::BindPipeline(Pipeline &pipeline)
{
    vkCmdBindPipeline(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<VkPipeline>(pipeline.GetVkPipelinePtr_()));
}

void CmdDraw::BindVertexBuffer(const VertexBuffer &buffer)
{
    VkBuffer b = static_cast<VkBuffer>(buffer.GetVkBufferPtr_());
    VkDeviceSize off = 0;
    vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), 0, 1, &b, &off);
}

void CmdDraw::BindIndexBuffer(const IndexBuffer &buffer)
{
    VkBuffer b = static_cast<VkBuffer>(buffer.GetVkBufferPtr_());
    vkCmdBindIndexBuffer(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), b, 0, VK_INDEX_TYPE_UINT32);
}

void CmdDraw::BindDescriptorSets(Pipeline &pipeline, int first_set, DescriptorSet *sets, size_t count)
{
	std::vector<VkDescriptorSet> vksets;
	vksets.reserve(count);
	for (int i = 0; i < count; ++i)
		vksets.push_back(static_cast<VkDescriptorSet>(sets[i].GetVkDescriptorSetPtr_()));

	vkCmdBindDescriptorSets(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<VkPipelineLayout>(pipeline.GetVkPipelineLayoutPtr_()),
			first_set, count,
			vksets.data(), 0, nullptr);
}

// defined in pipeline.cpp
extern VkShaderStageFlags GetVkShaderStageFlag_(ShaderStageBit type);

void CmdDraw::PushConstant(Pipeline &pipeline, const void* data)
{
	vkCmdPushConstants(
			static_cast<VkCommandBuffer>(buffer_.buffer_ptr_),
			static_cast<VkPipelineLayout>(pipeline.GetVkPipelineLayoutPtr_()),
			GetVkShaderStageFlag_(pipeline.GetPushConstantStage()),
			0,
			pipeline.GetPushConstantSize(),
			data);
}

void CmdDraw::Draw(uint32_t count, uint32_t instance)
{
    vkCmdDraw(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), count, instance, 0, 0);
}

void CmdDraw::DrawIndexed(uint32_t count, uint32_t instance)
{
    vkCmdDrawIndexed(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), count, instance, 0, 0, 0);
}

}
