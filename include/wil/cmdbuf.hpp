#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "descriptor.hpp"
#include "buffer.hpp"

namespace wil {

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

	void BindPipeline(Pipeline &pipeline);
	void BindVertexBuffer(const VertexBuffer &buffer);
	void BindIndexBuffer(const IndexBuffer &buffer);

	void BindDescriptorSets(Pipeline &pipeline, int first_set, DescriptorSet *sets, size_t count);
	void PushConstant(Pipeline &pipeline, const void* data);

	void Draw(uint32_t count, uint32_t instance);
	void DrawIndexed(uint32_t count, uint32_t instance);

private:
	CommandBuffer &buffer_;
};

}
