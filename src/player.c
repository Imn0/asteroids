#include <SDL2/SDL.h>

#include "game.h"
#include "netcode.h"
#include "physics.h"
#include "player.h"

void player_init(Player* player) {
    player->position = (V2f32){ .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 };
    player->angle_deg = 0.0f;
    player->velocity = (V2f32){ .x = 0.0f, .y = 0.0f };
    player->flags = (player_flags_t){ 0 };
    player->shoot_timer = 0.0f;
    player->bloop_timer = PLAYER_BOOP_OFF_TIME;
}

void player_boop(Player* player) {
    player->bloop_timer -= delta_time;
    if (player->bloop_timer < 0.0f) {
        if (player->ex_flags.boop) {
            player->ex_flags.boop = 0;
            player->bloop_timer = PLAYER_BOOP_OFF_TIME;
        }
        else {
            player->bloop_timer = PLAYER_BOOP_ON_TIME;
            player->ex_flags.boop = 1;
        }
    }
}

void player_shoot(Player* player) {
    if (player->shoot_timer <= 0.0f) {
        player->shoot_timer = PLAYER_SHOOT_COOLDOWN;

        // create local event
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_SHOOT;
        e->event.shoot = (EventShoot){ .position = player->position,
                                      .angle_deg = player->angle_deg,
                                      .initial_velocity = player->velocity };
        queue_enqueue(&state.event_queue, e);

        if (!network_state.online_disable) {
            Packet* p = packet_from_event(e);
            if (e == NULL) {
                return;
            }
            queue_enqueue(&network_state.transmit.tx_queue, (void*)p);
        }
    }
}

void player_process_input(Player* player) {

    player->flags = (player_flags_t){ .invisible = player->flags.invisible };
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
    player->angle_deg += (PLAYER_ROTATION_SPEED * rotation) * delta_time;
    if (player->angle_deg < 0.0f) {
        player->angle_deg += 360.0f;
    }
    if (player->angle_deg >= 360.0f) {
        player->angle_deg -= 360.0f;
    }

    // acceleration / movement
    if (player->flags.accelerate) {
        player_boop(player);
        player->velocity.x += sinf(deg_to_rad(player->angle_deg)) *
            PLAYER_ACCELERATION_SPEED * delta_time;
        player->velocity.y -= cosf(deg_to_rad(player->angle_deg)) *
            PLAYER_ACCELERATION_SPEED * delta_time;
    }
    else {
        player->ex_flags.boop = 0;
    }

    f32 speed = length(player->velocity);
    f32 factor = 1.0f;
    if (speed > PLAYER_MAX_SPEED) {
        factor = PLAYER_MAX_SPEED / speed;
    }
    player->velocity.x *= factor;
    player->velocity.y *= factor;

    player->position.x += player->velocity.x * delta_time;
    player->position.y += player->velocity.y * delta_time;

    player->phantom_player.phantom_enabled = false;
    if (player->position.x < PLAYER_SIZE) {
        player->phantom_player.position.x = WINDOW_WIDTH + player->position.x;
        player->phantom_player.position.y = player->position.y;
        player->phantom_player.phantom_enabled = true;
    }
    else if (player->position.x > WINDOW_WIDTH - PLAYER_SIZE) {
        player->phantom_player.position.x = player->position.x - WINDOW_WIDTH;
        player->phantom_player.position.y = player->position.y;
        player->phantom_player.phantom_enabled = true;
    }

    if (player->position.y < PLAYER_SIZE) {
        if (!player->phantom_player.phantom_enabled) {
            player->phantom_player.position.y = player->position.x;
        }
        player->phantom_player.position.y = WINDOW_HEIGHT + player->position.y;
        player->phantom_player.phantom_enabled = true;
    }
    else if (player->position.y > WINDOW_HEIGHT - PLAYER_SIZE) {
        if (!player->phantom_player.phantom_enabled) {
            player->phantom_player.position.y = player->position.x;
        }
        player->phantom_player.position.y = player->position.y - WINDOW_HEIGHT;
        player->phantom_player.phantom_enabled = true;
    }

    if (player->position.x < 0) player->position.x = WINDOW_WIDTH;
    if (player->position.x > WINDOW_WIDTH) player->position.x = 0;
    if (player->position.y < 0) player->position.y = WINDOW_HEIGHT;
    if (player->position.y > WINDOW_HEIGHT) player->position.y = 0;

    // deaceleration
    decelerate_v2f32(&player->velocity, PLAYER_ACCELERATION_SPEED / 10);

    queue_enqueue(&network_state.transmit.tx_queue,
                  packet_from_player(&local_player));
}

void player_render(Player* player) {
    if (player->flags.invisible) {
        return;
    }
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);

    for (i32 i = 0; i < PLAYER_SEGMENTS_SIZE; i++) {
        V2f32 start_point = rotate_point(player_segments[i][0], (V2f32) { 0, 0 },
                                         deg_to_rad(player->angle_deg));
        V2f32 end_point = rotate_point(player_segments[i][1], (V2f32) { 0, 0 },
                                       deg_to_rad(player->angle_deg));

        gfx_render_thick_line(
            state.renderer, (i32)start_point.x + player->position.x,
            (i32)start_point.y + player->position.y,
            (i32)end_point.x + player->position.x,
            (i32)end_point.y + player->position.y, LINE_THICKNESS);
    }

    if (player->phantom_player.phantom_enabled) {

        for (i32 i = 0; i < PLAYER_SEGMENTS_SIZE; i++) {
            V2f32 start_point =
                rotate_point(player_segments[i][0], (V2f32) { 0, 0 },
                             deg_to_rad(player->angle_deg));
            V2f32 end_point = rotate_point(player_segments[i][1], (V2f32) { 0, 0 },
                                           deg_to_rad(player->angle_deg));

            gfx_render_thick_line(
                state.renderer,
                (i32)start_point.x + player->phantom_player.position.x,
                (i32)start_point.y + player->phantom_player.position.y,
                (i32)end_point.x + player->phantom_player.position.x,
                (i32)end_point.y + player->phantom_player.position.y,
                LINE_THICKNESS);
        }
    }

    if (player->ex_flags.boop) {
        for (i32 i = 0; i < 2; i++) {
            V2f32 start_point =
                rotate_point(player_boop_segments[i][0], (V2f32) { 0, 0 },
                             deg_to_rad(player->angle_deg));
            V2f32 end_point =
                rotate_point(player_boop_segments[i][1], (V2f32) { 0, 0 },
                             deg_to_rad(player->angle_deg));

            gfx_render_thick_line(
                state.renderer, (i32)start_point.x + player->position.x,
                (i32)start_point.y + player->position.y,
                (i32)end_point.x + player->position.x,
                (i32)end_point.y + player->position.y, LINE_THICKNESS);
        }
    }
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
}
