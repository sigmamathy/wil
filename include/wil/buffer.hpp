#pragma once

#include "device.hpp"

namespace wil {

class VertexBuffer
{
public:

    VertexBuffer(Device &device, size_t size);

    ~VertexBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(VertexBuffer);

    void MapData(const void* src);

	VendorPtr GetVkBufferPtr_() const { return buffer_ptr_; }

    size_t GetSize() const { return size_; }

private:

	Device &device_;

    VendorPtr buffer_ptr_;
    VendorPtr memory_ptr_;
    size_t size_;
};

class IndexBuffer
{
public:

    IndexBuffer(Device &device, size_t size);

    ~IndexBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(IndexBuffer);

    void MapData(const unsigned* src);

	VendorPtr GetVkBufferPtr_() const { return buffer_ptr_; }

    size_t GetSize() const { return size_; }

private:

	Device &device_;

    VendorPtr buffer_ptr_;
    VendorPtr memory_ptr_;
    size_t size_;
};

class UniformBuffer
{
public:

    UniformBuffer(Device &device, size_t size);

    ~UniformBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(UniformBuffer);

    void Update(const void* src);

	VendorPtr GetVkBufferPtr_() const { return buffer_ptr_; }

    size_t GetSize() const { return size_; }

private:

	Device &device_;

    VendorPtr buffer_ptr_;
    VendorPtr memory_ptr_;
    size_t size_;
	void *data_;
};

class Texture
{
public:

	Texture(Device &dev, const std::string &path);

	Texture(Device &dev, const void *data, size_t size, uint32_t width, uint32_t height);

	~Texture();

	VendorPtr GetVkImageViewPtr_() const { return image_view_ptr_; }

	VendorPtr GetVkSamplerPtr_() const { return sampler_ptr_; }

private:

	void Init_(Device &dev, const void *data, size_t size, uint32_t width, uint32_t height);

	Device &device_;

	VendorPtr image_ptr_;
	VendorPtr memory_ptr_;
	VendorPtr image_view_ptr_;
	VendorPtr sampler_ptr_;
};

class DepthBuffer
{
public:

	DepthBuffer(Device &dev);

	~DepthBuffer();

	uint32_t GetFormat() const { return format_; }

	VendorPtr GetVkImageViewPtr_() const { return image_view_ptr_; }

private:

	Device &device_;

	VendorPtr image_ptr_;
	VendorPtr memory_ptr_;
	VendorPtr image_view_ptr_;

	uint32_t format_;
};

}
