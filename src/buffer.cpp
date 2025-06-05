#include <wil/buffer.hpp>
#include <wil/log.hpp>

#include <cstring>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

static uint32_t s_FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags flags)
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
    allocInfo.memoryTypeIndex = s_FindMemoryTypeIndex(pd, req.memoryTypeBits, props);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &result.second) != VK_SUCCESS)
		LogErr("Unable to allocate buffer memory");

    vkBindBufferMemory(device, result.first, result.second, 0);

    return result;
}

static VkCommandBuffer CreateTempCommandBuffer_(Device &device, const std::function<void(VkCommandBuffer&)> &fn)
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

    fn(cb);

    vkEndCommandBuffer(cb);

	return cb;
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

	VkCommandBuffer copy = CreateTempCommandBuffer_(device, [=](VkCommandBuffer &cb) {
        VkBufferCopy buffer_copy{};
        buffer_copy.srcOffset = 0; // Optional
        buffer_copy.dstOffset = 0; // Optional
        buffer_copy.size = size;
        vkCmdCopyBuffer(cb, stage, dst, 1, &buffer_copy);
	});

    VkSubmitInfo submit_i{};
    submit_i.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_i.commandBufferCount = 1;
    submit_i.pCommandBuffers = &copy;

    auto queue = static_cast<VkQueue>(device.GetGraphicsQueue().vkqueue);

    vkQueueSubmit(queue, 1, &submit_i, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(dev, static_cast<VkCommandPool>(device.GetVkCommandPoolPtr_()), 1, &copy);

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

}
