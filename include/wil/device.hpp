#pragma once

#include "core.hpp"
#include "algebra.hpp"
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
	void InitRenderPassAndFramebuffers_();

	VendorPtr device_ptr_, physical_ptr_;
	DeviceQueue graphics_queue_, present_queue_;

	VendorPtr swapchain_ptr_;
	uint32_t swapchain_format_;
	Uvec2 swapchain_extent_;
	std::vector<VendorPtr> image_views_ptr_;

	VendorPtr pool_ptr_;

	VendorPtr render_pass_ptr_;
	std::vector<VendorPtr> framebuffers_ptr_;

};

class CommandBuffer
{
public:
	CommandBuffer(Device &device);

	void Reset();
	void RecordDraw(uint32_t fb_index, const std::function<void(class CmdDraw&)> &fn);

	const VendorPtr GetVkCommandBufferPtr_() const { return buffer_ptr_; }

private:
	Device &device_;
	VendorPtr buffer_ptr_;
	friend class CmdDraw;
};

struct CmdDraw
{
	CmdDraw(CommandBuffer &buf) : buffer_(buf) {}

	void SetViewport(Fvec2 pos, Fvec2 size, float min_depth = 0.f, float max_depth = 1.f);
	void SetScissor(Ivec2 offset, Uvec2 extent);

	void BindPipeline(class Pipeline &pipeline);
	void BindVertexBuffer(class VertexBuffer &buffer);
	void BindIndexBuffer(class IndexBuffer &buffer);
	void BindDescriptorSet(class Pipeline &pipeline, class DescriptorSet &set);

	void Draw(uint32_t count, uint32_t instance);
	void DrawIndexed(uint32_t count, uint32_t instance);

private:
	CommandBuffer &buffer_;
};

}
