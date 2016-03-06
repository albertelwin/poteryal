
char const * BASIC_VERT_SRC = STRINGIFY_GLSL_SHADER(330 core,

layout(location = 0) in vec3 i_position;

uniform mat4 transform = mat4(1.0);

void main() {
	// gl_Position = transform * vec4(i_position, 1.0);
	gl_Position = vec4(i_position, 1.0);
}

);
