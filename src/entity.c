#include "entity.h"
#include "game.h"
#include "events.h"
#include "netcode.h"
#include "animation.h"

Entity* entity_create_rock(EntityCreateRockArgs args) {

    Entity* entity = malloc(sizeof(Entity));
    entity->type = ENTITY_ROCK;

    EntityRock* e = &entity->data.rock;
    *e = (EntityRock){ 0 };
    e->angle_deg = 0.0f;
    e->num_vertices = args.num_vertices;
    e->common.position = args.position;
    e->common.velocity = args.initial_velocity;

    u8 rng_idx = args.seed;

    i32 sign = rand_i32_seed(0, 1, rng_idx) == 1 ? 1 : -1;

    e->rotation_speed = sign * (ASTEROID_MAX_ROTATION_SPEED - rand_float_seed(-7.0f, 0.0f, rng_idx));
    e->rock_size = args.rock_size;
    e->base_radius = rock_sizes[args.rock_size];

    ASSERT(args.id != 0, "invaild or no id was given %d\n", args.id);
    e->common.id = args.id;

    for (i32 i = 0; i < args.num_vertices; i++) {
        f32 angle = (f32)i / args.num_vertices * TAU;
        f32 radius = e->base_radius *
            (1.0f - args.jaggedness +
             args.jaggedness * rand_float_seed(0, 1.0f, rng_idx));
        e->vertices[i].x = radius * cosf(angle);
        e->vertices[i].y = radius * sinf(angle);
        rng_idx += 1;
    }

    return entity;
}

Entity* entity_create_bullet(EntityCreateBulletArgs args) {

    Entity* entity = malloc(sizeof(Entity));
    entity->type = ENTITY_BULLET;

    EntityBullet* e = &entity->data.bullet;
    *e = (EntityBullet){ 0 };

    ASSERT(args.id != 0, "invaild or no id was given %d\n", args.id);
    e->common.id = args.id;

    e->common.position = args.position;
    e->last_position = args.position;
    e->common.velocity.x =
        sinf(deg_to_rad(args.angle_deg)) * BULLET_SPEED + args.initial_velocity.x;
    e->common.velocity.y =
        -cosf(deg_to_rad(args.angle_deg)) * BULLET_SPEED + args.initial_velocity.y;
    e->ttl = BULLET_INITIAL_TTL;
    e->bullet_origin = args.bullet_origin;

    return entity;
}

void rock_update(EntityRock* rock) {
    rock->common.position.x += rock->common.velocity.x * delta_time;
    rock->common.position.y += rock->common.velocity.y * delta_time;

    rock->angle_deg += rock->rotation_speed * delta_time;
    if (rock->angle_deg < 0.0f) {
        rock->angle_deg += 360.0f;
    }
    else if (rock->angle_deg > 360.0f) {
        rock->angle_deg -= 360.0f;
    }

    rock->phantom.phantom_enabled = false;
    if (rock->common.position.x < rock->base_radius) {
        rock->phantom.position.x = WINDOW_WIDTH + rock->common.position.x;
        rock->phantom.position.y = rock->common.position.y;
        rock->phantom.phantom_enabled = true;
    }
    else if (rock->common.position.x > WINDOW_WIDTH - rock->base_radius) {
        rock->phantom.position.x = rock->common.position.x - WINDOW_WIDTH;
        rock->phantom.position.y = rock->common.position.y;
        rock->phantom.phantom_enabled = true;
    }

    if (rock->common.position.y < rock->base_radius) {
        if (!rock->phantom.phantom_enabled) {
            rock->phantom.position.y = rock->common.position.x;
        }
        else {
            rock->phantom.position.y = WINDOW_HEIGHT + rock->common.position.y;
        }
        rock->phantom.phantom_enabled = true;
    }
    else if (rock->common.position.y > WINDOW_HEIGHT - rock->base_radius) {
        if (!rock->phantom.phantom_enabled) {
            rock->phantom.position.y = rock->common.position.x;
        }
        else {
            rock->phantom.position.y = rock->common.position.y - WINDOW_HEIGHT;
        }
        rock->phantom.phantom_enabled = true;
    }
}

void bullet_update(EntityBullet* bullet) {
    bullet->last_position.x = bullet->common.position.x;
    bullet->last_position.y = bullet->common.position.y;

    bullet->common.position.x += bullet->common.velocity.x * delta_time;
    bullet->common.position.y += bullet->common.velocity.y * delta_time;

    bullet->ttl -= delta_time;
    if (bullet->ttl < 0.0f) {
        bullet->common.flags.remove = 1;
    }
}


void rock_render(EntityRock* rock) {

    for (i32 i = 0; i < rock->num_vertices; i++) {
        i32 next = (i + 1) % rock->num_vertices;
        V2f32 rotated_current = rotate_point(rock->vertices[i], (V2f32) { 0, 0 },
                                             deg_to_rad(rock->angle_deg));
        V2f32 rotated_next = rotate_point(rock->vertices[next], (V2f32) { 0, 0 },
                                          deg_to_rad(rock->angle_deg));

        gfx_render_thick_line(
            state.renderer, (i32)rotated_current.x + rock->common.position.x,
            (i32)rotated_current.y + rock->common.position.y,
            (i32)rotated_next.x + rock->common.position.x,
            (i32)rotated_next.y + rock->common.position.y, LINE_THICKNESS, (SDL_Color) { .r = 255, .g = 255, .b = 255, .a = 255 });
    }

    if (rock->phantom.phantom_enabled) {
        for (i32 i = 0; i < rock->num_vertices; i++) {
            i32 next = (i + 1) % rock->num_vertices;
            V2f32 rotatedCurrent = rotate_point(
                rock->vertices[i], (V2f32) { 0, 0 }, deg_to_rad(rock->angle_deg));
            V2f32 rotatedNext =
                rotate_point(rock->vertices[next], (V2f32) { 0, 0 },
                             deg_to_rad(rock->angle_deg));

            gfx_render_thick_line(
                state.renderer,
                (i32)rotatedCurrent.x + rock->phantom.position.x,
                (i32)rotatedCurrent.y + rock->phantom.position.y,
                (i32)rotatedNext.x + rock->phantom.position.x,
                (i32)rotatedNext.y + rock->phantom.position.y, LINE_THICKNESS, (SDL_Color) { .r = 255, .g = 255, .b = 255, .a = 255 });
        }
    }
}

