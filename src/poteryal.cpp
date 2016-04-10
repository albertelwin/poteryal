
#include <poteryal.hpp>

#include <gl.cpp>
#include <math.cpp>

#include <raycast.vs>
#include <raycast.fs>

#define TEXTURE_CHANNELS 4

extern "C" void game_update_and_render(GameMemory * game_memory, GameInput * game_input) {
	ASSERT(sizeof(GameState) <= game_memory->size);
	GameState * game_state = (GameState *)game_memory->ptr;

	if(game_memory->reloaded) {
#define X(NAME, TYPE) NAME = (PFN##TYPE##PROC)wglGetProcAddress(#NAME);
		GL_FUNC_PTR_X
#undef X

		if(game_state->raycast_program) {
			glDeleteProgram(game_state->raycast_program);
		}

		game_state->raycast_program = gl_program();
		gl_compile_and_add_shader(game_state->raycast_program, RAYCAST_VS, GL_VERTEX_SHADER);
		gl_compile_and_add_shader(game_state->raycast_program, RAYCAST_FS, GL_FRAGMENT_SHADER);
		gl_link_program(game_state->raycast_program);

		game_state->pos_loc = glGetAttribLocation(game_state->raycast_program, "i_position");
		game_state->tex_loc = glGetUniformLocation(game_state->raycast_program, "u_tex");
		game_state->color_loc = glGetUniformLocation(game_state->raycast_program, "u_color");
		game_state->u_time = glGetUniformLocation(game_state->raycast_program, "u_time");
		game_state->inv_view_proj = glGetUniformLocation(game_state->raycast_program, "u_inv_view_proj");
		game_state->camera_pos = glGetUniformLocation(game_state->raycast_program, "u_camera_pos");
	}

	if(!game_memory->initialised) {
		game_memory->initialised = true;

		game_state->arena = memory_arena(game_state + 1, game_memory->size - sizeof(GameState));

		game_state->total_time = 0.0f;

		//NOTE: Don't forget the VAO!!
		glGenVertexArrays(1, &game_state->vertex_array);
		glBindVertexArray(game_state->vertex_array);

		f32 verts[] = {
			 1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f,-1.0f, 0.0f,

			-1.0f,-1.0f, 0.0f,
			 1.0f,-1.0f, 0.0f,
			 1.0f, 1.0f, 0.0f,
		};
		game_state->v_buf = gl_vertex_buffer(verts, ARRAY_COUNT(verts), 3, GL_STATIC_DRAW);

		game_state->tex_size = 64;
		game_state->texels = PUSH_ARRAY(&game_state->arena, u8, game_state->tex_size * game_state->tex_size * game_state->tex_size * TEXTURE_CHANNELS);
		for(u32 z = 0, i = 0; z < game_state->tex_size; z++) {
			for(u32 y = 0; y < game_state->tex_size; y++) {
				for(u32 x = 0; x < game_state->tex_size; x++, i += TEXTURE_CHANNELS) {
					Vec3 pos = vec3(0.0f);
					pos.x = (f32)x / (f32)(game_state->tex_size - 1);
					pos.y = (f32)y / (f32)(game_state->tex_size - 1);
					pos.z = (f32)z / (f32)(game_state->tex_size - 1);

					Vec4 color = vec4(pos, 0.05f);
					color.rgb *= color.a;

					game_state->texels[i + 0] = (u8)(color.r * 255.0f);
					game_state->texels[i + 1] = (u8)(color.g * 255.0f);
					game_state->texels[i + 2] = (u8)(color.b * 255.0f);
					game_state->texels[i + 3] = (u8)(color.a * 255.0f);
				}
			}
		}

		glGenTextures(1, &game_state->tex_id);
		ASSERT(game_state->tex_id);

		glBindTexture(GL_TEXTURE_3D, game_state->tex_id);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, game_state->tex_size, game_state->tex_size, game_state->tex_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, game_state->texels);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		for(u32 i = 0; i < ARRAY_COUNT(game_state->points); i++) {
			game_state->points[i] = rand_sample_in_sphere();
		}

		glDisable(GL_CULL_FACE);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glViewport(0, 0, game_input->back_buffer_width, game_input->back_buffer_height);

		glGenQueries(ARRAY_COUNT(game_state->perf_queries), game_state->perf_queries);
	}

	game_state->total_time += game_input->delta_time;

	if(game_state->perf_queries_queued) {
		i32 queries_available = 0;
		while(!queries_available) {
			u32 last_index = ARRAY_COUNT(game_state->perf_queries) - 1;
			glGetQueryObjectiv(game_state->perf_queries[last_index], GL_QUERY_RESULT_AVAILABLE, &queries_available);
		}

		u64 query_begin_time;
		glGetQueryObjectui64v(game_state->perf_queries[0], GL_QUERY_RESULT, &query_begin_time);

		u64 query_end_time;
		glGetQueryObjectui64v(game_state->perf_queries[1], GL_QUERY_RESULT, &query_end_time);

		u64 query_time_elapsed = query_end_time - query_begin_time;
		f64 query_time_elapsed_f64 = query_time_elapsed / 1000000.0;
		// printf("LOG: %fms\n", query_time_elapsed_f64);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	for(u32 z = 0, i = 0; z < game_state->tex_size; z++) {
		for(u32 y = 0; y < game_state->tex_size; y++) {
			for(u32 x = 0; x < game_state->tex_size; x++, i += TEXTURE_CHANNELS) {
				game_state->texels[i + 0] = 0;
				game_state->texels[i + 1] = 0;
				game_state->texels[i + 2] = 0;
				game_state->texels[i + 3] = 0;
			}
		}
	}

