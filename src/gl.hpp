
#ifndef GL_HPP_INCLUDED
#define GL_HPP_INCLUDED

#include <GL/gl.h>
#include <glext.h>

#define GL_FUNC_PTR_X \
	X(glAttachShader, GLATTACHSHADER) \
	X(glBindBuffer, GLBINDBUFFER) \
	X(glBindFramebuffer, GLBINDFRAMEBUFFER) \
	X(glBindRenderbuffer, GLBINDRENDERBUFFER) \
	X(glBlitFramebuffer, GLBLITFRAMEBUFFER) \
	X(glBufferData, GLBUFFERDATA) \
	X(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS) \
	X(glCompileShader, GLCOMPILESHADER) \
	X(glCreateProgram, GLCREATEPROGRAM) \
	X(glCreateShader, GLCREATESHADER) \
	X(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY) \
	X(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER) \
	X(glFramebufferTexture2D, GLFRAMEBUFFERTEXTURE2D) \
	X(glGenBuffers, GLGENBUFFERS) \
	X(glGenFramebuffers, GLGENFRAMEBUFFERS) \
	X(glGenRenderbuffers, GLGENRENDERBUFFERS) \
	X(glGetAttribLocation, GLGETATTRIBLOCATION) \
	X(glGetProgramInfoLog, GLGETPROGRAMINFOLOG) \
	X(glGetProgramiv, GLGETPROGRAMIV) \
	X(glGetShaderInfoLog, GLGETSHADERINFOLOG) \
	X(glGetShaderiv, GLGETSHADERIV) \
	X(glGetUniformLocation, GLGETUNIFORMLOCATION) \
	X(glLinkProgram, GLLINKPROGRAM) \
	X(glRenderbufferStorage, GLRENDERBUFFERSTORAGE) \
	X(glRenderbufferStorageMultisample, GLRENDERBUFFERSTORAGEMULTISAMPLE) \
	X(glShaderSource, GLSHADERSOURCE) \
	X(glTexImage2DMultisample, GLTEXIMAGE2DMULTISAMPLE) \
	X(glUniform1f, GLUNIFORM1F) \
	X(glUniform2f, GLUNIFORM2F) \
	X(glUniform3f, GLUNIFORM3F) \
	X(glUniform4f, GLUNIFORM4F) \
	X(glUseProgram, GLUSEPROGRAM) \
	X(glVertexAttribPointer, GLVERTEXATTRIBPOINTER) \
	\

#define X(name, type) typedef PFN##type##PROC name##__; name##__ name;
	GL_FUNC_PTR_X
#undef X

#define STRINGIFY_GLSL_SHADER(version, shader) "#version " #version "\n" #shader

#define GL_MAX_INFO_LOG_LENGTH 1024

// #if DEBUG_ENABLED
#if 0
#define GL_CHECK_ERRORS() { GLenum err = glGetError(); while(err != GL_NO_ERROR) { printf("ERROR: %s:%u: %s\n", (char *)__FILE__, __LINE__, gl_error_to_str(err)); err = glGetError(); } }
char * gl_error_to_str(GLenum err) {
	char const * str = "";

	switch(err) {
		case GL_NO_ERROR: {
			str = "GL_NO_ERROR";
			break;
		}

		case GL_INVALID_ENUM: {
			str = "GL_INVALID_ENUM";
			break;
		}

		case GL_INVALID_VALUE: {
			str = "GL_INVALID_VALUE";
			break;
		}

		case GL_INVALID_OPERATION: {
			str = "GL_INVALID_OPERATION";
			break;
		}

		case GL_INVALID_FRAMEBUFFER_OPERATION: {
			str = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		case GL_OUT_OF_MEMORY: {
			str = "GL_OUT_OF_MEMORY";
			break;
		}

		INVALID_CASE();
	}

	return (char *)str;
}
#else
#define GL_CHECK_ERRORS(...)
#endif

struct GLVertexBuffer {
	GLuint id;
	u32 vert_count;
	u32 components;
	u32 size;
};

struct GLTexture {
	GLuint id;
	u32 width;
	u32 height;
};

struct GLFrameBuffer {
	GLuint id;
	GLuint texture_id;
	GLuint depth_buffer_id;
	u32 width;
	u32 height;
};

GLuint gl_compile_shader(char const * shader_src, GLenum shader_type) {
	GLuint shader_id = glCreateShader(shader_type);
	glShaderSource(shader_id, 1, &shader_src, 0);
	glCompileShader(shader_id);

	GLint compile_result;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_result);
	if(compile_result == GL_FALSE) {
		GLint info_log_length;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
		if(info_log_length > GL_MAX_INFO_LOG_LENGTH) {
			printf("WARNING: Info log truncated, actual length was %d\n", info_log_length);
		}

		char info_log[GL_MAX_INFO_LOG_LENGTH];
		glGetShaderInfoLog(shader_id, GL_MAX_INFO_LOG_LENGTH, 0, info_log);
		printf("%s: %s\n", shader_src, info_log);
	}

	return shader_id;
}

