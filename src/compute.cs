
char const * COMPUTE_CS_SRC = STRINGIFY_GLSL_SHADER(440,

layout(local_size_x = 16, local_size_y = 16) in;

writeonly uniform image2D u_image;
uniform float u_time = 0.0;

void main() {
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	vec2 dim = vec2(gl_WorkGroupSize.xy * gl_NumWorkGroups.xy);
	color.xy = vec2(gl_GlobalInvocationID.xy) / dim;
	color.z = float(gl_LocalInvocationIndex) / float(gl_WorkGroupSize.x * gl_WorkGroupSize.y);

	ivec2 store_pos = ivec2(gl_GlobalInvocationID.xy);
	imageStore(u_image, store_pos, color);
}

);