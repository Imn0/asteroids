#include "netcode.h"

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

