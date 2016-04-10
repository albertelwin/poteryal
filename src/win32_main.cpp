
/* TODO ✓

Win32 exe path
Shader hot reloading

Bilinear texture writing
Trilinear texture writing
3D texture memory layout

DirectSound audio

*/

#define NOMINMAX
#include <windows.h>

#include <sys.hpp>

#include <gl.cpp>

#include <wglext.h>

#define WGL_FUNC_PTR_X \
	X(wglChoosePixelFormatARB, WGLCHOOSEPIXELFORMATARB) \
	X(wglCreateContextAttribsARB, WGLCREATECONTEXTATTRIBSARB) \
	X(wglGetExtensionsStringARB, WGLGETEXTENSIONSSTRINGARB) \
	X(wglSwapIntervalEXT, WGLSWAPINTERVALEXT) \
	\

#define X(name, type) typedef PFN##type##PROC name##__; name##__ name;
	WGL_FUNC_PTR_X
#undef X

#define WGL_MSAA_SAMPLES 8

#define WGL_EXT_X \
	X(WGL_ARB_create_context) \
	X(WGL_ARB_create_context_profile) \
	X(WGL_ARB_multisample) \
	X(WGL_ARB_pixel_format) \
	X(WGL_EXT_swap_control) \
	\

struct WGLInfo {
#define X(NAME) b32 NAME##_;
	WGL_EXT_X
#undef X

	i32 max_msaa_samples;
};

#include <poteryal.hpp>

struct Win32Game {
	GameUpdateAndRender update_and_render;

	HMODULE dll;
	u64 timestamp;
};

static b32 global_win32_running = false;

WGLInfo wgl_load_func_ptrs() {
	WGLInfo wgl_info;
	ZERO_STRUCT(&wgl_info);

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
#define X(name, type) name = (PFN##type##PROC)wglGetProcAddress(#name);
		WGL_FUNC_PTR_X
#undef X

		ASSERT(wglGetExtensionsStringARB);

		char wgl_ext_buf[4096];
		c_str_cpy(wgl_ext_buf, wglGetExtensionsStringARB(device_context));

		char * wgl_ext_it = wgl_ext_buf;
		while(*wgl_ext_it) {
			char * ext = wgl_ext_it;

			while(*wgl_ext_it) {
				char char_ = *wgl_ext_it;
				if(char_ == ' ') {
					u32 ext_len = (u32)(wgl_ext_it - ext);
					// printf("LOG: %.*s\n", ext_len, ext);

					if(false) {}
#define X(NAME) else if(str_eql(ext, ext_len, #NAME, c_str_len(#NAME))) { wgl_info.##NAME##_ = true; }
					WGL_EXT_X
#undef X

					break;
				}

				wgl_ext_it++;
			}

			wgl_ext_it++;
		}

#if GL_MAX_INTEGER_SAMPLES
		glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &wgl_info.max_msaa_samples);
#endif
	}
	else {
		INVALID_PATH();
	}

	wglMakeCurrent(0, 0);
	wglDeleteContext(gl_context);
	ReleaseDC(window, device_context);
	DestroyWindow(window);

	return wgl_info;
}

HGLRC win32_create_gl_context(HWND window, HDC device_context) {
	WGLInfo wgl_info = wgl_load_func_ptrs();
	ASSERT(wgl_info.WGL_ARB_create_context_);
	ASSERT(wgl_info.WGL_ARB_create_context_profile_);
	ASSERT(wgl_info.WGL_ARB_pixel_format_);

	i32 pixel_format_attribs[64];
	ZERO_ARRAY(pixel_format_attribs);

	u32 pixel_format_attrib_count = 0;
#define PUSH_PIXEL_FORMAT_ATTRIB(NAME, VALUE) pixel_format_attribs[pixel_format_attrib_count++] = (NAME); pixel_format_attribs[pixel_format_attrib_count++] = (VALUE)

	PUSH_PIXEL_FORMAT_ATTRIB(WGL_DRAW_TO_WINDOW_ARB, true);
	PUSH_PIXEL_FORMAT_ATTRIB(WGL_SUPPORT_OPENGL_ARB, true);
	PUSH_PIXEL_FORMAT_ATTRIB(WGL_DOUBLE_BUFFER_ARB, true);
	PUSH_PIXEL_FORMAT_ATTRIB(WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
	PUSH_PIXEL_FORMAT_ATTRIB(WGL_COLOR_BITS_ARB, 32);
	PUSH_PIXEL_FORMAT_ATTRIB(WGL_DEPTH_BITS_ARB, 24);
	PUSH_PIXEL_FORMAT_ATTRIB(WGL_STENCIL_BITS_ARB, 8);

	if(wgl_info.WGL_ARB_multisample_ && wgl_info.max_msaa_samples) {
		i32 msaa_samples = WGL_MSAA_SAMPLES;
		if(wgl_info.max_msaa_samples < msaa_samples) {
			msaa_samples = wgl_info.max_msaa_samples;
		}

		PUSH_PIXEL_FORMAT_ATTRIB(WGL_SAMPLE_BUFFERS_ARB, 1);
		PUSH_PIXEL_FORMAT_ATTRIB(WGL_SAMPLES_ARB, msaa_samples);
	}

#undef PUSH_PIXEL_FORMAT_ATTRIB
	ASSERT(pixel_format_attrib_count < ARRAY_COUNT(pixel_format_attribs));

	i32 pixel_format_index;
	u32 pixel_format_count;
	wglChoosePixelFormatARB(device_context, pixel_format_attribs, 0, 1, &pixel_format_index, &pixel_format_count);
	ASSERT(pixel_format_index);

	PIXELFORMATDESCRIPTOR pixel_format;
	DescribePixelFormat(device_context, pixel_format_index, sizeof(pixel_format), &pixel_format);
	b32 pixel_format_set = SetPixelFormat(device_context, pixel_format_index, &pixel_format);
	ASSERT(pixel_format_set);

	i32 context_flags = 0;
#if DEBUG_MODE
	context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

	i32 context_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, context_flags,
		0,
	};

	HGLRC gl_context = wglCreateContextAttribsARB(device_context, 0, context_attribs);
	ASSERT(gl_context);
	if(wglMakeCurrent(device_context, gl_context)) {
#define X(NAME, TYPE) NAME = (PFN##TYPE##PROC)wglGetProcAddress(#NAME);
		GL_FUNC_PTR_X
#undef X

#if DEBUG_MODE
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(gl_debug_callback, 0);
#endif

		GLInfo gl_info;
		ZERO_STRUCT(&gl_info);

		i32 ext_count;
		glGetIntegerv(GL_NUM_EXTENSIONS, &ext_count);
		for(i32 i = 0; i < ext_count; i++) {
			char * ext = (char *)glGetStringi(GL_EXTENSIONS, i);
			u32 ext_len = c_str_len(ext);

 			// printf("LOG: [%d]: %s\n", i, ext);

			if(false) {}
#define X(NAME) else if(str_eql(ext, ext_len, #NAME, c_str_len(#NAME))) { gl_info.##NAME##_ = true; }
			GL_EXT_X
#undef X
		}

		ASSERT(gl_info.GL_ARB_timer_query_);

		if(wglSwapIntervalEXT) {
			wglSwapIntervalEXT(1);
		}
	}
	else {
		INVALID_PATH();
	}

	return gl_context;
}

