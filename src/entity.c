#include "game.h"
#include "entity.h"

Entity* entity_create_rock(struct entity_create_rock_args args) {

    Entity* entity = malloc(sizeof(Entity));

    EntityRock* e = &entity->data.rock;
    *e = (EntityRock){ 0 };
    e->angle_deg = 0.0f;
    e->num_vertices = args.num_vertices;
    e->common.position = args.position;
    e->common.velocity = args.velocity;

    u8 rng_idx = args.seed;

    // TODO THIS vvv
    e->rotation_speed = ASTEROID_MAX_ROTATION_SPEED;
    e->base_radius = args.base_radius;

    for (i32 i = 0; i < args.num_vertices; i++) {
        f32 angle = (f32)i / args.num_vertices * TAU;
        f32 radius = args.base_radius * (1.0f - args.jaggedness + args.jaggedness * (f32)random_table[rng_idx] / 255);
        e->vertices[i].x = radius * cosf(angle);
        e->vertices[i].y = radius * sinf(angle);
        rng_idx += 1;
    }


    entity->type = ENTITY_ROCK;
    return entity;
}

Entity* entity_create_bullet(V2f32 position, V2f32 initial_velocity, f32 angle_deg) {

    Entity* entity = malloc(sizeof(Entity));

    EntityBullet* e = &entity->data.bullet;
    *e = (EntityBullet){ 0 };
    e->common.position = position;
    e->common.velocity.x = sinf(deg_to_rad(angle_deg)) * BULLET_SPEED + initial_velocity.x;
    e->common.velocity.y = -cosf(deg_to_rad(angle_deg)) * BULLET_SPEED + initial_velocity.y;
    e->ttl = BULLET_INITIAL_TTL;
    entity->type = ENTITY_BULLET;

    return entity;
}

void rock_update(EntityRock rock[static 1]) {

    rock->common.position.x += rock->common.velocity.x * delta_time;
    rock->common.position.y += rock->common.velocity.y * delta_time;

    rock->angle_deg += rock->rotation_speed * delta_time;
    if (rock->angle_deg < 0.0f) { rock->angle_deg += 360.0f; }
    else if (rock->angle_deg > 360.0f) { rock->angle_deg -= 360.0f; }

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
        rock->phantom.position.y = WINDOW_HEIGHT + rock->common.position.y;
        rock->phantom.phantom_enabled = true;
    }
    else if (rock->common.position.y > WINDOW_HEIGHT - rock->base_radius) {
        if (!rock->phantom.phantom_enabled) {
            rock->phantom.position.y = rock->common.position.x;
        }
        rock->phantom.position.y = rock->common.position.y - WINDOW_HEIGHT;
        rock->phantom.phantom_enabled = true;
    }
}

void bullet_update(EntityBullet bullet[static 1]) {
    bullet->common.position.x += bullet->common.velocity.x * delta_time;
    bullet->common.position.y += bullet->common.velocity.y * delta_time;

    bullet->ttl -= delta_time;
    if (bullet->ttl < 0.0f) {
        bullet->common.flags.remove = 1;
    }
}

void bullet_render(EntityBullet bullet[static 1]) {
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);

    // Define the rectangle with the center at (cx, cy)
    SDL_Rect rect;
    rect.w = 7;
    rect.h = 7;
    rect.x = (i32)bullet->common.position.x - rect.w / 2;
    rect.y = (i32)bullet->common.position.y - rect.h / 2;

    // Draw the rectangle
    SDL_RenderFillRect(state.renderer, &rect);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
}

void rock_render(EntityRock rock[static 1]) {
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);


    for (i32 i = 0; i < rock->num_vertices; i++) {
        i32 next = (i + 1) % rock->num_vertices;
        V2f32 rotated_current = rotate_point(rock->vertices[i], (V2f32) { 0, 0 }, deg_to_rad(rock->angle_deg));
        V2f32 rotated_next = rotate_point(rock->vertices[next], (V2f32) { 0, 0 }, deg_to_rad(rock->angle_deg));

        SDL_RenderDrawLine(state.renderer,
                           (i32)rotated_current.x + rock->common.position.x, (i32)rotated_current.y + rock->common.position.y,
                           (i32)rotated_next.x + rock->common.position.x, (i32)rotated_next.y + rock->common.position.y);
    }

    if(rock->phantom.phantom_enabled){
        for (i32 i = 0; i < rock->num_vertices; i++) {
        i32 next = (i + 1) % rock->num_vertices;
        V2f32 rotatedCurrent = rotate_point(rock->vertices[i], (V2f32) { 0, 0 }, deg_to_rad(rock->angle_deg));
        V2f32 rotatedNext = rotate_point(rock->vertices[next], (V2f32) { 0, 0 }, deg_to_rad(rock->angle_deg));

        SDL_RenderDrawLine(state.renderer,
                           (i32)rotatedCurrent.x + rock->phantom.position.x, (i32)rotatedCurrent.y + rock->phantom.position.y,
                           (i32)rotatedNext.x + rock->phantom.position.x, (i32)rotatedNext.y + rock->phantom.position.y);
    }
    }



    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
}

void entity_update(Entity entity[static 1]) {

    if (entity->data.common.position.x < 0) entity->data.common.position.x = WINDOW_WIDTH;
    else if (entity->data.common.position.x > WINDOW_WIDTH) entity->data.common.position.x = 0;
    if (entity->data.common.position.y < 0) entity->data.common.position.y = WINDOW_HEIGHT;
    else if (entity->data.common.position.y > WINDOW_HEIGHT) { entity->data.common.position.y = 0; }

    switch (entity->type) {
    case ENTITY_BULLET:
        bullet_update(&entity->data.bullet);
        break;
    case ENTITY_ROCK:
        rock_update(&entity->data.rock);
        break;
    }

}

void entity_render(Entity entity[static 1]) {
    switch (entity->type) {
    case ENTITY_BULLET:
        bullet_render(&entity->data.bullet);
        break;
    case ENTITY_ROCK:
        rock_render(&entity->data.rock);
        break;
    }

}

bool entity_check_collision_point(Entity entity[static 1], V2f32 point) {
    if (entity->type != ENTITY_ROCK) { return false; }

    EntityRock* rock = &entity->data.rock;
    bool inside = false;
    for (int i = 0, j = rock->num_vertices - 1; i < rock->num_vertices; j = i++) {
        if (((rock->vertices[i].y + rock->common.position.y > point.y) != (rock->vertices[j].y + rock->common.position.y > point.y)) &&
            (point.x < (rock->vertices[j].x - rock->vertices[i].x) * (point.y - rock->vertices[i].y - rock->common.position.y) / (rock->vertices[j].y - rock->vertices[i].y) + rock->vertices[i].x + rock->common.position.x)) {
            inside = !inside;
        }
    }
    return inside;
}