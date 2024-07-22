#pragma once

#include "common.h"

typedef struct __player {
    V2f position;
    f32 velocity, angle_deg;
}Player;

#define ROTATION_SPEED 90.0f
#define ACCELERATION_SPEED 1030.0f

void player_rotate(Player* player, i32 direction);
void player_accelerate(Player* player);
void player_update(Player* player);