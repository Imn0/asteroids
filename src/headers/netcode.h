#pragma once

#include <stdatomic.h>
#include <threads.h>

#include "common.h"
#include "game.h"
#include "player.h"

typedef enum PacketType { PACKET_PLAYER, PACKET_EVENT } PacketType;

typedef struct {
    f32 x, y, angle, v_x, v_y;
    player_flags_t flags;
} PlayerPacket;

typedef struct {
    Event event;
} EventPacket;

typedef struct {
    PacketType type;
    i32 time_stamp;
    i32 size;
    union {
        PlayerPacket player_packet;
        EventPacket event_packet;
        /* MORE */
    } payload;
} Packet;

typedef struct RemotePlayerState {
    mtx_t mutex;
    PlayerPacket player_state;
} RemotePlayerState;

typedef struct {
    // netcode
    bool is_server;
    bool online_disable;
    atomic_bool running;
    i32 socket_fd;
    struct {
        thrd_t receive_thrd;
        Queue rx_queue;
    } receive;
    struct {
        thrd_t transmit_thrd;
        Queue tx_queue;
    } transmit;
    thrd_t consumer_thrd;
    RemotePlayerState remote_player_state;
} NetworkState;

typedef struct {
    Queue* receive_packet_queue;
    bool* running;
} ThreadArgs;

extern NetworkState network_state;

func server_init();
func client_init(i32 client_port);
i32 receive_packets(void* arg);
i32 send_packets(void* args);

void network_consumer_init();

Packet* packet_from_player(Player* player);
Packet* packet_from_event(Event* event);

void update_player_from_queue(Queue* queue, Player* player);
