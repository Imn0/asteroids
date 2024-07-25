#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "game.h"
#include "common.h"
#include "settings.h"
#include "netcode.h"
#include "player.h"
#include "entity.h"

i32 game_init() {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS), "Failed to init SDL %s\n", SDL_GetError());

    char window_name[256];
    snprintf(window_name, 256, "Asteroids %s", network_state.is_server ? "Server" : "Client");

    state.window = SDL_CreateWindow(
        window_name,
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

    player_init(&local_player);
    player_init(&remote_player);

    ll_init(&state.entities);
    queue_init(&state.event_queue, 64);


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
    player_process_input(&local_player);

    return ERR_OK;
}

i32 game_process() {
    player_update(&local_player);
    player_update(&remote_player);

    Event* e = NULL;
    while (queue_dequeue(&state.event_queue, (void*)&e)) {
        switch (e->type) {
        case EVENT_TYPE_SHOOT:
            Entity* bullet = malloc(sizeof(Entity));
            entity_create_bullet(e->event.shoot.position, e->event.shoot.initial_velocity, e->event.shoot.angle_deg, bullet);
            ll_push(&state.entities, bullet);
            break;
        }
        free(e);
    }

    LinkedListIter iter;
    ll_iter_assign(&iter, &state.entities);
    funct_ret_t ret;
    while (!ll_iter_end(&iter)) {
        ret = entity_update(ll_iter_peek(&iter));
        if (ret == RET_ENTITY_REMOVE) {
            ll_iter_remove_at(&state.entities, &iter);
        }
        ll_iter_next(&iter);
    }

    return ERR_OK;
}

i32 game_update_remote() {
    queue_enqueue(
        &network_state.transmit.tx_queue,
        packet_from_player(&local_player));

    Packet* p = NULL;
    while (queue_dequeue(&network_state.receive.rx_queue, (void*)&p)) { ; }

    if (p == NULL) { return ERR_OK; /* no updates */ }
    PlayerPacket packet = p->payload.player;
    remote_player.angle_deg = packet.angle;
    remote_player.position.x = packet.x;
    remote_player.position.y = packet.y;
    remote_player.velocity.x = packet.v_x;
    remote_player.velocity.y = packet.v_y;
    memcpy(&remote_player.flags, &packet.flags, sizeof(player_flags_t));
    free(p);


    return ERR_OK;
}

i32 game_render() {

    // setup
    SDL_SetRenderTarget(state.renderer, state.render_target);
    SDL_RenderClear(state.renderer);

    // redner player
    player_render(&local_player);
    player_render(&remote_player);

    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);

    // Define the rectangle with the center at (cx, cy)
    SDL_Rect rect;
    rect.w = 10;
    rect.h = 10;
    rect.x = (int)local_player.position.x - rect.w / 2;
    rect.y = (int)local_player.position.y - rect.h / 2;

    // Draw the rectangle
    SDL_RenderFillRect(state.renderer, &rect);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);

    LinkedListIter iter;
    ll_iter_assign(&iter, &state.entities);
    while (!ll_iter_end(&iter)) {
        entity_render(ll_iter_peek(&iter));
        ll_iter_next(&iter);
    }


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
    return ERR_OK;
}

i32 game_teardown() {
    queue_destroy(&state.event_queue);
    SDL_DestroyTexture(state.render_target);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();

    return ERR_OK;
}