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
#include "game.h"
#include "netcode.h"
#include "player.h"
#include "settings.h"
#include "sfx.h"

void game_init() {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_TIMER),
           "Failed to init SDL %s\n", SDL_GetError());

    char window_name[256];
    snprintf(window_name, 256, "Asteroids %s",
             network_state.is_server ? "Server" : "Client");

    state.window = SDL_CreateWindow(
        window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ASSERT(state.window, "Failed to init window %s\n", SDL_GetError());

    state.renderer = SDL_CreateRenderer(state.window, -1,
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
            case SDLK_F1:
            {
                Animation* a = create_player_death_animation(
                    local_player.position, local_player.velocity,
                    local_player.angle_deg);
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

void game_process() {

    if (network_state.is_server) {
        state.new_rock_timer -= delta_time;
        if (state.new_rock_timer < 0) {

            // TODO figure out the timer 
            state.new_rock_timer = 10.0f;
            add_event_random_rock(ROCK_BIG);

        }
    }
    // TODO UFO

    player_update(&local_player, false);
    player_update(&remote_player, true);

    Event* e = NULL;
    funct_ret_t ret;
    while (queue_dequeue(&state.event_queue, (void*)&e) == OK) {
        switch (e->type) {
        case EVENT_TYPE_SHOOT:
        {
            Entity* bullet = create_bullet_from_event(e);
            ret = ll_push_back(&state.entities, bullet);
            break;
        }
        case EVENT_TYPE_NEW_ROCK:
        {
            Entity* rock = create_rock_from_event(e);
            ret = ll_push_front(&state.entities, rock);
            break;
        }
        case EVENT_PLAYER_DEATH:
        {
            Animation* a = create_player_death_animation(
                e->event.player_death.position, e->event.player_death.velocity,
                e->event.player_death.angle_deg);

            ll_push_back(&state.animations, a);

            break;
        }
        case EVENT_REMOVE_ENTITY:
        {
            ASSERT(!network_state.is_server, "server was comamnded to remvoe entity");

            LinkedListIter iter;
            ll_iter_assign(&iter, &state.entities);
            while (!ll_iter_end(&iter)) {
                Entity* entity = ll_iter_peek(&iter);
                if (entity->data.common.id == e->event.remove_entity.id_to_remove) {
                    entity->data.common.flags.remove = 1;

                    if (e->event.remove_entity.animation == SPARKLE) {
                        Animation* a = create_sprinkle_animation(
                            entity->data.common.position, rand_i32(10, 20),
                            rand_i32(0, 255));
                    ll_push_back(&state.animations, a);

                    }

                }
                ll_iter_next(&iter);
            }
            break;
        }
        case EVENT_PLAYER_ADD_POINTS:
        {
            ASSERT(!network_state.is_server, "server was comamnded to add points to itself");
            local_player.score += e->event.add_points.points_to_add;
            break;
        }

        }
        ASSERT(ret == 0, "game process : %d error\n", ret);
        free(e);
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

    // ? handle player deaths locally ?
    // all collisions, checked only on server 
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
                    Animation* a = create_sprinkle_animation(
                        entity_bullet->data.common.position, rand_i32(10, 20),
                        rand_i32(0, 255));

                    if (network_state.is_server) {
                        add_event_from_rock_kill(
                            entity->data.rock.rock_size, entity->data.rock.common.position, entity->data.rock.common.velocity);
                    }
                    play_sound(sfx_rock[entity->data.rock.rock_size]);
                    ll_push_back(&state.animations, a);

                    if (entity_bullet->data.bullet.bullet_origin == BULLET_LOCAL) {
                        player_add_score_rock_kill(&local_player, entity->data.rock.rock_size);
                    }

                    if (entity_bullet->data.bullet.bullet_origin == BULLET_REMOTE) {
                        i32 points_to_add = player_add_score_rock_kill(&remote_player, entity->data.rock.rock_size);
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


    // TODO handle this locally 
    // player collision
    LinkedListIter list_iter;
    ll_iter_assign_direction(&list_iter, &state.entities, LL_ITER_H_TO_T);
    bool local_player_colision = false;
    bool remote_player_colision = false;
    while (!ll_iter_end(&list_iter)) {
        Entity* e = ll_iter_peek(&list_iter);


        // player - rock 
        if (e->type == ENTITY_ROCK) {

            // TODO actual collision not the mid point
            local_player_colision |= entity_check_collision_point(e, local_player.position);
            remote_player_colision |= entity_check_collision_point(e, remote_player.position);
        }

        // player - ufo bullet 


        // player - ufo (ramming)



        ll_iter_next(&list_iter);

    }


    if (local_player_colision) {
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_PLAYER_DEATH;
        e->event.player_death.angle_deg = local_player.angle_deg;
        e->event.player_death.position = local_player.position;
        e->event.player_death.velocity = local_player.velocity;

        register_event_local(e);
        register_event_remote(e);
    }


    if (remote_player_colision) {
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_PLAYER_DEATH;
        e->event.player_death.angle_deg =remote_player.angle_deg;
        e->event.player_death.position =remote_player.position;
        e->event.player_death.velocity =remote_player.velocity;

        register_event_local(e);
        register_event_remote(e);
    }
}

void game_update_remote() {
    mtx_lock(&network_state.remote_player_state.mutex);

    remote_player.angle_deg =
        network_state.remote_player_state.player_state.angle;
    remote_player.position.x = network_state.remote_player_state.player_state.x;
    remote_player.position.y = network_state.remote_player_state.player_state.y;
    remote_player.velocity.x =
        network_state.remote_player_state.player_state.v_x;
    remote_player.velocity.y =
        network_state.remote_player_state.player_state.v_y;
    remote_player.flags = network_state.remote_player_state.player_state.flags;
    remote_player.score = network_state.remote_player_state.player_state.score;
    remote_player.lives = network_state.remote_player_state.player_state.lives;

    mtx_unlock(&network_state.remote_player_state.mutex);


    queue_enqueue(&network_state.transmit.tx_queue, packet_from_player(&local_player));
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
    player_render(&local_player, (SDL_Color) { .r = 255, .g = 255, .b = 255, .a = 255 });
    player_render(&remote_player, (SDL_Color) { .r = 128, .g = 255, .b = 128, .a = 255 });


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
