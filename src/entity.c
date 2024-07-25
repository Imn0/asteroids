#include "game.h"
#include "entity.h"


func entity_create_bullet(V2f32 position, V2f32 initial_velocity, f32 angle_deg, Entity entity[static 1]) {

    EntityBullet e;
    e.position = position;
    e.velocity.x = sinf(DEG_TO_RAD(angle_deg)) * BULLET_SPEED + initial_velocity.x;
    e.velocity.y = -cosf(DEG_TO_RAD(angle_deg)) * BULLET_SPEED + initial_velocity.y;
    e.ttl = BULLET_INITIAL_TTL;
    entity->type = ENTITY_BULLET;
    entity->data.bullet = e;

    return ERR_OK;
}

func bullet_update(EntityBullet bullet[static 1]) {
    bullet->position.x += bullet->velocity.x * delta_time;
    bullet->position.y += bullet->velocity.y * delta_time;

    bullet->ttl -= delta_time;

    if (bullet->ttl < 0.0f) {
        return RET_ENTITY_REMOVE;
    }
    return ERR_OK;
}

func bullet_render(EntityBullet bullet[static 1]) {
    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);

    // Define the rectangle with the center at (cx, cy)
    SDL_Rect rect;
    rect.w = 10;
    rect.h = 10;
    rect.x = (int)bullet->position.x - rect.w / 2;
    rect.y = (int)bullet->position.y - rect.h / 2;

    // Draw the rectangle
    SDL_RenderFillRect(state.renderer, &rect);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
    return ERR_OK;
}

func entity_update(Entity entity[static 1]) {

    switch (entity->type) {
    case ENTITY_BULLET:
        return bullet_update(&entity->data.bullet);
        break;
    case ENTITY_ROCK:
        printf("%s, ROCK\n", __FILE__);
        break;
    }
    return ERR_OK;
}

func entity_render(Entity entity[static 1]) {
    switch (entity->type) {
    case ENTITY_BULLET:
        bullet_render(&entity->data.bullet);
        break;
    case ENTITY_ROCK:
        break;
    }
    return ERR_OK;

}