
char const * BASIC_FS_SRC = STRINGIFY_GLSL_SHADER(330,

in vec2 tex_coord;

uniform sampler2D u_tex;
uniform vec4 u_color = vec4(1.0);

layout(location = 0) out vec4 o_color;

void main() {
	o_color = texture2D(u_tex, tex_coord);
}

);