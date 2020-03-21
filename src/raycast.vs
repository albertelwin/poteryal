
char const * RAYCAST_VS = GLSL_STRINGIFY(150,

in vec3 i_position;

out vec2 tex_coord;

void main() {
	gl_Position = vec4(i_position, 1.0);
	tex_coord = i_position.xy * 0.5f + 0.5f;
}

);
