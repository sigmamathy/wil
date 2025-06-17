#pragma once

#include "core.hpp"
#include "algebra.hpp"
#include <string>
#include <functional>

namespace wil {

struct KeyEvent
{
    int code;
    bool down;
};

struct MouseEvent
{
    int button;
    bool down;
};

struct CursorEvent
{
    Ivec2 pos;
};

struct ScrollEvent
{
    Ivec2 offset;
};

struct FramebufferResizeEvent
{
	Ivec2 size;
};

struct WindowMoveEvent
{
	Ivec2 pos;
};

struct WindowFocusEvent
{
	bool focused;
};

enum WindowEventType
{
	KEY_EVENT					= 0x1,
	MOUSE_EVENT					= 0x2,
	CURSOR_EVENT				= 0x4,
	SCROLL_EVENT				= 0x8,
	WINDOW_CLOSE_EVENT			= 0x10,
	FRAMEBUFFER_RESIZE_EVENT	= 0x20,
	WINDOW_MOVE_EVENT			= 0x40,
	WINDOW_FOCUS_EVENT			= 0x80,
};

struct WindowEvent
{
    union
    {
        KeyEvent ke;
        MouseEvent me;
        CursorEvent ce;
        ScrollEvent se;
		FramebufferResizeEvent fre;
		WindowMoveEvent wme;
		WindowFocusEvent wfe;
    };

	WindowEventType type;
};

WIL_ENUM_DEFINE_OR_OPERATOR(WindowEventType);

using WindowEventHandler = std::function<void(WindowEvent&)>;

struct WindowCtor
{
	Ivec2 size = {600, 480};
	std::string title = "";
	bool resizable = false;
};

class Window
{
public:

	Window(void *vkinst, const WindowCtor &ctor);

	~Window();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Window);

	void SetEventHandler(const WindowEventHandler &handler) { event_handler_ = handler; }

	Ivec2 GetFramebufferSize() const;

	VendorPtr GetGlfwWindowPtr_() { return window_ptr_; }

	VendorPtr GetVkSurfacePtr_() { return surface_ptr_; }

private:
	VendorPtr vkinst_;
	VendorPtr window_ptr_, surface_ptr_;
	WindowEventHandler event_handler_;
};

}
