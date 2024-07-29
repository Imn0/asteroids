#include <SDL.h>
#include <SDL_image.h>
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

void generate_rocks(i32 rock_num) {
    srand(time(NULL));
    for (i32 i = 0; i < rock_num; i++) {
        /* code */

        u8 seed = rand();

        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_NEW_ROCK;
        EventRock* rock = &e->event.rock;
        *rock = (EventRock){
            .base_radius = rand_float(25.0f, 130.0f),
            .jaggedness = rand_float(0.7f, 0.95f),
            .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
            .position = (V2f32){.x = rand_float(0.0f, WINDOW_WIDTH),
                                .y = rand_float(0.0f, WINDOW_HEIGHT)},
            .initial_velocity = (V2f32){.x = rand_float(5.0f, 45.0f),
                                        .y = rand_float(5.0f, 45.0f)},
            .seed = seed };

        queue_enqueue(&state.event_queue, e);

        if (!network_state.online_disable) {
            Packet* packet = packet_from_event(e);
            queue_enqueue(&network_state.transmit.tx_queue, packet);
        }
    }
}

void game_init() {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS),
           "Failed to init SDL %s\n", SDL_GetError());

    char window_name[256];
    snprintf(window_name, 256, "Asteroids %s",
             network_state.is_server ? "Server" : "Client");

    state.window = SDL_CreateWindow(
        window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ASSERT(state.window, "Failed to init window %s\n", SDL_GetError());

    state.renderer = SDL_CreateRenderer(state.window, -1,
                                        SDL_RENDERER_ACCELERATED
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

    TTF_Init();
    state.font = TTF_OpenFont(font_path, 24);
    state.small_font = TTF_OpenFont(font_path, 12);
    ASSERT((state.font != NULL), "failed to load font");
    state.textures.size = 0;
}

void game_get_input() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            state.exit = true;
            break;
        case SDL_KEYDOWN:
            switch (ev.key.keysym.sym) {
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
    player_process_input(&local_player);
}

void game_process() {

    if (network_state.is_server) {
        state.new_rock_timer -= delta_time;
        if (state.new_rock_timer < 0) {
            state.new_rock_timer = 3.0f;
            generate_rocks(1);
        }
    }

    player_update(&local_player);
    player_update(&remote_player);

    Event* e = NULL;
    funct_ret_t ret;
    while (queue_dequeue(&state.event_queue, (void*)&e) == OK) {
        switch (e->type) {
        case EVENT_TYPE_SHOOT:
        {
            Entity* bullet = entity_create_bullet(
                e->event.shoot.position, e->event.shoot.initial_velocity,
                e->event.shoot.angle_deg);
            ret = ll_push_back(&state.entities, bullet);
            break;
        }
        case EVENT_TYPE_NEW_ROCK:
        {
            Entity* rock = entity_create_rock((struct entity_create_rock_args) {
                .base_radius = e->event.rock.base_radius,
                    .jaggedness = e->event.rock.jaggedness,
                    .num_vertices = e->event.rock.num_vertices,
                    .position = e->event.rock.position,
                    .velocity = e->event.rock.initial_velocity,
                    .seed = e->event.rock.seed
            });
            ret = ll_push_front(&state.entities, rock);
            break;
        }
        }
        ASSERT(ret == 0, "game process : %d error\n", ret);
        free(e);
    }

    // collisions
    LinkedListIter bullet_iter;
    LinkedListIter rock_iter;
    Entity* entity_rock = NULL;
    Entity* entity_bullet = NULL;
    ll_iter_assign_direction(&bullet_iter, &state.entities, LL_ITER_T_TO_H);
    while (!ll_iter_end(&bullet_iter)) {
        entity_bullet = ll_iter_peek(&bullet_iter);
        if (entity_bullet->type != ENTITY_BULLET) {
            break;
        }

        ll_iter_assign_direction(&rock_iter, &state.entities, LL_ITER_H_TO_T);
        while (!ll_iter_end(&rock_iter)) {
            entity_rock = ll_iter_peek(&rock_iter);
            if (entity_rock->type != ENTITY_ROCK) {
                break;
            }

            bool colision = entity_check_collision_line(
                ll_iter_peek(&rock_iter),
                entity_bullet->data.bullet.common.position,
                entity_bullet->data.bullet.last_position);

            if (colision) {
                Animation* a = create_sprinkle_animation(
                    entity_bullet->data.common.position, rand_i32(10, 20),
                    rand_i32(0, 255));
                ll_push_back(&state.animations, a);
                ll_iter_remove_at(&state.entities, &bullet_iter);
                ll_iter_remove_at(&state.entities, &rock_iter);
                break;
            }

            ll_iter_next(&rock_iter);
        }
        ll_iter_next(&bullet_iter);
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
    player_render(&local_player);
    player_render(&remote_player);

    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);

    // Define the rectangle with the center at (cx, cy)
    SDL_Rect rect;
    rect.w = 10;
    rect.h = 10;
    rect.x = (int)local_player.position.x - rect.w / 2;
    rect.y = (int)local_player.position.y - rect.h / 2;

    // Draw the rectangle
    SDL_RenderFillRect(state.renderer, &rect);
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
