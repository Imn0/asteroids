#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "game.h"
#include "common.h"
#include "settings.h"

extern State state;

i32 game_init() {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO), "Failed to init SDL %s\n", SDL_GetError());

    state.window = SDL_CreateWindow(
        "Asteroids",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_VULKAN);
    ASSERT(state.window, "Failed to init window %s\n", SDL_GetError());

    state.renderer = SDL_CreateRenderer(
        state.window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    ASSERT(state.renderer, "Failed to init renderer %s\n", SDL_GetError());

    state.render_target = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
    ASSERT(state.render_target, "Failed to create render target %s\n", SDL_GetError());

    return ERR_OK;
}

