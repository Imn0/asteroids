#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "common.h"
#include "game.h"
#include "settings.h"
#include "netcode.h"
#include "player.h"

State state;

Player remote_player;
Player local_player;

NetworkState network_state;
f32 delta_time;

i32 main(i32 argc, i8* argv[]) {
    i32 fps = 0;
    i32 last_time_fps = 0;
    char buffer[32];
    const u32 frame_delay = 1000 / GAME_FPS_LIMIT;
    bool online_disable = true;
    network_state.is_server = true;
    if (argc > 1 && !strcmp(argv[1], "client")) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s client <port>\n", argv[0]);
            return -1;
        }
        online_disable = false;
        network_state.is_server = false;
    } if (argc > 1 && !strcmp(argv[1], "multi")) {
        online_disable = false;
    }


    packet_queue_init(&network_state.receive.rx_queue, 64);
    packet_queue_init(&network_state.transmit.tx_queue, 64);

    game_init();
    game_load_imgs();
    if (!online_disable) {

        if (network_state.is_server == true) {
            server_init();
        }
        else {
            i32 client_port = 0;
            client_port = atoi(argv[2]);
            client_init(client_port);
        }
    }

    u32 last_time = SDL_GetTicks(), current_time;
    f32 remote_update_accumulator = 0.0f;
    const f32 remote_update_interval = 1.0f / REMOTE_UPDATE_FREQUENCY;
    u32 frame_start;

    while (!state.exit) {
        fps++;
        current_time = SDL_GetTicks();
        frame_start = current_time;
        delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        remote_update_accumulator += delta_time;


        game_get_input();
        if (state.exit) { continue; }
        if (!online_disable) {
            if (remote_update_accumulator >= remote_update_interval) {
                game_update_remote();
                remote_update_accumulator -= remote_update_interval;
            }
        }

        game_process();
        game_render();
        u32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay) {
            SDL_Delay(frame_delay - frame_time);
        }
        if (SDL_GetTicks() - last_time_fps > 1000) {
            snprintf(buffer, sizeof(buffer), "[%d]", fps);
            SDL_SetWindowTitle(state.window, buffer);
            fps = 0;
            last_time_fps = SDL_GetTicks();
        }

    }

    game_teardown();

    return 0;
}