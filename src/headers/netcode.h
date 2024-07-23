#pragma once

#include <threads.h>

#include "common.h"

typedef struct {
    PacketQueue* receive_packet_queue;
    bool* running;
} ThreadArgs;

i32 server_init();
i32 client_init(i32 client_port);
i32 receive_packets(void* arg);
i32 send_packets(void* args);

i32 packet_queue_init(PacketQueue* queue, usize max_size);
i32 packet_queue_enqueue(PacketQueue* queue, Packet* packet);
i32 packet_queue_dequeue(PacketQueue* queue, Packet** packet);
void packet_queue_destroy(PacketQueue* queue);

Packet* packet_from_player(Player* player);
void update_player_from_queue(PacketQueue* queue, Player* player);