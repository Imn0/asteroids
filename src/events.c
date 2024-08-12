#include "events.h"
#include "animation.h"
#include "game.h"
#include "netcode.h"
#include "sfx.h"

func handle_event(Event* e) {
    ASSERT(e != NULL, "??????\n");
    funct_ret_t ret;
    switch (e->type) {
    case EVENT_REMOVE_ENTITY: {
        ret = handle_event_remove_entity(&e->event.remove_entity);
        break;
    }
    case EVENT_REMOTE_PLAYER_ADD_POINTS: {
        ret = handle_event_remote_add_points(&e->event.add_points);
        break;
    }
    case EVENT_TYPE_SHOOT: {
        ret = handle_event_shoot(&e->event.shoot);
        break;
    }
    case EVENT_TYPE_NEW_ROCK: {
        ret = handle_event_new_rock(&e->event.rock);
        break;
    }
    case EVENT_LOCAL_PLAYER_DEATH: {
        ret = handle_event_local_player_death(&e->event.local_player_death);
        break;
    }
    case EVENT_REMOTE_PLAYER_DEATH: {
        ret = handle_event_remote_player_death(&e->event.remote_player_death);
        break;
    }
    case EVENT_UFO_SPAWN: {
        ret = handle_event_ufo_spawn(&e->event.ufo_spawn);
        break;
    }
    case EVENT_UFO_KILL: {
        ret = handle_event_ufo_kill(&e->event.ufo_kill);
        break;
    }
    }
    free(e);
    return ret;
}

void add_event_shoot(V2f32 position, f32 angle_deg, V2f32 velocity, BulletOrigin origin) {
    if (origin == BULLET_LOCAL) {
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_SHOOT;

        i32 id = rand() + 1;

        e->event.shoot = (EventShoot) { .position = position,
                                        .angle_deg = angle_deg,
                                        .initial_velocity = velocity,
                                        .bullet_origin = BULLET_LOCAL,
                                        .id = id };
        Event* e_for_remote = malloc(sizeof(Event));
        e_for_remote->type = EVENT_TYPE_SHOOT;
        e_for_remote->event.shoot = (EventShoot) { .position = position,
                                                   .angle_deg = angle_deg,
                                                   .initial_velocity = velocity,
                                                   .bullet_origin = BULLET_REMOTE,
                                                   .id = id };
        register_event_local(e);
        register_event_remote(e_for_remote);
    } else if (origin == BULLET_UFO) {
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_SHOOT;

        i32 id = rand() + 1;

        e->event.shoot = (EventShoot) { .position = position,
                                        .angle_deg = angle_deg,
                                        .initial_velocity = velocity,
                                        .bullet_origin = BULLET_UFO,
                                        .id = id };
        register_event_local(e);
        register_event_remote(e);
    } else {
        printf("tried to shoot remote bullet from local\n");
    }
}

func handle_event_shoot(EventShoot* e) {
    play_sound(SFX_SHOOT);
    Entity* bullet = entity_create_bullet((EntityCreateBulletArgs*)e);
    return ll_push_back(&state.entities, bullet);
}

void remote_kill_entity(i32 id, EntityRemoveAnimation animation_type) {
    ASSERT(network_state.is_server, "client tried to kill entity on remote\n");

    Event* e = malloc(sizeof(Event));
    e->type = EVENT_REMOVE_ENTITY;
    e->event.remove_entity.animation = animation_type;
    e->event.remove_entity.id_to_remove = id;
    register_event_remote(e);
}

void add_event_remove_entity(i32 id, EntityRemoveAnimation animation_type) {
    Event* e = malloc(sizeof(Event));
    e->type = EVENT_REMOVE_ENTITY;
    e->event.remove_entity.animation = animation_type;
    e->event.remove_entity.id_to_remove = id;
    register_event_local(e);
    register_event_remote(e);
}

func handle_event_remove_entity(EventRemoveEntity* e) {
    LinkedListIter iter;
    ll_iter_assign(&iter, &state.entities);
    funct_ret_t r = OK;
    while (!ll_iter_end(&iter)) {
        Entity* entity = ll_iter_peek(&iter);
        if (entity->data.common.id == e->id_to_remove) {
            entity->data.common.flags.remove = 1;

            if (e->animation == SPARKLE) {
                Animation* a = create_sprinkle_animation(entity->data.common.position,
                                                         rand_i32(10, 20),
                                                         rand_i32(0, 255));
                r = ll_push_back(&state.animations, a);
            }
        }
        ll_iter_next(&iter);
    }
    return r;
}

void remote_add_points(i32 amount_to_add) { add_event_remote_add_points(amount_to_add); }

void add_event_remote_add_points(i32 amount_to_add) {
    Event* e = malloc(sizeof(Event));
    e->type = EVENT_REMOTE_PLAYER_ADD_POINTS;
    e->event.add_points.points_to_add = amount_to_add;
    register_event_remote(e);
}

// remote is now local
func handle_event_remote_add_points(EventPlayerAddPoints* e) {
    local_player.score += e->points_to_add;
    return OK;
}

