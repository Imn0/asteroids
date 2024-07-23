#pragma once

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <threads.h>
#include <SDL2/SDL.h>

#include "settings.h"

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef size_t usize;
typedef ssize_t isize;

typedef struct _v2_f32 { f32 x; f32 y; } V2f32;
typedef struct _v2_i32 { i32 x; i32 y; } V2i32;

typedef struct {
    // SDL 
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* render_target;
    struct { SDL_Texture* arr[TEXTURES_CAPACITY]; i32 size; } textures;
    const u8* keystate;

    // game stuff
    bool exit;
}State;

extern f32 delta_time;
extern State state;

#define PI 3.14159265359f
#define TAU (2.0f * PI)
#define PI_2 (PI / 2.0f)
#define PI_4 (PI / 4.0f)

#define DEG_TO_RAD(_d) ((_d) * (PI / 180.0f))
#define DOT(_v0, _v1) ({ __typeof__(_v0) __v0 = (_v0), __v1 = (_v1); (__v0.x * __v1.x) + (__v0.y * __v1.y); })
#define LENGTH(_vl) ({ __typeof__(_vl) __vl = (_vl); sqrtf(DOT(__vl, __vl)); })
#define RAD_TO_DEG(_d) ((_d) * (180.0f / PI))

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

typedef enum ReturnType {
    ERR_OK = 0,
    QUEUE_OK,
    QUEUE_EMPTY,
    ERR_NETWORK
}ReturnType;