#pragma once

#include "common.h"

#define PLAYER_BOOP_OFF_TIME 0.2f
#define PLAYER_BOOP_ON_TIME 0.5f

#define PLAYER_SEGMENTS_SIZE 3

static const V2f32 player_segments[PLAYER_SEGMENTS_SIZE][2] = {
    // also used in animation
    {{0, -32}, {-20.42553, 32}},
    {{0, -32}, {20.42553, 32}},
    {{-15, 15}, {15, 15}},
};

static const V2f32 player_boop_segments[][2] = {
    {{0, 44}, {-10, 15}},
    {{0, 44}, {10, 15}},
};

typedef struct player_flags_t {
    u32 rotate_left : 1;
    u32 rotate_right : 1;
    u32 accelerate : 1;
    u32 invisible : 1;
} player_flags_t;

typedef struct player_flags_ex_t {
    u32 boop : 1;
} player_flags_ex_t;

typedef struct {
    V2f32 position, velocity;
    f32 angle_deg;
    player_flags_t flags;
    player_flags_ex_t ex_flags;
    f32 shoot_timer;
    f32 bloop_timer;
    struct {
        bool phantom_enabled;
        V2f32 position;
    } phantom_player;
} Player;

extern Player remote_player;
extern Player local_player;

void player_init(Player* player);
void player_process_input(Player* player);
void player_update(Player* player);
void player_render(Player* player);
