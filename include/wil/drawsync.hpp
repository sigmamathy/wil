#pragma once

#include "core.hpp"
#include "device.hpp"
#include <vector>

namespace wil {

class DrawPresentSynchronizer
{
public:

	DrawPresentSynchronizer(Device &device, uint32_t num_of_render);

	~DrawPresentSynchronizer();

	uint32_t AcquireImageIndex();

	void SubmitDraw(const std::vector<CommandBuffer*> &buffers);

	void PresentToScreen(uint32_t image_index);

private:
	Device &device_;
	VendorPtr image_available_semaphore_;
	std::vector<VendorPtr> render_semaphores_;
	VendorPtr in_flight_fence_;
};

}
