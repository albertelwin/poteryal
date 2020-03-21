
#ifndef SYS_HPP_INCLUDED
#define SYS_HPP_INCLUDED

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define __TOKEN_STRINGIFY(x) #x
#define TOKEN_STRINGIFY(x) __TOKEN_STRINGIFY(x)

#if DEBUG_MODE
	#if defined(WIN32)
		#define __FORCE_EXIT() *((int *)(0)) = 0;
	#elif defined(__EMSCRIPTEN__)
		#define __FORCE_EXIT() emscripten_force_exit(EXIT_FAILURE)
	#endif

	#define __PRINT_ASSERT(x) printf("ASSERT: %s\n", x)

	#define __ASSERT(x) \
		if(!(x)) { \
			__PRINT_ASSERT("" #x " : " __FILE__ " : " TOKEN_STRINGIFY(__LINE__)); \
			__FORCE_EXIT(); \
		}

	#define ASSERT(x) __ASSERT(x)
	#define INVALID_PATH() ASSERT(!"Invalid path")
	#define INVALID_CASE() default: { ASSERT(!"Invalid case"); break; }
#else
	#define ASSERT(...)
	#define INVALID_CASE(...) default: { break; }
#endif

#define ARRAY_COUNT(x) (sizeof((x)) / sizeof((x)[0]))

#define KILOBYTES(x) (1024 * (x))
#define MEGABYTES(x) (1024 * KILOBYTES(x))
#define GIGABYTES(x) (1024 * MEGABYTES(x))
#define TERABYTES(x) (1024 * GIGABYTES(x))

#define ALIGN(x, y) (((x) + ((y) - 1)) & ~((y) - 1))
#define ALIGN4(x) ALIGN(x, 4)
#define ALIGN8(x) ALIGN(x, 8)
#define ALIGN16(x) ALIGN(x, 16)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define SWAP(t, x, y) { t tmp__ = (x); (x) = (y); (y) = tmp__; }

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef uint32_t b32;

#define U32_MAX 4294967295

#define F32_MAX 1e+37f

#define F32_SIGN_MASK 0x80000000
#define F32_EXPONENT_MASK 0x7F800000
#define F32_MANTISSA_MASK 0x007FFFFF

inline b32 f32_is_nan(f32 val) {
	u32 bits = *((u32 *)&val);
	return (bits & F32_EXPONENT_MASK) == F32_EXPONENT_MASK && (bits & F32_MANTISSA_MASK) != 0;
}

inline b32 f32_is_inf(f32 val) {
	u32 bits = *((u32 *)&val);
	return (bits & F32_EXPONENT_MASK) == F32_EXPONENT_MASK && (bits & F32_MANTISSA_MASK) == 0;
}

inline b32 f32_is_neg(f32 val) {
	u32 bits = *((u32 *)&val);
	return (bits & F32_SIGN_MASK) == F32_SIGN_MASK;
}

inline u32 c_str_len(char const * str) {
	u32 len = 0;
	while(*str) {
		str++;
		len++;
	}

	return len;
}

inline void c_str_cpy(char * dst, char const * src) {
	while(*src) {
		*dst++ = *src++;
	}

	*dst = 0;
}

inline b32 c_str_eql(char const * str0, char const * str1) {
	while(*str0 && *str1) {
		if(*str0++ != *str1++) {
			break;
		}
	}

	return *str0 == 0 && *str1 == 0;
}

inline void c_str_cat(char * dst, char * src) {
	while(*dst) {
		dst++;
	}

	while(*src) {
		*dst++ = *src++;
	}

	*dst = 0;
}

inline b32 str_eql(char * x, u32 x_len, char * y, u32 y_len) {
	b32 equal;
	if(x_len == y_len) {
		equal = true;

		for(u32 i = 0; i < x_len; i++) {
			if(x[i] != y[i]) {
				equal = false;
				break;
			}
		}
	}
	else {
		equal = false;
	}

	return equal;
}

struct MemoryPtr {
	size_t size;
	u8 * ptr;
};

struct MemoryArena {
	size_t size;
	size_t used;

	u8 * base_address;

	u32 temp_count;
};

struct TemporaryMemory {
	MemoryArena * arena;
	size_t used_snapshot;
};

inline MemoryArena memory_arena(void * base_address, size_t size) {
	MemoryArena arena = {};
	arena.size = size;
	arena.used = 0;
	arena.base_address = (u8 *)base_address;
	arena.temp_count = 0;
	return arena;
}

inline void zero_memory_arena(MemoryArena * arena) {
	arena->used = 0;
	arena->temp_count = 0;
	for(size_t i = 0; i < arena->size; i++) {
		arena->base_address[i] = 0;
	}
}

#define ZERO_STRUCT(x) zero_memory(x, sizeof(*x))
#define ZERO_ARRAY(x) zero_memory((x), sizeof((x)));
inline void zero_memory(void * ptr, size_t size) {
	u8 * ptr_u8 = (u8 *)ptr;
	for(size_t i = 0; i < size; i++) {
		ptr_u8[i] = 0;
	}
}

inline void copy_memory(void * dst, void * src, size_t size) {
	u8 * dst_u8 = (u8 *)dst;
	u8 * src_u8 = (u8 *)src;
	for(size_t i = 0; i < size; i++) {
		dst_u8[i] = src_u8[i];
	}
}

#define PUSH_STRUCT(arena, type, ...) (type *)push_memory_(arena, sizeof(type), ##__VA_ARGS__)
#define PUSH_ARRAY(arena, type, length, ...) (type *)push_memory_(arena, sizeof(type) * (length), ##__VA_ARGS__)
#define PUSH_MEMORY(arena, type, size, ...) (type *)push_memory_(arena, size, ##__VA_ARGS__)
inline void * push_memory_(MemoryArena * arena, size_t size, b32 zero = true) {
	size_t aligned_size = ALIGN16(size);
	ASSERT((arena->used + aligned_size) <= arena->size);

	void * ptr = arena->base_address + arena->used;
	if(zero) {
		zero_memory(ptr, aligned_size);
	}

	arena->used += aligned_size;

	return ptr;
}

inline TemporaryMemory begin_temp_memory(MemoryArena * arena) {
	TemporaryMemory temp_memory = {};
	temp_memory.arena = arena;
	temp_memory.used_snapshot = arena->used;

	arena->temp_count++;

	return temp_memory;
}

inline void end_temp_memory(TemporaryMemory temp_memory) {
	MemoryArena * arena = temp_memory.arena;
	ASSERT(arena->temp_count);
	ASSERT(arena->used >= temp_memory.used_snapshot);

	arena->used = temp_memory.used_snapshot;
	arena->temp_count--;
}

inline void check_arena(MemoryArena * arena) {
	ASSERT(!arena->temp_count);
}

inline MemoryArena allocate_sub_arena(MemoryArena * arena, size_t size) {
	size_t aligned_size = ALIGN16(size);

	MemoryArena sub_arena = {};
	sub_arena.size = aligned_size;
	sub_arena.used = 0;
	sub_arena.base_address = PUSH_MEMORY(arena, u8, aligned_size);
	return sub_arena;
}

#define PUSH_COPY_ARRAY(arena, type, array, length) (type *)push_copy_memory_(arena, array, sizeof(type) * (length))
inline void * push_copy_memory_(MemoryArena * arena, void * src, size_t size) {
	void * dst = push_memory_(arena, size);
	copy_memory(dst, src, size);
	return dst;
}

#define ALLOC_STRUCT(type, ...) (type *)alloc_memory_(sizeof(type), ##__VA_ARGS__)
#define ALLOC_ARRAY(type, length, ...) (type *)alloc_memory_(sizeof(type) * length, ##__VA_ARGS__)
#define ALLOC_MEMORY(type, size, ...) (type *)alloc_memory_(size, ##__VA_ARGS__)
inline void * alloc_memory_(size_t size, b32 zero = true) {
	void * ptr = malloc(size);
	if(zero) {
		zero_memory(ptr, size);
	}

	return ptr;
}

#define FREE_MEMORY(ptr) free_memory_(ptr)
inline void free_memory_(void * ptr) {
	free(ptr);
}

struct Str {
	char * ptr;
	u32 len;
	u32 max_len;
};

inline Str * allocate_str(MemoryArena * arena, u32 max_len) {
	Str * str = PUSH_STRUCT(arena, Str);
	str->max_len = max_len;
	str->len = 0;
	str->ptr = PUSH_ARRAY(arena, char, max_len);
	return str;
}

inline Str str_fixed_size(char * ptr, u32 max_len) {
	Str str;
	str.ptr = (char *)ptr;
	str.len = 0;
	str.max_len = max_len;
	return str;
}

inline Str str_from_c_str(char const * c_str) {
	Str str;
	str.max_len = str.len = c_str_len(c_str);
	str.ptr = (char *)c_str;
	return str;
}

inline void str_clear(Str * str) {
	str->len = 0;
	str->ptr[0] = 0;
}

inline b32 str_eql(Str * x, char * y) {
	return str_eql(x->ptr, x->len, y, c_str_len(y));
}

inline b32 str_eql(Str * x, Str * y) {
	return str_eql(x->ptr, x->len, y->ptr, y->len);
}

inline void str_push(Str * str, char char_) {
	ASSERT(str->len < (str->max_len - 1));

	str->ptr[str->len++] = char_;
	str->ptr[str->len] = 0;
}

inline void str_push_c_str(Str * str, char const * val) {
	while(*val) {
		str_push(str, *val++);
	}
}

inline void str_push_u32(Str * str, u32 val) {
	if(!val) {
		str_push(str, '0');
	}
	else {
		char * dst = str->ptr + str->len;

		u32 val_ = val;
		while(val_) {
			dst++;
			str->len++;
			val_ /= 10;
		}

		*dst-- = 0;

		while(val) {
			*dst-- = '0' + val % 10;
			val /= 10;
		}
	}
}

inline void str_push_f32(Str * str, f32 val, u32 precision = 0) {
	if(f32_is_neg(val)) {
		str_push(str, '-');
		val = -val;
	}

	if(f32_is_inf(val)) {
		str_push_c_str(str, "inf");
	}
	else if(f32_is_nan(val)) {
		str_push_c_str(str, "nan");
	}
	else {
		str_push_u32(str, (u32)val);
		str_push(str, '.');

		if(precision == 0) {
			precision = 4;
		}

		u32 p = 10;
		//TODO: Can we do this without a loop??
		for(u32 i = 1; i < precision; i++) {
			p *= 10;
		}

		u32 frc = (u32)((val - (u32)val) * p);

		p /= 10;
		while(p) {
			str_push(str, '0' + ((frc / p) % 10));
			p /= 10;
		}
	}
}

inline void str_print(Str * str, char const * fmt, ...) {
	va_list args;
	va_start(args, fmt);

	while(*fmt) {
		if(fmt[0] == '%') {
			//TODO: Support multiple digit precision values!!
			u32 precision = 0;
			if(fmt[1] == '.') {
				precision = fmt[2] - '0';
				fmt += 2;
			}

			switch(fmt[1]) {
				case 's': {
					str_push_c_str(str, va_arg(args, char *));
					break;
				}

				case 'f': {
					str_push_f32(str, (f32)va_arg(args, f64), precision);
					break;
				}

				case 'u': {
					str_push_u32(str, va_arg(args, u32));
					break;
				}

				default: {
					ASSERT(false);
					break;
				}
			}

			fmt += 2;
		}
		else {
			str_push(str, *fmt++);
		}
	}

	str->ptr[str->len] = 0;
	va_end(args);
}

#define KEY_DOWN_BIT 0
#define KEY_PRESSED_BIT 1
#define KEY_RELEASED_BIT 2

#define KEY_DOWN (1 << KEY_DOWN_BIT)
#define KEY_PRESSED (1 << KEY_PRESSED_BIT)
#define KEY_RELEASED (1 << KEY_RELEASED_BIT)

inline MemoryPtr read_file_to_memory(char const * file_name) {
	MemoryPtr mem_ptr = {};

	FILE * file_ptr = fopen(file_name, "rb");
	if(file_ptr) {
		fseek(file_ptr, 0, SEEK_END);
		mem_ptr.size = (size_t)ftell(file_ptr);
		rewind(file_ptr);

		mem_ptr.ptr = ALLOC_MEMORY(u8, mem_ptr.size);
		size_t read_result = fread(mem_ptr.ptr, 1, mem_ptr.size, file_ptr);
		ASSERT(read_result == mem_ptr.size);

		fclose(file_ptr);
	}

	return mem_ptr;
}

#endif