
/* TODO ✓

[ ] Trilinear texture writing
[ ] 3D texture memory layout

[ ] Robust OpenGL function pointers -> https://www.opengl.org/wiki/Load_OpenGL_Functions#Windows_2
[ ] Variable sized static array in structs for Str/etc.

*/

#include <sys.hpp>
#include <gl.cpp>

#include <wglext.h>

PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;

#define WGL_FUNC_PTR_X \
	X(wglChoosePixelFormatARB, WGLCHOOSEPIXELFORMATARB) \
	X(wglCreateContextAttribsARB, WGLCREATECONTEXTATTRIBSARB) \
	\

#define X(name, type) typedef PFN##type##PROC name##__; name##__ name;
	WGL_FUNC_PTR_X
#undef X

#define WGL_MSAA_SAMPLES 8

#include <mmsystem.h>

#include <poteryal.cpp>

static b32 global_win32_running = false;

void win32_load_gl_func_ptrs() {
	WNDCLASSA window_class;
	ZERO_STRUCT(&window_class);
	window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = DefWindowProcA;
	window_class.hInstance = GetModuleHandleA(0);
	window_class.hCursor = LoadCursorA(0, IDC_ARROW);
	window_class.lpszClassName = "DummyClass";

	HWND window = 0;
	if(RegisterClassA(&window_class)) {
		window = CreateWindowExA(0, window_class.lpszClassName, "DummyWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, window_class.hInstance, 0);
	}
	ASSERT(window);

	HDC device_context = GetDC(window);

	PIXELFORMATDESCRIPTOR pixel_format;
	ZERO_STRUCT(&pixel_format);
	pixel_format.nSize = sizeof(pixel_format);
	pixel_format.nVersion = 1;
	pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixel_format.iPixelType = PFD_TYPE_RGBA;
	pixel_format.cColorBits = 32;
	pixel_format.cDepthBits = 24;
	pixel_format.cStencilBits = 8;

	i32 pixel_format_index = ChoosePixelFormat(device_context, &pixel_format);
	ASSERT(pixel_format_index);
	DescribePixelFormat(device_context, pixel_format_index, sizeof(pixel_format), &pixel_format);
	b32 pixel_format_set = SetPixelFormat(device_context, pixel_format_index, &pixel_format);
	ASSERT(pixel_format_set);

	HGLRC gl_context = wglCreateContext(device_context);
	if(wglMakeCurrent(device_context, gl_context)) {
		wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
		ASSERT(wglGetExtensionsStringARB);

#define X(name, type) name = (PFN##type##PROC)wglGetProcAddress(#name);
		WGL_FUNC_PTR_X
#undef X

		//TODO: Check to make sure any extensions we're using are available!!
#if 0
		char wgl_ext_buf[4096];
		c_str_cpy(wgl_ext_buf, wglGetExtensionsStringARB(device_context));

		u32 wgl_ext_count = 0;
		char * wgl_exts[256];
		zero_memory(wgl_exts, sizeof(wgl_exts));

		char * wgl_ext_it = wgl_ext_buf;
		while(*wgl_ext_it) {
			wgl_exts[wgl_ext_count++] = wgl_ext_it;
			while(*wgl_ext_it) {
				char char_ = *wgl_ext_it;
				if(char_ == ' ') {
					*wgl_ext_it = '\0';
					break;
				}

				wgl_ext_it++;
			}

			wgl_ext_it++;
		}
#endif

	}
	else {
		INVALID_PATH();
	}

	wglMakeCurrent(0, 0);
	wglDeleteContext(gl_context);
	ReleaseDC(window, device_context);
	DestroyWindow(window);
}

HGLRC win32_create_gl_context(HWND window, HDC device_context) {
	win32_load_gl_func_ptrs();

	i32 pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, true,
		WGL_SUPPORT_OPENGL_ARB, true,
		WGL_DOUBLE_BUFFER_ARB, true,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_SAMPLES_ARB, WGL_MSAA_SAMPLES,
		0,
	};

	i32 pixel_format_index;
	u32 pixel_format_count;
	wglChoosePixelFormatARB(device_context, pixel_format_attribs, 0, 1, &pixel_format_index, &pixel_format_count);
	ASSERT(pixel_format_index);

	PIXELFORMATDESCRIPTOR pixel_format;
	DescribePixelFormat(device_context, pixel_format_index, sizeof(pixel_format), &pixel_format);
	b32 pixel_format_set = SetPixelFormat(device_context, pixel_format_index, &pixel_format);
	ASSERT(pixel_format_set);

	i32 context_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 4,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0,
	};

	HGLRC gl_context = wglCreateContextAttribsARB(device_context, 0, context_attribs);
	ASSERT(gl_context);
	if(wglMakeCurrent(device_context, gl_context)) {
#define X(name, type) name = (PFN##type##PROC)wglGetProcAddress(#name);
		GL_FUNC_PTR_X
#undef X
	}
	else {
		INVALID_PATH();
	}

	return gl_context;
}

