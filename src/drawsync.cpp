#include <wil/drawsync.hpp>
#include <wil/log.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

DrawPresentSynchronizer::DrawPresentSynchronizer(Device &device, uint32_t num_of_render)
	: device_(device)
{
    VkDevice dev = static_cast<VkDevice>(device.GetVkDevicePtr_());

    VkSemaphoreCreateInfo semaphore_ci{};
    semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_ci{};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphore ia;
	if (vkCreateSemaphore(dev, &semaphore_ci, nullptr, &ia) != VK_SUCCESS)
		WIL_LOGERROR("Unable to create semaphore");
	image_available_semaphore_ = ia;

	render_semaphores_.reserve(num_of_render);
	for (int i = 0; i < num_of_render; ++i) {
		VkSemaphore sm;
		if (vkCreateSemaphore(dev, &semaphore_ci, nullptr, &sm) != VK_SUCCESS)
			WIL_LOGERROR("Unable to create semaphore");
		render_semaphores_.push_back(sm);
	}

	VkFence fence;
	if (vkCreateFence(dev, &fence_ci, nullptr, &fence) != VK_SUCCESS)
		WIL_LOGERROR("Unable to create fence");
	in_flight_fence_ = fence;
}

DrawPresentSynchronizer::~DrawPresentSynchronizer()
{
    VkDevice dev = static_cast<VkDevice>(device_.GetVkDevicePtr_());
    vkDestroySemaphore(dev, static_cast<VkSemaphore>(image_available_semaphore_), nullptr);
	for (auto ptr : render_semaphores_)
		vkDestroySemaphore(dev, static_cast<VkSemaphore>(ptr), nullptr);
    vkDestroyFence(dev, static_cast<VkFence>(in_flight_fence_), nullptr);
}

bool DrawPresentSynchronizer::AcquireImageIndex(uint32_t *index)
{
    VkDevice dev = static_cast<VkDevice>(device_.GetVkDevicePtr_());
	auto fence = static_cast<VkFence>(in_flight_fence_);
    vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);

    VkResult r = vkAcquireNextImageKHR(dev, static_cast<VkSwapchainKHR>(device_.GetVkSwapchainPtr_()), UINT64_MAX,
			static_cast<VkSemaphore>(image_available_semaphore_), VK_NULL_HANDLE, index);
	if (r == VK_ERROR_OUT_OF_DATE_KHR) return false;
	else if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) WIL_LOGERROR("Unable to acquire image index");
    return true;
}

void DrawPresentSynchronizer::SubmitDraw(const std::vector<CommandBuffer*> &buffers)
{
	if (buffers.size() != render_semaphores_.size())
		WIL_LOGWARN("Number of command buffers provided is {}, expected {}", buffers.size(), render_semaphores_.size());

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	
	auto wait = static_cast<VkSemaphore>(image_available_semaphore_);
	auto buf = static_cast<VkCommandBuffer>(buffers[0]->GetVkCommandBufferPtr_());
	auto signal = static_cast<VkSemaphore>(render_semaphores_[0]);
	auto in_flight = static_cast<VkFence>(in_flight_fence_);

    vkResetFences(static_cast<VkDevice>(device_.GetVkDevicePtr_()), 1, &in_flight);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &wait;
    submit.pWaitDstStageMask = wait_stages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &buf;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &signal;

    if (vkQueueSubmit(static_cast<VkQueue>(device_.GetGraphicsQueue().vkqueue), 1, &submit,
				buffers.size() == 1 ? in_flight : VK_NULL_HANDLE) != VK_SUCCESS)
		WIL_LOGERROR("Unable to submit draw command");

	for (size_t i = 1; i < buffers.size(); ++i)
	{
		wait = static_cast<VkSemaphore>(render_semaphores_[i-1]);
		buf = static_cast<VkCommandBuffer>(buffers[i]->GetVkCommandBufferPtr_());
		signal = static_cast<VkSemaphore>(render_semaphores_[i]);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &wait;
		submit.pWaitDstStageMask = wait_stages;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &buf;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &signal;

		if (vkQueueSubmit(static_cast<VkQueue>(device_.GetGraphicsQueue().vkqueue), 1, &submit,
					i == buffers.size() - 1 ? in_flight : VK_NULL_HANDLE) != VK_SUCCESS)
			WIL_LOGERROR("Unable to submit draw command");
	}
}

bool DrawPresentSynchronizer::PresentToScreen(uint32_t image_index)
{
    VkSwapchainKHR swapchains[] = { static_cast<VkSwapchainKHR>(device_.GetVkSwapchainPtr_()) };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	auto wait = static_cast<VkSemaphore>(render_semaphores_.back());
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &wait;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &image_index;

    VkResult r = vkQueuePresentKHR(static_cast<VkQueue>(device_.GetPresentQueue().vkqueue), &presentInfo);
	if (r == VK_ERROR_OUT_OF_DATE_KHR) return false;
	else if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) WIL_LOGERROR("Unable to present to screen");
	return true;
}

}
