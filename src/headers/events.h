#pragma once

#include "entity.h"
#include "common.h"

typedef enum {
    EVENT_TYPE_SHOOT,
    EVENT_TYPE_NEW_ROCK,
} EventType;

typedef struct {
    V2f32 position, initial_velocity;
    f32 angle_deg;
} EventShoot;

typedef struct {
    V2f32 position, initial_velocity;
    i32 num_vertices;
    f32 jaggedness;
    RockSize rock_size;
    u8 seed;
} EventRock;

typedef struct {
    EventType type;
    union {
        EventShoot shoot;
        EventRock rock;
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
