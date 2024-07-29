#include <SDL2/SDL_timer.h>

#include "debug.h"
#include "entity.h"
#include "game.h"

DebugState debug_state = { 0 };

static u64 loop_start;
static u64 render_start;

#define TEXT_MARGIN 10
#define GRAPH_WIDTH 350
#define GRAPH_HEIGHT 70
#define GRAPH_POINTS 1000
#define GRAPH_MARGIN 50
#define SCALE_STEPS 5
#define LABEL_MARGIN 30
#define INFO_AREA_HEIGHT 100

void draw_graph(SDL_Surface* surface, f64* times, Uint32 color, int graph_index,
                const char* label, int current_index);
void debug_loop_start() { loop_start = SDL_GetPerformanceCounter(); }

void debug_before_render() {
    render_start = SDL_GetPerformanceCounter();
    debug_state.logic_time = (render_start - loop_start) /
        (f64)SDL_GetPerformanceFrequency() * 1000.0;
}

void debug_after_render() {
    u64 now = SDL_GetPerformanceCounter();
    debug_state.render_time =
        (now - render_start) / (f64)SDL_GetPerformanceFrequency() * 1000.0;
    debug_state.frame_time =
        (now - loop_start) / (f64)SDL_GetPerformanceFrequency() * 1000.0;
}

void draw_info(SDL_Surface* surface, int y_offset) {
    SDL_Color text_color = { 255, 255, 255, 255 };
    i32 line_height = 20;
    i32 x_margin = 10;

    char fps_text[50];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.2f",
             1000.0 / debug_state.frame_time);
    SDL_Surface* fps_surface =
        TTF_RenderText_Solid(state.font, fps_text, text_color);
    if (fps_surface) {
        SDL_Rect fps_rect = { x_margin, y_offset, fps_surface->w,
                             fps_surface->h };
        SDL_BlitSurface(fps_surface, NULL, surface, &fps_rect);
        SDL_FreeSurface(fps_surface);
    }

    // char other_info[50];
    // snprintf(other_info, sizeof(other_info), "Other Info: %d", some_value);
    // SDL_Surface *other_surface = TTF_RenderText_Solid(state.font, other_info,
    // text_color); if (other_surface) {
    //     SDL_Rect other_rect = {x_margin, y_offset + line_height,
    //     other_surface->w, other_surface->h}; SDL_BlitSurface(other_surface,
    //     NULL, surface, &other_rect); SDL_FreeSurface(other_surface);
    // }
}

void draw_mouse_colisions() {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    LinkedListIter rock_iter;
    Entity* entity_rock = NULL;

    SDL_SetRenderDrawColor(state.renderer, 0, 255, 0, 255);
    ll_iter_assign_direction(&rock_iter, &state.entities, LL_ITER_H_TO_T);

    while (!ll_iter_end(&rock_iter)) {
        entity_rock = ll_iter_peek(&rock_iter);
        if (entity_rock->type != ENTITY_ROCK) {
            break;
        }

        // V2f32 mouse_point =
        //     rotate_point((V2f32){.x = mouseX, .y = mouseY}, (V2f32){0},
        //                  -deg_to_rad(entity_rock->data.rock.angle_deg));
        if (entity_check_collision_point(entity_rock, (V2f32) { .x = mouseX, .y = mouseY })) {
            SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
            break;
        }
        ll_iter_next(&rock_iter);
    }

    SDL_Rect cursorRect = { mouseX - 5, mouseY - 5, 4, 4 };
    SDL_RenderFillRect(state.renderer, &cursorRect);
}

