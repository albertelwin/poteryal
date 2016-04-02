
#ifndef GL_HPP_INCLUDED
#define GL_HPP_INCLUDED

#include <GL/gl.h>
#include <glext.h>

#define GL_FUNC_PTR_X \
	X(glActiveTexture, GLACTIVETEXTURE) \
	X(glAttachShader, GLATTACHSHADER) \
	X(glBindBuffer, GLBINDBUFFER) \
	X(glBindFramebuffer, GLBINDFRAMEBUFFER) \
	X(glBindImageTexture, GLBINDIMAGETEXTURE) \
	X(glBindRenderbuffer, GLBINDRENDERBUFFER) \
	X(glBindVertexArray, GLBINDVERTEXARRAY) \
	X(glBlitFramebuffer, GLBLITFRAMEBUFFER) \
	X(glBufferData, GLBUFFERDATA) \
	X(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS) \
	X(glCompileShader, GLCOMPILESHADER) \
	X(glCreateProgram, GLCREATEPROGRAM) \
	X(glCreateShader, GLCREATESHADER) \
	X(glDebugMessageCallback, GLDEBUGMESSAGECALLBACK) \
	X(glDispatchCompute, GLDISPATCHCOMPUTE) \
	X(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY) \
	X(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER) \
	X(glFramebufferTexture2D, GLFRAMEBUFFERTEXTURE2D) \
	X(glGenBuffers, GLGENBUFFERS) \
	X(glGenFramebuffers, GLGENFRAMEBUFFERS) \
	X(glGenQueries, GLGENQUERIES) \
	X(glGenRenderbuffers, GLGENRENDERBUFFERS) \
	X(glGenVertexArrays, GLGENVERTEXARRAYS) \
	X(glGetAttribLocation, GLGETATTRIBLOCATION) \
	X(glGetProgramInfoLog, GLGETPROGRAMINFOLOG) \
	X(glGetProgramiv, GLGETPROGRAMIV) \
	X(glGetQueryObjectiv, GLGETQUERYOBJECTIV) \
	X(glGetQueryObjectui64v, GLGETQUERYOBJECTUI64V) \
	X(glGetShaderInfoLog, GLGETSHADERINFOLOG) \
	X(glGetShaderiv, GLGETSHADERIV) \
	X(glGetStringi, GLGETSTRINGI) \
	X(glGetUniformLocation, GLGETUNIFORMLOCATION) \
	X(glLinkProgram, GLLINKPROGRAM) \
	X(glQueryCounter, GLQUERYCOUNTER) \
	X(glRenderbufferStorage, GLRENDERBUFFERSTORAGE) \
	X(glRenderbufferStorageMultisample, GLRENDERBUFFERSTORAGEMULTISAMPLE) \
	X(glShaderSource, GLSHADERSOURCE) \
	X(glTexImage2DMultisample, GLTEXIMAGE2DMULTISAMPLE) \
	X(glTexImage3D, GLTEXIMAGE3D) \
	X(glUniform1f, GLUNIFORM1F) \
	X(glUniform1i, GLUNIFORM1I) \
	X(glUniform2f, GLUNIFORM2F) \
	X(glUniform3f, GLUNIFORM3F) \
	X(glUniform4f, GLUNIFORM4F) \
	X(glUniformMatrix4fv, GLUNIFORMMATRIX4FV) \
	X(glUseProgram, GLUSEPROGRAM) \
	X(glVertexAttribPointer, GLVERTEXATTRIBPOINTER) \
	\

#define X(name, type) typedef PFN##type##PROC name##__; name##__ name;
	GL_FUNC_PTR_X
#undef X

#define GL_EXT_X \
	X(GL_ARB_timer_query) \
	\

struct GLInfo {
#define X(NAME) b32 NAME##_;
	GL_EXT_X
#undef X
};

#define GLSL_STRINGIFY(version, shader) "#version " #version "\n" #shader

#define GL_MAX_INFO_LOG_LENGTH 1024

#if DEBUG_MODE

#define GL_CHECK_ERRORS() { GLenum err = glGetError(); while(err != GL_NO_ERROR) { printf("ERROR: %s:%u: %s\n", (char *)__FILE__, __LINE__, gl_error_to_str(err)); err = glGetError(); } }
inline char * gl_error_to_str(GLenum err) {
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

inline char * gl_debug_src_to_str(GLenum src) {
	char * str = "";

	switch(src) {
		case GL_DEBUG_SOURCE_API: {
			str = "GL_DEBUG_SOURCE_API";
			break;
		}

		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
			str = "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
			break;
		}

		case GL_DEBUG_SOURCE_SHADER_COMPILER: {
			str = "GL_DEBUG_SOURCE_SHADER_COMPILER";
			break;
		}

		case GL_DEBUG_SOURCE_THIRD_PARTY: {
			str = "GL_DEBUG_SOURCE_THIRD_PARTY";
			break;
		}

		case GL_DEBUG_SOURCE_APPLICATION: {
			str = "GL_DEBUG_SOURCE_APPLICATION";
			break;
		}

		case GL_DEBUG_SOURCE_OTHER: {
			str = "GL_DEBUG_SOURCE_OTHER";
			break;
		}

		INVALID_CASE();
	}

	return str;
}

inline char * gl_debug_type_to_str(GLenum type) {
	char * str = "";

	switch(type) {
		case GL_DEBUG_TYPE_ERROR: {
			str = "GL_DEBUG_TYPE_ERROR";
			break;
		}

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
			str = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
			break;
		}

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
			str = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
			break;
		}

		case GL_DEBUG_TYPE_PORTABILITY: {
			str = "GL_DEBUG_TYPE_PORTABILITY";
			break;
		}

		case GL_DEBUG_TYPE_PERFORMANCE: {
			str = "GL_DEBUG_TYPE_PERFORMANCE";
			break;
		}

		case GL_DEBUG_TYPE_OTHER: {
			str = "GL_DEBUG_TYPE_OTHER";
			break;
		}

		case GL_DEBUG_TYPE_MARKER: {
			str = "GL_DEBUG_TYPE_MARKER";
			break;
		}

		case GL_DEBUG_TYPE_PUSH_GROUP: {
			str = "GL_DEBUG_TYPE_PUSH_GROUP";
			break;
		}

		case GL_DEBUG_TYPE_POP_GROUP: {
			str = "GL_DEBUG_TYPE_POP_GROUP";
			break;
		}

		INVALID_CASE();
	}

	return str;
}

inline char * gl_debug_severity_to_str(GLenum severity) {
	char * str = "";

	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH: {
			str = "GL_DEBUG_SEVERITY_HIGH";
			break;
		}

		case GL_DEBUG_SEVERITY_MEDIUM: {
			str = "GL_DEBUG_SEVERITY_MEDIUM";
			break;
		}

		case GL_DEBUG_SEVERITY_LOW: {
			str = "GL_DEBUG_SEVERITY_LOW";
			break;
		}

		case GL_DEBUG_SEVERITY_NOTIFICATION: {
			str = "GL_DEBUG_SEVERITY_NOTIFICATION";
			break;
		}

		INVALID_CASE();
	}

	return str;
}

inline void gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei len, GLchar const * msg, void const * user_ptr) {
	printf("ERROR: %s | %s | %s | %.*s\n", gl_debug_src_to_str(src), gl_debug_type_to_str(type), gl_debug_severity_to_str(severity), len, msg);
	// INVALID_PATH();
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

#endif