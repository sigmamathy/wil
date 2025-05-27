#pragma once

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

struct WindowEvent
{
    union
    {
        KeyEvent ke;
        MouseEvent me;
        CursorEvent ce;
        ScrollEvent se;
    };

    enum EventType
    {
        KEY_EVENT,
        MOUSE_EVENT,
        CURSOR_EVENT,
        SCROLL_EVENT,
		WINDOW_CLOSE_EVENT
    } type;
};

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

	Window(const Window&) = delete;

	void SetEventHandler(const WindowEventHandler &handler) { event_handler_ = handler; }

	void *GetSurfacePtr() { return surface_ptr_; }

	Ivec2 GetFramebufferSize() const;

private:
	void *vkinst_;
	void *window_ptr_, *surface_ptr_;
	WindowEventHandler event_handler_;
};

}
