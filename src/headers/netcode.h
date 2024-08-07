#pragma once

#include <stdatomic.h>

#if !defined(WIN32) || defined(_MSC_VER) // MSCV and linux/mac
#include <threads.h>
#else
#include "fake_threads.h" // MinGW
#endif

#include "common.h"
#include "events.h"
#include "game.h"
#include "player.h"
#include "ufo.h"

typedef enum PacketType { PACKET_PLAYER, PACKET_EVENT, PACKET_UFO } PacketType;

typedef struct {
    f32 x, y, angle, v_x, v_y;
    player_flags_t flags;
    i32 score;
    i32 lives;
} PlayerPacket;

typedef struct {
    Event event;
} EventPacket;

typedef struct {
    f32 x, y, angle, v_x, v_y;
    UfoForm current_form;
} UfoPacket;

typedef struct {
    UfoPacket ufo;
    mtx_t mutex;
} UfoState;

typedef struct {
    PacketType type;
    i32 time_stamp;
    i32 size;
    union {
        PlayerPacket player_packet;
        EventPacket event_packet;
        UfoPacket ufo_packet;
    } payload;
} Packet;

typedef struct RemotePlayerState {
    PlayerPacket player_state;
    mtx_t mutex;
} RemotePlayerState;

typedef struct {
    // netcode
    bool is_server;
    bool online_disable;
    atomic_int running;
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
    UfoState ufo_state;
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
Packet* packet_from_ufo(Ufo* ufo);
