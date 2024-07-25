#include "common.h"

typedef enum {
    ENTITY_BULLET,
    ENTITY_ROCK
} EntityType;

typedef struct {
    V2f32 position, velocity;
    f32 ttl;
} EntityBullet;

typedef struct {
    V2f32 position, velocity;
} EntityRock;

typedef struct {
    EntityType type;
    union {
        EntityBullet bullet;
        EntityRock rock;
    } data;
} Entity;

func entity_create_bullet(V2f32 position, V2f32 initial_velocity, f32 angle_deg, Entity entity[static 1]);
func entity_update(Entity entity[static 1]);
func entity_render(Entity entity[static 1]);