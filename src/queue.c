#include "common.h"
#include "netcode.h"

Packet* packet_from_player(Player* player) {
    Packet* packet = malloc(sizeof(Packet));
    packet->type = PACKET_PLAYER;
    packet->time_stamp = -1;
    packet->size = sizeof(PlayerPacket);
    packet->payload.player = (PlayerPacket){
        .angle = player->angle_deg,
        .rotation_speed = player->rotation_speed,
        .speed = player->speed,
        .x = player->position.x,
        .y = player->position.y
    };

    return packet;
}

i32 packet_queue_init(PacketQueue* queue, usize max_size) {
    queue->front = queue->rear = NULL;
    queue->size = 0;
    queue->max_size = max_size;

    if (mtx_init(&queue->mutex, mtx_plain) != thrd_success) {
        return false;
    }

    if (cnd_init(&queue->not_empty) != thrd_success) {
        mtx_destroy(&queue->mutex);
        return false;
    }

    return true;
}

size_t queue_remove_front(PacketQueue* queue, size_t count) {
    mtx_lock(&queue->mutex);

    size_t removed = 0;
    while (removed < count && queue->front != NULL) {
        PacketQueueNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
        removed++;
    }

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    queue->size -= removed;

    mtx_unlock(&queue->mutex);

    return removed;
}

i32 packet_queue_enqueue(PacketQueue* queue, Packet* packet) {
    PacketQueueNode* new_node = malloc(sizeof(PacketQueueNode));
    if (!new_node) return false;

    new_node->packet = packet;
    new_node->next = NULL;
    mtx_lock(&queue->mutex);

    // Check if the queue has reached its maximum size
    if (queue->size >= queue->max_size) {
        size_t to_remove = queue->max_size/4;
        mtx_unlock(&queue->mutex);
        queue_remove_front(queue, to_remove);
        mtx_lock(&queue->mutex);
    }

    if (queue->rear == NULL) {
        queue->front = queue->rear = new_node;
    }
    else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }

    queue->size++;

    cnd_signal(&queue->not_empty);
    mtx_unlock(&queue->mutex);
    return true;
}

i32 packet_queue_dequeue(PacketQueue* queue, Packet** packet) {
    mtx_lock(&queue->mutex);
    
    if (queue->front == NULL) {
        mtx_unlock(&queue->mutex);
        return false;  // Queue is empty
    }
    
    PacketQueueNode* temp = queue->front;
    *packet = temp->packet;
    
    queue->front = queue->front->next;
    
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    queue->size--;
    
    mtx_unlock(&queue->mutex);
    
    free(temp);
    return true;
}


void packet_queue_destroy(PacketQueue* queue) {
    mtx_lock(&queue->mutex);

    while (queue->front != NULL) {
        PacketQueueNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }

    mtx_unlock(&queue->mutex);
    mtx_destroy(&queue->mutex);
    cnd_destroy(&queue->not_empty);
}

void update_player_from_queue(PacketQueue* queue, Player* player) {
    // TODO for now in queu are only player packets 
    Packet* p = NULL;
    while (packet_queue_dequeue(queue, &p)) {

    }

    if (p == NULL) { return; }

    PlayerPacket packet = p->payload.player;
    player->angle_deg = packet.angle;
    player->position.x = packet.x;
    player->position.y = packet.y;
    player->rotation_speed = packet.rotation_speed;
    player->speed = packet.speed;
    printf("freeing %ud\n", p);
    free(p);
}