LRESULT CALLBACK win32_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	LRESULT result = 0;

	switch(message) {
		case WM_CLOSE:
		case WM_DESTROY: {
			global_win32_running = false;
			break;
		}

		//TODO: Do we need this??
		case WM_PAINT: {
			PAINTSTRUCT paint;
			BeginPaint(window, &paint);
			EndPaint(window, &paint);

			break;
		}

		default: {
			result = DefWindowProcA(window, message, w_param, l_param);
			break;
		}
	}

	return result;
}

HWND CreateGLWindow();

int main() {
	WNDCLASSA window_class;
	ZERO_STRUCT(&window_class);
	window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = win32_proc;
	window_class.hInstance = GetModuleHandleA(0);
	window_class.hCursor = LoadCursorA(0, IDC_ARROW);
	window_class.lpszClassName = "Win32Class";

	LARGE_INTEGER perf_freq;
	QueryPerformanceFrequency(&perf_freq);
	timeBeginPeriod(1);

	LARGE_INTEGER begin_perf_count;
	QueryPerformanceCounter(&begin_perf_count);

	if(RegisterClassA(&window_class)) {
		// u32 client_width = 640, client_height = 360;
		u32 client_width = 1280, client_height = 720;
		u32 window_flags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

		RECT window_rect;
		ZERO_STRUCT(&window_rect);
		window_rect.left = 0;
		window_rect.top = 0;
		window_rect.right = client_width;
		window_rect.bottom = client_height;
		AdjustWindowRect(&window_rect, window_flags, false);

		u32 window_width = (u32)(window_rect.right - window_rect.left);
		u32 window_height = (u32)(window_rect.bottom - window_rect.top);

		//TODO: Ensure this is correct!!
		i32 window_offset_x = -((i32)window_width - (i32)client_width) / 2;
		
		HWND window = CreateWindowExA(0, window_class.lpszClassName, "Win32Window", window_flags, window_offset_x, 0, window_width, window_height, 0, 0, window_class.hInstance, 0);
		ASSERT(window);

		HDC device_context = GetDC(window);
		HGLRC gl_context = win32_create_gl_context(window, device_context);

		char * gl_version = (char *)glGetString(GL_VERSION);

		GameMemory game_memory;
		ZERO_STRUCT(&game_memory);
		game_memory.size = MEGABYTES(32);
		game_memory.ptr = (u8 *)VirtualAlloc(0, game_memory.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		GameInput game_input;
		ZERO_STRUCT(&game_input);
		game_input.back_buffer_width = client_width;
		game_input.back_buffer_height = client_height;

		global_win32_running = true;
		while(global_win32_running) {
			MSG message;
			while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE)) {
				switch(message.message) {
					case WM_QUIT: {
						global_win32_running = false;
						break;
					}

					case WM_SYSKEYDOWN:
					case WM_SYSKEYUP:
					case WM_KEYDOWN:
					case WM_KEYUP: {
						u32 vk_code = (u32)message.wParam;
						if(vk_code == VK_ESCAPE) {
							global_win32_running = false;
						}

						b32 alt = message.lParam && (1 << 29);
						if(alt) {
							if(vk_code == VK_F4) {
								global_win32_running = false;
							}
						}

						break;
					}

					default: {
						TranslateMessage(&message);
						DispatchMessageA(&message);
						break;
					}
				}
			}

			game_update_and_render(&game_memory, &game_input);
			if(game_input.quit) {
				global_win32_running = false;
			}

			SwapBuffers(device_context);

			LARGE_INTEGER end_perf_count;
			QueryPerformanceCounter(&end_perf_count);

			i64 delta_time_i64 = ((end_perf_count.QuadPart - begin_perf_count.QuadPart) * 1000000) / perf_freq.QuadPart;
			f64 delta_time_f64 = (f64)delta_time_i64 / 1000000.0;
			game_input.delta_time = (f32)delta_time_f64; 

			begin_perf_count = end_perf_count;

			char gl_str_buf[256];
			Str gl_str = str_fixed_size(gl_str_buf, ARRAY_COUNT(gl_str_buf));
			str_print(&gl_str, "OpenGL %s - %f", gl_version, delta_time_f64);
			SetWindowTextA(window, gl_str.ptr);
		}

		wglMakeCurrent(0, 0);
		wglDeleteContext(gl_context);
		ReleaseDC(window, device_context);
	}

	return 0;
}