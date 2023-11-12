/*
A Window class that receives messages in a slightly higher level format.
Subclass and implement the events. Note the CreateGLContext() utility function.

Example windows message loop implementation:

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	TT::Window window;
	window.Show();
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
		// Force UI repaints as fast as possible
		// TT::PaintEvent event;
		// window.HandleEvent(event);
		// Detect when all windows are closed and exit this loop.
		if (!TT::Window::HasVisibleWindows())
			PostQuitMessage(0);
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
		enum class EType {
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
		EType type = EType::Invalid;
	};

	enum class Modifiers {
		None = 0b000,
		Ctrl = 0b001,
		Shift = 0b010,
		Alt = 0b100,
	};

	struct PaintEvent : public Event {
		PaintEvent() { type = Event::EType::Paint; }
	};

	struct ResizeEvent : public Event {
		ResizeEvent() { type = Event::EType::Resize; }
		ResizeEvent(int width, int height) : width(width), height(height) { type = Event::EType::Resize; }
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
		WheelEvent() { type = Event::EType::Wheel; }
		int delta = 0;
		int x = 0;
		int y = 0;
		Modifiers modifiers = Modifiers::None;
	};

    struct FocusEvent : public Event {
        FocusEvent() { type = Event::EType::Focus; }
        // 0 if focus lost, 1 if focus gained, -1 if unknown
        char hasMouseFocus = -1;
        char hasKeyboardFocus = -1;
    };

    struct CloseEvent : public Event {
        CloseEvent() { type = Event::EType::Close; }
    };

	class Window {
		// the window callback can't be a bound function, so we have to track it ourselves
		static std::unordered_map<HWND__*, Window*> dankjewelwindows;
		static LRESULT __stdcall windowProc(HWND__* hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

		HWND__* window;
		int _width;
		int _height;

		void handleEvent(const Event& event);

	protected:
		bool enableMouseTracking = false;
		unsigned int mouseButtonStates = 0;

		virtual void onPaintEvent(const PaintEvent& event) {}
		virtual void onResizeEvent(const ResizeEvent& event) {}
		virtual void onMouseEvent(const MouseEvent& event) {}
		virtual void onWheelEvent(const WheelEvent& event) {}
		virtual void onKeyEvent(const KeyEvent& event) {}
		virtual void onFocusEvent(const FocusEvent& event) {}
		virtual void onCloseEvent(const CloseEvent& event) { delete this; }

	public:
		static bool hasVisibleWindows();

		Window(const char_t* windowTitle = "Window", HINSTANCE__* hInstance = 0);
		~Window();

		void repaint() const;
		void show();
		int width() const;
		int height() const;

		std::function<void(Event&)> eventHandler; // Uses Window::handlEvent() by default.
		HDC__* getGLContext() const;
		HDC__* createGLContext() const;
	};
}
