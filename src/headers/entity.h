#pragma once
#include "common.h"

typedef enum { ENTITY_BULLET, ENTITY_ROCK } EntityType;

typedef struct {
    V2f32 position, velocity;
    struct {
        u32 remove : 1;
    } flags;
} EntityCommon;

typedef struct {
    EntityCommon common;
    f32 ttl;
    V2f32 last_position;
} EntityBullet;

typedef enum {
    ROCK_SMALL = 0,
    ROCK_MEDIUM,
    ROCK_BIG
} RockSize;

struct entity_create_rock_args {
    V2f32 position, velocity;
    i32 num_vertices;
    f32 jaggedness;
    RockSize rock_size;
    u8 seed;
};

static f32 rock_sizes[] = {
    [ROCK_SMALL] = 45.0f,
    [ROCK_MEDIUM] = 75.0f,
    [ROCK_BIG] = 120.0f,
};

typedef struct {
    EntityCommon common;
    f32 angle_deg, rotation_speed;
    i32 num_vertices;
    V2f32 vertices[ASTEROID_MAX_POINTS];
    f32 base_radius;
    struct {
        bool phantom_enabled;
        V2f32 position;
    } phantom;
    RockSize rock_size;
} EntityRock;

typedef struct {
    EntityType type;
    union {
        EntityCommon common;
        EntityBullet bullet;
        EntityRock rock;
    } data;
} Entity;

Entity* entity_create_rock(struct entity_create_rock_args args);
Entity* entity_create_bullet(V2f32 position, V2f32 initial_velocity,
                             f32 angle_deg);
extern void entity_update(Entity* entity);
extern void entity_render(Entity* entity);

bool entity_check_collision_point(Entity* entity1, V2f32 point);
bool entity_check_collision_line(Entity* entity, V2f32 point1,
                                 V2f32 point2);
