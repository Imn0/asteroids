#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <time.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "animation.h"
#include "common.h"
#include "debug.h"
#include "entity.h"
#include "events.h"
#include "game.h"
#include "netcode.h"
#include "player.h"
#include "settings.h"
#include "sfx.h"
#include "ufo.h"

Ufo game_ufo;
void add_event_from_rock_kill(RockSize killed_rock_size, V2f32 killed_pos, V2f32 killed_velocity);

void game_init() {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_TIMER),
           "Failed to init SDL %s\n",
           SDL_GetError());

    char window_name[256];
    snprintf(window_name, 256, "Asteroids %s", network_state.is_server ? "Server" : "Client");

    state.window = SDL_CreateWindow(window_name,
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    WINDOW_WIDTH,
                                    WINDOW_HEIGHT,
                                    SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ASSERT(state.window, "Failed to init window %s\n", SDL_GetError());

    state.renderer = SDL_CreateRenderer(state.window,
                                        -1,
#ifdef SOFTWARE_RENDERING
                                        SDL_RENDERER_SOFTWARE
#else
                                        SDL_RENDERER_ACCELERATED
#endif
#ifdef VSYNC_ENABLED
                                                | SDL_RENDERER_PRESENTVSYNC
#endif

    );
    ASSERT(state.renderer, "Failed to init renderer %s\n", SDL_GetError());

    SDL_RenderSetLogicalSize(state.renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    player_init(&local_player);
    player_init(&remote_player);

    ll_init(&state.entities);
    ll_init(&state.animations);
    queue_init(&state.event_queue, 64);

    game_ufo.form = UFO_NONE;
    game_ufo.ufo_timer = 0.0f;

    state.new_rock_timer = 3.0f;
}

void game_load_assets() {
    char* exe_path = SDL_GetBasePath();
    char font_path[1024];
    snprintf(font_path, sizeof(font_path), "%s/../assets/fonts/Montserrat-Regular.ttf", exe_path);

    ASSERT(TTF_Init() == 0, "TTF failed to initialize");
    state.font = TTF_OpenFont(font_path, 32);
    state.big_font = TTF_OpenFont(font_path, 56);
    state.small_font = TTF_OpenFont(font_path, 12);
    ASSERT((state.font != NULL), "failed to load font");
    state.textures.size = 0;

    init_sound();
}

void game_get_input() {
    SDL_Event ev;
    bool shoot_key_down = false;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            state.exit = true;
            break;
        case SDL_KEYDOWN:
            switch (ev.key.keysym.sym) {
            case SDLK_SPACE:
                if (ev.key.repeat == 0)
                    shoot_key_down = true;
                break;
#ifdef DEBUG_ENABLED
            case SDLK_F1: {
                Animation* a = create_player_death_animation(local_player.position,
                                                             local_player.velocity,
                                                             local_player.angle_deg,
                                                             (SDL_Color) { LOCAL_PLAYER_COLOR,
                                                                           255 });
                local_player.flags.invisible = 1;
                ll_push_back(&state.animations, a);
                break;
            }
            case SDLK_F2:
                local_player.flags.invisible = 0;
                break;
            case SDLK_F3:
                debug_state.show_info = !debug_state.show_info;
                break;
            case SDLK_F4:
                debug_state.stop_logic = !debug_state.stop_logic;
                break;
#endif
            default:
                break;
            }

            break;
        }
    }
    state.keystate = SDL_GetKeyboardState(NULL);
    player_process_input(&local_player, shoot_key_down);
}

bool check_game_over() {
    bool remote_ok = true;
    if (!network_state.online_disable) {
        if (remote_player.flags.perma_dead) {
            remote_ok = false;
        }
    }

    bool local_ok = true;
    if (local_player.flags.perma_dead) {
        local_ok = false;
    }

    return local_ok || remote_ok;
}

