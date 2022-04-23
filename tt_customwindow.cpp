#include "tt_customwindow.h"
#include "tt_messages.h"
#include <windows.h>


namespace TT {
	Window::Window(const char* windowTitle, HINSTANCE hInstance) {
		// Create our own window class
		if (!haveWindowClass) {
			haveWindowClass = true;

			WNDCLASSA wc = { 0 };
			wc.lpfnWndProc = Window::WindowProc;
			wc.lpszClassName = "TTWindowClass";
			RegisterClassA(&wc);
		}

		window = CreateWindowExA(0, "TTWindowClass", windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
		Assert(window);
		dankjewelwindows[window] = this;

		eventHandler = std::bind(&Window::HandleEvent, this, std::placeholders::_1);
		// CreateGLContext();
		// Show();
	}

	int Window::Width() const {
		return width;
	}

	int Window::Height() const {
		return height;
	}

	void Window::Show() {
		ShowWindow(window, 1);
		SetActiveWindow(window);
		SetFocus(window);

		RECT rect;
		GetClientRect(window, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}

	HDC Window::CreateGLContext() {
		HDC device = GetDC(window);
		const PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0 };
		SetPixelFormat(device, ChoosePixelFormat(device, &pfd), &pfd);
		wglMakeCurrent(device, wglCreateContext(device));
		return device;
	}

	HDC Window::GetGLContext() {
		return GetDC(window);
	}

	bool Window::HasVisibleWindows() {
		for (auto pair : dankjewelwindows) {
			if (IsWindowVisible(pair.second->window))
				return true;
		}
		return false;
	}

	// Callback forwarding
	std::unordered_map<HWND, Window*> Window::dankjewelwindows;
	bool Window::haveWindowClass = false;
	unsigned int Window::modifierStates = 0;
	static MouseEvent CreateMouseEvent(WPARAM wParam, LPARAM lParam, Event::EType type, int button) {
		MouseEvent event;
		event.type = type;
		event.button = button;
		if ((wParam & MK_CONTROL) == MK_CONTROL) {
			event.modifiers = (TT::Modifiers)((int)event.modifiers | (int)TT::Modifiers::Ctrl);
		}
		if ((wParam & MK_SHIFT) == MK_SHIFT) {
			event.modifiers = (TT::Modifiers)((int)event.modifiers | (int)TT::Modifiers::Shift);
		}
		event.x = ((int)(short)LOWORD(lParam));
		event.y = ((int)(short)HIWORD(lParam));
		return event;
	}

	// TODO: We can try and do focus events but they are out of scope today; see WM_MOUSEACTIVATE, & WM_MOUSELEAVE
	// TODO: Need to investigate WM_SYSKEYDOWN and could look into media keys
	LRESULT __stdcall Window::WindowProc(HWND__* hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		static unsigned char keyboard_state[256] = { 0 };
		// This proc not only forwards the event handling to the Window class, but also repackages the data.
		// Skip windows that we did not create.
		auto it = dankjewelwindows.find(hwnd);
		if (it == dankjewelwindows.end() || it->second->eventHandler == nullptr)
			return DefWindowProcA(hwnd, uMsg, wParam, lParam);
		switch (uMsg) {
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_PAINT: {
			PAINTSTRUCT ignored;
			BeginPaint(hwnd, &ignored);
			PaintEvent event;
			it->second->eventHandler(event);
			EndPaint(hwnd, &ignored);
			break;
		}
		case WM_SIZE: {
			RECT rect;
			GetClientRect(it->second->window, &rect);
			it->second->width = rect.right - rect.left;
			it->second->height = rect.bottom - rect.top;
			ResizeEvent event { it->second->width, it->second->height };
			it->second->eventHandler(event);
			break;
		}
		case WM_SIZING: {
			RECT rect;
			GetClientRect(it->second->window, &rect);
			it->second->width = rect.right - rect.left;
			it->second->height = rect.bottom - rect.top;
			ResizeEvent event { it->second->width, it->second->height };
			it->second->eventHandler(event);
			break;
		}
		case WM_LBUTTONDOWN:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDown, 0); it->second->eventHandler(event); }
		break;
		case WM_LBUTTONDBLCLK:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDoubleClick, 0); it->second->eventHandler(event); }
		break;
		case WM_LBUTTONUP:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseUp, 0); it->second->eventHandler(event); }
		break;
		case WM_MBUTTONDOWN:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDown, 1); it->second->eventHandler(event); }
		break;
		case WM_MBUTTONDBLCLK:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDoubleClick, 1); it->second->eventHandler(event); }
		break;
		case WM_MBUTTONUP:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseUp, 1); it->second->eventHandler(event); }
		break;
		case WM_RBUTTONDOWN:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDown, 2); it->second->eventHandler(event); }
		break;
		case WM_RBUTTONDBLCLK:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDoubleClick, 2); it->second->eventHandler(event); }
		break;
		case WM_RBUTTONUP:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseUp, 2); it->second->eventHandler(event); }
		break;
		case WM_XBUTTONDOWN:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDown, (wParam >> 4) == 0x0001 ? 3 : 4); it->second->eventHandler(event); }
		break;
		case WM_XBUTTONDBLCLK:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseDoubleClick, (wParam >> 4) == 0x0001 ? 3 : 4); it->second->eventHandler(event); }
		break;
		case WM_XBUTTONUP:
		{ auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseUp, (wParam >> 4) == 0x0001 ? 3 : 4); it->second->eventHandler(event); }
		break;
		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_MOUSEWHEEL:
		{
			WheelEvent event;
			event.type = Event::EType::Wheel;
			if ((wParam & MK_CONTROL) == MK_CONTROL) {
				event.modifiers = (TT::Modifiers)((int)event.modifiers | (int)TT::Modifiers::Ctrl);
			}
			if ((wParam & MK_SHIFT) == MK_SHIFT) {
				event.modifiers = (TT::Modifiers)((int)event.modifiers | (int)TT::Modifiers::Shift);
			}
			event.x = ((int)(short)LOWORD(lParam));
			event.y = ((int)(short)HIWORD(lParam));
			event.delta = (short)HIWORD(wParam);
			it->second->eventHandler(event);
			break;
		}
		case WM_SETCURSOR:
			// For some reason the arrow always also shows the waiting icon
			// but other cursors don't.
			SetCursor(LoadCursor(0, IDC_HAND));
			break;
		case WM_MOUSEMOVE:
		{
			auto event = CreateMouseEvent(wParam, lParam, Event::EType::MouseMove, -1);
			it->second->eventHandler(event);
			break;
		}
		case WM_KEYDOWN:
		{
			char buffer[sizeof(WORD) + 1] = { 0 };
			if (GetKeyboardState(keyboard_state)) {
				int numChars = ToAsciiEx((UINT)wParam,
					(lParam >> 16) & 0b1111111,
					keyboard_state,
					(unsigned short*)buffer, 0 /*assume no menu active*/,
					GetKeyboardLayout(0));
			}
			// if 1 we have a single char in buffer[0]
			// if 2 we have a diacritic in buffer[1], I think that makes this a utf16 char?
			// for now let's just ignore that...
			/*std::string s;
			if (numChars == 0)
				s = Format("VK: %d\n", wParam);
			else
				s = Format("CHR: %d %s\n", buffer[0], buffer);
			OutputDebugStringA(s.c_str());*/
			KeyEvent event;
			event.isRepeat = (bool)(lParam & 0xF);
			event.key = (UINT)wParam;
			event.chr = buffer[0];
			event.type = Event::EType::KeyDown;

			switch (wParam) {
			case VK_SHIFT: modifierStates |= 0b000'000'001u; break;
			case VK_LSHIFT: modifierStates |= 0b000'000'010u; break;
			case VK_RSHIFT: modifierStates |= 0b000'000'100u; break;
			case VK_CONTROL: modifierStates |= 0b000'001'000u; break;
			case VK_LCONTROL: modifierStates |= 0b000'010'000u; break;
			case VK_RCONTROL: modifierStates |= 0b000'100'000u; break;
			}

			it->second->eventHandler(event);
			break;
		}
		case WM_KEYUP:
		{ // Reimplemented from key down
			char buffer[sizeof(WORD) + 1] = { 0 };
			if (GetKeyboardState(keyboard_state))
				int numChars = ToAsciiEx((UINT)wParam, (lParam >> 16) & 0b1111111, keyboard_state, (unsigned short*)buffer, 0, GetKeyboardLayout(0));
			KeyEvent event;
			event.isRepeat = false;
			event.key = (UINT)wParam;
			event.chr = buffer[0];
			event.type = Event::EType::KeyUp;

			switch (wParam) {
			case VK_SHIFT: modifierStates &= ~0b000'000'001u; break;
			case VK_LSHIFT: modifierStates &= ~0b000'000'010u; break;
			case VK_RSHIFT: modifierStates &= ~0b000'000'100u; break;
			case VK_CONTROL: modifierStates &= ~0b000'001'000u; break;
			case VK_LCONTROL: modifierStates &= ~0b000'010'000u; break;
			case VK_RCONTROL: modifierStates &= ~0b000'100'000u; break;
			}
			int modifiers = (((modifierStates & 0b000111) != 0) ? (int)Modifiers::Shift : 0) | (((modifierStates & 0b111000) != 0) ? (int)Modifiers::Ctrl : 0);
			event.modifiers = (Modifiers)modifiers;

			it->second->eventHandler(event);
			break;
		}
		default:
			return DefWindowProcA(hwnd, uMsg, wParam, lParam);
		}
		return 1;
	}

	void Window::HandleEvent(TT::Event& event) {
		if (event.type == TT::Event::EType::Paint) {
			OnPaintEvent((TT::PaintEvent&)event);
		} else if (event.type == TT::Event::EType::Resize)
			OnResizeEvent((TT::ResizeEvent&)event);
		else if (event.type == TT::Event::EType::MouseDown) {
			mouseButtonStates |= 1u << ((TT::MouseEvent&)event).button;
			OnMouseEvent((TT::MouseEvent&)event);
		} else if (event.type == TT::Event::EType::MouseUp) {
			mouseButtonStates &= ~(1u << ((TT::MouseEvent&)event).button);
			OnMouseEvent((TT::MouseEvent&)event);
		} else if (event.type == TT::Event::EType::MouseMove && (enableMouseTracking || mouseButtonStates != 0))
			OnMouseEvent((TT::MouseEvent&)event);
		else if (event.type == TT::Event::EType::MouseDoubleClick)
			OnMouseEvent((TT::MouseEvent&)event);
		else if (event.type == TT::Event::EType::Wheel)
			OnWheelEvent((TT::WheelEvent&)event);
		else if (event.type == TT::Event::EType::KeyDown || event.type == TT::Event::EType::KeyUp)
			OnKeyEvent((TT::KeyEvent&)event);
	}

	void Window::Repaint() { InvalidateRect(window, 0, true); }
}
