#include <wil/appimpl.hpp>
#include <wil/log.hpp>
#include <wil/pipeline.hpp>
#include <wil/drawsync.hpp>

#include <cassert>
#include <chrono>
#include <algorithm>
#include <cstring>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

static App *appinst_;
static VkInstance vkinstance_;
static VkDebugUtilsMessengerEXT vkdebug_;

static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugMessageCallback_(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                       VkDebugUtilsMessageTypeFlagsEXT type,
                       const VkDebugUtilsMessengerCallbackDataEXT* data,
                       void* user_data)
{
	LogVulkan(severity, data->pMessage);
	return VK_FALSE;
}

static void InitAPIs_()
{
	glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    VkApplicationInfo app_i{};
    app_i.sType					= VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_i.pApplicationName		= "";
    app_i.applicationVersion	= VK_MAKE_VERSION(1, 0, 0);
    app_i.pEngineName			= "No Engine";
    app_i.engineVersion			= VK_MAKE_VERSION(1, 0, 0);
    app_i.apiVersion			= VK_API_VERSION_1_3;

    VkInstanceCreateInfo inst_ci{};
    inst_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_ci.pApplicationInfo = &app_i;

    uint32_t glfw_ext_count;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

#ifndef NDEBUG

    auto constexpr validation_layer = "VK_LAYER_KHRONOS_validation";

    // check if system support VK_LAYER_KHRONOS_validation layer
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> av_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, av_layers.data());
    if (std::find_if(av_layers.begin(), av_layers.end(),
        [](VkLayerProperties const& prop) -> bool {
			return std::strcmp(prop.layerName, validation_layer) == 0;
        }) == av_layers.end())
	{
		WIL_LOGERROR("Unable to find VK_LAYER_KHRONOS_validation layer");
	}

    // setup validation layer
    inst_ci.enabledLayerCount = 1;
    inst_ci.ppEnabledLayerNames = &validation_layer;

    // setup extensions (glfw + debug)
    std::vector extensions(glfw_extensions, glfw_extensions + glfw_ext_count);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    inst_ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    inst_ci.ppEnabledExtensionNames = extensions.data();

    // setup debug messenger parameters
    VkDebugUtilsMessengerCreateInfoEXT debug_ci{};
    debug_ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_ci.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_ci.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_ci.pfnUserCallback = DebugMessageCallback_;
    debug_ci.pUserData = nullptr;
    inst_ci.pNext = &debug_ci;

#else

    // setup extensions (glfw)
    inst_ci.enabledLayerCount = 0;
    inst_ci.enabledExtensionCount = glfw_ext_count;
    inst_ci.ppEnabledExtensionNames = glfw_extensions;

#endif

    if (vkCreateInstance(&inst_ci, nullptr, &vkinstance_) != VK_SUCCESS) {
		WIL_LOGFATAL("Unable to initialize Vulkan 1.3");
	}

#ifndef NDEBUG

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vkinstance_, "vkCreateDebugUtilsMessengerEXT"));
	if (!func) WIL_LOGERROR("Unable to locate vkCreateDebugUtilsMessengerEXT");
    func(vkinstance_, &debug_ci, nullptr, &vkdebug_);

#endif
}

static void TerminateAPIs_()
{
#ifndef NDEBUG

    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vkinstance_, "vkDestroyDebugUtilsMessengerEXT"));
	if (!func) WIL_LOGERROR("Unable to locate vkDestroyDebugUtilsMessengerEXT");
    func(vkinstance_, vkdebug_, nullptr);

#endif

	vkDestroyInstance(vkinstance_, nullptr);
	glfwTerminate();
}

App::App() : active_(true) {}

App &GetApp() {
	return *appinst_;
}

void appimpl(App *app, int argc, char **argv)
{
	appinst_ = app;
	InitAPIs_();

	AppInitCtx ctx;
	app->OnInit(ctx);

	WIL_ASSERT(!ctx.scenes_.empty() && "At least one scene are required to be provided");
	WIL_ASSERT(!ctx.start_scene.empty() && "Start scene needs to be specified");
	WIL_ASSERT(ctx.frames_in_flight && "Frames in flight must have value >= 1");

	app->frames_in_flight_ = ctx.frames_in_flight;

	app->window_ = new Window(vkinstance_, ctx.window);
	app->device_ = new Device(vkinstance_, app->window_->GetVkSurfacePtr_(),
			app->window_->GetFramebufferSize(), ctx.vsync);

	Window &window = *app->window_;
	Device &device = *app->device_;

	bool framebuffer_resized = false;

	window.SetEventHandler([&](WindowEvent &ev) {
		if (ctx.close_button_op && ev.type == ev.WINDOW_CLOSE_EVENT)
			app->active_ = false;
		if (ev.type == ev.FRAMEBUFFER_RESIZE_EVENT)
			framebuffer_resized = true;
		app->OnWindowEvent(ev);
	});

	for (auto [name, fn] : ctx.scenes_) {
		Scene *s = fn(device);
		app->scenes_[std::string(name)] = s;
	}

	app->current_scene_ = app->scenes_.at(ctx.start_scene);

	FrameData frame;
	frame.index = 0;

	auto prev = std::chrono::high_resolution_clock::now();

	while (app->active_) 
	{
		auto now = std::chrono::high_resolution_clock::now();
		frame.elapsed = (prev - now).count();
		if (frame.elapsed > 1.f) frame.elapsed = 1.f; // cap the time
		prev = now;
		frame.app_time = glfwGetTime();

		if (!app->current_scene_->Update(frame) || framebuffer_resized)
		{
			framebuffer_resized = false;
			app->device_->RecreateSwapchain(app->window_,
					app->window_->GetFramebufferSize(), ctx.vsync);
		}

		glfwPollEvents();
		frame.index = (frame.index + 1) % app->frames_in_flight_;
	}

	app->device_->WaitIdle();

	for (auto [_, scene] : app->scenes_) {
		delete scene;
	}

	delete app->device_;
	delete app->window_;

	TerminateAPIs_();
}

}
