
char const * BASIC_FRAG_SRC = STRINGIFY_GLSL_SHADER(330 core,

uniform vec4 u_color = vec4(1.0);

layout(location = 0) out vec4 o_color;

void main() {
	o_color = u_color;
}

);