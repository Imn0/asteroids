#include <math.h>

#include "player.h"


void player_rotate(Player* player, i32 direction) {
    player->angle_deg += (direction * ROTATION_SPEED) * delta_time;
    player->angle_deg = fmod(player->angle_deg, 360.0f);
}

void player_accelerate(Player* player) {
    player->velocity += ACCELERATION_SPEED * delta_time;
}

void player_update(Player* player) {
    player->position.x += player->velocity * cos(DEG_TO_RAD(player->angle_deg)) * delta_time;
    player->position.y += player->velocity * sin(DEG_TO_RAD(player->angle_deg)) * delta_time;
    player->velocity *= 0.95f;
}