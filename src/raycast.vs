
char const * RAYCAST_VS = GLSL_STRINGIFY(330,

layout(location = 0) in vec3 i_position;

out vec2 tex_coord;

void main() {
	gl_Position = vec4(i_position, 1.0);
	tex_coord = i_position.xy * 0.5f + 0.5f;
}

);