#if 1
	f32 r_255 = 1.0f / 255.0f;

	for(u32 i = 0; i < ARRAY_COUNT(game_state->points); i++) {
		Vec3 point = game_state->points[i];

		f32 len = length(point);
		Vec3 pos = normalize(point) * (1.0f - len * len);
		Vec3 pos01 = pos * 0.5f + 0.5f;

		u32 u = (u32)(pos01.x * (game_state->tex_size - 1));
		u32 v = (u32)(pos01.y * (game_state->tex_size - 1));
		u32 w = (u32)(pos01.z * (game_state->tex_size - 1));

		ASSERT(u >= 0 && u < game_state->tex_size);
		ASSERT(v >= 0 && v < game_state->tex_size);
		ASSERT(w >= 0 && w < game_state->tex_size);

		u32 sample = (u * game_state->tex_size * game_state->tex_size + v * game_state->tex_size + w) * TEXTURE_CHANNELS;

		f32 r = game_state->texels[sample + 0] * r_255;
		f32 g = game_state->texels[sample + 1] * r_255;
		f32 b = game_state->texels[sample + 2] * r_255;
		f32 a = game_state->texels[sample + 3] * r_255;

		Vec4 rgba = vec4(point * 0.5f + 0.5f, 0.1f);
		// Vec4 rgba = vec4(1.0f, 1.0f, 1.0f, 0.1f);
		rgba.rgb *= rgba.a;

		//TODO: Verify this is correct!
		r = clamp01((1.0f - a) * rgba.r + r);
		g = clamp01((1.0f - a) * rgba.g + g);
		b = clamp01((1.0f - a) * rgba.b + b);
		a = clamp01((1.0f - a) * rgba.a + a);

		game_state->texels[sample + 0] = (u8)(r * 255.0f);
		game_state->texels[sample + 1] = (u8)(g * 255.0f);
		game_state->texels[sample + 2] = (u8)(b * 255.0f);
		game_state->texels[sample + 3] = (u8)(a * 255.0f);
	}

	glBindTexture(GL_TEXTURE_3D, game_state->tex_id);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, game_state->tex_size, game_state->tex_size, game_state->tex_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, game_state->texels);
#endif

	glUseProgram(game_state->raycast_program);

	glBindBuffer(GL_ARRAY_BUFFER, game_state->v_buf.id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, game_state->tex_id);
	glUniform1i(game_state->tex_loc, 0);

	f32 gray = sin((f32)game_state->total_time) * 0.5f + 0.5f;
	glUniform4f(game_state->color_loc, gray, gray, gray, 1.0f);

	f32 camera_dist = 2.0f;
	f32 camera_speed = 1.0f / 24.0f;

	Vec3 camera_pos = vec3(0.0f);
	camera_pos.x = cos(game_state->total_time * TAU * camera_speed) * camera_dist;
	camera_pos.z = sin(game_state->total_time * TAU * camera_speed) * camera_dist;

	Mat4 view = look_at(camera_pos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

	f32 aspect_ratio = (f32)game_input->back_buffer_width / (f32)game_input->back_buffer_height;
	Mat4 proj = perspective_projection(aspect_ratio, 60.0f, 0.1f, F32_MAX);

	Mat4 view_proj = proj * view;
	Mat4 inv_view_proj = inverse(view_proj);

	glUniform1f(game_state->u_time, game_state->total_time);
	glUniform3f(game_state->camera_pos, camera_pos.x, camera_pos.y, camera_pos.z);
	glUniformMatrix4fv(game_state->inv_view_proj, 1, false, inv_view_proj.v);

	glVertexAttribPointer(game_state->pos_loc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(game_state->pos_loc);

	glQueryCounter(game_state->perf_queries[0], GL_TIMESTAMP);

	glDrawArrays(GL_TRIANGLES, 0, game_state->v_buf.vert_count);

	glQueryCounter(game_state->perf_queries[1], GL_TIMESTAMP);
	game_state->perf_queries_queued = true;
}