#pragma once
#include <SDL2/SDL.h>

#include "common.h"

/* init code */

i32 game_init();
i32 game_load_imgs();

/* main loop code */

i32 game_get_input();
i32 game_process();
i32 game_update_remote();
i32 game_render();

i32 game_teardown();