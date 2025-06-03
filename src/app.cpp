#include <wil/appimpl.hpp>
#include <wil/log.hpp>
#include <wil/pipeline.hpp>
#include <wil/drawsync.hpp>

#include <algorithm>
#include <cstring>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

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
		LogErr("Unable to find VK_LAYER_KHRONOS_validation layer");
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

    if (vkCreateInstance(&inst_ci, nullptr, &vkinstance_) != VK_SUCCESS) {
		LogFatal("Unable to initialize Vulkan 1.3");
	}

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vkinstance_, "vkCreateDebugUtilsMessengerEXT"));
	if (!func) LogErr("Unable to locate vkCreateDebugUtilsMessengerEXT");
    func(vkinstance_, &debug_ci, nullptr, &vkdebug_);
}

static void TerminateAPIs_()
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vkinstance_, "vkDestroyDebugUtilsMessengerEXT"));
	if (!func) LogErr("Unable to locate vkDestroyDebugUtilsMessengerEXT");
    func(vkinstance_, vkdebug_, nullptr);

	vkDestroyInstance(vkinstance_, nullptr);
	glfwTerminate();
}

App::App() : active_(true) {}

AppInitCtx App::CreateAppInitCtx_()
{
	AppInitCtx ctx;
	ctx.layers_ = &layers_;
	return ctx;
}

void appimpl(App *app, int argc, char **argv)
{
	InitAPIs_();

	AppInitCtx ctx = app->CreateAppInitCtx_();
	app->OnInit(ctx);

	app->window_ = new Window(vkinstance_, ctx.window);

	app->window_->SetEventHandler([&](WindowEvent &ev) {
		if (ctx.close_button_op && ev.type == ev.WINDOW_CLOSE_EVENT)
			app->active_ = false;
		app->OnWindowEvent(ev);
	});

	app->device_ = new Device(vkinstance_, app->window_->GetVkSurfacePtr_(), app->window_->GetFramebufferSize());

	for (auto [_, layer] : app->layers_)
		layer->Init(*app->device_);

	LogInfo("wwww");

	auto* sync = new DrawPresentSynchronizer(*app->device_, 1);

	while (app->active_) 
	{
		uint32_t index = sync->AcquireImageIndex();
		std::vector<CommandBuffer*> cmds;
		cmds.push_back(&app->layers_.begin()->second->Render(index));
		sync->SubmitDraw(cmds);
		sync->PresentToScreen(index);

		glfwPollEvents();
	}

	app->device_->WaitIdle();

	delete sync;

	for (auto [_, layer] : app->layers_)
		layer->Free();

	delete app->device_;
	delete app->window_;

	for (auto [_, layer] : app->layers_)
		delete layer;

	TerminateAPIs_();
}

}
