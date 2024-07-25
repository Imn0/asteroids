#pragma once
#include <SDL2/SDL.h>

#include "common.h"

typedef struct {
    // SDL 
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* render_target;
    struct { SDL_Texture* arr[TEXTURES_CAPACITY]; i32 size; } textures;
    const u8* keystate;

    // game stuff
    bool exit;
    Queue event_queue;
    LinkedList entities;
}State;

typedef enum {
    EVENT_TYPE_SHOOT,
} EventType;

typedef struct {
    V2f32 position, initial_velocity;
    f32 angle_deg;
} EventShoot;

typedef struct {
    EventType type;
    union {
        EventShoot shoot;
    } event;
} Event;

extern f32 delta_time;
extern State state;

/* init code */

i32 game_init();
i32 game_load_imgs();

/* main loop code */

i32 game_get_input();
i32 game_process();
i32 game_update_remote();
i32 game_render();

i32 game_teardown();