#pragma once

#include "entity.h"
#include "common.h"
#include "animation.h"
#include "ufo.h"

typedef enum {
    EVENT_REMOVE_ENTITY,
    EVENT_PLAYER_ADD_POINTS,
    EVENT_TYPE_SHOOT,
    EVENT_TYPE_NEW_ROCK,
    EVENT_LOCAL_PLAYER_DEATH,
    EVENT_REMOTE_PLAYER_DEATH,
    EVENT_UFO_SPAWN,
    EVENT_UFO_KILL,
} EventType;

typedef enum {
    NONE, 
    SPARKLE,
} EntityRemoveAnimation;

typedef struct {
    i32 id_to_remove;
    EntityRemoveAnimation animation;
} EventRemoveEntity;

typedef struct {
    i32 points_to_add;
} EventPlayerAddPoints;

typedef struct {
    V2f32 position, velocity;
    f32 angle_deg;
} EventPlayerDeath;

typedef struct {
    V2f32 position, velocity;
    UfoForm form;
} EventUfoSpawn;

typedef struct {
    V2f32 ufo_position, ufo_velocity, bullet_velocity;
} EventUfoKill;

typedef EntityCreateRockArgs EventRock;
typedef EntityCreateBulletArgs EventShoot;

typedef struct {
    EventType type;
    union {
        EventPlayerAddPoints add_points;
        EventRemoveEntity remove_entity;
        EventShoot shoot;
        EventRock rock;
        EventPlayerDeath player_death;
        EventUfoSpawn ufo_spawn;
        EventUfoKill ufo_kill;
    } event;
} Event;

void add_event_starting_rocks();

/**
 * @brief Spawns a rock with given size at the edge of the screen away from players
 *
 * @param size
 */
void add_event_random_rock(RockSize size);
void add_event_random_rocks(i32 count, RockSize size);
void add_event_from_rock_kill(RockSize killed_rock_size, V2f32 killed_pos, V2f32 killed_velocity);

void register_event_local(Event* e);
void register_event_remote(Event* e);

Entity* create_bullet_from_event(Event* e);
Entity* create_rock_from_event(Event* e);

void add_event_ufo(UfoForm form);
void make_ufo_from_event(Event* e, Ufo* ufo);
Animation* make_animation_from_event(Event* e);

void remote_kill_entity(i32 id, EntityRemoveAnimation animation_type);
void remote_add_points(i32 amount_to_add);
