#include "ufo.h"

#include "game.h"
#include "netcode.h"

#define UFO_SEGMENTS_SIZE 10

static const V2f32 ufo_segments[UFO_SEGMENTS_SIZE][2] = {
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

f32 objective(f32 theta, f32 va_x, f32 va_y, f32 xa, f32 ya, f32 xb, f32 yb, f32 vb_x, f32 vb_y) {
    f32 vp_x = va_x + sin(theta) * BULLET_SPEED;
    f32 vp_y = va_y + (-cos(theta)) * BULLET_SPEED;

    f32 t_x = (xb - xa) / (vp_x - vb_x);
    f32 t_y = (yb - ya) / (vp_y - vb_y);

    return fabsf(t_x - t_y);
}

void ufo_shoot(Ufo* ufo) {

    ufo->shoot_timer = PLAYER_SHOOT_COOLDOWN * 5;

    V2f32 player_pos = local_player.position;
    V2f32 player_vel = local_player.velocity;

    if (!network_state.online_disable && rand_float(0, 1) < 0.5f) {
        player_pos = remote_player.position;
        player_vel = remote_player.velocity;
    }

    f32 va_x = game_ufo.velocity.x, va_y = game_ufo.velocity.y, xa = game_ufo.position.x,
        ya = game_ufo.position.y, xb = player_pos.x, yb = player_pos.y, vb_x = player_vel.x,
        vb_y = player_vel.y;

    f32 a = 0, b = 2 * M_PI;
    f32 gr = (sqrt(5) + 1) / 2;
    f32 c = b - (b - a) / gr;
    f32 d = a + (b - a) / gr;

    while (fabs(c - d) > 1e-2) {
        if (objective(c, va_x, va_y, xa, ya, xb, yb, vb_x, vb_y) <
            objective(d, va_x, va_y, xa, ya, xb, yb, vb_x, vb_y)) {
            b = d;
        } else {
            a = c;
        }

        c = b - (b - a) / gr;
        d = a + (b - a) / gr;
    }

    f32 angle_deg = rad_to_deg((b + a) / 2);

    if (game_ufo.form == UFO_BIG) {
        angle_deg += rand_float(-20, 20);
    } else if (game_ufo.form == UFO_SMALL) {
        angle_deg += rand_float(-5, 5);
    }

    add_event_shoot(ufo->position, angle_deg, ufo->velocity, BULLET_UFO);
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
        V2f32 start_point = rotate_point((V2f32) { ufo_scale * ufo_segments[i][0].x,
                                                   ufo_scale * ufo_segments[i][0].y },
                                         (V2f32) { 0, 0 },
                                         deg_to_rad(ufo->angle_deg));
        V2f32 end_point = rotate_point((V2f32) { ufo_scale * ufo_segments[i][1].x,
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
