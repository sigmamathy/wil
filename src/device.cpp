#include <wil/device.hpp>
#include <wil/log.hpp>
#include <wil/pipeline.hpp>

#include <array>
#include <vector>
#include <unordered_set>
#include <limits>
#include <algorithm>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

void DeviceQueue::WaitIdle()
{
	vkQueueWaitIdle(static_cast<VkQueue>(vkqueue));
}

Device::Device(VendorPtr vkinst, VendorPtr vksurface, Ivec2 fbsize)
{
	InitDevice_(vkinst, vksurface);
	InitSwapchain_(vksurface, fbsize);
	InitCommandPool_();
	InitRenderPassAndFramebuffers_();
}

void Device::InitDevice_(VendorPtr vkinst, VendorPtr vksurface)
{
	VkInstance instance = static_cast<VkInstance>(vkinst);

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
	if (!device_count) {
		LogFatal("No physical rendering device can be found");
	}

    device_count = 1; // choose the first device
	VkPhysicalDevice phys;
    vkEnumeratePhysicalDevices(instance, &device_count, &phys);
	physical_ptr_ = phys;

    constexpr std::array device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    uint32_t family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &family_count, nullptr);
    std::vector<VkQueueFamilyProperties> families(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &family_count, families.data());

    bool graphics_ok = false;
    bool present_ok = false;

    for (int i = 0; i < families.size() && !(graphics_ok && present_ok); i++)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_.family_index = i;
            graphics_ok = true;
        }

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(phys, i, static_cast<VkSurfaceKHR>(vksurface), &present_support);
        if (present_support) {
            present_queue_.family_index = i;
            present_ok = true;
        }
    }

	if (!graphics_ok || !present_ok) {
		LogFatal("The primary physical device does not support graphics or present operation");
	}

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    float priority = 1.0f;

    for (uint32_t index : std::unordered_set{graphics_queue_.family_index, present_queue_.family_index})
    {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = index;
        info.queueCount = 1;
        info.pQueuePriorities = &priority;
        queue_create_infos.push_back(info);
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_ci{};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.pQueueCreateInfos = queue_create_infos.data();
    device_ci.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_ci.pEnabledFeatures = &device_features;
    device_ci.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_ci.ppEnabledExtensionNames = device_extensions.data();

    auto validation_layer = "VK_LAYER_KHRONOS_validation";
    device_ci.enabledLayerCount = 1;
    device_ci.ppEnabledLayerNames = &validation_layer;

	VkDevice device;
    if (vkCreateDevice(phys, &device_ci, nullptr, &device) != VK_SUCCESS) {
		LogFatal("Unable to create logical device");
	}

	device_ptr_ = device;

    vkGetDeviceQueue(device, graphics_queue_.family_index, 0,
			reinterpret_cast<VkQueue*>(&graphics_queue_.vkqueue));
    vkGetDeviceQueue(device, present_queue_.family_index, 0,
			reinterpret_cast<VkQueue*>(&present_queue_.vkqueue));
}

