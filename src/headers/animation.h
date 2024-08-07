#pragma once
#include "common.h"
#include "player.h"

#define ANIMATION_SPRINKLE_TIME 3.0f

typedef enum {
    ANIMATION_PLAYER_DEATH,
    ANIMATION_SPRINKLE,
} AnimationType;

typedef struct {
    f32 ttl;
} AnimationCommon;

typedef struct {
    AnimationCommon common;
    V2f32 segment_positions[PLAYER_SEGMENTS_SIZE * 2];
    V2f32 segment_velocities[PLAYER_SEGMENTS_SIZE * 2];
    f32 angle_deg;
    SDL_Color color;
} AnimationPlayerDeath;

typedef struct {
    AnimationCommon common;
    V2f32 position;
    i32 num_sprinkles;
    u8 seed;
} AnimationSprinkle;

typedef struct {
    AnimationType type;
    union {
        AnimationCommon common;
        AnimationSprinkle sprinkle;
        AnimationPlayerDeath player_death;
    } animation;
} Animation;

Animation* create_player_death_animation(V2f32 position,
                                         V2f32 velocity,
                                         f32 angle_deg,
                                         SDL_Color color);
Animation* create_sprinkle_animation(V2f32 position,
                                     i32 num_sprinkles,
                                     u8 seed);
extern void animation_update(Animation* animation);
extern void animation_render(Animation* animation);