void add_event_new_rock(RockSize rock_size, V2f32 position, V2f32 velocity) {

    u8 seed = rand_i32(0, 255);
    Event* e = malloc(sizeof(Event));
    e->type = EVENT_TYPE_NEW_ROCK;
    EventRock* rock = &e->event.rock;
    *rock = (EventRock) { .rock_size = rock_size,
                          .jaggedness = rand_float(0.7f, 0.95f),
                          .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
                          .position = position,
                          .initial_velocity = velocity,
                          .seed = seed,
                          .id = rand() + 1 };
    register_event_local(e);
    register_event_remote(e);
}

func handle_event_new_rock(EventRock* e) {
    Entity* rock = entity_create_rock((EntityCreateRockArgs*)e);
    return ll_push_front(&state.entities, rock);
}

void add_event_player_death(V2f32 position, V2f32 velocity, f32 angle_deg) {
    add_event_local_player_death(position, velocity, angle_deg);
    add_event_remote_player_death(position, velocity, angle_deg);
}

void add_event_local_player_death(V2f32 position, V2f32 velocity, f32 angle_deg) {
    Event* e = malloc(sizeof(Event));
    e->type = EVENT_LOCAL_PLAYER_DEATH;
    e->event.local_player_death.angle_deg = angle_deg;
    e->event.local_player_death.position = position;
    e->event.local_player_death.velocity = velocity;

    register_event_local(e);
}

func handle_event_local_player_death(EventLocalPlayerDeath* e) {
    local_player.angle_deg = 0.0f;
    local_player.position = (V2f32) { .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 };
    local_player.velocity = (V2f32) { 0 };
    local_player.ex_flags.invincible = 1;
    if (local_player.lives == 0) {
        local_player.flags.perma_dead = 1;
    }

    local_player.lives = max(local_player.lives - 1, 0);

    local_player.invincibility_timer = 3.0f;
    local_player.flags.invisible = 1;
    play_sound(SFX_DEATH);
    Animation* a = create_player_death_animation(e->position,
                                                 e->velocity,
                                                 e->angle_deg,
                                                 (SDL_Color) { LOCAL_PLAYER_COLOR, 255 });

    return ll_push_back(&state.animations, a);
}

void add_event_remote_player_death(V2f32 position, V2f32 velocity, f32 angle_deg) {
    Event* e_remote = malloc(sizeof(Event));
    e_remote->type = EVENT_REMOTE_PLAYER_DEATH;
    e_remote->event.remote_player_death.angle_deg = angle_deg;
    e_remote->event.remote_player_death.position = position;
    e_remote->event.remote_player_death.velocity = velocity;

    register_event_remote(e_remote);
}

func handle_event_remote_player_death(EventRemotePlayerDeath* e) {
    play_sound(SFX_DEATH);
    Animation* a = create_player_death_animation(e->position,
                                                 e->velocity,
                                                 e->angle_deg,
                                                 (SDL_Color) { REMOTE_PLAYER_COLOR, 255 });

    return ll_push_back(&state.animations, a);
}

void add_event_ufo_spawn(UfoForm form, V2f32 position, f32 speed, f32 direction_deg) {
    Event* e = malloc(sizeof(Event));

    e->type = EVENT_UFO_SPAWN;

    EventUfoSpawn* ufo = &e->event.ufo_spawn;
    ufo->form = form;
    ufo->position = position;

    f32 x = sinf(deg_to_rad(direction_deg)) * speed;
    f32 y = -cosf(deg_to_rad(direction_deg)) * speed;
    ufo->velocity = (V2f32) { x, y };

    register_event_local(e);
    register_event_remote(e);
}
func handle_event_ufo_spawn(EventUfoSpawn* e) {
    game_ufo.angle_deg = 0.0f; // ??

    game_ufo.form = e->form;
    game_ufo.position = e->position;
    game_ufo.velocity = e->velocity;
    game_ufo.ufo_timer = 0.0f;
    game_ufo.shoot_timer = 0.0f;

    return OK;
}

void add_event_ufo_kill(V2f32 ufo_position, V2f32 ufo_velocity, V2f32 bullet_velocity) {
    Event* e = malloc(sizeof(Event));
    e->type = EVENT_UFO_KILL;
    EventUfoKill* ufo = &e->event.ufo_kill;
    ufo->bullet_velocity = bullet_velocity;
    ufo->ufo_position = ufo_position;
    ufo->ufo_velocity = ufo_velocity;

    register_event_local(e);
    register_event_remote(e);
}
func handle_event_ufo_kill(EventUfoKill* e) {

    // TODO fancy animation
    funct_ret_t ret;
    Animation* a = create_sprinkle_animation(e->ufo_position, rand_i32(10, 20), rand_i32(0, 255));
    ret = ll_push_back(&state.animations, a);

    game_ufo.form = UFO_NONE;
    game_ufo.ufo_timer = 0.0f;

    return ret;
}

void register_event_local(Event* e) { queue_enqueue(&state.event_queue, e); }

void register_event_remote(Event* e) {
    if (!network_state.online_disable) {
        Packet* packet = packet_from_event(e);
        queue_enqueue(&network_state.transmit.tx_queue, packet);
    }
}
