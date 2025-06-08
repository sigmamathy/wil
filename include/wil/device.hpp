#pragma once

#include "core.hpp"
#include "algebra.hpp"
#include "display.hpp"
#include <cstdint>
#include <vector>
#include <functional>

namespace wil {

struct DeviceQueue
{
	VendorPtr vkqueue;
	uint32_t family_index;
	void WaitIdle();
};

class Device
{
public:

	Device(VendorPtr vkinst, VendorPtr vksurface, Ivec2 fbsize, bool vsync);

	~Device();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Device);

	void WaitIdle();

	Uvec2 GetSwapchainExtent() const { return swapchain_extent_; }

	DeviceQueue GetGraphicsQueue() const { return graphics_queue_; }

	DeviceQueue GetPresentQueue() const { return present_queue_; }

	void RecreateSwapchain(Window *win, Ivec2 fbsize, bool vsync);

	VendorPtr GetVkDevicePtr_() { return device_ptr_; }

	VendorPtr GetVkPhysicalDevicePtr_() { return physical_ptr_; }

	VendorPtr GetVkSwapchainPtr_() { return swapchain_ptr_; }

	VendorPtr GetVkCommandPoolPtr_() { return pool_ptr_; }

	VendorPtr GetVkRenderPassPtr_() { return render_pass_ptr_; }

	VendorPtr GetVkFramebufferPtr_(uint32_t index) { return framebuffers_ptr_[index]; }

private:

	void InitDevice_(VendorPtr vkinst, VendorPtr vksurface);
	void InitSwapchain_(VendorPtr vksurface, Ivec2 fbsize, bool vsync);
	void InitCommandPool_();
	void InitRenderPass_();
	void InitFramebuffers_();

	VendorPtr device_ptr_, physical_ptr_;
	DeviceQueue graphics_queue_, present_queue_;

	VendorPtr swapchain_ptr_;
	uint32_t swapchain_format_;
	Uvec2 swapchain_extent_;
	std::vector<VendorPtr> image_views_ptr_;

	VendorPtr pool_ptr_;

	class DepthBuffer *depth_buffer_;
	VendorPtr render_pass_ptr_;
	std::vector<VendorPtr> framebuffers_ptr_;

};


}