void bullet_render(EntityBullet* bullet) {
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
    if (bullet->bullet_origin == BULLET_REMOTE) {
        SDL_SetRenderDrawColor(state.renderer, 128, 255, 128, 255);
    }
    else if (bullet->bullet_origin == BULLET_UFO) {
        SDL_SetRenderDrawColor(state.renderer, 255, 64, 64, 255);
    }

    SDL_Rect rect = {
        .w = 7,
        .h = 7,
        .x = (i32)bullet->common.position.x - rect.w / 2,
        .y = (i32)bullet->common.position.y - rect.h / 2 };

    SDL_RenderFillRect(state.renderer, &rect);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
}

inline void entity_update(Entity* entity) {

    if (entity->data.common.position.x < 0)
        entity->data.common.position.x = WINDOW_WIDTH;
    else if (entity->data.common.position.x > WINDOW_WIDTH)
        entity->data.common.position.x = 0;
    if (entity->data.common.position.y < 0)
        entity->data.common.position.y = WINDOW_HEIGHT;
    else if (entity->data.common.position.y > WINDOW_HEIGHT) {
        entity->data.common.position.y = 0;
    }

    switch (entity->type) {
    case ENTITY_BULLET:
        bullet_update(&entity->data.bullet);
        break;
    case ENTITY_ROCK:
        rock_update(&entity->data.rock);
        break;
    }
}

inline void entity_render(Entity* entity) {
    switch (entity->type) {
    case ENTITY_BULLET:
        bullet_render(&entity->data.bullet);
        break;
    case ENTITY_ROCK:
        rock_render(&entity->data.rock);
        break;
    }
}

bool entity_check_collision_point(Entity* entity, V2f32 point) {
    if (entity->type != ENTITY_ROCK) {
        return false;
    }
    bool inside = false;
    EntityRock* rock = &entity->data.rock;

    point = (V2f32){ .x = point.x - rock->common.position.x, .y = point.y - rock->common.position.y };

    V2f32 phantom_point;
    if (rock->phantom.phantom_enabled) {
        phantom_point = (V2f32){ .x = point.x - rock->phantom.position.x, .y = point.y - rock->phantom.position.y };
        phantom_point = rotate_point(phantom_point, (V2f32) { 0 }, deg_to_rad(-rock->angle_deg));

    }

    point = rotate_point(point, (V2f32) { 0 }, deg_to_rad(-rock->angle_deg));

    for (i32 i = 0, j = rock->num_vertices - 1; i < rock->num_vertices; j = i++) {
        if (((rock->vertices[i].y > point.y) != (rock->vertices[j].y > point.y)) &&
            (point.x < (rock->vertices[j].x - rock->vertices[i].x) * (point.y - rock->vertices[i].y) / (rock->vertices[j].y - rock->vertices[i].y) + rock->vertices[i].x))
            inside = !inside;
    }

    if (inside || !rock->phantom.phantom_enabled) { return inside; }

    inside = false;
    for (i32 i = 0, j = rock->num_vertices - 1; i < rock->num_vertices; j = i++) {
        if (((rock->vertices[i].y > phantom_point.y) != (rock->vertices[j].y > phantom_point.y)) &&
            (phantom_point.x < (rock->vertices[j].x - rock->vertices[i].x) * (phantom_point.y - rock->vertices[i].y) / (rock->vertices[j].y - rock->vertices[i].y) + rock->vertices[i].x))
            inside = !inside;
    }
    return inside;

}

bool line_intersects_line(V2f32 p1, V2f32 p2, V2f32 q1, V2f32 q2) {
    float s1_x = p2.x - p1.x;
    float s1_y = p2.y - p1.y;
    float s2_x = q2.x - q1.x;
    float s2_y = q2.y - q1.y;

    float s = (-s1_y * (p1.x - q1.x) + s1_x * (p1.y - q1.y)) /
        (-s2_x * s1_y + s1_x * s2_y);
    float t = (s2_x * (p1.y - q1.y) - s2_y * (p1.x - q1.x)) /
        (-s2_x * s1_y + s1_x * s2_y);

    return s >= 0 && s <= 1 && t >= 0 && t <= 1;
}

bool entity_check_collision_line(Entity* entity, V2f32 point1,
                                 V2f32 point2) {
    if (entity->type != ENTITY_ROCK) {
        return false;
    }

    EntityRock* rock = &entity->data.rock;

    if (entity_check_collision_point(entity, point1) ||
        entity_check_collision_point(entity, point2)) {
        return true;
    }

    for (int i = 0, j = rock->num_vertices - 1; i < rock->num_vertices;
         j = i++) {
        V2f32 v1 = { rock->vertices[i].x + rock->common.position.x,
                    rock->vertices[i].y + rock->common.position.y };
        V2f32 v2 = { rock->vertices[j].x + rock->common.position.x,
                    rock->vertices[j].y + rock->common.position.y };
        if (line_intersects_line(point1, point2, v1, v2)) {
            return true;
        }
    }

    return false;
}
