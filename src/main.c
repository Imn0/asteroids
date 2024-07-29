#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

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
// network - host packet types
// player collison death
// points
// rock spawning
// menu

i32 main(i32 argc, char* argv[]) {
    network_state.online_disable = true;
    network_state.is_server = true;

    int opt;
    i32 client_port = 0;
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

    game_init();
    game_load_assets();
    if (!network_state.online_disable) {
        if (network_state.is_server == true) {
            ASSERT(server_init() == OK, " ");
        }
        else {
            ASSERT(client_init(client_port) == OK, " ");
        }
    }
    else {
        remote_player.flags.invisible = 1;
    }

    u32 last_time = SDL_GetTicks(), current_time;
    f32 remote_update_accumulator = 0.0f;
    const f32 remote_update_interval = 1.0f / REMOTE_UPDATE_FREQUENCY;

    generate_rocks(10);

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
        if (!network_state.online_disable) {
            if (remote_update_accumulator >= remote_update_interval) {
                game_update_remote();
                remote_update_accumulator -= remote_update_interval;
            }
        }

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
