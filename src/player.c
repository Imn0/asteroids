#include <SDL.h>

#include "events.h"
#include "game.h"
#include "netcode.h"
#include "physics.h"
#include "player.h"
#include "sfx.h"

void player_render_impl(f32 angle_deg, V2f32 position, SDL_Color color);

void player_init(Player* player) {
    player->position = (V2f32) { .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 };
    player->angle_deg = 0.0f;
    player->velocity = (V2f32) { .x = 0.0f, .y = 0.0f };
    player->flags = (player_flags_t) { 0 };
    player->shoot_timer = 0.0f;
    player->bloop_timer = PLAYER_BOOP_OFF_TIME;
    player->score = 0;
    player->lives = 3;
}

// engine effects
void player_boop(Player* player) {
    player->bloop_timer -= delta_time;
    if (player->bloop_timer < 0.0f) {
        if (player->ex_flags.boop) {
            player->ex_flags.boop = 0;
            player->bloop_timer = PLAYER_BOOP_OFF_TIME;
        } else {
            player->bloop_timer = PLAYER_BOOP_ON_TIME;
            player->ex_flags.boop = 1;
            play_sound(SFX_THRUST);
        }
    }
}

// TODO burst mode
void player_shoot(Player* player) {
    if (player->shoot_timer > 0.0f) {
        return;
    }

    player->shoot_timer = PLAYER_SHOOT_COOLDOWN;

    player->velocity.x -= sinf(deg_to_rad(player->angle_deg)) * PLAYER_ACCELERATION_SPEED * 0.09f;
    player->velocity.y += cosf(deg_to_rad(player->angle_deg)) * PLAYER_ACCELERATION_SPEED * 0.09f;

    add_event_shoot(player->position, player->angle_deg, player->velocity, BULLET_LOCAL);
}

void player_process_input(Player* player, bool shoot) {

    player->flags.rotate_left = 0;
    player->flags.rotate_right = 0;
    player->flags.accelerate = 0;
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

    if (shoot) {
        player_shoot(player);
    }

    // if (state.keystate[SDL_SCANCODE_SPACE]) {
    // }
}

i32 player_add_score_rock_kill(Player* player, RockSize rock_size) {
    u32 additional_score = 0;
    switch (rock_size) {
    case ROCK_BIG:
        additional_score = BIG_ROCK_KILL_POINTS;
        break;
    case ROCK_MEDIUM:
        additional_score = MEDIUM_ROCK_KILL_POINTS;
        break;
    case ROCK_SMALL:
        additional_score = SMALL_ROCK_KILL_POINTS;
        break;
    }
    player->score += additional_score;
    return additional_score;
}

void player_render_score(Player* player, bool is_remote) {
    f32 y = 0.0f;

    // points
    SDL_Color color = { LOCAL_PLAYER_COLOR, 255 };

    if (is_remote) {
        y = 120.0f;
        color = (SDL_Color) { REMOTE_PLAYER_COLOR, .a = 255 };
    }

    char score[64];
    snprintf(score, sizeof(score), "%d", player->score);
    SDL_Surface* score_surface = TTF_RenderText_Solid(state.big_font, score, color);
    SDL_Rect score_rect = { 20, y, score_surface->w, score_surface->h };

    SDL_Texture* score_texture = SDL_CreateTextureFromSurface(state.renderer, score_surface);
    SDL_RenderCopy(state.renderer, score_texture, NULL, &score_rect);

    SDL_FreeSurface(score_surface);
    SDL_DestroyTexture(score_texture);

    // lives
    y += 96.0f;
    float x = 0.0f;
    for (i32 i = 0; i < player->lives; i++) {
        x += 40.0f;
        player_render_impl(0.0f, (V2f32) { x, y }, color);
    }
}

