#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "game.h"
#include "common.h"
#include "settings.h"
#include "netcode.h"
#include "player.h"

extern Player player;

i32 game_init() {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "Failed to init SDL %s\n", SDL_GetError());

    state.window = SDL_CreateWindow(
        "Asteroids",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
    ASSERT(state.window, "Failed to init window %s\n", SDL_GetError());

    state.renderer = SDL_CreateRenderer(
        state.window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    ASSERT(state.renderer, "Failed to init renderer %s\n", SDL_GetError());

    state.render_target = SDL_CreateTexture(
        state.renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH,
        WINDOW_HEIGHT);
    ASSERT(state.render_target, "Failed to create render target %s\n", SDL_GetError());

    player.angle_deg = 0;
    player.position = ((V2f) { .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 });
    player.speed = 0;
    player.rotation_speed = 0;
    state.exit = false;


    return ERR_OK;
}

i32 game_load_imgs() {
    state.textures.size = 0;
    SDL_Surface* surface = IMG_Load("../assets/imgs/arrow.png");
    state.textures.arr[state.textures.size++] = SDL_CreateTextureFromSurface(state.renderer, surface);
    SDL_FreeSurface(surface);

    return ERR_OK;
}

i32 game_get_input() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            state.exit = true;
            break;
        }
    }
    state.keystate = SDL_GetKeyboardState(NULL);
    player_process_input(&player);
}

i32 game_process() {
    player_update(&player);
    player_update(&state.remote_player);
    return ERR_OK;
}

i32 game_update_remote() {
    packet_queue_enqueue(
        &network_state.transmit.tx_queue,
        packet_from_player(&player));

    Packet* p = NULL;
    while (packet_queue_dequeue(&network_state.receive.rx_queue, &p)) { ; }

    if (p == NULL) { return ERR_OK; /* no updates */ }
    PlayerPacket packet = p->payload.player;
    state.remote_player.angle_deg = packet.angle;
    state.remote_player.position.x = packet.x;
    state.remote_player.position.y = packet.y;
    state.remote_player.rotation_speed = packet.rotation_speed;
    state.remote_player.speed = packet.rotation_speed;
    free(p);

    return ERR_OK;
}

i32 game_render() {

    // setup
    SDL_SetRenderTarget(state.renderer, state.render_target);
    SDL_RenderClear(state.renderer);

    // redner player
    SDL_Rect player_dstRect = { (int)player.position.x, (int)player.position.y, SPRITE_SIZE, SPRITE_SIZE };
    SDL_RenderCopyEx(
        state.renderer,
        state.textures.arr[0],
        NULL,
        &player_dstRect,
        player.angle_deg,
        NULL,
        SDL_FLIP_NONE);

    SDL_Rect remote_player_dstRect = { (int)state.remote_player.position.x, (int)state.remote_player.position.y, SPRITE_SIZE, SPRITE_SIZE };
    SDL_RenderCopyEx(
        state.renderer,
        state.textures.arr[0],
        NULL,
        &remote_player_dstRect,
        state.remote_player.angle_deg,
        NULL,
        SDL_FLIP_NONE);


    // present
    SDL_SetRenderTarget(state.renderer, NULL);
    SDL_RenderClear(state.renderer);
    int window_width, window_height;
    SDL_GetWindowSize(state.window, &window_width, &window_height);

#ifdef WINDOW_SCALING
    // Calculate scaling factor and destination rectangle
    float aspect_ratio = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    int scaled_width = window_width;
    int scaled_height = window_width / aspect_ratio;
    if (scaled_height > window_height) {
        scaled_height = window_height;
        scaled_width = window_height * aspect_ratio;
    }
    SDL_Rect screen_rect = {
        .x = (window_width - scaled_width) / 2,
        .y = (window_height - scaled_height) / 2,
        .w = scaled_width,
        .h = scaled_height };

    // Render the texture to the screen
    SDL_RenderCopy(state.renderer, state.render_target, NULL, &screen_rect);
#else
    SDL_RenderCopy(state.renderer, state.render_target, NULL, NULL);
#endif
    SDL_RenderPresent(state.renderer);
    return 0;
}

i32 game_teardown() {
    //TODO
}