#include <math.h>
#include <SDL2/SDL.h>
#include "player.h"

void player_apply_physics(Player* player) {
    // TODO
    player->speed -= (ACCELERATION_SPEED / 15) * delta_time;

}

void player_rotate(Player* player, i32 direction) {
    player->angle_deg += (direction * ROTATION_SPEED) * delta_time;
    if (player->angle_deg < 0.0f) { player->angle_deg += 360.0f; }
    else if (player->angle_deg >= 360.0f) { player->angle_deg -= 360.0f; }
}

void player_accelerate(Player* player) {
    player->speed += ACCELERATION_SPEED * delta_time;
    player->position.x += sinf(DEG_TO_RAD(player->angle_deg));
    player->position.y -= cosf(DEG_TO_RAD(player->angle_deg));
}

void player_process_input(Player* player) {
    // rotate left
    if (state.keystate[SDL_SCANCODE_LEFT]) {
        player_rotate(player, -1);
    }
    // rotate right
    if (state.keystate[SDL_SCANCODE_RIGHT]) {
        player_rotate(player, 1);
    }
    // accelerate
    if (state.keystate[SDL_SCANCODE_UP]) {
        player_accelerate(player);
    }
}

void player_update(Player* player){
    player_apply_physics(player);
}
