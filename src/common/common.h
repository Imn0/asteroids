#pragma once

#include <stdint.h>
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

typedef struct _v2_f32 { f32 x; f32 y; } V2f;
typedef struct _v2_i32 { i32 x; i32 y; } V2i32;

enum PacketType {
    PACKET_PLAYER
};

typedef struct {
    f32 x, y, angle, speed, rotation_speed;
} PlayerPacket;

typedef struct {
    u8 type;
    i32 time_stamp;
    i32 size;
    union {
        PlayerPacket player;
        /* MORE */
    } payload;
} Packet;

typedef struct PacketQueueNode {
    Packet* packet;
    struct PacketQueueNode* next;
} PacketQueueNode;

typedef struct {
    struct PacketQueueNode* front;
    struct PacketQueueNode* rear;
    mtx_t mutex;
    cnd_t not_empty;
    usize size;
    usize max_size;
} PacketQueue;

typedef struct {
    V2f position;
    f32 speed, angle_deg, rotation_speed;
}Player;

typedef struct {
    // netcode
    bool is_server;
    i32 socket_fd;
    struct {
        thrd_t receive_thrd;
        PacketQueue rx_queue;
    }receive;
    struct {
        thrd_t transmit_thrd;
        PacketQueue tx_queue;
    }transmit;
} NetworkState;

typedef struct {
    // SDL 
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* render_target;
    struct { SDL_Texture* arr[TEXTURES_CAPACITY]; i32 size; } textures;
    const u8* keystate;

    // game stuff
    bool exit;
    Player remote_player;
}State;

extern f32 delta_time;
extern State state;
extern NetworkState network_state;

#define PI 3.14159265359f
#define TAU (2.0f * PI)
#define PI_2 (PI / 2.0f)
#define PI_4 (PI / 4.0f)

#define DEG_TO_RAD(_d) ((_d) * (PI / 180.0f))
#define RAD_TO_DEG(_d) ((_d) * (180.0f / PI))

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

typedef enum ReturnType {
    ERR_OK = 0,
    QUEUE_OK,
    QUEUE_EMPTY,
}ReturnType;