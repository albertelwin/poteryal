
/* TODO ✓

[ ] Trilinear texture writing
[ ] 3D texture memory layout
[ ] Fix raycast alpha blending

[ ] Robust OpenGL function pointers -> https://www.opengl.org/wiki/Load_OpenGL_Functions#Windows_2
[ ] WGL extensions -> https://www.opengl.org/wiki/Creating_an_OpenGL_Context_(WGL)#A_Note_on_Platforms
[ ] Variable sized static array in structs for Str/etc.

*/

#include <sys.hpp>
#include <gl.cpp>

#include <mmsystem.h>

#include <poteryal.cpp>

static b32 global_win32_running = false;

HGLRC win32_create_gl_context(HWND window, HDC device_context) {
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

	HGLRC gl_context = wglCreateContext(device_context);
	if(wglMakeCurrent(device_context, gl_context)) {
	//TODO: Robust function pointers!!
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
		u32 client_width = 640, client_height = 360;
		// u32 client_width = 1280, client_height = 720;
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