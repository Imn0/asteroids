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
    LinkedList animations;
    f32 new_rock_timer;
}State;

typedef enum {
    EVENT_TYPE_SHOOT,
    EVENT_TYPE_NEW_ROCK,
} EventType;

typedef struct {
    V2f32 position, initial_velocity;
    f32 angle_deg;
} EventShoot;

typedef struct {
    V2f32 position, initial_velocity;
    i32 num_vertices;
    f32 jaggedness;
    f32 base_radius;
    u8 seed;
} EventRock;

typedef struct {
    EventType type;
    union {
        EventShoot shoot;
        EventRock rock;
    } event;
} Event;

extern f32 delta_time;
extern State state;

/* init code */

void game_init();
void game_load_imgs();

/* main loop code */

void game_get_input();
void game_process();
void game_update_remote();
void game_render();

void generate_rocks(i32 rock_num);
void game_teardown();
