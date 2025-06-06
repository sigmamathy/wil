#include <wil/buffer.hpp>
#include <wil/log.hpp>

#include <cstring>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

namespace wil {

static uint32_t FindMemoryTypeIndex_(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(device, &mp);

    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
    {
        if (filter & (1 << i) && (mp.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

	return -1;

}

static std::pair<VkBuffer, VkDeviceMemory>
CreateBufferAndAllocateMemory_(VkDevice device, VkPhysicalDevice pd, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props)
{
    std::pair<VkBuffer, VkDeviceMemory> result;

    VkBufferCreateInfo buffer_ci{};
    buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_ci.size = size;
    buffer_ci.usage = usage;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &buffer_ci, nullptr, &result.first) != VK_SUCCESS)
		LogErr("Unable to create buffer");

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(device, result.first, &req);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = req.size;
    allocInfo.memoryTypeIndex = FindMemoryTypeIndex_(pd, req.memoryTypeBits, props);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &result.second) != VK_SUCCESS)
		LogErr("Unable to allocate buffer memory");

    vkBindBufferMemory(device, result.first, result.second, 0);

    return result;
}

static VkCommandBuffer BeginSingleTimeCommandBuffer_(Device &device)
{
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = static_cast<VkCommandPool>(device.GetVkCommandPoolPtr_());
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

	VkCommandBuffer cb;
    if (vkAllocateCommandBuffers(static_cast<VkDevice>(device.GetVkDevicePtr_()), &info, &cb) != VK_SUCCESS)
		LogErr("Unable to create command buffer");

    VkCommandBufferBeginInfo begin_i{};
    begin_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_i.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cb, &begin_i);

	return cb;
}

static void EndSingleTimeCommandBuffer_(Device &device, VkCommandBuffer cb, const DeviceQueue &queue)
{
    vkEndCommandBuffer(cb);

    VkSubmitInfo submit_i{};
    submit_i.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_i.commandBufferCount = 1;
    submit_i.pCommandBuffers = &cb;

	auto q = static_cast<VkQueue>(queue.vkqueue);

    vkQueueSubmit(q, 1, &submit_i, VK_NULL_HANDLE);
    vkQueueWaitIdle(q);

    vkFreeCommandBuffers(static_cast<VkDevice>(device.GetVkDevicePtr_()),
			static_cast<VkCommandPool>(device.GetVkCommandPoolPtr_()), 1, &cb);
}

