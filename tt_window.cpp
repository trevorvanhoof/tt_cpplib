#include "tt_window.h"
#include "tt_messages.h"
#include "windont.h"
#include "tt_gl.h"


namespace TT {
	Window::Window(const char_t* windowTitle, HINSTANCE hInstance) {
		// Create our own window class
		static bool haveWindowClass = false;
		if (!haveWindowClass) {
			haveWindowClass = true;

			WNDCLASS wc = { 0 };
			wc.lpfnWndProc = Window::windowProc;
			wc.lpszClassName = TT_LIT("TTWindowClass");
			wc.style = CS_DBLCLKS;
			RegisterClass(&wc);
		}

		window = CreateWindowEx(0, TT_LIT("TTWindowClass"), windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
		assert(window);
		dankjewelwindows[window] = this;

		eventHandler = std::bind(&Window::handleEvent, this, std::placeholders::_1);
		// CreateGLContext();
		// Show();
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

	namespace {
		// https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
		typedef HGLRC(*wglCreateContextAttribsARBP)(HDC hDC, HGLRC hshareContext, const int* attribList);
		typedef bool (*wglChoosePixelFormatARBP)(HDC hdc, const int* piAttribIList, const float* pfAttribFList, unsigned int nMaxFormats, int* piFormats, unsigned int* nNumFormats);

		// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt for all values
		constexpr const int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
		constexpr const int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
		constexpr const int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
		constexpr const int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;
		constexpr const int WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB = 0x00000002;

		// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt for all values
		constexpr const int WGL_DRAW_TO_WINDOW_ARB = 0x2001;
		constexpr const int WGL_ACCELERATION_ARB = 0x2003;
		constexpr const int WGL_SUPPORT_OPENGL_ARB = 0x2010;
		constexpr const int WGL_DOUBLE_BUFFER_ARB = 0x2011;
		constexpr const int WGL_PIXEL_TYPE_ARB = 0x2013;
		constexpr const int WGL_COLOR_BITS_ARB = 0x2014;
		constexpr const int WGL_DEPTH_BITS_ARB = 0x2022;
		constexpr const int WGL_STENCIL_BITS_ARB = 0x2023;
		constexpr const int WGL_FULL_ACCELERATION_ARB = 0x2027;
		constexpr const int WGL_TYPE_RGBA_ARB = 0x202B;

		wglCreateContextAttribsARBP wglCreateContextAttribsARB;
		wglChoosePixelFormatARBP wglChoosePixelFormatARB;

		void initContext2(HDC device) {
			// Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
			int pixel_format_attribs[] = {
				WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
				WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
				WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,         32,
				WGL_DEPTH_BITS_ARB,         24,
				WGL_STENCIL_BITS_ARB,       8,
				0
			};

			int pixel_format;
			unsigned int num_formats;
			wglChoosePixelFormatARB(device, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
			assertFatal(num_formats != 0);

			PIXELFORMATDESCRIPTOR pfd;
			DescribePixelFormat(device, pixel_format, sizeof(pfd), &pfd);
			assertFatal(SetPixelFormat(device, pixel_format, &pfd));
		}
	}

	HDC Window::createGLContext() const {
#if 0
		HDC device = GetDC(window);
		const PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0 };
		SetPixelFormat(device, ChoosePixelFormat(device, &pfd), &pfd);
		// Old style context initialization
		wglMakeCurrent(device, wglCreateContext(device));
#else
		// New style context initialization
		{
			// Create a fake window
			HWND dummyW = CreateWindowExA(0, "edit", "", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0);
			// Create a fake device
			HDC dummyD = GetDC(dummyW);
			const PIXELFORMATDESCRIPTOR pfd = { 
				sizeof(PIXELFORMATDESCRIPTOR), // Size of this struct
				1, // Version
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Draw flags
				PFD_TYPE_RGBA, // iPixelType
				32, // ColorBIts
				0, 
				0, 
				0,
				0,
				0, 
				0,
				8, // AlphaBits
				0,
				0, 
				0, 
				0,
				0, 
				0,
				24, // DepthBits
				8, // StencilBits
				0, 
				0, 
				0, 
				0, 
				0, 
				0 
			};

			SetPixelFormat(dummyD, ChoosePixelFormat(dummyD, &pfd), &pfd);
			// Create a fake old style context and activate it
			HGLRC dummyC = wglCreateContext(dummyD);
			wglMakeCurrent(dummyD, dummyC);
			
			int j, n;
			glGetIntegerv(GL_MAJOR_VERSION, &j);
			glGetIntegerv(GL_MINOR_VERSION, &n);

			// Now we can load function pointers required for new style initialization
			wglCreateContextAttribsARB = (wglCreateContextAttribsARBP)wglGetProcAddress("wglCreateContextAttribsARB");
			wglChoosePixelFormatARB = (wglChoosePixelFormatARBP)wglGetProcAddress("wglChoosePixelFormatARB");
			// Delete fake resources
			wglMakeCurrent(dummyD, 0);
			wglDeleteContext(dummyC);
			ReleaseDC(dummyW, dummyD);
			DestroyWindow(dummyW);
		}

		HDC device = GetDC(window);

		// Set up a pixel format for this device
		initContext2(device);

		// Set up a GL context for this device
		const int attribList[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 6,
			// WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			0, // End
		};
		
		HGLRC ctx = wglCreateContextAttribsARB(device, nullptr, attribList);
		wglMakeCurrent(device, ctx);

#endif

		return device;
	}

	HDC Window::getGLContext() const {
		return GetDC(window);
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
        MouseEvent createMouseEvent(WPARAM wParam, LPARAM lParam, Event::EType type, int button) {
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
                // TODO: If ToUnicodeEx writes more chars than fit the bfufer
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
            event.isRepeat = (bool)(lParam & 0xF);
            event.key = (UINT)wParam;
            event.txt = buffer;
            event.type = isKeyDown ? Event::EType::KeyDown : Event::EType::KeyUp;

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

	// TODO: Need to investigate WM_SYSKEYDOWN and could look into media keys
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
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
            mouse = 1;
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
			return DefWindowProcA(hwnd, msg, wParam, lParam);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SETCURSOR:
			// For some reason the arrow always also shows the waiting icon
			// but other cursors don't.
			SetCursor(LoadCursor(0, IDC_HAND));
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN: {
		    auto event = createMouseEvent(wParam, lParam, Event::EType::MouseDown, mouse);
		    it->second->eventHandler(event);
		    break;
		}

		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK: {
		    auto event = createMouseEvent(wParam, lParam, Event::EType::MouseDoubleClick, mouse);
		    it->second->eventHandler(event);
		    break;
        }

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP: {
		    auto event = createMouseEvent(wParam, lParam, Event::EType::MouseUp, mouse);
		    it->second->eventHandler(event);
		    break;
        }

		case WM_MOUSEMOVE: {
			auto event = createMouseEvent(wParam, lParam, Event::EType::MouseMove, -1);
			it->second->eventHandler(event);
			break;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
            auto event = createKeyEvent(wParam, lParam, true);
			it->second->eventHandler(event);
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP: {
            auto event = createKeyEvent(wParam, lParam, false);
			it->second->eventHandler(event);
			break;
		}

		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_MOUSEWHEEL: {
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
			return DefWindowProcA(hwnd, msg, wParam, lParam);
		}

		return 1;
	}

	void Window::handleEvent(const TT::Event& event) {
		if (event.type == TT::Event::EType::Paint) {
			onPaintEvent((const TT::PaintEvent&)event);
		} else if (event.type == TT::Event::EType::Resize)
			onResizeEvent((const TT::ResizeEvent&)event);
		else if (event.type == TT::Event::EType::MouseDown) {
			mouseButtonStates |= 1u << ((TT::MouseEvent&)event).button;
			onMouseEvent((const TT::MouseEvent&)event);
		} else if (event.type == TT::Event::EType::MouseUp) {
			mouseButtonStates &= ~(1u << ((TT::MouseEvent&)event).button);
			onMouseEvent((const TT::MouseEvent&)event);
		} else if (event.type == TT::Event::EType::MouseMove && (enableMouseTracking || mouseButtonStates != 0))
			onMouseEvent((const TT::MouseEvent&)event);
		else if (event.type == TT::Event::EType::MouseDoubleClick)
			onMouseEvent((const TT::MouseEvent&)event);
		else if (event.type == TT::Event::EType::Wheel)
			onWheelEvent((const TT::WheelEvent&)event);
		else if (event.type == TT::Event::EType::KeyDown || event.type == TT::Event::EType::KeyUp)
			onKeyEvent((const TT::KeyEvent&)event);
		else if (event.type == TT::Event::EType::Focus)
			onFocusEvent((const TT::FocusEvent&)event);
		else if (event.type == TT::Event::EType::Close)
			onCloseEvent((const TT::CloseEvent&)event);
	}

	void Window::repaint() const {
	    InvalidateRect(window, 0, true);
	}

	HWND Window::windowHandle() const {
		return window;
	}
}
