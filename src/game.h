#pragma once
#include <SDL2/SDL.h>

#include "common.h"

typedef struct __state {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* render_target;
}State;

i32 game_init();
i32 game_load_imgs();
i32 game_get_input();
i32 game_process();
i32 game_render();