void player_update(Player* player, bool bool_is_remote) {

    if (player->flags.perma_dead == 1) {
        return;
    }

    if (player->shoot_timer > 0.0f) {
        player->shoot_timer -= delta_time;
    }

    if (player->invincibility_timer > 0.0f) {
        f32 blip_time = 0.0f;
        if (player->invincibility_timer > 2.5f) {
            blip_time = 0.75f;
        } else if (player->invincibility_timer > 1.5f) {
            blip_time = 0.55f;
        } else if (player->invincibility_timer > 1.0f) {
            blip_time = 0.45f;
        } else if (player->invincibility_timer > 0.5f) {
            blip_time = 0.35f;
        } else {
            blip_time = 0.25f;
        }

        f32 x = fmodf(player->invincibility_timer, blip_time);
        if (x > blip_time / 2) {
            player->flags.invisible = 0;
        } else {
            player->flags.invisible = 1;
        }

        player->invincibility_timer -= delta_time;
    } else {
        player->flags.invisible = 0;
        player->ex_flags.invincible = 0;
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
        player->velocity.x += sinf(deg_to_rad(player->angle_deg)) * PLAYER_ACCELERATION_SPEED *
                              delta_time;
        player->velocity.y -= cosf(deg_to_rad(player->angle_deg)) * PLAYER_ACCELERATION_SPEED *
                              delta_time;
    } else {
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
    } else if (player->position.x > WINDOW_WIDTH - PLAYER_SIZE) {
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
    } else if (player->position.y > WINDOW_HEIGHT - PLAYER_SIZE) {
        if (!player->phantom_player.phantom_enabled) {
            player->phantom_player.position.y = player->position.x;
        }
        player->phantom_player.position.y = player->position.y - WINDOW_HEIGHT;
        player->phantom_player.phantom_enabled = true;
    }

    if (player->position.x < 0)
        player->position.x = WINDOW_WIDTH;
    if (player->position.x > WINDOW_WIDTH)
        player->position.x = 0;
    if (player->position.y < 0)
        player->position.y = WINDOW_HEIGHT;
    if (player->position.y > WINDOW_HEIGHT)
        player->position.y = 0;

    decelerate_v2f32(&player->velocity, PLAYER_ACCELERATION_SPEED / 10);
}

void player_render(Player* player, SDL_Color color) {
    if (player->flags.invisible) {
        return;
    }
    player_render_impl(player->angle_deg, player->position, color);

    if (player->phantom_player.phantom_enabled) {
        player_render_impl(player->angle_deg, player->phantom_player.position, color);
    }

    if (player->ex_flags.boop) {
        for (i32 i = 0; i < 2; i++) {
            V2f32 start_point = rotate_point(player_boop_segments[i][0],
                                             (V2f32) { 0, 0 },
                                             deg_to_rad(player->angle_deg));
            V2f32 end_point = rotate_point(player_boop_segments[i][1],
                                           (V2f32) { 0, 0 },
                                           deg_to_rad(player->angle_deg));

            gfx_render_thick_line(state.renderer,
                                  (i32)start_point.x + player->position.x,
                                  (i32)start_point.y + player->position.y,
                                  (i32)end_point.x + player->position.x,
                                  (i32)end_point.y + player->position.y,
                                  LINE_THICKNESS,
                                  color);
        }
    }
}

void player_render_impl(f32 angle_deg, V2f32 position, SDL_Color color) {
    for (i32 i = 0; i < PLAYER_SEGMENTS_SIZE; i++) {
        V2f32 start_point = rotate_point(player_segments[i][0],
                                         (V2f32) { 0, 0 },
                                         deg_to_rad(angle_deg));
        V2f32 end_point = rotate_point(player_segments[i][1],
                                       (V2f32) { 0, 0 },
                                       deg_to_rad(angle_deg));
        gfx_render_thick_line(state.renderer,
                              (i32)start_point.x + position.x,
                              (i32)start_point.y + position.y,
                              (i32)end_point.x + position.x,
                              (i32)end_point.y + position.y,
                              LINE_THICKNESS,
                              color);
    }
}
