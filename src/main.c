#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "common.h"
#include "game.h"
#include "settings.h"
#include "player.h"

State state;
Player player;

#define SPRITE_SIZE 64
#define NUM_SPRITES 10000

typedef struct {
    float x, y;
    float angle;
} Sprite;


f32 delta_time;

i32 main() {
    
    game_init();

    SDL_Surface* surface = IMG_Load("../assets/imgs/arrow.png");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(state.renderer, surface);
    SDL_FreeSurface(surface);

    player.angle_deg = 0;
    player.position = ((V2f) { .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 });
    player.velocity = 0;

    u32 last_time = SDL_GetTicks(), current_time;

    Sprite sprites[NUM_SPRITES];
    for (int i = 0; i < NUM_SPRITES; i++) {
        sprites[i].x = rand() % (WINDOW_WIDTH - SPRITE_SIZE);
        sprites[i].y = rand() % (WINDOW_HEIGHT - SPRITE_SIZE);
        sprites[i].angle = (float)(rand() % 360);
    }

    bool exit = false;
    while (!exit) {
        current_time = SDL_GetTicks();
        delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                exit = true;
                break;
            }
        }

        SDL_SetRenderTarget(state.renderer, state.render_target);
        SDL_RenderClear(state.renderer);

        SDL_Rect dstRect = { (int)player.position.x, (int)player.position.y, SPRITE_SIZE, SPRITE_SIZE };
        SDL_RenderCopyEx(state.renderer, texture, NULL, &dstRect, player.angle_deg, NULL, SDL_FLIP_NONE);


        for (int i = 0; i < NUM_SPRITES; i++) {
            sprites[i].angle += ROTATION_SPEED * delta_time;
            if (sprites[i].angle >= 360.0f) {
                sprites[i].angle -= 360.0f;
            }

            SDL_Rect dstRect = { (int)sprites[i].x, (int)sprites[i].y, SPRITE_SIZE, SPRITE_SIZE };
            SDL_RenderCopyEx(state.renderer, texture, NULL, &dstRect, sprites[i].angle, NULL, SDL_FLIP_NONE);
        }

        SDL_SetRenderTarget(state.renderer, NULL);
        SDL_RenderClear(state.renderer);
        int window_width, window_height;
        SDL_GetWindowSize(state.window, &window_width, &window_height);

        // Calculate scaling factor and destination rectangle
        float aspect_ratio = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
        int scaled_width = window_width;
        int scaled_height = window_width / aspect_ratio;
        if (scaled_height > window_height) {
            scaled_height = window_height;
            scaled_width = window_height * aspect_ratio;
        }
        SDL_Rect screen_rect = { (window_width - scaled_width) / 2, (window_height - scaled_height) / 2, scaled_width, scaled_height };

        // Render the texture to the screen
        SDL_RenderCopy(state.renderer, state.render_target, NULL, &screen_rect);
        SDL_RenderPresent(state.renderer);
    }

    return 0;
}