#include "animation.h"
#include "game.h"
#include "physics.h"
#include "player.h"

Animation* create_player_death_animation(V2f32 position,
                                         V2f32 velocity,
                                         f32 angle_deg,
                                         SDL_Color color) {
    Animation* animation = malloc(sizeof(Animation));
    animation->type = ANIMATION_PLAYER_DEATH;
    AnimationPlayerDeath* pd = &animation->animation.player_death;
    *pd = (AnimationPlayerDeath) { 0 };

    pd->color = color;
    velocity.x *= 0.3f;
    velocity.y *= 0.3f;
    pd->angle_deg = angle_deg;
    for (i32 i = 0; i < PLAYER_SEGMENTS_SIZE * 2; i++) {
        pd->segment_positions[i] = position;
        pd->segment_velocities[i] = (V2f32) {
            .x = velocity.x + rand_float(-50.0f, 50.0f),
            .y = velocity.y + rand_float(-50.0f, 50.0f),
        };
    }
    pd->common.ttl = 1.3f;

    return animation;
}

Animation* create_sprinkle_animation(V2f32 position,
                                     i32 num_sprinkles,
                                     u8 seed) {
    Animation* animation = malloc(sizeof(Animation));
    animation->type = ANIMATION_SPRINKLE;
    AnimationSprinkle* sprinkle = &animation->animation.sprinkle;
    *sprinkle = (AnimationSprinkle) { 0 };

    sprinkle->position = position;
    sprinkle->common.ttl = ANIMATION_SPRINKLE_TIME;
    sprinkle->seed = seed;
    sprinkle->num_sprinkles = num_sprinkles;

    return animation;
}

void update_player_death_animation(AnimationPlayerDeath* animation) {

    for (i32 i = 0; i < PLAYER_SEGMENTS_SIZE * 2; i++) {
        animation->segment_positions[i]
                .x += animation->segment_velocities[i].x * delta_time;
        animation->segment_positions[i]
                .y += animation->segment_velocities[i].y * delta_time;
        decelerate_v2f32(&animation->segment_velocities[i],
                         PLAYER_ACCELERATION_SPEED / 27);
    }
    animation->common.ttl -= delta_time;
}

void render_player_death_animation(AnimationPlayerDeath* animation) {
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);

    for (i32 i = 0; i < PLAYER_SEGMENTS_SIZE * 2; i++) {
        V2f32 start_point = rotate_point(
                player_segments[i % PLAYER_SEGMENTS_SIZE][0],
                (V2f32) { 0, 0 },
                deg_to_rad(animation->angle_deg));
        V2f32 end_point = rotate_point(
                player_segments[i % PLAYER_SEGMENTS_SIZE][1],
                (V2f32) { 0, 0 },
                deg_to_rad(animation->angle_deg));

        gfx_render_thick_line(
                state.renderer,
                (i32)start_point.x + animation->segment_positions[i].x,
                (i32)start_point.y + animation->segment_positions[i].y,
                (i32)end_point.x + animation->segment_positions[i].x,
                (i32)end_point.y + animation->segment_positions[i].y,
                LINE_THICKNESS,
                animation->color);
    }

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
}

void render_sprinkle_animation(AnimationSprinkle* animation) {
    u8 seed = animation->seed;
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);

    for (i32 i = 0; i < animation->num_sprinkles; i++) {
        f32 angle = (f32)i / animation->num_sprinkles * TAU +
                    rand_float_seed(-0.3f, 0.3f, seed++);

        f32 d_dist = rand_float_seed(-5.0f, 5.0f, seed++);
        f32 d_x = rand_float_seed(0.0f, 15.0f, seed++);
        f32 d_y = rand_float_seed(0.0f, 15.0f, seed++);

        f32 ttl_percent = 1 - (animation->common.ttl / ANIMATION_SPRINKLE_TIME);
        f32 dist = (logf(ttl_percent + 0.5) + 1) * (30.0 + d_dist);
        f32 x = animation->position.x + sinf(angle) * dist + d_x;
        f32 y = animation->position.y - cosf(angle) * dist + d_y;

        SDL_Rect rect;
        rect.w = 7;
        rect.h = 7;
        rect.x = (int)x - rect.w / 2;
        rect.y = (int)y - rect.h / 2;

        SDL_RenderFillRect(state.renderer, &rect);
    }
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);

    animation->common.ttl -= delta_time;
}

inline void animation_update(Animation* animation) {

    switch (animation->type) {
    case ANIMATION_PLAYER_DEATH:
        update_player_death_animation(&animation->animation.player_death);
        break;
    case ANIMATION_SPRINKLE:
        // do nothing all updates are in the animation phase
        break;
    }
}

inline void animation_render(Animation* animation) {
    switch (animation->type) {
    case ANIMATION_PLAYER_DEATH:
        render_player_death_animation(&animation->animation.player_death);
        break;
    case ANIMATION_SPRINKLE:
        render_sprinkle_animation(&animation->animation.sprinkle);
        break;
    }
}