static void
CopyViaStagingBuffer_(Device &device, VkDeviceSize size, void const* src, VkBuffer dst)
{
	auto dev = static_cast<VkDevice>(device.GetVkDevicePtr_());

    auto [stage, stage_mem] = CreateBufferAndAllocateMemory_(
			dev,
			static_cast<VkPhysicalDevice>(device.GetVkPhysicalDevicePtr_()),
			size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(dev, stage_mem, 0, size, 0, &data);
    memcpy(data, src, size);
    vkUnmapMemory(dev, stage_mem);

	VkCommandBuffer cb = BeginSingleTimeCommandBuffer_(device);

	VkBufferCopy buffer_copy{};
	buffer_copy.size = size;
	vkCmdCopyBuffer(cb, stage, dst, 1, &buffer_copy);

	EndSingleTimeCommandBuffer_(device, cb, device.GetGraphicsQueue());

    vkDestroyBuffer(dev, stage, nullptr);
    vkFreeMemory(dev, stage_mem, nullptr);
}

VertexBuffer::VertexBuffer(Device &device, size_t size)
    : device_(device), size_(size)
{
    auto [fst, snd] = CreateBufferAndAllocateMemory_(
			static_cast<VkDevice>(device.GetVkDevicePtr_()),
			static_cast<VkPhysicalDevice>(device.GetVkPhysicalDevicePtr_()),
			size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    buffer_ptr_ = fst;
    memory_ptr_ = snd;
}

VertexBuffer::~VertexBuffer()
{
	auto dev = static_cast<VkDevice>(device_.GetVkDevicePtr_());
    vkDestroyBuffer(dev, static_cast<VkBuffer>(buffer_ptr_), nullptr);
    vkFreeMemory(dev, static_cast<VkDeviceMemory>(memory_ptr_), nullptr);
}

void VertexBuffer::MapData(const void *src)
{
    CopyViaStagingBuffer_(device_, size_, src, static_cast<VkBuffer>(buffer_ptr_));
}

IndexBuffer::IndexBuffer(Device &device, size_t size)
    : device_(device), size_(size)
{
    auto [fst, snd] = CreateBufferAndAllocateMemory_(
			static_cast<VkDevice>(device.GetVkDevicePtr_()),
			static_cast<VkPhysicalDevice>(device.GetVkPhysicalDevicePtr_()),
			size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    buffer_ptr_ = fst;
    memory_ptr_ = snd;
}

IndexBuffer::~IndexBuffer()
{
	auto dev = static_cast<VkDevice>(device_.GetVkDevicePtr_());
    vkDestroyBuffer(dev, static_cast<VkBuffer>(buffer_ptr_), nullptr);
    vkFreeMemory(dev, static_cast<VkDeviceMemory>(memory_ptr_), nullptr);
}

void IndexBuffer::MapData(const unsigned *src)
{
    CopyViaStagingBuffer_(device_, size_, src, static_cast<VkBuffer>(buffer_ptr_));
}

UniformBuffer::UniformBuffer(Device &device, size_t size)
    : device_(device), size_(size)
{
    auto [b, m] = CreateBufferAndAllocateMemory_(
			static_cast<VkDevice>(device.GetVkDevicePtr_()),
			static_cast<VkPhysicalDevice>(device.GetVkPhysicalDevicePtr_()),
			size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    buffer_ptr_ = b;
    memory_ptr_ = m;

    vkMapMemory(static_cast<VkDevice>(device.GetVkDevicePtr_()), m, 0, size, 0, &data_);
}

UniformBuffer::~UniformBuffer()
{
	auto dev = static_cast<VkDevice>(device_.GetVkDevicePtr_());
    vkDestroyBuffer(dev, static_cast<VkBuffer>(buffer_ptr_), nullptr);
    vkFreeMemory(dev, static_cast<VkDeviceMemory>(memory_ptr_), nullptr);
}

void UniformBuffer::Update(void const* src)
{
	std::memcpy(data_, src, size_);
}

static std::pair<VkImage, VkDeviceMemory>
CreateImageAndAllocateMemory_(VkDevice dev, VkPhysicalDevice pd, uint32_t width, uint32_t height,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
	VkImageCreateInfo image_ci{};
	image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_ci.imageType = VK_IMAGE_TYPE_2D;
	image_ci.extent.width = static_cast<uint32_t>(width);
	image_ci.extent.height = static_cast<uint32_t>(height);
	image_ci.extent.depth = 1;
	image_ci.mipLevels = 1;
	image_ci.arrayLayers = 1;
	image_ci.format = format;
	image_ci.tiling = tiling;
	image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_ci.usage = usage;
	image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
	image_ci.flags = 0; // Optional
	
	VkImage image;
	if (vkCreateImage(dev, &image_ci, nullptr, &image) != VK_SUCCESS)
		LogErr("Unable to create image");

	VkMemoryRequirements memreq;
	vkGetImageMemoryRequirements(dev, image, &memreq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memreq.size;
	allocInfo.memoryTypeIndex = FindMemoryTypeIndex_(pd, memreq.memoryTypeBits, properties);

	VkDeviceMemory image_mem;
	if (vkAllocateMemory(dev, &allocInfo, nullptr, &image_mem) != VK_SUCCESS)
		LogErr("Unable to allocate image memory");

	vkBindImageMemory(dev, image, image_mem, 0);

	return {image, image_mem};
}

static void TransitionImageLayout_(Device &device, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer_(device);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} 
	else 
	{
		LogErr("Invalid argument in transitioning image layout");
	}

	vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
			);

	EndSingleTimeCommandBuffer_(device, commandBuffer, device.GetGraphicsQueue());
}

static void CopyBufferToImage_(Device &device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer cb = BeginSingleTimeCommandBuffer_(device);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
			cb,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);

	EndSingleTimeCommandBuffer_(device, cb, device.GetGraphicsQueue());
}

Texture::Texture(Device &device, const std::string &path)
	: device_(device)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	VkDeviceSize size = width * height * 4;

	if (!pixels) LogErr("Unable to load image " + path);

    auto [sb, sbm] = CreateBufferAndAllocateMemory_(
			static_cast<VkDevice>(device.GetVkDevicePtr_()),
			static_cast<VkPhysicalDevice>(device.GetVkPhysicalDevicePtr_()),
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	auto dev = static_cast<VkDevice>(device.GetVkDevicePtr_());
	auto pd = static_cast<VkPhysicalDevice>(device.GetVkPhysicalDevicePtr_());

	void* data;
	vkMapMemory(dev, sbm, 0, size, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(size));
	vkUnmapMemory(dev, sbm);

	stbi_image_free(pixels);

	auto [image, mem] = CreateImageAndAllocateMemory_(
			dev,
			pd,
			width,
			height,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	image_ptr_ = image;
	memory_ptr_ = mem;

	TransitionImageLayout_(device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage_(device, sb, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	TransitionImageLayout_(device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(dev, sb, nullptr);
	vkFreeMemory(dev, sbm, nullptr);

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView image_view;
	if (vkCreateImageView(dev, &viewInfo, nullptr, &image_view) != VK_SUCCESS)
		LogErr("Unable to create texture image view");
	image_view_ptr_ = image_view;


	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(pd, &properties);
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	
	VkSampler sampler;
	if (vkCreateSampler(dev, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		LogErr("Unable to create texture sampler");
	sampler_ptr_ = sampler;
}

Texture::~Texture()
{
	auto dev = static_cast<VkDevice>(device_.GetVkDevicePtr_());
	vkDestroySampler(dev, static_cast<VkSampler>(sampler_ptr_), nullptr);
	vkDestroyImageView(dev, static_cast<VkImageView>(image_view_ptr_), nullptr);
	vkDestroyImage(dev, static_cast<VkImage>(image_ptr_), nullptr);
    vkFreeMemory(dev, static_cast<VkDeviceMemory>(memory_ptr_), nullptr);
}

}
