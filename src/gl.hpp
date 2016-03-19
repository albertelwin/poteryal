
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
	X(glBlitFramebuffer, GLBLITFRAMEBUFFER) \
	X(glBufferData, GLBUFFERDATA) \
	X(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS) \
	X(glCompileShader, GLCOMPILESHADER) \
	X(glCreateProgram, GLCREATEPROGRAM) \
	X(glCreateShader, GLCREATESHADER) \
	X(glDispatchCompute, GLDISPATCHCOMPUTE) \
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

#endif