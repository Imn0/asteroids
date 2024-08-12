#pragma once

#include "animation.h"
#include "common.h"
#include "entity.h"
#include "ufo.h"

typedef enum {
    EVENT_REMOVE_ENTITY,
    EVENT_REMOTE_PLAYER_ADD_POINTS,
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

typedef EventPlayerDeath EventLocalPlayerDeath;
typedef EventPlayerDeath EventRemotePlayerDeath;

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
        EventLocalPlayerDeath local_player_death;
        EventRemotePlayerDeath remote_player_death;
        EventUfoSpawn ufo_spawn;
        EventUfoKill ufo_kill;
    } event;
} Event;

// TODO add handling generic event here
// TODO clear up naming scheme

func handle_event(Event* e);

void add_event_shoot(V2f32 position, f32 angle_deg, V2f32 velocity, BulletOrigin origin);
func handle_event_shoot(EventShoot* e);

void remote_kill_entity(i32 id, EntityRemoveAnimation animation_type);
void add_event_remove_entity(i32 id, EntityRemoveAnimation animation_type);
func handle_event_remove_entity();

void remote_add_points(i32 amount_to_add);
void add_event_remote_add_points(i32 amount_to_add);
func handle_event_remote_add_points(EventPlayerAddPoints* e);

void add_event_new_rock(RockSize rock_size, V2f32 position, V2f32 velocity);
func handle_event_new_rock(EventRock* e);

void add_event_player_death(V2f32 position, V2f32 velocity, f32 angle_deg);

void add_event_local_player_death(V2f32 position, V2f32 velocity, f32 angle_deg);
func handle_event_local_player_death(EventLocalPlayerDeath* e);

void add_event_remote_player_death(V2f32 position, V2f32 velocity, f32 angle_deg);
func handle_event_remote_player_death(EventRemotePlayerDeath* e);

void add_event_ufo_spawn(UfoForm form, V2f32 position, f32 speed, f32 direction_deg);
func handle_event_ufo_spawn(EventUfoSpawn* e);

void add_event_ufo_kill(V2f32 ufo_position, V2f32 ufo_velocity, V2f32 bullet_velocity);
func handle_event_ufo_kill(EventUfoKill* e);

void register_event_local(Event* e);
void register_event_remote(Event* e);

// void add_event_starting_rocks();

// /**
//  * @brief Spawns a rock with given size at the edge of the screen away from
//  * players
//  *
//  * @param size
//  */
// void add_event_random_rock(RockSize size);
// void add_event_random_rocks(i32 count, RockSize size);
// void add_event_from_rock_kill(RockSize killed_rock_size, V2f32 killed_pos, V2f32
// killed_velocity);

// Entity* create_bullet_from_event(Event* e);
// Entity* create_rock_from_event(Event* e);

// void add_event_ufo_spawn(UfoForm form);
// void add_event_ufo_kill(EventUfoKill args);
// void make_ufo_from_event(Event* e, Ufo* ufo);
// Animation* make_animation_from_event(Event* e);

// void remote_add_points(i32 amount_to_add);
