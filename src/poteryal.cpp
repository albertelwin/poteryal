
#include <poteryal.hpp>

#include <math.cpp>

void game_update_and_render(GameMemory * game_memory, GameInput * game_input) {
	ASSERT(sizeof(GameState) <= game_memory->size);
	GameState * game_state = (GameState *)game_memory->ptr;

	if(!game_memory->initialised) {
		game_memory->initialised = true;

		game_state->arena = memory_arena(game_state + 1, game_memory->size - sizeof(GameState));

		game_state->total_time = 0.0f;

		game_state->basic_program = gl_program();
		gl_compile_and_add_shader(game_state->basic_program, BASIC_VS_SRC, GL_VERTEX_SHADER);
		gl_compile_and_add_shader(game_state->basic_program, BASIC_FS_SRC, GL_FRAGMENT_SHADER);
		gl_link_program(game_state->basic_program);
		game_state->pos_loc = glGetAttribLocation(game_state->basic_program, "i_position");
		game_state->tex_loc = glGetUniformLocation(game_state->basic_program, "u_tex");
		game_state->color_loc = glGetUniformLocation(game_state->basic_program, "u_color");
		game_state->u_time = glGetUniformLocation(game_state->basic_program, "u_time");
		game_state->inv_view_proj = glGetUniformLocation(game_state->basic_program, "u_inv_view_proj");
		game_state->camera_pos = glGetUniformLocation(game_state->basic_program, "u_camera_pos");

		f32 verts[] = {
			 1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f,-1.0f, 0.0f,
			-1.0f,-1.0f, 0.0f,
			 1.0f,-1.0f, 0.0f,
			 1.0f, 1.0f, 0.0f,
		};
		game_state->v_buf = gl_vertex_buffer(verts, ARRAY_COUNT(verts), 1, GL_STATIC_DRAW);

		u32 texture_channels = 4;
		u32 texture_size = 16;
		u8 * texels = PUSH_ARRAY(&game_state->arena, u8, texture_size * texture_size * texture_size);
		for(u32 z = 0, i = 0; z < texture_size; z++) {
			for(u32 y = 0; y < texture_size; y++) {
				for(u32 x = 0; x < texture_size; x++, i += texture_channels) {

					math::Vec3 pos = math::vec3(0.0f);
					pos.x = (f32)x / (f32)(texture_size - 1);
					pos.y = (f32)y / (f32)(texture_size - 1);
					pos.z = (f32)z / (f32)(texture_size - 1);

					f32 v = ((f32)rand() / (f32)RAND_MAX);

					f32 r = pos.x;
					f32 g = pos.y;
					f32 b = pos.z;
					f32 a = v * MAX(1.0f - math::length(pos * 2.0f - 1.0f) * 1.1f, 0.0f) * 0.2f;

					texels[i + 0] = (u8)(r * 255.0f);
					texels[i + 1] = (u8)(g * 255.0f);
					texels[i + 2] = (u8)(b * 255.0f);
					texels[i + 3] = (u8)(a * 255.0f);
				}
			}
		}

		game_state->volume_tex_size = texture_size;
		glGenTextures(1, &game_state->volume_tex_id);
		ASSERT(game_state->volume_tex_id);

		glBindTexture(GL_TEXTURE_3D, game_state->volume_tex_id);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, texture_size, texture_size, texture_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glViewport(0, 0, game_input->back_buffer_width, game_input->back_buffer_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	game_state->total_time += game_input->delta_time;

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(game_state->basic_program);

	glBindBuffer(GL_ARRAY_BUFFER, game_state->v_buf.id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, game_state->volume_tex_id);
	glUniform1i(game_state->tex_loc, 0);

	f32 gray = sin((f32)game_state->total_time) * 0.5f + 0.5f;
	glUniform4f(game_state->color_loc, gray, gray, gray, 1.0f);

	f32 camera_speed = 1.0f / 20.0f;

	math::Vec3 camera_pos = math::vec3(0.0f);
	camera_pos.x = math::cos(game_state->total_time * math::TAU * camera_speed) * 2.0f;
	camera_pos.z = math::sin(game_state->total_time * math::TAU * camera_speed) * 2.0f;

	math::Mat4 view = math::look_at(camera_pos, math::vec3(0.0f), math::VEC3_UP);

	f32 aspect_ratio = (f32)game_input->back_buffer_width / (f32)game_input->back_buffer_height;
	math::Mat4 proj = math::perspective_projection(aspect_ratio, 60.0f, 0.1f, F32_MAX);

	math::Mat4 view_proj = proj * view;
	math::Mat4 inv_view_proj = math::inverse(view_proj);

	glUniform1f(game_state->u_time, game_state->total_time);
	glUniform3f(game_state->camera_pos, camera_pos.x, camera_pos.y, camera_pos.z);
	glUniformMatrix4fv(game_state->inv_view_proj, 1, false, inv_view_proj.v);

	glVertexAttribPointer(game_state->pos_loc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(game_state->pos_loc);

	glDrawArrays(GL_TRIANGLES, 0, game_state->v_buf.vert_count);
}