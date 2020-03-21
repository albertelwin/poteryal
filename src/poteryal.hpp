
#ifndef POTERYAL_HPP
#define POTERYAL_HPP

//TODO: Remove this!!
#define NOMINMAX
#include <windows.h>

#include <sys.hpp>
#include <gl.hpp>

#include <math.hpp>

enum GameKey {
	GameKey_up,
	GameKey_down,
	GameKey_left,
	GameKey_right,

	GameKey_return,

	GameKey_count,
};

struct GameInput {
	u32 back_buffer_width;
	u32 back_buffer_height;

	f32 delta_time;
	b32 quit;

	u8 keys[GameKey_count];
};

struct GameMemory {
	size_t size;
	u8 * ptr;

	b32 reloaded;
	b32 initialised;
};

struct GameState {
	MemoryArena arena;

	f32 total_time;

	b32 perf_queries_queued;
	u32 perf_queries[2];

	u32 raycast_program;
	//TODO: Generate all these vars from the shader itself!!
	u32 i_position;
	u32 u_tex;
	u32 u_color;
	u32 u_time;
	u32 u_inv_view_proj;
	u32 u_camera_pos;

	u32 vertex_array;
	GLVertexBuffer v_buf;

	u32 tex_id;
	u32 tex_size;
	u8 * texels;

	Vec3 points[4096 * 4];
};

extern "C" typedef void (* GameUpdateAndRender)(GameMemory * game_memory, GameInput * game_input);

#endif