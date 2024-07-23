#pragma once

#include <threads.h>
#include <stdatomic.h>

#include "common.h"
#include "player.h"

enum PacketType {
    PACKET_PLAYER
};

typedef struct {
    f32 x, y, angle, v_x, v_y;
    player_flags_t flags;
} PlayerPacket;

typedef struct {
    u8 type;
    i32 time_stamp;
    i32 size;
    union {
        PlayerPacket player;
        /* MORE */
    } payload;
} Packet;

typedef struct PacketQueueNode {
    Packet* packet;
    struct PacketQueueNode* next;
} PacketQueueNode;

typedef struct {
    struct PacketQueueNode* front;
    struct PacketQueueNode* rear;
    mtx_t mutex;
    cnd_t not_empty;
    usize size;
    usize max_size;
} PacketQueue;

typedef struct {
    // netcode
    bool is_server;
    atomic_bool running;
    i32 socket_fd;
    struct {
        thrd_t receive_thrd;
        PacketQueue rx_queue;
    }receive;
    struct {
        thrd_t transmit_thrd;
        PacketQueue tx_queue;
    }transmit;
    thrd_t accept_thrd;
} NetworkState;


typedef struct {
    PacketQueue* receive_packet_queue;
    bool* running;
} ThreadArgs;

extern NetworkState network_state;

i32 server_init();
i32 client_init(i32 client_port);
i32 accept_connections(void* arg);
i32 receive_packets(void* arg);
i32 send_packets(void* args);

i32 packet_queue_init(PacketQueue* queue, usize max_size);
i32 packet_queue_enqueue(PacketQueue* queue, Packet* packet);
i32 packet_queue_dequeue(PacketQueue* queue, Packet** packet);
void packet_queue_destroy(PacketQueue* queue);

Packet* packet_from_player(Player* player);
void update_player_from_queue(PacketQueue* queue, Player* player);