#pragma once

#include "common.h"
#include "entity.h"

#define PLAYER_SCALE 0.66f
#define PLAYER_SIZE 64 * PLAYER_SCALE // used for turning on phantom
#define PLAYER_BOOP_OFF_TIME 0.05f
#define PLAYER_BOOP_ON_TIME 0.05f
#define PLAYER_SEGMENTS_SIZE 3

static const V2f32 player_segments[PLAYER_SEGMENTS_SIZE][2] = {
    // also used in animation
    { { 0 * PLAYER_SCALE, -32 * PLAYER_SCALE },
     { -20.42553 * PLAYER_SCALE, 32 * PLAYER_SCALE } },
    { { 0 * PLAYER_SCALE, -32 * PLAYER_SCALE },
     { 20.42553 * PLAYER_SCALE, 32 * PLAYER_SCALE }  },
    { { -15 * PLAYER_SCALE, 15 * PLAYER_SCALE },
     { 15 * PLAYER_SCALE, 15 * PLAYER_SCALE }        },
};

static const V2f32 player_boop_segments[][2] = {
    { { 0 * PLAYER_SCALE, 44 * PLAYER_SCALE },
     { -10 * PLAYER_SCALE, 15 * PLAYER_SCALE } },
    { { 0 * PLAYER_SCALE, 44 * PLAYER_SCALE },
     { 10 * PLAYER_SCALE, 15 * PLAYER_SCALE }  },
};

typedef struct player_flags_t {
    u32 rotate_left : 1;
    u32 rotate_right : 1;
    u32 accelerate : 1;
    u32 invisible : 1;
    u32 perma_dead : 1;
} player_flags_t;

typedef struct player_flags_ex_t {
    u32 boop : 1;
    u32 invincible : 1;
} player_flags_ex_t;

typedef struct {
    V2f32 position, velocity;
    f32 angle_deg;
    player_flags_t flags;
    player_flags_ex_t ex_flags;
    f32 shoot_timer;
    f32 bloop_timer;
    f32 invincibility_timer;
    u32 score;
    u8 lives;
    struct {
        bool phantom_enabled;
        V2f32 position;
    } phantom_player;
} Player;

extern Player remote_player;
extern Player local_player;

void player_init(Player* player);
void player_process_input(Player* player, bool shoot);
i32 player_add_score_rock_kill(Player* player, RockSize rock_size);
void player_render_score(Player* player, bool bool_is_remote);

void player_update(Player* player, bool bool_is_remote);

void player_render(Player* player, SDL_Color color);
