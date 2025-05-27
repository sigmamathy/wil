#include <wil/device.hpp>
#include <wil/log.hpp>

#include <array>
#include <vector>
#include <unordered_set>
#include <limits>
#include <algorithm>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

Device::Device(void *vkinst, void *vksurface, Ivec2 fbsize)
{
	InitDevice(vkinst, vksurface);
	InitSwapchain(vksurface, fbsize);
}

void Device::InitDevice(void *vkinst, void *vksurface)
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

void Device::InitSwapchain(void *vksurface, Ivec2 fbsize)
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

    if (vkCreateSwapchainKHR(device, &swapchain_ci, nullptr,
				reinterpret_cast<VkSwapchainKHR*>(&swapchain_ptr_)) != VK_SUCCESS)
	{
		LogFatal("Unable to create swapchain");
	}

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
			LogFatal("Unable to create image views");
		image_views_ptr_[i] = iv;
    }
}

Device::~Device()
{
	auto device = static_cast<VkDevice>(device_ptr_);

    for (auto view : image_views_ptr_)
        vkDestroyImageView(device, static_cast<VkImageView>(view), nullptr);
    vkDestroySwapchainKHR(device, static_cast<VkSwapchainKHR>(swapchain_ptr_), nullptr);
    vkDestroyDevice(device, nullptr);
}

}
