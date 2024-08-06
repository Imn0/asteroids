#include "netcode.h"

Packet* packet_from_ufo(Ufo* ufo){
    Packet* packet = malloc(sizeof(Packet));
    if (packet == NULL) {
        return NULL;
    }

    packet->type = PACKET_UFO;

    packet->payload.ufo_packet.angle = ufo->angle_deg;
    packet->payload.ufo_packet.current_form = ufo->form;
    packet->payload.ufo_packet.x = ufo->position.x;
    packet->payload.ufo_packet.y = ufo->position.y;
    packet->payload.ufo_packet.v_x = ufo->velocity.x;
    packet->payload.ufo_packet.v_y = ufo->velocity.y;

    return packet;
}

Packet* packet_from_player(Player* player) {
    Packet* packet = malloc(sizeof(Packet));
    if (packet == NULL) {
        return NULL;
    }

    packet->type = PACKET_PLAYER;
    packet->time_stamp = -1;
    packet->size = sizeof(PlayerPacket);
    packet->payload.player_packet = (PlayerPacket){
        .angle = player->angle_deg,
        .v_x = player->velocity.x,
        .v_y = player->velocity.y,
        .x = player->position.x,
        .y = player->position.y,
    };
    packet->payload.player_packet.flags = player->flags;
    packet->payload.player_packet.lives = player->lives;
    packet->payload.player_packet.score = player->score;

    return packet;
}

Packet* packet_from_event(Event* event) {
    Packet* packet = malloc(sizeof(Packet));
    if (packet == NULL) {
        return NULL;
    }

    packet->type = PACKET_EVENT;
    packet->time_stamp = -1;
    packet->size = sizeof(EventPacket);
    packet->payload.event_packet.event = *event;

    return packet;
}