void game_server_process() {
    if (check_game_over()) {
        // TODO THIS
    }

    // ROCKS
    state.new_rock_timer -= delta_time;
    if (state.new_rock_timer < 0) {
        // TODO figure out the timer
        state.new_rock_timer = 7.0f;

        add_event_new_rock(
                ROCK_BIG,
                get_random_screen_edge_position_away_from(150.0f,
                                                          local_player.position,
                                                          network_state.online_disable
                                                                  ? local_player.position
                                                                  : remote_player.position),
                (V2f32) { .x = rand_float(-25.0f, 25.0f), .y = rand_float(-25.0f, 25.0f) });
    }
    // UFO
    u32 max_score = local_player.score;
    if (!network_state.online_disable) {
        if (remote_player.score > max_score) {
            max_score = remote_player.score;
        }
    }

    UfoForm ufo_to_add = UFO_NONE;

    if (max_score < 10000) {
        if (game_ufo.form == UFO_NONE && game_ufo.ufo_timer > 1.0f) {
            ufo_to_add = rand_float(0.0f, 1.0f) < 0.5f ? UFO_SMALL : UFO_BIG;
        }
    } else if (max_score < 25000) {
        if (game_ufo.form == UFO_NONE && game_ufo.ufo_timer > 2.0f) {
            ufo_to_add = rand_float(0.0f, 1.0f) < 0.75f ? UFO_BIG : UFO_SMALL;
        }
    } else {
        if (game_ufo.form == UFO_NONE && game_ufo.ufo_timer > 2.0f) {
            ufo_to_add = UFO_SMALL;
        }
    }
    if (ufo_to_add != UFO_NONE) {
        add_event_ufo_spawn(ufo_to_add,
                            get_random_screen_edge_position_away_from(
                                    100.0f,
                                    local_player.position,
                                    network_state.online_disable ? local_player.position
                                                                 : remote_player.position),
                            rand_float(50.0f, 75.0f) * (ufo_to_add == UFO_SMALL ? 2.78 : 1),
                            rand_float(0, 360));
    }
}

