#include "game.h"
#include "events.h"
#include "netcode.h"

void add_event_starting_rocks() {

    ASSERT(network_state.is_server, "client tried to add add starting rocks\n");


    for (i32 i = 0; i < 7; i++) {
        u8 seed = rand_i32(0, 255);

        // ! functions to make events for spawning rocks 

        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_NEW_ROCK;
        EventRock* rock = &e->event.rock;
        *rock = (EventRock){
            .rock_size = rand_i32(1,2),
            .jaggedness = rand_float(0.7f, 0.95f),
            .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
            .position = (V2f32){.x = rand_float_range(2, 0.0f, WINDOW_WIDTH / 2 - 50.0f, WINDOW_WIDTH / 2 + 50.0f, (f32)WINDOW_WIDTH),
                                .y = rand_float_range(2, 0.0f, WINDOW_HEIGHT / 2 - 50.0f, WINDOW_HEIGHT / 2 + 50.0f, (f32)WINDOW_HEIGHT)},
            .initial_velocity = (V2f32){.x = rand_float(-25.0f, 25.0f),
                                        .y = rand_float(-25.0f, 25.0f)},
            .seed = seed,
            .id = rand() + 1 };

        register_event_local(e);
        register_event_remote(e);
    }
}

void add_event_random_rock(RockSize size) {

    ASSERT(network_state.is_server, "client tried to add a rock\n");

    f32 x = 0.0f;
    f32 y = 0.0f;

    f32 min_dist = 250.0f;
    i32 max_tries = 32;
    i32 i = 0;

    do {
        i32 wall = rand_i32(0, 3);
        switch (wall) {
        case 0:
            x = 0.0f;
            y = rand_float(0.0f, WINDOW_HEIGHT);
            break;
        case 1:
            x = WINDOW_WIDTH;
            y = rand_float(0.0f, WINDOW_HEIGHT);
            break;
        case 2:
            x = rand_float(0.0f, WINDOW_WIDTH);
            y = 0.0f;
            break;
        case 3:
        default:
            x = rand_float(0.0f, WINDOW_WIDTH);
            y = WINDOW_WIDTH;
            break;
        }
        if (dist((V2f32) { x, y }, local_player.position) > min_dist
            && (network_state.online_disable || dist((V2f32) { x, y }, remote_player.position) > min_dist)) {
            break;
        }


    } while (i++ < max_tries);

    u8 seed = rand_i32(0, 255);

    Event* e = malloc(sizeof(Event));
    e->type = EVENT_TYPE_NEW_ROCK;
    EventRock* rock = &e->event.rock;
    *rock = (EventRock){
        .rock_size = size,
        .jaggedness = rand_float(0.7f, 0.95f),
        .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
        .position = (V2f32){ x, y },
        .initial_velocity = (V2f32){.x = rand_float(-25.0f, 25.0f),
                                    .y = rand_float(-25.0f, 25.0f)},
        .seed = seed,
        .id = rand() + 1 };
    register_event_local(e);
    register_event_remote(e);

}

void add_event_random_rocks(i32 count, RockSize size) {
    for (i32 i = 0; i < count; i++) {
        add_event_random_rock(size);
    }
}

void add_event_from_rock_kill(RockSize killed_rock_size, V2f32 killed_pos, V2f32 killed_velocity) {

    ASSERT(network_state.is_server, "client tried to add a rock kill\n");


    if (killed_rock_size == ROCK_SMALL) { return; }

    RockSize new_size = ROCK_MEDIUM;
    if (killed_rock_size == ROCK_MEDIUM) { new_size = ROCK_SMALL; }

    {
        u8 seed = rand_i32(0, 255);
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_NEW_ROCK;
        EventRock* rock = &e->event.rock;
        *rock = (EventRock){
            .rock_size = new_size,
            .jaggedness = rand_float(0.7f, 0.95f),
            .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
            .position = killed_pos,
            .initial_velocity = (V2f32){.x = rand_float_range(2, -2.7, -2.3, 2.3, 2.7) * killed_velocity.y,
                                        .y = rand_float_range(2, -2.7, -2.3, 2.3, 2.7) * killed_velocity.x},
            .seed = seed,
            .id = rand() + 1 };

        register_event_local(e);
        register_event_remote(e);
    }
    {
        u8 seed = rand_i32(0, 255);
        Event* e = malloc(sizeof(Event));
        e->type = EVENT_TYPE_NEW_ROCK;
        EventRock* rock = &e->event.rock;
        *rock = (EventRock){
            .rock_size = new_size,
            .jaggedness = rand_float(0.7f, 0.95f),
            .num_vertices = rand_i32(8, ASTEROID_MAX_POINTS - 1),
            .position = killed_pos,
            .initial_velocity = (V2f32){.x = -rand_float_range(2, -2.7, -2.3, 2.3, 2.7) * killed_velocity.y,
                                        .y = -rand_float_range(2, -2.7, -2.3, 2.3, 2.7) * killed_velocity.x},
            .seed = seed,
            .id = rand() + 1 };

        register_event_local(e);
        register_event_remote(e);

    }
}

void register_event_local(Event* e) {
    queue_enqueue(&state.event_queue, e);
}

void register_event_remote(Event* e) {
    if (!network_state.online_disable) {
        Packet* packet = packet_from_event(e);
        queue_enqueue(&network_state.transmit.tx_queue, packet);
    }
}



Entity* create_bullet_from_event(Event* e) {
    ASSERT(e->type == EVENT_TYPE_SHOOT, "invalid event passed to create_bullet_from_event\n");
    Entity* entity = entity_create_bullet(e->event.shoot);
    return entity;
}

Entity* create_rock_from_event(Event* e) {
    ASSERT(e->type == EVENT_TYPE_NEW_ROCK, "invalid event passed to create_rock_from_event\n");
    Entity* entity = entity_create_rock(e->event.rock); // EntityCreateRockArgs is same as EventRock

    return entity;
}


void remote_kill_entity(i32 id, EntityRemoveAnimation animation_type) {
    ASSERT(network_state.is_server, "client tried to kill entity on remote\n");

    Event* e = malloc(sizeof(Event));
    e->type = EVENT_REMOVE_ENTITY;
    e->event.remove_entity.animation = animation_type;
    e->event.remove_entity.id_to_remove = id;
    register_event_remote(e);
}

void remote_add_points(i32 amount_to_add) {

    ASSERT(network_state.is_server, "client tried to kill entity on remote\n");

    Event* e = malloc(sizeof(Event));
    e->type = EVENT_PLAYER_ADD_POINTS;
    e->event.add_points.points_to_add = amount_to_add;
    register_event_remote(e);
}
