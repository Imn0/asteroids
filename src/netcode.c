#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "netcode.h"

i32 network_consumer() {
    Packet* p = NULL;
    while (network_state.running) {
        while (queue_dequeue(&network_state.receive.rx_queue, (void*)&p) ==
               0) {
            if (p == NULL) {
                break; /* no updates */
            }
            switch (p->type) {
            case PACKET_PLAYER:
            {
                PlayerPacket packet = p->payload.player_packet;
                mtx_lock(&network_state.remote_player_state.mutex);
                network_state.remote_player_state.player_state.angle =
                    packet.angle;
                network_state.remote_player_state.player_state.x = packet.x;
                network_state.remote_player_state.player_state.y = packet.y;
                network_state.remote_player_state.player_state.v_x = packet.v_x;
                network_state.remote_player_state.player_state.v_y = packet.v_y;
                network_state.remote_player_state.player_state.flags =
                    packet.flags;
                mtx_unlock(&network_state.remote_player_state.mutex);
                break;
            }
            case PACKET_EVENT:
            {
                Event* e = malloc(sizeof(Event));
                *e = p->payload.event_packet.event;
                queue_enqueue(&state.event_queue, e);
                break;
            }
            }
            free(p);
        }
        nanosleep(&(struct timespec) { .tv_nsec = 1000, .tv_sec = 0 }, NULL);
    }
    return OK;
}

void network_consumer_init() {
    thrd_create(&network_state.consumer_thrd, network_consumer, NULL);
}

func common_init() {
    network_consumer_init();
    atomic_store(&network_state.running, true);

    if (mtx_init(&network_state.remote_player_state.mutex, mtx_plain) !=
        thrd_success) {
        return ERR_MTX_INIT;
    }

    queue_init(&network_state.receive.rx_queue, 64);
    queue_init(&network_state.transmit.tx_queue, 64);
    return OK;
}

func server_init() {
    ASSERT(common_init() == OK, "failed to init mtx");
    const i32 fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
        return ERR_BIND_SOC;
    }
    socklen_t addr_len = sizeof(addr);
    getsockname(fd, (struct sockaddr*)&addr, &addr_len);
    printf("Server is on port %d\n", (int)ntohs(addr.sin_port));
    if (listen(fd, 1)) {
        return ERR_LISTEN_SOC;
    }
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    network_state.socket_fd = accept(fd, (struct sockaddr*)&caddr, &caddr_len);
    printf("connection accepted\n");
    thrd_create(&network_state.receive.receive_thrd, receive_packets,
                &network_state.socket_fd);
    thrd_create(&network_state.transmit.transmit_thrd, send_packets,
                &network_state.socket_fd);

    return OK;
}

func client_init(i32 client_port) {
    common_init();
    network_state.socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)client_port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(network_state.socket_fd, (struct sockaddr*)&addr,
                sizeof(addr))) {
        return ERR_CONNECT;
    }

    thrd_create(&network_state.receive.receive_thrd, receive_packets,
                &network_state.socket_fd);
    thrd_create(&network_state.transmit.transmit_thrd, send_packets,
                &network_state.socket_fd);

    return OK;
}

i32 receive_packets(void* args) {
    i32 socket_fd = *(i32*)args;

    while (atomic_load(&network_state.running)) {
        Packet* packet = malloc(sizeof(Packet));
        ssize_t bytes_received = recv(socket_fd, packet, sizeof(Packet), 0);

        if (bytes_received == 0) {
            // Connection closed by the client
            printf("Client disconnected.\n");
            free(packet);
            break;
        }
        else if (bytes_received < 0) {
            if (errno == EINTR) {
                // Interrupted system call, try again
                free(packet);
                continue;
            }
            else {
             // Other error occurred
                perror("recv error");
                free(packet);
                break;
            }
        }
        queue_enqueue(&network_state.receive.rx_queue, packet);
    }

    close(socket_fd);
    return OK;
}

i32 send_packets(void* args) {
    i32 socket_fd = *(i32*)args;

    while (atomic_load(&network_state.running)) {

        Packet* p = NULL;
        while (queue_dequeue(&network_state.transmit.tx_queue, (void*)&p) ==
               OK) {
            if (p == NULL) {
                continue;
            }
            ssize_t bytes_sent = send(socket_fd, p, sizeof(Packet), 0);
            if (bytes_sent < 0) {
                if (errno == EINTR) {
                    // Interrupted system call, try again
                    continue;
                }
                else {
                 // Other error occurred
                    perror("send error");
                    free(p);
                    return ERR_NETWORK;
                }
            }
            free(p);
        }
        nanosleep(&(struct timespec) { .tv_nsec = 1000, .tv_sec = 0 }, NULL);
    }

    return OK;
}
