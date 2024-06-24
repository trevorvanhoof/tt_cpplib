/*
TODO: The cursor does not always revert to the right type when the mouse leaves the window.

A Window class that receives messages in a slightly higher level format.
Subclass and implement the events. Note the CreateGLContext() utility function.

Example windows message loop implementation:

#include <windont.h>
#include <tt_window.h>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	TT::Window window;
	window.show();
	MSG msg;
	bool quit = false;

	do {
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				quit = true;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Detect when all windows are closed and exit this loop.
		if (!TT::Window::hasVisibleWindows())
			PostQuitMessage(0);
	} while (!quit);
	return (int)msg.wParam;
}

For rendering with OpenGL, you may choose to use this instead:

#include <windont.h>
#include <tt_window.h>
#include <tt_gl.h>

class App : public TT::Window {
public:
	App() : TT::Window() {
		createGLContext();
		TT::loadGLFunctions();
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		show();
	}

private:
	void onResizeEvent(const TT::ResizeEvent& event) override {
		glViewport(0, 0, event.width, event.height);
	}

	void onPaintEvent(const TT::PaintEvent& event) override {
		glClear(GL_COLOR_BUFFER_BIT);
		SwapBuffers(getGLContext());
	}
};

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	App window;
	MSG msg;
	bool quit = false;

	// Track frame timings
	ULONGLONG a = GetTickCount64();
	ULONGLONG b;
	constexpr ULONGLONG FPS = 30;
	constexpr ULONGLONG t = 1000 / FPS;

	do {
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				quit = true;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Force UI repaints as fast as possible
		window.repaint();
		// Detect when all windows are closed and exit this loop.
		if (!TT::Window::hasVisibleWindows())
			PostQuitMessage(0);

		// Sleep for remaining frame time
		b = a;
		a = GetTickCount64();
		b = a - b;
		if (b < t) Sleep((DWORD)(t - b));
	} while (!quit);
	return (int)msg.wParam;
}
*/
#pragma once

#include <functional>
#include <unordered_map>
#include <string>

struct HINSTANCE__;
struct HWND__;
struct HDC__;
#ifdef _WIN64
typedef unsigned long long WPARAM;
typedef long long LPARAM;
typedef long long LRESULT;
#else
typedef unsigned int WPARAM;
typedef long LPARAM;
typedef long LRESULT;
#endif

#ifndef TT_LIT
#ifdef UNICODE
#define TT_LIT(text) L##text
typedef wchar_t char_t;
typedef std::wstring str_t;
#else
#define TT_LIT(text) text
typedef char char_t;
typedef std::string str_t;
#endif
#endif

namespace TT {
	struct Event {
		enum class Type {
			Invalid,
			MouseDown,
			MouseUp,
			MouseDoubleClick,
			MouseMove,
			Wheel,
			KeyDown,
			KeyUp,
			Resize,
			Paint,
			Focus,
			Close,
		};
		Type type = Type::Invalid;
	};

	enum class Modifiers {
		None = 0b000,
		Ctrl = 0b001,
		Shift = 0b010,
		Alt = 0b100,
	};

	struct PaintEvent : public Event {
		PaintEvent() { type = Event::Type::Paint; }
	};

	struct ResizeEvent : public Event {
		ResizeEvent() { type = Event::Type::Resize; }
		ResizeEvent(int width, int height) : width(width), height(height) { type = Event::Type::Resize; }
		int width = 0;
		int height = 0;
	};

	struct MouseEvent : public Event {
		int button = 0; // -1 in case of move
		int x = 0;
		int y = 0;
		Modifiers modifiers = Modifiers::None;
	};

	struct KeyEvent : public Event {
		unsigned int key; // virtual key code, TODO: make some massive enum mapping to avoid having this as windows-specific
		str_t txt; // translated character(s)
		bool isRepeat;
		Modifiers modifiers = Modifiers::None;
	};

	struct WheelEvent : public Event {
		WheelEvent() { type = Event::Type::Wheel; }
		int delta = 0;
		int x = 0;
		int y = 0;
		Modifiers modifiers = Modifiers::None;
	};

    struct FocusEvent : public Event {
        FocusEvent() { type = Event::Type::Focus; }
        // 0 if focus lost, 1 if focus gained, -1 if unknown
        char hasMouseFocus = -1;
        char hasKeyboardFocus = -1;
    };

    struct CloseEvent : public Event {
        CloseEvent() { type = Event::Type::Close; }
    };

    enum class WindowFlags : unsigned char {
        // If windowed is not allowed, window will go to fullscreen automatically on startup (or show if startup does not work).
        AllowWindowed      = 0b0000'0001,
        AllowFullscreen    = 0b0000'0010,
        // Only one of these is used at a time, they mean the same if the window does not support fullscreen and windowed modes at the same time.
        AltEnterMaximize   = 0b0000'0100,
        AltEnterFullscreen = 0b0000'1000,
        // Only one of these is used at a time
        F11Maximize        = 0b0001'0000,
        F11Fullscreen      = 0b0010'0000,
        // If both of these are true, and we can go to windowed mode, esc first enters windowed mode and then closes the window.
        EscLeaveFullscreen = 0b0100'0000,
        EscClose           = 0b1000'0000,
        // Defaults
        Default = AllowWindowed | AllowFullscreen | AltEnterMaximize | F11Fullscreen | EscLeaveFullscreen,
        Demo = AllowFullscreen | EscClose,
    };

	class Window {
		// the window callback can't be a bound function, so we have to track it ourselves
		static std::unordered_map<HWND__*, Window*> dankjewelwindows;
		static LRESULT __stdcall windowProc(HWND__* hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam);

		int _width;
		int _height;

		void handleEvent(const Event& event);

        // Fullscreen helpers
        struct WindowState {
            bool isFullscreen;
            bool isMaximized;
            int style;
            int exStyle;
            int rect[4];
        };

        WindowState state;

        static void toggleFullscreen(HWND__* hwnd, WindowState& ioState);
        static void toggleMaximize(HWND__* hwnd, WindowState& ioState);

	protected:
		HWND__* window;

		bool enableMouseTracking = false;
		unsigned int mouseButtonStates = 0;
        WindowFlags flags = WindowFlags::Default;

		virtual void onPaintEvent(const PaintEvent& event) {}
		virtual void onResizeEvent(const ResizeEvent& event) {}
		virtual void onMouseEvent(const MouseEvent& event) {}
		virtual void onWheelEvent(const WheelEvent& event) {}
		virtual void onKeyEvent(const KeyEvent& event) {}
		virtual void onFocusEvent(const FocusEvent& event) {}
		virtual void onCloseEvent(const CloseEvent& event) {}

	public:
		static bool hasVisibleWindows();

		Window(const char_t* windowTitle = TT_LIT("Window"), HINSTANCE__* hInstance = 0, WindowFlags flags = WindowFlags::Default);
		~Window();

		void repaint() const;
		void show();
		int width() const;
		int height() const;
        void close() const;
        void toggleFullscreen();
        void toggleMaximize();

		std::function<void(Event&)> eventHandler; // Uses Window::handlEvent() by default.

		HWND__* windowHandle() const;
	};
}
