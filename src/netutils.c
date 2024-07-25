#include "netcode.h"

Packet* packet_from_player(Player* player) {
    Packet* packet = malloc(sizeof(Packet));
    packet->type = PACKET_PLAYER;
    packet->time_stamp = -1;
    packet->size = sizeof(PlayerPacket);
    packet->payload.player = (PlayerPacket){
        .angle = player->angle_deg,
        .v_x = player->velocity.x,
        .v_y = player->velocity.y,
        .x = player->position.x,
        .y = player->position.y,
    };
    memcpy(&packet->payload.player.flags, &player->flags, sizeof(player_flags_t));

    return packet;
}

void update_player_from_queue(Queue* queue, Player* player) {
    Packet* p = NULL;
    while (queue_dequeue(queue, (void*)&p)) {
    }

    if (p == NULL) { return; }

    PlayerPacket packet = p->payload.player;
    player->angle_deg = packet.angle;
    player->position.x = packet.x;
    player->position.y = packet.y;

    player->velocity.x = packet.v_x;
    player->velocity.y = packet.v_y;
    printf("freeing %p\n", p);
    free(p);
}