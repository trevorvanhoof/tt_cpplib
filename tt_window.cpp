#include "tt_window.h"
#include "tt_messages.h"
#include "windont.h"


namespace TT {
	Window::Window(const char_t* windowTitle, HINSTANCE hInstance, WindowFlags flags) : flags(flags) {
		// Create our own window class
		static bool haveWindowClass = false;
		if (!haveWindowClass) {
			haveWindowClass = true;

			WNDCLASS wc = { 0 };
			wc.lpfnWndProc = Window::windowProc;
			wc.lpszClassName = TT_LIT("TTWindowClass");
			wc.style = CS_DBLCLKS;
            wc.hCursor = LoadCursor(nullptr, IDC_HAND);
			RegisterClass(&wc);
		}

		window = CreateWindowEx(0, TT_LIT("TTWindowClass"), windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
		assert(window);
		dankjewelwindows[window] = this;

        // If windowed is not allowed, window will go to fullscreen automatically on startup.
        if( (int)flags & (int)WindowFlags::AllowFullscreen && !((int)flags & (int)WindowFlags::AllowWindowed) )
            toggleFullscreen();

		eventHandler = std::bind(&Window::handleEvent, this, std::placeholders::_1);
	}

	Window::~Window() {
	    DestroyWindow(window);
	    dankjewelwindows.erase(window);
	}

	int Window::width() const {
		return _width;
	}

	int Window::height() const {
		return _height;
	}

	void Window::show() {
		ShowWindow(window, 1);
		SetActiveWindow(window);
		SetFocus(window);

		RECT rect;
		GetClientRect(window, &rect);
		_width = rect.right - rect.left;
		_height = rect.bottom - rect.top;
	}

	bool Window::hasVisibleWindows() {
		for (auto pair : dankjewelwindows) {
			if (IsWindowVisible(pair.second->window))
				return true;
		}
		return false;
	}

	// Callback forwarding
	std::unordered_map<HWND, Window*> Window::dankjewelwindows;

	namespace {
        MouseEvent createMouseEvent(WPARAM wParam, LPARAM lParam, Event::Type type, int button) {
            MouseEvent event;
            event.type = type;
            event.button = button;
            if ((wParam & MK_CONTROL) == MK_CONTROL)
                event.modifiers = (TT::Modifiers)((int)event.modifiers | (int)TT::Modifiers::Ctrl);
            if ((wParam & MK_SHIFT) == MK_SHIFT)
                event.modifiers = (TT::Modifiers)((int)event.modifiers | (int)TT::Modifiers::Shift);
            if(GetKeyState(VK_MENU) & 0x8000)
                event.modifiers = (TT::Modifiers)((int)event.modifiers | (int)TT::Modifiers::Alt);
            event.x = ((int)(short)LOWORD(lParam));
            event.y = ((int)(short)HIWORD(lParam));
            return event;
        }

        struct KeyboardModifiers {
            int state = 0;

            void press(size_t wParam) {
                switch (wParam) {
                case VK_SHIFT:    state |= 0b000'000'001u; break;
                case VK_LSHIFT:   state |= 0b000'000'010u; break;
                case VK_RSHIFT:   state |= 0b000'000'100u; break;
                case VK_CONTROL:  state |= 0b000'001'000u; break;
                case VK_LCONTROL: state |= 0b000'010'000u; break;
                case VK_RCONTROL: state |= 0b000'100'000u; break;
                case VK_MENU:     state |= 0b001'000'000u; break;
                case VK_LMENU:    state |= 0b010'000'000u; break;
                case VK_RMENU:    state |= 0b100'000'000u; break;
                }
            }

            void release(size_t wParam) {
                switch (wParam) {
                case VK_SHIFT:    state &= ~0b000'000'001u; break;
                case VK_LSHIFT:   state &= ~0b000'000'010u; break;
                case VK_RSHIFT:   state &= ~0b000'000'100u; break;
                case VK_CONTROL:  state &= ~0b000'001'000u; break;
                case VK_LCONTROL: state &= ~0b000'010'000u; break;
                case VK_RCONTROL: state &= ~0b000'100'000u; break;
                case VK_MENU:     state &= ~0b001'000'000u; break;
                case VK_LMENU:    state &= ~0b010'000'000u; break;
                case VK_RMENU:    state &= ~0b100'000'000u; break;
                }
            }
        };

        KeyEvent createKeyEvent(size_t wParam, ptrdiff_t lParam, bool isKeyDown) {
            // Global to track the current modifiers pressed.
            static KeyboardModifiers modifiers;

            // Global to cache keyboard state into.
            static unsigned char keyboardState[256] = { 0 };

#ifdef UNICODE
            // Translate the key into unicode characters typed.
            wchar_t buffer[16] = { 0 };
            if (GetKeyboardState(keyboardState)) {
                // TODO: If ToUnicodeEx writes more chars than fit the buffer
                // we will be writing out of bounds here. Check this, and preferably
                // figure out the max wbuffer size required.
                buffer[ToUnicodeEx(
                    (UINT)wParam,
                    (lParam >> 16) & 0b1111111,
                    keyboardState,
                    buffer,
                    16,
                    0,
                    GetKeyboardLayout(0)
                )] = L'\0';
            }
#else
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
#endif

            // SYSKEY events on Windows should actually get sent to the menu bar
            // to trigger shortcuts like ALT->F to open the File menu. For now we
            // will ignore this.
            // bool isMenuCommand = lParam & (1 << 29);

            // Build the event.
            KeyEvent event;
            event.isRepeat = (bool)(lParam & (1 << 30));
            event.key = (UINT)wParam;
            event.txt = buffer;
            event.type = isKeyDown ? Event::Type::KeyDown : Event::Type::KeyUp;

            if (isKeyDown)
                modifiers.press(wParam);
            else
                modifiers.release(wParam);

            event.modifiers = (Modifiers)(
                  (((modifiers.state & 0b000'000'111) != 0) ? (int)Modifiers::Shift : 0)
                | (((modifiers.state & 0b000'111'000) != 0) ? (int)Modifiers::Ctrl : 0)
                | (((modifiers.state & 0b111'000'000) != 0) ? (int)Modifiers::Alt : 0));

            return event;
        }
	}

	LRESULT __stdcall Window::windowProc(HWND__* hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam) {
		static unsigned char keyboard_state[256] = { 0 };
		// This proc not only forwards the event handling to the Window class, but also repackages the data.
		// Skip windows that we did not create.
		auto it = dankjewelwindows.find(hwnd);
		if (it == dankjewelwindows.end() || it->second == nullptr || it->second->eventHandler == nullptr)
			return DefWindowProc(hwnd, msg, wParam, lParam);

        // Detect mouse button
        int mouse = -1;
        switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
            mouse = 0;
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
            mouse = 1;
            break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONUP:
            mouse = 2;
            break;
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK:
        case WM_XBUTTONUP:
            mouse = (wParam >> 4) == 0x0001 ? 3 : 4;
            break;
        }

		switch (msg) {
		case WM_CLOSE:
			{ CloseEvent e; it->second->eventHandler(e); }
			return DefWindowProc(hwnd, msg, wParam, lParam);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN: {
		    auto event = createMouseEvent(wParam, lParam, Event::Type::MouseDown, mouse);
		    it->second->eventHandler(event);
		    break;
		}

		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK: {
		    auto event = createMouseEvent(wParam, lParam, Event::Type::MouseDoubleClick, mouse);
		    it->second->eventHandler(event);
		    break;
        }

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP: {
		    auto event = createMouseEvent(wParam, lParam, Event::Type::MouseUp, mouse);
		    it->second->eventHandler(event);
		    break;
        }

		case WM_MOUSEMOVE: {
			auto event = createMouseEvent(wParam, lParam, Event::Type::MouseMove, -1);
			it->second->eventHandler(event);
			break;
		}

        case WM_SYSKEYDOWN:
            if(lParam & (1 << 29)) { LRESULT result = DefWindowProcA(hwnd, msg, wParam, lParam); if(result) return result; }
		case WM_KEYDOWN: {
            auto event = createKeyEvent(wParam, lParam, true);

            WindowFlags flags = it->second->flags;

            bool allowWindowed = (int)flags & (int)WindowFlags::AllowWindowed;
            bool allowFullscreen = (int)flags & (int)WindowFlags::AllowFullscreen;
            bool altEnterFullscreen = (int)flags & (int)WindowFlags::AltEnterFullscreen;
            bool f11Fullscreen = (int)flags & (int)WindowFlags::F11Fullscreen;

            // Alt+Enter
            if((((int)flags & (int)WindowFlags::AltEnterMaximize) || altEnterFullscreen) &&
                event.key == VK_RETURN && ((int)event.modifiers & (int)TT::Modifiers::Alt)) {
                // We can toggle fullscreen and this shortcut should toggle.
                if (allowWindowed && allowFullscreen && altEnterFullscreen)
                    it->second->toggleFullscreen();
                // We allow winowed mode so we toggle maximize, will be a noop if we are in fullscreen mode.
                else if (allowWindowed)
                    it->second->toggleMaximize();
            }

            // F11
            else if((((int)flags & (int)WindowFlags::F11Maximize) || f11Fullscreen) && event.key == VK_F11) {
                // We can toggle fullscreen and this shortcut should toggle.
                if (allowWindowed && allowFullscreen && f11Fullscreen)
                    it->second->toggleFullscreen();
                // We allow winowed mode so we toggle maximize, will be a noop if we are in fullscreen mode.
                else if (allowWindowed)
                    it->second->toggleMaximize();
            }

            // Esc
            else if(event.key == VK_ESCAPE) {
                if ((int)flags & (int)WindowFlags::EscLeaveFullscreen && allowWindowed && it->second->state.isFullscreen)
                    it->second->toggleFullscreen();
                else if ((int)flags & (int)WindowFlags::EscClose)
                    it->second->close();
            } else {
                it->second->eventHandler(event);
            }

			break;
		}
		case WM_SYSKEYUP:
            if(lParam & (1 << 29)) { LRESULT result = DefWindowProcA(hwnd, msg, wParam, lParam); if(result) return result; }
		case WM_KEYUP: {
            auto event = createKeyEvent(wParam, lParam, false);
			it->second->eventHandler(event);
			break;
		}

		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_MOUSEWHEEL: {
			WheelEvent event;
			event.type = Event::Type::Wheel;
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

		case WM_SIZE:
		case WM_SIZING: {
			RECT rect;
			GetClientRect(it->second->window, &rect);
			it->second->_width = rect.right - rect.left;
			it->second->_height = rect.bottom - rect.top;
			ResizeEvent event { it->second->_width, it->second->_height };
			it->second->eventHandler(event);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT ignored;
			BeginPaint(hwnd, &ignored);
			PaintEvent event;
			it->second->eventHandler(event);
			EndPaint(hwnd, &ignored);
			break;
		}

        case WM_MOUSEACTIVATE: {
            FocusEvent event;
            event.hasMouseFocus = 1;
            it->second->eventHandler(event);
            break;
        }
        case WM_CAPTURECHANGED: {
            FocusEvent event;
            event.hasMouseFocus = 0;
            it->second->eventHandler(event);
            break;
        }
        case WM_SETFOCUS: {
            FocusEvent event;
            event.hasKeyboardFocus = 1;
            it->second->eventHandler(event);
            break;
        }
        case WM_KILLFOCUS: {
            FocusEvent event;
            event.hasKeyboardFocus = 0;
            it->second->eventHandler(event);
            break;
        }
        
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		return 1;
	}

	void Window::handleEvent(const TT::Event& event) {
		if (event.type == TT::Event::Type::Paint) {
			onPaintEvent((const TT::PaintEvent&)event);
		} else if (event.type == TT::Event::Type::Resize)
			onResizeEvent((const TT::ResizeEvent&)event);
		else if (event.type == TT::Event::Type::MouseDown) {
			mouseButtonStates |= 1u << ((TT::MouseEvent&)event).button;
			onMouseEvent((const TT::MouseEvent&)event);
		} else if (event.type == TT::Event::Type::MouseUp) {
			mouseButtonStates &= ~(1u << ((TT::MouseEvent&)event).button);
			onMouseEvent((const TT::MouseEvent&)event);
		} else if (event.type == TT::Event::Type::MouseMove && (enableMouseTracking || mouseButtonStates != 0))
			onMouseEvent((const TT::MouseEvent&)event);
		else if (event.type == TT::Event::Type::MouseDoubleClick)
			onMouseEvent((const TT::MouseEvent&)event);
		else if (event.type == TT::Event::Type::Wheel)
			onWheelEvent((const TT::WheelEvent&)event);
		else if (event.type == TT::Event::Type::KeyDown || event.type == TT::Event::Type::KeyUp)
			onKeyEvent((const TT::KeyEvent&)event);
		else if (event.type == TT::Event::Type::Focus)
			onFocusEvent((const TT::FocusEvent&)event);
		else if (event.type == TT::Event::Type::Close)
			onCloseEvent((const TT::CloseEvent&)event);
	}

	void Window::repaint() const {
	    InvalidateRect(window, 0, true);
	}

	HWND Window::windowHandle() const {
		return window;
	}

    void Window::toggleFullscreen(HWND hwnd, WindowState& ioState) {
        // Based on chromium implementation, see https://stackoverflow.com/questions/2382464/win32-full-screen-and-hiding-taskbar
        // Save current window state if not already fullscreen.
        if (!ioState.isFullscreen) {
            // Save current window information.  We force the window into restored mode
            // before going fullscreen because Windows doesn't seem to hide the
            // taskbar if the window is in the maximized state.
            ioState.isMaximized = IsZoomed(hwnd);
            if (ioState.isMaximized)
                ::SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
            ioState.style = GetWindowLong(hwnd, GWL_STYLE);
            ioState.exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            RECT winRect;
            GetWindowRect(hwnd, &winRect);
            ioState.rect[0] = winRect.left;
            ioState.rect[1] = winRect.top;
            ioState.rect[2] = winRect.right - winRect.left;
            ioState.rect[3] = winRect.bottom - winRect.top;
        }

        ioState.isFullscreen = !ioState.isFullscreen;

        if (ioState.isFullscreen) {
            // Set new window style and size.
            SetWindowLong(hwnd, GWL_STYLE, ioState.style & ~(WS_CAPTION | WS_THICKFRAME));
            SetWindowLong(hwnd, GWL_EXSTYLE, ioState.exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

            // On expand, if we're given a window rect, grow to it, otherwise do not resize.
            /*for metro*/ {
                MONITORINFO monitor;
                monitor.cbSize = sizeof(monitor);
                GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST),
                    &monitor);
                SetWindowPos(hwnd, NULL, monitor.rcMonitor.left, monitor.rcMonitor.top,
                    monitor.rcMonitor.right - monitor.rcMonitor.left, monitor.rcMonitor.bottom - monitor.rcMonitor.top,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            }
        } else {
            // If SetWindowPos() doesn't redraw, the taskbar won't be repainted.
            SetWindowLong(hwnd, GWL_STYLE, ioState.style);
            SetWindowLong(hwnd, GWL_EXSTYLE, ioState.exStyle);
            /*for metro*/ {
                // On restore, resize to the previous saved rect size.
                SetWindowPos(hwnd, NULL, ioState.rect[0], ioState.rect[1], ioState.rect[2], ioState.rect[3],
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            }
            if (ioState.isMaximized)
                SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
    }

    void Window::toggleMaximize(HWND hwnd, WindowState& ioState) {
        // Do nothing if fullscreen.
        if (ioState.isFullscreen)
            return;
        if (IsZoomed(hwnd))
            SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
        else
            SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }

    void Window::close() const {
        PostMessage(window, WM_CLOSE, 0, 0);
    }

    void Window::toggleFullscreen() {
        toggleFullscreen(window, state);
    }

    void Window::toggleMaximize() {
        toggleMaximize(window, state);
    }
}
