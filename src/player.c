#include <math.h>
#include <SDL2/SDL.h>

#include "player.h"
#include "game.h"

void player_init(Player* player) {
    player->position = (V2f32){ .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 };
    player->angle_deg = 0.0f;
    player->velocity = (V2f32){ .x = 0.0f, .y = 0.0f };
    player->flags = (player_flags_t){ 0 };
    player->shoot_timer = 0.0f;
}

void player_shoot(Player* player) {
    if (player->shoot_timer <= 0.0f) {
        player->shoot_timer = SHOOT_COOLDOWN;
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_SHOOT;
        e->event.shoot = (EventShoot){ .position = player->position, .angle_deg = player->angle_deg , .initial_velocity = player->velocity };
        queue_enqueue(&state.event_queue, e);
    }
}

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
    if (state.keystate[SDL_SCANCODE_SPACE]) {
        player_shoot(player);
    }
}

void player_update(Player* player) {
    if (player->shoot_timer > 0.0f) {
        player->shoot_timer -= delta_time;
    }

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


    player->position.x += player->velocity.x * delta_time;
    player->position.y += player->velocity.y * delta_time;


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

void player_render(Player* player) {
    SDL_Rect player_dst_rect = { (int)local_player.position.x - PLAYER_SIZE / 2, (int)local_player.position.y - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderCopyEx(
        state.renderer,
        state.textures.arr[0],
        NULL,
        &player_dst_rect,
        local_player.angle_deg,
        NULL,
        SDL_FLIP_NONE);

}