#include <SDL.h>
#include <SDL_image.h>

#ifndef _MSC_VER
#include <getopt.h>
#include <unistd.h>
#endif

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "common.h"
#include "debug.h"
#include "game.h"
#include "netcode.h"
#include "player.h"
#include "settings.h"

State state;

Player remote_player;
Player local_player;

NetworkState network_state;
f32 delta_time;

// TODO:
// events refactor + cleanup
// player burst, shield
// player collison death
// ufo
// menu
// network - host packet types

i32 main(i32 argc, char* argv[]) {
    srand(time(NULL));

#if defined(_WIN32) && defined(DEBUG_ENABLED)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

#endif

    network_state.online_disable = true;
    network_state.is_server = true;
    i32 opt;
    i32 client_port = 0;
#ifndef _MSC_VER
    while ((opt = getopt(argc, argv, "sc:fp:d")) != -1) {
        switch (opt) {
        case 's':
            network_state.online_disable = false;
            network_state.is_server = true;
            break;
        case 'c':
        case 'p':
            network_state.online_disable = false;
            network_state.is_server = false;
            client_port = atoi(optarg);
            break;
        case 'f':
            setvbuf(stdout, NULL, _IONBF, 0);
            break;
        default:
            fprintf(stderr, "Usage: %s [-s]\n", argv[0]);
            return -1;
        }
    }
#else
    if (argc > 1 && !strcmp(argv[1], "client")) {
        if (argc != 3) {
            fprintf(stderr, "not enough args!");
            return -1;
        }
        network_state.online_disable = false;
        network_state.is_server = false;
        sscanf(argv[2], "%d", &client_port);
    } else if (argc > 1 && !strcmp(argv[1], "server")) {
        network_state.online_disable = false;
        network_state.is_server = true;
    }

#endif

    game_init();
    game_load_assets();

#if !defined(WIN32) || defined(_MSC_VER) // MSCV and linux/mac
    if (!network_state.online_disable) {
        if (network_state.is_server == true) {
            ASSERT(server_init() == OK, " ");
        } else {
            ASSERT(client_init(client_port) == OK, " ");
        }
    }

    else
#endif
    {
        remote_player.flags.invisible = 1;
    }

    u32 last_time = SDL_GetTicks(), current_time;
    f32 remote_update_accumulator = 0.0f;
    const f32 remote_update_interval = 1.0f / REMOTE_UPDATE_FREQUENCY;

    if (network_state.is_server) {

        for (i32 i = 0; i < 8; i++) {
            u8 seed = rand_i32(0, 255);

            Event* e = malloc(sizeof(Event));
            e->type = EVENT_TYPE_NEW_ROCK;
            EventRock* rock = &e->event.rock;
            *rock = (EventRock) {
                .rock_size = rand_i32(1, 2),
                .jaggedness = rand_float(0.7f, 0.95f),
                .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
                .position = (V2f32) { .x = rand_float_range(2,
                                     0.0f, WINDOW_WIDTH / 2 - 50.0f,
                                     WINDOW_WIDTH / 2 + 50.0f,
                                     (f32)WINDOW_WIDTH),
                                     .y = rand_float_range(2,
                                     0.0f, WINDOW_HEIGHT / 2 - 50.0f,
                                     WINDOW_HEIGHT / 2 + 50.0f,
                                     (f32)WINDOW_HEIGHT) },
                .initial_velocity = (V2f32) { .x = rand_float(-25.0f, 25.0f),
                                     .y = rand_float(-25.0f, 25.0f) },
                .seed = seed,
                .id = rand() + 1
            };

            register_event_local(e);
            register_event_remote(e);
        }
    }

    SDL_version a;
    SDL_GetVersion(&a);

    while (!state.exit) {
#ifdef DEBUG_ENABLED
        debug_loop_start();
#endif
        current_time = SDL_GetTicks();
        delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        remote_update_accumulator += delta_time;

        game_get_input();
        if (state.exit) {
            continue;
        }

#if !defined(WIN32) || defined(_MSC_VER) // MSCV and linux/mac
        if (!network_state.online_disable) {
            if (remote_update_accumulator >= remote_update_interval) {
                game_update_remote();
                remote_update_accumulator -= remote_update_interval;
            }
        }
#endif

#ifdef DEBUG_ENABLED
        if (!debug_state.stop_logic)
#endif
            game_process();

#ifdef DEBUG_ENABLED
        debug_before_render();
#endif

        game_render();
#ifdef DEBUG_ENABLED
        debug_after_render();
#endif
    }

    game_teardown();

    return 0;
}