void win32_load_game_dll(Win32Game * game) {
	u64 timestamp = 0;

	WIN32_FILE_ATTRIBUTE_DATA dll_attribs;
	if(GetFileAttributesEx("bin/Poteryal.dll", GetFileExInfoStandard, &dll_attribs)) {
		timestamp = ((u64)dll_attribs.ftLastWriteTime.dwHighDateTime << 32) + dll_attribs.ftLastWriteTime.dwLowDateTime;

		WIN32_FILE_ATTRIBUTE_DATA lock_attribs;
		if(!GetFileAttributesEx("bin/lock", GetFileExInfoStandard, &lock_attribs)) {
			if(!game->dll || timestamp > game->timestamp) {
				if(game->dll) {
					FreeLibrary(game->dll);
				}

				b32 copied = CopyFile("bin/Poteryal.dll", "bin/Poteryal__.dll", false);
				ASSERT(copied);

				game->dll = LoadLibraryA("bin/Poteryal__.dll");
				ASSERT(game->dll);

				game->update_and_render = (GameUpdateAndRender)GetProcAddress(game->dll, "game_update_and_render");
			}

			game->timestamp = timestamp;
		}
	}

	ASSERT(game->update_and_render);
}

LRESULT CALLBACK win32_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	LRESULT result = 0;

	switch(message) {
		case WM_CLOSE:
		case WM_DESTROY: {
			global_win32_running = false;
			break;
		}

		default: {
			result = DefWindowProcA(window, message, w_param, l_param);
			break;
		}
	}

	return result;
}

void win32_process_key(GameInput * game_input, GameKey key, b32 is_down) {
	u8 key_state = game_input->keys[key];

	//TODO: Process multiple key transitions per frame!!
	if(is_down) {
		if(!(key_state & KEY_DOWN)) {
			key_state |= KEY_PRESSED;
		}

		key_state |= KEY_DOWN;
	}
	else {
		if(key_state & KEY_DOWN) {
			key_state |= KEY_RELEASED;
		}

		key_state &= ~KEY_DOWN;
	}

	game_input->keys[key] = key_state;
}

int main() {
	WNDCLASSA window_class;
	ZERO_STRUCT(&window_class);
	window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = win32_proc;
	window_class.hInstance = GetModuleHandleA(0);
	window_class.hCursor = LoadCursorA(0, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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

		Win32Game game;
		ZERO_STRUCT(&game);

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
			win32_load_game_dll(&game);

			for(u32 i = 0; i < ARRAY_COUNT(game_input.keys); i++) {
				game_input.keys[i] &= ~KEY_PRESSED;
				game_input.keys[i] &= ~KEY_RELEASED;
			}

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

						b32 alt = message.lParam & (1 << 29);
						if(alt) {
							if(vk_code == VK_F4) {
								global_win32_running = false;
							}
						}

						b32 is_down = (message.lParam & (1 << 31)) == 0;

						switch(vk_code) {
							case VK_UP: {
								win32_process_key(&game_input, GameKey_up, is_down);
								break;
							}

							case VK_DOWN: {
								win32_process_key(&game_input, GameKey_down, is_down);
								break;
							}

							case VK_LEFT: {
								win32_process_key(&game_input, GameKey_left, is_down);
								break;
							}

							case VK_RIGHT: {
								win32_process_key(&game_input, GameKey_right, is_down);
								break;
							}

							case VK_RETURN: {
								win32_process_key(&game_input, GameKey_return, is_down);
								break;
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

			if(game.update_and_render) {
				game.update_and_render(&game_memory, &game_input);
			}

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