
char const * BASIC_VS_SRC = STRINGIFY_GLSL_SHADER(330,

layout(location = 0) in vec3 i_position;

out vec2 tex_coord;

void main() {
	gl_Position = vec4(i_position, 1.0);
	tex_coord = i_position.xy * 0.5f + 0.5f;
}

);
