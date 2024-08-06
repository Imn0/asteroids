#include "game.h"
#include "ufo.h"


void ufo_update(Ufo* ufo) {
    ufo->ufo_timer += delta_time;
    if (ufo->form == UFO_NONE) {
        return;
    }
    ufo->position.x += ufo->velocity.x * delta_time;
    ufo->position.y += ufo->velocity.y * delta_time;

    if (ufo->position.x < 0)
        ufo->position.x = WINDOW_WIDTH;
    else if (ufo->position.x > WINDOW_WIDTH)
        ufo->position.x = 0;
    if (ufo->position.y < 0)
        ufo->position.y = WINDOW_HEIGHT;
    else if (ufo->position.y > WINDOW_HEIGHT) {
        ufo->position.y = 0;
    }

}

void ufo_render(Ufo* ufo) {
    if (ufo->form == UFO_NONE) {
        return;
    }

    SDL_Rect r = { .h = 20, .w = 20, .x = ufo->position.x , .y = ufo->position.y };
    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(state.renderer, &r);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);

}
