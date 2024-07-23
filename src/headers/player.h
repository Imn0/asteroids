#pragma once

#include "common.h"

typedef struct player_flags_t{
    i32 rotate_left : 1;
    i32 rotate_right : 1;
    i32 accelerate : 1;
} player_flags_t;

typedef struct {
    V2f32 position;
    f32 angle_deg;
    V2f32 velocity;
    player_flags_t flags;
} Player;

extern Player remote_player;
extern Player local_player;

void player_process_input(Player* player);
void player_update(Player* player);
