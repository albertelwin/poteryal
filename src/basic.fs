
char const * BASIC_FS_SRC = GLSL_STRINGIFY(330,

in vec2 tex_coord;

uniform sampler3D u_tex;
uniform vec4 u_color = vec4(1.0);
uniform float u_time = 0.0;

uniform mat4 u_inv_view_proj;
uniform vec3 u_camera_pos;

layout(location = 0) out vec4 o_color;

//NOTE: Tavian Barnes Ray-AABB Intersection -> https://tavianator.com/fast-branchless-raybounding-box-intersections/
bool intersect(vec3 box_min, vec3 box_max, vec3 ro, vec3 rd, out float t_min, out float t_max) {
	vec3 i_rd = vec3(1.0) / rd;

	float tx1 = (box_min.x - ro.x) * i_rd.x;
	float tx2 = (box_max.x - ro.x) * i_rd.x;

	t_min = min(tx1, tx2);
	t_max = max(tx1, tx2);

	float ty1 = (box_min.y - ro.y) * i_rd.y;
	float ty2 = (box_max.y - ro.y) * i_rd.y;

	t_min = max(t_min, min(ty1, ty2));
	t_max = min(t_max, max(ty1, ty2));

	float tz1 = (box_min.z - ro.z) * i_rd.z;
	float tz2 = (box_max.z - ro.z) * i_rd.z;

	t_min = max(t_min, min(tz1, tz2));
	t_max = min(t_max, max(tz1, tz2));

	return t_max >= t_min;
}

void main() {
	vec4 clip_pos = vec4(tex_coord.xy * 2.0 - 1.0, -1.0, 1.0);
	vec4 world_pos = u_inv_view_proj * clip_pos;
	world_pos.xyz /= world_pos.w;

	vec3 ro = u_camera_pos;
	vec3 rd = normalize(world_pos.xyz - u_camera_pos);

	vec3 box_min = vec3(-1.0);
	vec3 box_max = vec3(1.0);

	vec4 dst = vec4(0.0);

	float t_min;
	float t_max;
	bool hit = intersect(box_min, box_max, ro, rd, t_min, t_max);
	if(hit) {
		float t = t_min;
		float t_step = 0.02;

		for(int i = 0; i < 128; i++) {
			vec3 pos = ro + rd * t;
			vec3 uv = pos * 0.5 + 0.5;
			vec4 src = texture(u_tex, uv);

			//TODO: How does this blending work??
			dst = (1.0 - dst.a) * src + dst;

			t += t_step;
			if(t > t_max || dst.a >= 1.0) {
				break;
			}
		}
	}

	o_color = vec4(dst.rgb, 1.0);
}

);