static VkSurfaceFormatKHR ChooseSurfaceFormat_(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    for (auto const& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return formats[0];
}

static VkPresentModeKHR ChoosePresentMode_(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::vector<VkPresentModeKHR> modes;
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    modes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

    for (auto const& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapExtent_(Ivec2 fbsize, VkSurfaceCapabilitiesKHR const& caps)
{
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return caps.currentExtent;

    VkExtent2D actual = {
        static_cast<uint32_t>(fbsize.x),
        static_cast<uint32_t>(fbsize.y)
    };

    actual.width = std::clamp(actual.width, caps.minImageExtent.width, caps.maxImageExtent.width);
    actual.height = std::clamp(actual.height, caps.minImageExtent.height, caps.maxImageExtent.height);

    return actual;
}

void Device::InitSwapchain_(VendorPtr vksurface, Ivec2 fbsize)
{
    VkSurfaceKHR surface = static_cast<VkSurfaceKHR>(vksurface);
	VkPhysicalDevice phys = static_cast<VkPhysicalDevice>(physical_ptr_);

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surface, &caps);

    auto [format, color_space]      = ChooseSurfaceFormat_(phys, surface);
    VkPresentModeKHR present_mode   = ChoosePresentMode_(phys, surface);
    VkExtent2D extent               = ChooseSwapExtent_(fbsize, caps);

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        image_count = caps.maxImageCount;

    VkSwapchainCreateInfoKHR swapchain_ci{};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.surface = surface;
    swapchain_ci.minImageCount = image_count;
    swapchain_ci.imageFormat = format;
    swapchain_ci.imageColorSpace = color_space;
    swapchain_ci.imageExtent = extent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = { graphics_queue_.family_index, present_queue_.family_index };

    if (graphics_queue_.family_index != present_queue_.family_index)
    {
        swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_ci.queueFamilyIndexCount = 2;
        swapchain_ci.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_ci.queueFamilyIndexCount = 0;
        swapchain_ci.pQueueFamilyIndices = nullptr;
    }

    swapchain_ci.preTransform = caps.currentTransform;
    swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.presentMode = present_mode;
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;

	auto device = static_cast<VkDevice>(device_ptr_);

	VkSwapchainKHR sc;
    if (vkCreateSwapchainKHR(device, &swapchain_ci, nullptr, &sc) != VK_SUCCESS)
	{
		LogFatal("Unable to create swapchain");
	}
	swapchain_ptr_ = sc;

	swapchain_extent_ = { extent.width, extent.height };
	swapchain_format_ = format;

    std::vector<VkImage> images;
    images.resize(image_count);
    vkGetSwapchainImagesKHR(device, static_cast<VkSwapchainKHR>(swapchain_ptr_), &image_count, images.data());
    image_views_ptr_.resize(image_count);

    for (uint32_t i = 0; i < image_count; i++)
    {
        VkImageViewCreateInfo view_ci{};
        view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_ci.image = images[i];
        view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_ci.format = format;
        view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_ci.subresourceRange.baseMipLevel = 0;
        view_ci.subresourceRange.levelCount = 1;
        view_ci.subresourceRange.baseArrayLayer = 0;
        view_ci.subresourceRange.layerCount = 1;

		VkImageView iv;
        if (vkCreateImageView(device, &view_ci, nullptr, &iv) != VK_SUCCESS)
			LogFatal("Unable to create image view " + std::to_string(i));
		image_views_ptr_[i] = iv;
    }
}

void Device::InitCommandPool_()
{
	VkCommandPoolCreateInfo pool_ci{};
    pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_ci.queueFamilyIndex = graphics_queue_.family_index;

	VkCommandPool pool;
    if (vkCreateCommandPool(static_cast<VkDevice>(device_ptr_), &pool_ci, nullptr, &pool) != VK_SUCCESS)
		LogFatal("Unable to create command pool");
	pool_ptr_ = pool;
}

void Device::InitRenderPassAndFramebuffers_()
{
    VkAttachmentDescription description{};
    description.format = static_cast<VkFormat>(swapchain_format_);
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference reference{};
    reference.attachment = 0;
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &reference;

    VkRenderPassCreateInfo render_pass_ci{};
    render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &description;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

	auto device = static_cast<VkDevice>(device_ptr_);

	VkRenderPass rp;
    if (vkCreateRenderPass(device, &render_pass_ci, nullptr, &rp) != VK_SUCCESS)
		LogFatal("Unable to create render pass");
	render_pass_ptr_ = rp;

    framebuffers_ptr_.resize(image_views_ptr_.size());

    for (int i = 0; i < framebuffers_ptr_.size(); i++)
    {
		VkImageView iv = static_cast<VkImageView>(image_views_ptr_[i]);

        VkFramebufferCreateInfo framebuffer_ci{};
        framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_ci.renderPass = rp;
        framebuffer_ci.attachmentCount = 1;
        framebuffer_ci.pAttachments = &iv;
        framebuffer_ci.width = swapchain_extent_.x;
        framebuffer_ci.height = swapchain_extent_.y;
        framebuffer_ci.layers = 1;

		VkFramebuffer fb;
        if (vkCreateFramebuffer(device, &framebuffer_ci, nullptr, &fb) != VK_SUCCESS)
			LogFatal("Unable to create framebuffer " + std::to_string(i));
		framebuffers_ptr_[i] = fb;
    }
}

Device::~Device()
{
	auto device = static_cast<VkDevice>(device_ptr_);

	for (auto fb : framebuffers_ptr_)
		vkDestroyFramebuffer(device, static_cast<VkFramebuffer>(fb), nullptr);
    vkDestroyRenderPass(device, static_cast<VkRenderPass>(render_pass_ptr_), nullptr);
	vkDestroyCommandPool(device, static_cast<VkCommandPool>(pool_ptr_), nullptr);
    for (auto view : image_views_ptr_)
        vkDestroyImageView(device, static_cast<VkImageView>(view), nullptr);
    vkDestroySwapchainKHR(device, static_cast<VkSwapchainKHR>(swapchain_ptr_), nullptr);
    vkDestroyDevice(device, nullptr);
}

void Device::WaitIdle()
{
	vkDeviceWaitIdle(static_cast<VkDevice>(device_ptr_));
}

CommandBuffer::CommandBuffer(Device &device) : device_(device)
{
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = static_cast<VkCommandPool>(device.GetVkCommandPoolPtr_());
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

	VkCommandBuffer cb;
    if (vkAllocateCommandBuffers(static_cast<VkDevice>(device.GetVkDevicePtr_()), &info, &cb) != VK_SUCCESS)
		LogErr("Unable to create command buffer");
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
		LogErr("Unable to start recording command buffer");

    VkRenderPassBeginInfo render_pass_i{};
    render_pass_i.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_i.renderPass = static_cast<VkRenderPass>(device_.GetVkRenderPassPtr_());
    render_pass_i.framebuffer = static_cast<VkFramebuffer>(device_.GetVkFramebufferPtr_(fb_index));
    render_pass_i.renderArea.offset = { 0, 0 };
	auto ext = device_.GetSwapchainExtent();
    render_pass_i.renderArea.extent = VkExtent2D{ext.x, ext.y};

    VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    render_pass_i.clearValueCount = 1;
    render_pass_i.pClearValues = &clear_color;

    vkCmdBeginRenderPass(buffer, &render_pass_i, VK_SUBPASS_CONTENTS_INLINE);

	CmdDraw cmd(*this);
	fn(cmd);

    vkCmdEndRenderPass(buffer);
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
		LogErr("Unable to end recording command buffer");
}

void CmdDraw::BindPipeline(Pipeline &pipeline)
{
    vkCmdBindPipeline(static_cast<VkCommandBuffer>(buffer_.buffer_ptr_), VK_PIPELINE_BIND_POINT_GRAPHICS,
			static_cast<VkPipeline>(pipeline.GetVkPipelinePtr_()));
}

}
