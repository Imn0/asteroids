#if !defined(_WIN32) || defined(_MSC_VER)

#if !defined (_WIN32)  // all windows
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

#elif defined(_MSC_VER) // mscv has threads which are needed
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <threads.h>
#include <stdbool.h>
#include <windows.h>


#pragma comment(lib, "Ws2_32.lib")
#endif

#include "common.h"
#include "netcode.h"
#include "events.h"

i32 network_consumer(void* args) {
    (void)args;
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
                network_state.remote_player_state.player_state.angle = packet.angle;
                network_state.remote_player_state.player_state.x = packet.x;
                network_state.remote_player_state.player_state.y = packet.y;
                network_state.remote_player_state.player_state.v_x = packet.v_x;
                network_state.remote_player_state.player_state.v_y = packet.v_y;
                network_state.remote_player_state.player_state.flags = packet.flags;
                network_state.remote_player_state.player_state.lives = packet.lives;
                network_state.remote_player_state.player_state.score = packet.score;
                mtx_unlock(&network_state.remote_player_state.mutex);
                break;
            }
            case PACKET_EVENT:
            {
                Event* e = malloc(sizeof(Event));
                *e = p->payload.event_packet.event;
                register_event_local(e);
                break;
            }
            }
            free(p);
        }

#ifdef _WIN32
        // Sleep(1);
#else
        nanosleep(&(struct timespec) { .tv_nsec = 1000, .tv_sec = 0 }, NULL);
#endif
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

#ifdef _MSC_VER

    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return ERR_GENRIC_BAD;
    }
    SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return ERR_GENRIC_BAD;
    }
    // bind to open port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;  // Use 0 to bind to any open port

    if (bind(ListenSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return ERR_GENRIC_BAD;
    }
        // read port
    socklen_t addr_len = sizeof(addr);
    getsockname(ListenSocket, (struct sockaddr*)&addr, &addr_len);
    printf("server is on port %d\n", (int)ntohs(addr.sin_port));

    if (listen(ListenSocket, 1) == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return ERR_GENRIC_BAD;
    }

    // accept incoming connection
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    SOCKET ClientSocket = accept(ListenSocket, (struct sockaddr*)&caddr, &caddr_len);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return ERR_GENRIC_BAD;
    }


    printf("connection accepted\n");
    thrd_create(&network_state.receive.receive_thrd, receive_packets,
                (void*)(int*)ClientSocket);
    thrd_create(&network_state.transmit.transmit_thrd, send_packets,
                (void*)(int*)ClientSocket);
#else
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
#endif

    return OK;
}

func client_init(i32 client_port) {
    common_init();
#ifdef _MSC_VER

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return ERR_GENRIC_BAD;
    }

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return ERR_GENRIC_BAD;
    }

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)client_port);

    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        printf("inet_pton failed\n");
        closesocket(fd);
        WSACleanup();
        return ERR_GENRIC_BAD;
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("connect error: %d\n", WSAGetLastError());
        closesocket(fd);
        WSACleanup();
        return ERR_GENRIC_BAD;
    }

    thrd_create(&network_state.receive.receive_thrd, receive_packets,
                (void*)(int*)fd);
    thrd_create(&network_state.transmit.transmit_thrd, send_packets,
                (void*)(int*)fd);
#else
    network_state.socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)client_port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(network_state.socket_fd, (struct sockaddr*)&addr, sizeof(addr))) {
        return ERR_CONNECT;
    }


    thrd_create(&network_state.receive.receive_thrd, receive_packets,
                &network_state.socket_fd);
    thrd_create(&network_state.transmit.transmit_thrd, send_packets,
                &network_state.socket_fd);
#endif
    return OK;
}

i32 receive_packets(void* args) {

#ifndef _WIN32
    i32 socket_fd = *(i32*)args;
#else
    SOCKET socket = (SOCKET)(int*)args;
#endif

    while (atomic_load(&network_state.running)) {
        Packet* packet = malloc(sizeof(Packet));

#ifndef _WIN32
        ssize_t bytes_received = recv(socket_fd, packet, sizeof(Packet), 0);
#else 
        i32 bytes_received = recv(socket, (char*)packet, sizeof(Packet), 0);
#endif

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


#ifndef _WIN32
    close(socket_fd);
#else
    closesocket(socket);
#endif

    return OK;
}

i32 send_packets(void* args) {

#ifndef _WIN32
    i32 socket_fd = *(i32*)args;
#else
    SOCKET socket = (SOCKET)(int*)args;
#endif

    while (atomic_load(&network_state.running)) {

        Packet* p = NULL;
        while (1) {
        // while (queue_dequeue(&network_state.transmit.tx_queue, (void*)&p) == OK) {

            queue_dequeue(&network_state.transmit.tx_queue, (void*)&p);
            if (p == NULL) {
                continue;
            }


#ifndef _WIN32
            ssize_t bytes_sent = send(socket_fd, p, sizeof(Packet), 0);
#else
            int bytes_sent = send(socket, (char*)p, sizeof(Packet), 0);
#endif


            if (bytes_sent < 0) {
                if (errno == EINTR) {
                    // Interrupted system call, try again
                    continue;
                }
                else {
                 // Other error occurred
                    // perror("send error");
                    free(p);
                    printf("send error");
                    // return ERR_NETWORK;
                }
            }
            free(p);
        }
#ifdef _WIN32
        // Sleep(1);
#else
        nanosleep(&(struct timespec) { .tv_nsec = 1000, .tv_sec = 0 }, NULL);
#endif

    }

    return OK;
}
#endif
