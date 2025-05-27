#pragma once

#include "algebra.hpp"
#include <cstdint>
#include <vector>

namespace wil {

struct DeviceQueue
{
	void *vkqueue;
	uint32_t family_index;
};

class Device
{
public:

	Device(void *vkinst, void *vksurface, Ivec2 fbsize);

	~Device();

private:

	void InitDevice(void *vkinst, void *vksurface);
	void InitSwapchain(void *vksurface, Ivec2 fbsize);

	void *device_ptr_, *physical_ptr_;
	DeviceQueue graphics_queue_, present_queue_;

	void *swapchain_ptr_;
	uint32_t swapchain_format_;
	Uvec2 swapchain_extent_;
	std::vector<void*> image_views_ptr_;

};

}
