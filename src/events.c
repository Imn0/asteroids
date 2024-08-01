#include "game.h"
#include "events.h"
#include "netcode.h"

void add_event_starting_rocks() {
    srand(time(NULL));
    for (i32 i = 0; i < 7; i++) {
        u8 seed = rand();

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
            .seed = seed };

        queue_enqueue(&state.event_queue, e);

        if (!network_state.online_disable) {
            Packet* packet = packet_from_event(e);
            queue_enqueue(&network_state.transmit.tx_queue, packet);
        }
    }
}

void add_event_random_rock(RockSize size) {
    // TODO
}

void add_event_random_rocks(i32 count, RockSize size) {
    // TODO
}


void add_event_from_rock_kill(RockSize killed_rock_size, V2f32 killed_pos, V2f32 killed_velocity) {

    if (killed_rock_size == ROCK_SMALL) { return; }

    RockSize new_size = ROCK_MEDIUM;
    if (killed_rock_size == ROCK_MEDIUM) { new_size = ROCK_SMALL; }


    // TODO BETTER LOGIC FOR VELOCITY  
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
            .initial_velocity = (V2f32){.x = 1.67 * killed_velocity.y,
                                        .y = 1.67 * killed_velocity.x},
            .seed = seed };

        queue_enqueue(&state.event_queue, e);
        if (!network_state.online_disable) {
            Packet* packet = packet_from_event(e);
            queue_enqueue(&network_state.transmit.tx_queue, packet);
        }
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
            .initial_velocity = (V2f32){.x = -1.67 * killed_velocity.y,
                                        .y = -1.67 * killed_velocity.x},
            .seed = seed };

        queue_enqueue(&state.event_queue, e);
        if (!network_state.online_disable) {
            Packet* packet = packet_from_event(e);
            queue_enqueue(&network_state.transmit.tx_queue, packet);
        }

    }
}
