#include "ufo.h"

#include "game.h"
#include "netcode.h"

#define UFO_SEGMENTS_SIZE 10

const static V2f32 ufo_segments[UFO_SEGMENTS_SIZE][2] = {
    { { -32, 0 },   { -25, 10 }  },
    { { -32, 0 },   { -25, -10 } },
    { { -32, 0 },   { 32, 0 }    },
    { { 32, 0 },    { 25, 10 }   },
    { { 32, 0 },    { 25, -10 }  },
    { { 25, 10 },   { -25, 10 }  },
    { { 25, -10 },  { -25, -10 } },
    { { -15, -10 }, { -12, -19 } },
    { { 15, -10 },  { 12, -19 }  },
    { { -13, -19 }, { 13, -19 }  },
};

void ufo_shoot(Ufo* ufo) {
    // TODO placeholder

    if (ufo->shoot_timer > 0.0f) {
        return;
    }

    ufo->shoot_timer = PLAYER_SHOOT_COOLDOWN * 3;

    // create event
    Event* e = malloc(sizeof(Event));
    e->type = EVENT_TYPE_SHOOT;

    i32 id = rand() + 1;

    e->event.shoot = (EventShoot) { .position = ufo->position,
                                    .angle_deg = ufo->angle_deg,
                                    .initial_velocity = ufo->velocity,
                                    .bullet_origin = BULLET_UFO,
                                    .id = id };
    register_event_local(e);
    register_event_remote(e);
}

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

    if (!network_state.is_server) {
        return;
    }

    ufo->shoot_timer -= delta_time;
    if (ufo->shoot_timer < 0.0f) {
        ufo_shoot(ufo);
    }
}

void ufo_render(Ufo* ufo) {
    if (ufo->form == UFO_NONE) {
        return;
    }

    f32 ufo_scale = ufo->form == UFO_SMALL ? 1 : 2.5;
    for (i32 i = 0; i < UFO_SEGMENTS_SIZE; i++) {
        V2f32 start_point = rotate_point(
                (V2f32) { ufo_scale * ufo_segments[i][0].x,
                          ufo_scale * ufo_segments[i][0].y },
                (V2f32) { 0, 0 },
                deg_to_rad(ufo->angle_deg));
        V2f32 end_point = rotate_point(
                (V2f32) { ufo_scale * ufo_segments[i][1].x,
                          ufo_scale * ufo_segments[i][1].y },
                (V2f32) { 0, 0 },
                deg_to_rad(ufo->angle_deg));
        gfx_render_thick_line(state.renderer,
                              (i32)start_point.x + ufo->position.x,
                              (i32)start_point.y + ufo->position.y,
                              (i32)end_point.x + ufo->position.x,
                              (i32)end_point.y + ufo->position.y,
                              LINE_THICKNESS,
                              (SDL_Color) { 255, 255, 255, 255 });
    }
}
