#pragma once

#include "common.h"

typedef struct player_flags_t{
    i32 rotate_left : 1;
    i32 rotate_right : 1;
    i32 accelerate : 1;
} player_flags_t;

typedef struct {
    V2f32 position, velocity;
    f32 angle_deg;
    player_flags_t flags;
    f32 shoot_timer;
} Player;

extern Player remote_player;
extern Player local_player;

void player_init(Player* player);
void player_process_input(Player* player);
void player_update(Player* player);
void player_render(Player* player);