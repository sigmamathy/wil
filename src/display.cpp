#include <wil/display.hpp>
#include <wil/log.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

static auto& GetEventHandler_(GLFWwindow *win)
{
	return *static_cast<WindowEventHandler*>(glfwGetWindowUserPointer(win));
}

static void CreateWindowCallback_(GLFWwindow* window)
{
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
        if (action == GLFW_REPEAT) return;
        auto& func = GetEventHandler_(win);
        WindowEvent ev;
        ev.type = WindowEvent::KEY_EVENT;
        ev.ke.code = key, ev.ke.down = action;
        func(ev);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) -> void {
        auto& func = GetEventHandler_(win);
        WindowEvent ev;
        ev.type = WindowEvent::MOUSE_EVENT;
        ev.me.button = button, ev.me.down = action;
        func(ev);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) -> void {
        auto& func = GetEventHandler_(win);
        WindowEvent ev;
        ev.type = WindowEvent::CURSOR_EVENT;
        ev.ce.pos = {xpos, ypos};
        func(ev);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoff, double yoff) -> void {
        auto& func = GetEventHandler_(win);
        WindowEvent ev;
        ev.type = WindowEvent::SCROLL_EVENT;
        ev.se.offset = {xoff, yoff};
        func(ev);
    });

	glfwSetWindowCloseCallback(window, [](GLFWwindow *win) -> void {
        auto& func = GetEventHandler_(win);
		WindowEvent ev;
		ev.type = ev.WINDOW_CLOSE_EVENT;
        func(ev);
	});

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int width, int height) {
        auto& func = GetEventHandler_(win);
		WindowEvent ev;
		ev.type = ev.FRAMEBUFFER_RESIZE_EVENT;
		ev.fre.size = { width, height };
		func(ev);
	});
}

Window::Window(void *vkinst, const WindowCtor &ctor) : vkinst_(vkinst), event_handler_([](auto&){})
{
	glfwWindowHint(GLFW_RESIZABLE, ctor.resizable);
	GLFWwindow* window = glfwCreateWindow(ctor.size.x, ctor.size.y, ctor.title.c_str(), 0, 0);
	if (!window)
		WIL_LOGFATAL("Unable to create GLFW window");
	window_ptr_ = window;
    glfwSetWindowUserPointer(window, &event_handler_);
    CreateWindowCallback_(window);

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(static_cast<VkInstance>(vkinst), window, nullptr, &surface) != VK_SUCCESS)
		WIL_LOGFATAL("Unable to create window surface");
	surface_ptr_ = surface;
}

Window::~Window()
{
    vkDestroySurfaceKHR(static_cast<VkInstance>(vkinst_), static_cast<VkSurfaceKHR>(surface_ptr_), nullptr);
	glfwDestroyWindow(static_cast<GLFWwindow*>(window_ptr_));
}

Ivec2 Window::GetFramebufferSize() const
{
	Ivec2 res;
	glfwGetFramebufferSize(static_cast<GLFWwindow*>(window_ptr_), &res.x, &res.y);
	return res;
}

}
