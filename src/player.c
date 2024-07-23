#include <math.h>
#include <SDL2/SDL.h>
#include "player.h"

void player_process_input(Player* player) {

    memset(&player->flags, 0, sizeof(player->flags));
    // rotate left
    if (state.keystate[SDL_SCANCODE_LEFT]) {
        player->flags.rotate_left = 1;
        player->flags.rotate_right = 0;
    }
    // rotate right
    if (state.keystate[SDL_SCANCODE_RIGHT]) {
        player->flags.rotate_right = 1;
        player->flags.rotate_left = 0;
    }
    // accelerate
    if (state.keystate[SDL_SCANCODE_UP]) {
        player->flags.accelerate = 1;
    }
}

void player_update(Player* player) {

    // rotation
    i32 rotation = 0;
    if (player->flags.rotate_left) {
        rotation -= 1;
    }
    if (player->flags.rotate_right) {
        rotation += 1;
    }
    player->angle_deg += (ROTATION_SPEED * rotation) * delta_time;
    if (player->angle_deg < 0.0f) { player->angle_deg += 360.0f; }
    if (player->angle_deg >= 360.0f) { player->angle_deg -= 360.0f; }


    if (player->flags.accelerate) {
        player->velocity.x += sinf(DEG_TO_RAD(player->angle_deg)) * ACCELERATION_SPEED * delta_time;
        player->velocity.y -= cosf(DEG_TO_RAD(player->angle_deg)) * ACCELERATION_SPEED * delta_time;
    }

    f32 speed = LENGTH(player->velocity);
    f32 factor = 1.0f;
    if (speed > PLAYER_MAX_SPEED) {
        factor = PLAYER_MAX_SPEED / speed;
    }
    player->velocity.x *= factor;
    player->velocity.y *= factor;


    player->position.x += player->velocity.x;
    player->position.y += player->velocity.y;


    // player x,y is top left 
    if (player->position.x + PLAYER_SIZE / 2 < 0) player->position.x = WINDOW_WIDTH - PLAYER_SIZE / 2;
    if (player->position.x + PLAYER_SIZE / 2 > WINDOW_WIDTH) player->position.x = 0 - PLAYER_SIZE / 2;
    if (player->position.y + PLAYER_SIZE / 2 < 0) player->position.y = WINDOW_HEIGHT - PLAYER_SIZE / 2;
    if (player->position.y + PLAYER_SIZE / 2 > WINDOW_HEIGHT) player->position.y = 0 - PLAYER_SIZE / 2;

    i32 i = 0;
    if (fabs(player->velocity.x) > 0.1f) {
        if (player->velocity.x < 0) { i = 1; }
        if (player->velocity.x > 0) { i = -1; }
        player->velocity.x += ((i * ACCELERATION_SPEED) / 4) * delta_time;
    }
    else {
        player->velocity.x = 0.0f;
    }

    i = 0;
    if (fabs(player->velocity.y) > 0.1f) {

        if (player->velocity.y < 0) { i = 1; }
        if (player->velocity.y > 0) { i = -1; }
        player->velocity.y += ((i * ACCELERATION_SPEED) / 4) * delta_time;
    }
    else {
        player->velocity.y = 0.0f;
    }
}
