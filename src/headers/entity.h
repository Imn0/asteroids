#include "common.h"

typedef enum {
    ENTITY_BULLET,
    ENTITY_ROCK
} EntityType;

typedef struct {
    V2f32 position, velocity;
    struct {
        u32 remove : 1;
    } flags;
} EntityCommon;

typedef struct {
    EntityCommon common;
    f32 ttl;
} EntityBullet;

struct entity_create_rock_args {
    V2f32 position, velocity;
    i32 num_vertices;
    f32 jaggedness;
    f32 base_radius;
    u8 seed;
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
Entity* entity_create_bullet(V2f32 position, V2f32 initial_velocity, f32 angle_deg);
void entity_update(Entity entity[static 1]);
void entity_render(Entity entity[static 1]);

bool entity_check_collision_point(Entity entity1[static 1], V2f32 point);
