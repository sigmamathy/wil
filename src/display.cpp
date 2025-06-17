#include <wil/display.hpp>
#include <wil/log.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

void Window::CreateWindowCallbacks_()
{
	GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr_);

	static constexpr auto getdata = [](GLFWwindow *win) -> WindowData& {
		return *static_cast<WindowData*>(glfwGetWindowUserPointer(win)); };

    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
        if (action == GLFW_REPEAT) return;
        auto& data = getdata(win);
        WindowEvent ev;
        ev.type = KEY_EVENT;
        ev.ke.code = key, ev.ke.down = action, ev.ke.mods = mods;
		data.event_handler(ev);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) -> void {
        auto& data = getdata(win);
        WindowEvent ev;
        ev.type = MOUSE_EVENT;
        ev.me.button = button, ev.me.down = action;
		data.event_handler(ev);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) -> void {
        auto& data = getdata(win);
        WindowEvent ev;
        ev.type = CURSOR_EVENT;
        ev.ce.pos = {xpos, ypos};
		ev.ce.delta = ev.ce.pos - data.cursor_pos;
		data.cursor_pos = ev.ce.pos;
		data.event_handler(ev);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoff, double yoff) -> void {
        auto& data = getdata(win);
        WindowEvent ev;
        ev.type = SCROLL_EVENT;
        ev.se.offset = {xoff, yoff};
		data.event_handler(ev);
    });

	glfwSetWindowCloseCallback(window, [](GLFWwindow *win) -> void {
        auto& data = getdata(win);
		WindowEvent ev;
		ev.type = WINDOW_CLOSE_EVENT;
		data.event_handler(ev);
	});

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int width, int height) {
        auto& data = getdata(win);
		WindowEvent ev;
		ev.type = FRAMEBUFFER_RESIZE_EVENT;
		ev.fre.size = { width, height };
		data.event_handler(ev);
	});

	glfwSetWindowPosCallback(window, [](GLFWwindow *win, int x, int y) {
        auto& data = getdata(win);
		WindowEvent ev;
		ev.type = WINDOW_MOVE_EVENT;
		ev.wme.pos = {x, y};
		data.event_handler(ev);
	});

	glfwSetWindowFocusCallback(window, [](GLFWwindow *win, int focused) {
        auto& data = getdata(win);
		WindowEvent ev;
		ev.type = WINDOW_FOCUS_EVENT;
		ev.wfe.focused = focused;
		data.event_handler(ev);
	});
}

const std::vector<Monitor> &GetMonitors()
{
	static std::vector<Monitor> monitors;
	
	if (monitors.empty())
	{
		int count;
		GLFWmonitor** ms = glfwGetMonitors(&count);
		monitors.reserve(count);

		for (int i = 0; i < count; i++)
		{
			auto* vid = glfwGetVideoMode(ms[i]);
			monitors.emplace_back(
				Ivec2(vid->width, vid->height),
				glfwGetMonitorName(ms[i]),
				ms[i]
			);
		}
	}

	return monitors;
}

Window::Window(VendorPtr vkinst, const WindowCtor &ctor) : vkinst_(vkinst)
{
	glfwWindowHint(GLFW_RESIZABLE, ctor.resizable);
	glfwWindowHint(GLFW_DECORATED, ctor.decorated);
	glfwWindowHint(GLFW_AUTO_ICONIFY, ctor.auto_iconify);
	glfwWindowHint(GLFW_FLOATING, ctor.always_on_top);
	glfwWindowHint(GLFW_MAXIMIZED, ctor.maximized);

	WIL_ASSERT(ctor.size != WIL_MONITOR_SIZE || ctor.monitor >= 0
			&& "monitor have value >= 0 if WIL_MONITOR_SIZE is used");

	Ivec2 size = ctor.size == WIL_MONITOR_SIZE ? GetMonitors()[ctor.monitor].size : ctor.size;

	GLFWwindow* window = glfwCreateWindow(
			size.x,
			size.y,
			ctor.title.c_str(),
			ctor.monitor >= 0 ? static_cast<GLFWmonitor*>(GetMonitors()[ctor.monitor].monitor_ptr_) : 0,
			0);

	window_ptr_ = window;

	if (!window)
		WIL_LOGFATAL("Unable to create GLFW window");

	data_.event_handler = [] (auto&) {};
	data_.cursor_enable = ctor.cursor_enable;
	data_.cursor_visible = ctor.cursor_visible;
	data_.cursor_pos = GetCursorPosition();

	if (!data_.cursor_enable) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else if (!data_.cursor_visible) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glfwSetWindowUserPointer(window, &data_);
    CreateWindowCallbacks_();

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

bool Window::IsKeyPressed(KeyCode keycode) const
{
	GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr_);
	return glfwGetKey(window, keycode) == GLFW_PRESS;
}

bool Window::IsMouseButtonPressed(MouseButton button) const
{
	GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr_);
	return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

Fvec2 Window::GetCursorPosition() const
{
	GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr_);
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return { x, y };
}

bool Window::IsFocused() const
{
	GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr_);
	return glfwGetWindowAttrib(window, GLFW_FOCUSED);
}

void Window::SetCursorEnable(bool enable)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr_);
	data_.cursor_enable = enable;
	if (enable) SetCursorVisible(data_.cursor_visible);
	else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::SetCursorVisible(bool visible)
{
	data_.cursor_visible = visible;
	if (data_.cursor_enable) {
		GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr_);
		glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
		data_.cursor_pos = GetCursorPosition();
	}
}

}