void game_process() {
    if (network_state.is_server) {
        game_server_process();
    }

    player_update(&local_player, false);
    ufo_update(&game_ufo);

    if (!network_state.online_disable) {
        player_update(&remote_player, true);
    }

    Event* e = NULL;
    funct_ret_t ret;
    while (queue_dequeue(&state.event_queue, (void*)&e) == OK) {
        ret = handle_event(e);
        ASSERT(ret == 0, "game process : %d error\n", ret);
    }

    // physics
    LinkedListIter iter;
    ll_iter_assign(&iter, &state.entities);
    while (!ll_iter_end(&iter)) {
        Entity* entity = ll_iter_peek(&iter);
        entity_update(entity);
        if (entity->data.common.flags.remove == 1) {
            ll_iter_remove_at(&state.entities, &iter);
        }
        ll_iter_next(&iter);
    }

    ll_iter_assign(&iter, &state.animations);
    while (!ll_iter_end(&iter)) {
        Animation* animation = ll_iter_peek(&iter);
        animation_update(animation);
        if (animation->animation.common.ttl < 0.0f) {
            ll_iter_remove_at(&state.animations, &iter);
        }
        ll_iter_next(&iter);
    }

    // Player colision
    LinkedListIter list_iter;
    ll_iter_assign_direction(&list_iter, &state.entities, LL_ITER_H_TO_T);
    bool local_player_colision = false;
    if (local_player.ex_flags.invincible == 1) {
        goto player_collision_skip;
    }
    while (!ll_iter_end(&list_iter)) {
        Entity* e = ll_iter_peek(&list_iter);

        // player - rock
        if (e->type == ENTITY_ROCK) {
            // TODO actual collision not the mid point
            local_player_colision = entity_check_collision_point(e, local_player.position);
            if (local_player_colision) {
                debug_log_death(local_player.position, e);
                break;
            }
        }

        // player - ufo bullet
        else if (e->type == ENTITY_BULLET && e->data.bullet.bullet_origin == BULLET_UFO) {
            if (dist(local_player.position, e->data.common.position) < 10.0f) {
                local_player_colision = true;
            }

            if (local_player_colision) {
                debug_log_death(local_player.position, e);
                break;
            }
        }

        ll_iter_next(&list_iter);
    }

    if (local_player_colision) {
        add_event_player_death(local_player.position,
                               local_player.velocity,
                               local_player.angle_deg);
    }

player_collision_skip:

    // all other (non player death) collisions, checked only on server
    if (!network_state.is_server) {
        return;
    }

    // bullet - rock and bulelt - ufo
    LinkedListIter bullet_iter;
    LinkedListIter rock_iter;
    Entity* entity = NULL;
    Entity* entity_bullet = NULL;
    ll_iter_assign_direction(&bullet_iter, &state.entities, LL_ITER_T_TO_H);
    while (!ll_iter_end(&bullet_iter)) {
        entity_bullet = ll_iter_peek(&bullet_iter);
        if (entity_bullet->type != ENTITY_BULLET) {
            break;
        }

        // check ufo - bullet collision
        if (game_ufo.form != UFO_NONE && entity_bullet->data.bullet.bullet_origin != BULLET_UFO) {
            if (dist(game_ufo.position, entity_bullet->data.common.position) < 55.0f) {

                entity_bullet->data.bullet.common.flags.remove = 1;
                remote_kill_entity(entity_bullet->data.common.id, NONE);

                add_event_ufo_kill(game_ufo.position,
                                   game_ufo.velocity,
                                   entity_bullet->data.common.velocity);

                ll_iter_next(&bullet_iter);
                continue;
            }
        }

        ll_iter_assign_direction(&rock_iter, &state.entities, LL_ITER_H_TO_T);
        while (!ll_iter_end(&rock_iter)) {
            entity = ll_iter_peek(&rock_iter);
            if (entity->type == ENTITY_BULLET) {
                break;
            }

            if (entity->type == ENTITY_ROCK) {
                // bullet - rock

                bool colision = entity_check_collision_line(
                        ll_iter_peek(&rock_iter),
                        entity_bullet->data.bullet.common.position,
                        entity_bullet->data.bullet.last_position);

                if (colision) {
                    Animation* a = create_sprinkle_animation(entity_bullet->data.common.position,
                                                             rand_i32(10, 20),
                                                             rand_i32(0, 255));

                    if (network_state.is_server) {
                        add_event_from_rock_kill(entity->data.rock.rock_size,
                                                 entity->data.rock.common.position,
                                                 entity->data.rock.common.velocity);
                    }
                    play_sound(sfx_rock[entity->data.rock.rock_size]);
                    ll_push_back(&state.animations, a);

                    if (entity_bullet->data.bullet.bullet_origin == BULLET_LOCAL) {
                        player_add_score_rock_kill(&local_player, entity->data.rock.rock_size);
                    }

                    if (entity_bullet->data.bullet.bullet_origin == BULLET_REMOTE) {
                        i32 points_to_add = player_add_score_rock_kill(&remote_player,
                                                                       entity->data.rock.rock_size);
                        remote_add_points(points_to_add);
                    }

                    entity_bullet->data.bullet.common.flags.remove = 1;
                    remote_kill_entity(entity_bullet->data.common.id, NONE);

                    entity->data.common.flags.remove = 1;
                    remote_kill_entity(entity->data.common.id, SPARKLE);

                    break;
                }
            }

            ll_iter_next(&rock_iter);
        }
        ll_iter_next(&bullet_iter);
    }

    // ufo - rock
    ll_iter_assign_direction(&rock_iter, &state.entities, LL_ITER_H_TO_T);
    while (!ll_iter_end(&rock_iter)) {
        entity = ll_iter_peek(&rock_iter);
        if (entity->type != ENTITY_ROCK) {
            break;
        }

        // TODO
        if (dist(game_ufo.position, entity->data.common.position) < 55.0f) {

            entity->data.common.flags.remove = 1;
            remote_kill_entity(entity->data.common.id, SPARKLE);

            add_event_ufo_kill(game_ufo.position, game_ufo.velocity, entity->data.common.velocity);
        }
        ll_iter_next(&rock_iter);
    }
}

