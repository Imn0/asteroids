#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <threads.h>
#include <errno.h>

#include "common.h"
#include "netcode.h"

i32 server_init() {
    atomic_store(&network_state.running, true);
    const int fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("bind error:");
        return -1;
    }
    socklen_t addr_len = sizeof(addr);
    getsockname(fd, (struct sockaddr*)&addr, &addr_len);
    printf("Server is on port %d\n", (int)ntohs(addr.sin_port));
    if (listen(fd, 1)) {
        perror("listen error:");
        return -1;
    }
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    network_state.socket_fd = accept(fd, (struct sockaddr*)&caddr, &caddr_len);
    printf("connection accepted\n");
    thrd_create(
        &network_state.receive.receive_thrd,
        receive_packets,
        &network_state.socket_fd);
    thrd_create(
        &network_state.transmit.transmit_thrd,
        send_packets,
        &network_state.socket_fd);

    return ERR_OK;
}

i32 client_init(i32 client_port) {
    atomic_store(&network_state.running, true);
    network_state.socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)client_port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(network_state.socket_fd, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("connect error:");
        return -1;
    }

    thrd_create(
        &network_state.receive.receive_thrd,
        receive_packets,
        &network_state.socket_fd);
    thrd_create(
        &network_state.transmit.transmit_thrd,
        send_packets,
        &network_state.socket_fd);

    return ERR_OK;
}

i32 receive_packets(void* args) {
    i32 socket_fd = *(i32*)args;

    while (atomic_load(&network_state.running)) {
        Packet* packet = malloc(sizeof(Packet));
        ssize_t bytes_received = recv(
            socket_fd,
            packet,
            sizeof(Packet),
            0);

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

        packet_queue_enqueue(&network_state.receive.rx_queue, packet);
    }

    close(socket_fd);
    return ERR_OK;
}

i32 send_packets(void* args) {
    i32 socket_fd = *(i32*)args;

    while (atomic_load(&network_state.running)) {
        Packet* p = NULL;
        while (packet_queue_dequeue(&network_state.transmit.tx_queue, &p)) {
            if (p == NULL) { continue; }
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
    }

    return ERR_OK;
}