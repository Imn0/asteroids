#pragma once
#include <SDL.h>
#include <SDL_ttf.h>

#include "common.h"

typedef struct {
    // SDL
    SDL_Window* window;
    SDL_Renderer* renderer;
    struct {
        SDL_Texture* arr[TEXTURES_CAPACITY];
        i32 size;
    } textures;
    TTF_Font* font;
    TTF_Font* small_font;
    const u8* keystate;

    // game stuff
    bool exit;
    Queue event_queue;
    LinkedList entities;
    LinkedList animations;
    f32 new_rock_timer;
} State;

extern f32 delta_time;
extern State state;

/* init code */

void game_init();
void game_load_assets();

/* main loop code */

void game_get_input();
void game_process();
void game_update_remote();
void game_render();

void game_teardown();
