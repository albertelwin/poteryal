
/* TODO ✓

[ ] Robust OpenGL function pointers -> https://www.opengl.org/wiki/Load_OpenGL_Functions#Windows_2
[ ] WGL extensions -> https://www.opengl.org/wiki/Creating_an_OpenGL_Context_(WGL)#A_Note_on_Platforms
[ ] Bring in math library

[ ] Variable sized static array in structs for Str/etc.

*/

#include <sys.hpp>

#include <mmsystem.h>

#include <gl.hpp>
#include <basic.vs>
#include <basic.fs>
#include <compute.cs>

#include <math.h>

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
	b32 pixel_format_set = SetPixelFormat(device_context, pixel_format_index, &pixel_format);

	HGLRC gl_context = wglCreateContext(device_context);
	wglMakeCurrent(device_context, gl_context);

	//TODO: Robust function pointers!!
#define X(name, type) name = (PFN##type##PROC)wglGetProcAddress(#name);
	GL_FUNC_PTR_X
#undef X

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

		u32 basic_program = gl_program();
		gl_compile_and_add_shader(basic_program, BASIC_VS_SRC, GL_VERTEX_SHADER);
		gl_compile_and_add_shader(basic_program, BASIC_FS_SRC, GL_FRAGMENT_SHADER);
		gl_link_program(basic_program);

		u32 pos_loc = glGetAttribLocation(basic_program, "i_position");
		u32 tex_loc = glGetUniformLocation(basic_program, "u_tex");
		u32 color_loc = glGetUniformLocation(basic_program, "u_color");

		u32 compute_program = gl_program();
		gl_compile_and_add_shader(compute_program, COMPUTE_CS_SRC, GL_COMPUTE_SHADER);
		gl_link_program(compute_program);

		i32 work_group_size[3];
		glGetProgramiv(compute_program, GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);

		u32 image_loc = glGetUniformLocation(compute_program, "u_image");
		u32 time_loc = glGetUniformLocation(compute_program, "u_time");

		f32 verts[] = {
			 1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f,-1.0f, 0.0f,
			-1.0f,-1.0f, 0.0f,
			 1.0f,-1.0f, 0.0f,
			 1.0f, 1.0f, 0.0f,
		};
		GLVertexBuffer v_buf = gl_vertex_buffer(verts, ARRAY_COUNT(verts), 1, GL_STATIC_DRAW);

#if 0
		u32 texture_channels = 4;
		u32 texture_size = 64;
		u8 * texels = ALLOC_ARRAY(u8, texture_size * texture_size * texture_channels);
		for(u32 y = 0; y < texture_size; y++) {
			for(u32 x = 0; x < texture_size; x++) {
				u32 i4 = (y * texture_size + x) * 4;

				texels[i4 + 0] = 255;
				texels[i4 + 1] = 0;
				texels[i4 + 2] = 0;
				texels[i4 + 3] = 255;
			}
		}
		GLTexture tex = gl_texture(texels, texture_size, texture_size, GL_RGBA, GL_NEAREST, GL_CLAMP_TO_EDGE);
#endif

		GLTexture tex;
		ZERO_STRUCT(&tex);
		tex.width = 1280;
		tex.height = 720;

		glGenTextures(1, &tex.id);

		glBindTexture(GL_TEXTURE_2D, tex.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glViewport(0, 0, client_width, client_height);

		f64 delta_time = 0.0f;
		f64 total_time = 0.0f;

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

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(compute_program);

			glActiveTexture(GL_TEXTURE0);
			glBindImageTexture(0, tex.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
			glUniform1i(image_loc, 0);

			glUniform1f(time_loc, (f32)total_time);

			glDispatchCompute(tex.width / work_group_size[0], tex.height / work_group_size[1], 1);

			glUseProgram(basic_program);

			glBindBuffer(GL_ARRAY_BUFFER, v_buf.id);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex.id);
			glUniform1i(tex_loc, 0);

			f32 gray = sin((f32)total_time) * 0.5f + 0.5f;
			glUniform4f(color_loc, gray, gray, gray, 1.0f);

			glVertexAttribPointer(pos_loc, 3, GL_FLOAT, 0, 0, 0);
			glEnableVertexAttribArray(pos_loc);

			glDrawArrays(GL_TRIANGLES, 0, v_buf.vert_count);

			SwapBuffers(device_context);

			LARGE_INTEGER end_perf_count;
			QueryPerformanceCounter(&end_perf_count);

			i64 delta_time_i64 = ((end_perf_count.QuadPart - begin_perf_count.QuadPart) * 1000000) / perf_freq.QuadPart;
			delta_time = (f64)delta_time_i64 / 1000000.0;
			total_time += delta_time;

			begin_perf_count = end_perf_count;

			char gl_str_buf[256];
			Str gl_str = str_fixed_size(gl_str_buf, ARRAY_COUNT(gl_str_buf));
			str_print(&gl_str, "OpenGL %s - %f", gl_version, delta_time);
			SetWindowTextA(window, gl_str.ptr);
		}

		wglMakeCurrent(0, 0);
		wglDeleteContext(gl_context);
		ReleaseDC(window, device_context);
	}

	return 0;
}