void debug_render() {
    if (!debug_state.show_info) return;

    static f64 frame_times[GRAPH_POINTS] = { 0 };
    static f64 logic_times[GRAPH_POINTS] = { 0 };
    static f64 render_times[GRAPH_POINTS] = { 0 };
    static i32 current_index = 0;

    // update arrays
    frame_times[current_index] = debug_state.frame_time;
    logic_times[current_index] = debug_state.logic_time;
    render_times[current_index] = debug_state.render_time;
    current_index = (current_index + 1) % GRAPH_POINTS;

    i32 total_height =
        (GRAPH_HEIGHT + GRAPH_MARGIN) * 3 + LABEL_MARGIN + INFO_AREA_HEIGHT;
    SDL_Surface* graph_surface =
        SDL_CreateRGBSurface(0, GRAPH_WIDTH, total_height, 32, 0xFF000000,
                             0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!graph_surface) {
        printf("Unable to create graph surface: %s\n", SDL_GetError());
        return;
    }

    // background
    SDL_FillRect(graph_surface, NULL,
                 SDL_MapRGB(graph_surface->format, 40, 40, 40));

    // Define colors
    Uint32 frame_color = SDL_MapRGB(graph_surface->format, 0, 255, 0);  // Green
    Uint32 logic_color = SDL_MapRGB(graph_surface->format, 255, 0, 0);  // Red
    Uint32 render_color = SDL_MapRGB(graph_surface->format, 0, 0, 255); // Blue

    // graphs
    draw_graph(graph_surface, frame_times, frame_color, 0, "Frame Time (ms)",
               current_index);
    draw_graph(graph_surface, logic_times, logic_color, 1, "Logic Time (ms)",
               current_index);
    draw_graph(graph_surface, render_times, render_color, 2, "Render Time (ms)",
               current_index);

    // additional information
    draw_info(graph_surface, (GRAPH_HEIGHT + GRAPH_MARGIN) * 3 + LABEL_MARGIN);

    SDL_Texture* graph_texture =
        SDL_CreateTextureFromSurface(state.renderer, graph_surface);
    if (!graph_texture) {
        printf("Unable to create texture from surface: %s\n", SDL_GetError());
        SDL_FreeSurface(graph_surface);
        return;
    }

    draw_mouse_colisions();

    SDL_Rect render_quad = { 0, 0, GRAPH_WIDTH, total_height };
    SDL_RenderCopy(state.renderer, graph_texture, NULL, &render_quad);

    SDL_DestroyTexture(graph_texture);
    SDL_FreeSurface(graph_surface);
}

void draw_graph(SDL_Surface* surface, f64* times, Uint32 color, int graph_index,
                const char* label, int current_index) {
    int y_offset = graph_index * (GRAPH_HEIGHT + GRAPH_MARGIN) + LABEL_MARGIN;
    f64 max_time = 33.33; // Assume 30 FPS as maximum

    // label
    SDL_Surface* label_surface = TTF_RenderText_Solid(
        state.small_font, label, (SDL_Color) { 255, 255, 255, 255 });
    if (label_surface) {
        SDL_Rect label_rect = { 0, y_offset - LABEL_MARGIN, label_surface->w,
                               label_surface->h };
        SDL_BlitSurface(label_surface, NULL, surface, &label_rect);
        SDL_FreeSurface(label_surface);
    }

    // scale
    for (int i = 0; i <= SCALE_STEPS; i++) {
        int y = y_offset + GRAPH_HEIGHT - (i * GRAPH_HEIGHT / SCALE_STEPS);
        SDL_Rect line_rect = { 0, y, GRAPH_WIDTH, 1 };
        SDL_FillRect(surface, &line_rect,
                     SDL_MapRGB(surface->format, 100, 100, 100));

        char scale_text[10];
        snprintf(scale_text, sizeof(scale_text), "%.1f",
                 i * max_time / SCALE_STEPS);
        SDL_Surface* text_surface = TTF_RenderText_Solid(
            state.small_font, scale_text, (SDL_Color) { 255, 255, 255, 255 });
        if (text_surface) {
            SDL_Rect text_rect = { 0, y - text_surface->h / 2, text_surface->w,
                                  text_surface->h };
            SDL_BlitSurface(text_surface, NULL, surface, &text_rect);
            SDL_FreeSurface(text_surface);
        }
    }

    // graph
    for (int i = 0; i < GRAPH_POINTS - 1; i++) {
        int x1 = i * GRAPH_WIDTH / GRAPH_POINTS;
        int x2 = (i + 1) * GRAPH_WIDTH / GRAPH_POINTS;
        int y1 = y_offset + GRAPH_HEIGHT -
            (int)(times[(current_index + i) % GRAPH_POINTS] / max_time *
                  GRAPH_HEIGHT);
        int y2 = y_offset + GRAPH_HEIGHT -
            (int)(times[(current_index + i + 1) % GRAPH_POINTS] /
                  max_time * GRAPH_HEIGHT);

        SDL_Rect line_rect = { x1, SDL_min(y1, y2), x2 - x1 + 1,
                              abs(y2 - y1) + 1 };
        SDL_FillRect(surface, &line_rect, color);
    }
}