GLuint gl_link_program(GLuint vert_id, GLuint frag_id) {
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vert_id);
	glAttachShader(program_id, frag_id);
	glLinkProgram(program_id);

	GLint link_result;
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_result);
	if(link_result == GL_FALSE) {
		GLint info_log_length;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
		if(info_log_length > GL_MAX_INFO_LOG_LENGTH) {
			printf("WARNING: Info log truncated, actual length was %d\n", info_log_length);
		}

		char info_log[GL_MAX_INFO_LOG_LENGTH];
		glGetProgramInfoLog(program_id, GL_MAX_INFO_LOG_LENGTH, 0, info_log);
		printf("%s\n", info_log);
	}

	return program_id;
}

GLVertexBuffer gl_vertex_buffer(f32 const * vert_data, u32 vert_data_count, u32 components, GLenum usage_flag) {
	GLVertexBuffer vertex_buffer = {};
	vertex_buffer.vert_count = vert_data_count / components;
	vertex_buffer.components = components;
	vertex_buffer.size = vert_data_count * sizeof(f32);

	glGenBuffers(1, &vertex_buffer.id);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.id);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer.size, vert_data, usage_flag);

	return vertex_buffer;
}

GLFrameBuffer gl_frame_buffer(u32 width, u32 height, b32 use_depth, GLint min_filter = GL_NEAREST, GLint mag_filter = GL_NEAREST) {
	GLFrameBuffer frame_buffer = {};
	frame_buffer.width = width;
	frame_buffer.height = height;

	glGenFramebuffers(1, &frame_buffer.id);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.id);

	glGenTextures(1, &frame_buffer.texture_id);
	glBindTexture(GL_TEXTURE_2D, frame_buffer.texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_buffer.texture_id, 0);

	if(use_depth) {
		glGenRenderbuffers(1, &frame_buffer.depth_buffer_id);
		glBindRenderbuffer(GL_RENDERBUFFER, frame_buffer.depth_buffer_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frame_buffer.depth_buffer_id);			
	}

	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return frame_buffer;
}

GLFrameBuffer gl_msaa_frame_buffer(u32 width, u32 height, u32 sample_count) {
	GLFrameBuffer frame_buffer = {};
	frame_buffer.width = width;
	frame_buffer.height = height;
	// frame_buffer.sample_count = sample_count;

	glGenFramebuffers(1, &frame_buffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.id);

	glGenTextures(1, &frame_buffer.texture_id);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, frame_buffer.texture_id);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sample_count, GL_RGB, width, height, false);

	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, frame_buffer.texture_id, 0);

	glGenRenderbuffers(1, &frame_buffer.depth_buffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, frame_buffer.depth_buffer_id);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, sample_count, GL_DEPTH_COMPONENT16, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frame_buffer.depth_buffer_id);

	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return frame_buffer;
}

void gl_resolve_msaa_frame_buffer(GLFrameBuffer * msaa_frame_buffer, GLFrameBuffer * resolve_frame_buffer) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_frame_buffer->id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_frame_buffer->id);
	glBlitFramebuffer(0, 0, msaa_frame_buffer->width, msaa_frame_buffer->height, 0, 0, resolve_frame_buffer->width, resolve_frame_buffer->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

GLTexture gl_texture(u8 * texture_data, u32 width, u32 height, GLenum format, GLint filter, GLint wrap_mode) {
	GLTexture tex = {};
	tex.width = width;
	tex.height = height;

	glGenTextures(1, &tex.id);
	ASSERT(tex.id);

	glBindTexture(GL_TEXTURE_2D, tex.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, texture_data);

	//TODO: Anisotropic filtering!!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

	GL_CHECK_ERRORS();

	return tex;
}

#endif