void game_update_remote() {
    mtx_lock(&network_state.remote_player_state.mutex);
    PlayerPacket* packet = &network_state.remote_player_state.player_state;
    remote_player.angle_deg = packet->angle;
    remote_player.position.x = packet->x;
    remote_player.position.y = packet->y;
    remote_player.velocity.x = packet->v_x;
    remote_player.velocity.y = packet->v_y;
    remote_player.flags = packet->flags;
    remote_player.score = packet->score;
    remote_player.lives = packet->lives;

    mtx_unlock(&network_state.remote_player_state.mutex);
    queue_enqueue(&network_state.transmit.tx_queue, packet_from_player(&local_player));

    if (network_state.is_server) {
        queue_enqueue(&network_state.transmit.tx_queue, packet_from_ufo(&game_ufo));
    } else {
        mtx_lock(&network_state.ufo_state.mutex);
        game_ufo.angle_deg = network_state.ufo_state.ufo.angle;
        game_ufo.form = network_state.ufo_state.ufo.current_form;
        game_ufo.position.x = network_state.ufo_state.ufo.x;
        game_ufo.position.y = network_state.ufo_state.ufo.y;
        game_ufo.velocity.x = network_state.ufo_state.ufo.v_x;
        game_ufo.velocity.y = network_state.ufo_state.ufo.v_y;
        mtx_unlock(&network_state.ufo_state.mutex);
    }
}

void game_render() {
    // setup
    SDL_RenderClear(state.renderer);

    SDL_Rect logicalRect = { 0, 0, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT };
    SDL_SetRenderDrawColor(state.renderer, BACKGROUND_COLOR, 255);
    SDL_RenderFillRect(state.renderer, &logicalRect);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
#ifdef DEBUG_ENABLED
    debug_render();
#endif

    // redner player
    player_render_score(&local_player, false);
    if (!network_state.online_disable) {
        player_render_score(&remote_player, true);
    }
    player_render(&local_player, (SDL_Color) { LOCAL_PLAYER_COLOR, .a = 255 });
    player_render(&remote_player, (SDL_Color) { REMOTE_PLAYER_COLOR, .a = 255 });
    ufo_render(&game_ufo);

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
    LinkedListIter iter;
    ll_iter_assign(&iter, &state.entities);
    while (!ll_iter_end(&iter)) {
        entity_render(ll_iter_peek(&iter));
        ll_iter_next(&iter);
    }

    ll_iter_assign(&iter, &state.animations);
    while (!ll_iter_end(&iter)) {
        animation_render(ll_iter_peek(&iter));
        ll_iter_next(&iter);
    }

    // present
    SDL_RenderPresent(state.renderer);
}

void game_teardown() {
    queue_destroy(&state.event_queue);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

void add_event_from_rock_kill(RockSize killed_rock_size, V2f32 killed_pos, V2f32 killed_velocity) {

    ASSERT(network_state.is_server, "client tried to add a rock kill\n");

    if (killed_rock_size == ROCK_SMALL) {
        return;
    }

    RockSize new_size = ROCK_MEDIUM;
    if (killed_rock_size == ROCK_MEDIUM) {
        new_size = ROCK_SMALL;
    }

    {
        u8 seed = rand_i32(0, 255);
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_NEW_ROCK;
        EventRock* rock = &e->event.rock;
        *rock = (EventRock) {
            .rock_size = new_size,
            .jaggedness = rand_float(0.7f, 0.95f),
            .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
            .position = killed_pos,
            .initial_velocity = (V2f32) { .x = rand_float_range(2, -2.7, -2.3, 2.3, 2.7) *
                                               killed_velocity.y,
                                         .y = rand_float_range(2, -2.7, -2.3, 2.3, 2.7) *
                                               killed_velocity.x },
            .seed = seed,
            .id = rand() + 1
        };

        register_event_local(e);
        register_event_remote(e);
    }
    {
        u8 seed = rand_i32(0, 255);
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_NEW_ROCK;
        EventRock* rock = &e->event.rock;
        *rock = (EventRock) {
            .rock_size = new_size,
            .jaggedness = rand_float(0.7f, 0.95f),
            .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
            .position = killed_pos,
            .initial_velocity = (V2f32) { .x = -rand_float_range(2, -2.7, -2.3, 2.3, 2.7) *
                                               killed_velocity.y,
                                         .y = -rand_float_range(2, -2.7, -2.3, 2.3, 2.7) *
                                               killed_velocity.x },
            .seed = seed,
            .id = rand() + 1
        };

        register_event_local(e);
        register_event_remote(e);
    }
}
