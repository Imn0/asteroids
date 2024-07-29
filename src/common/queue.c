#include "common.h"
#include "netcode.h"
#include "player.h"

func queue_init(Queue queue[static 1], usize max_size) {
    queue->front = queue->rear = NULL;
    queue->size = 0;
    queue->max_size = max_size;

    if (mtx_init(&queue->mutex, mtx_plain) != thrd_success) {
        return ERR_MTX_INIT;
    }

    if (cnd_init(&queue->not_empty) != thrd_success) {
        mtx_destroy(&queue->mutex);
        return ERR_MTX_INIT;
    }

    return OK;
}

usize queue_remove_front(Queue queue[static 1], usize count) {
    mtx_lock(&queue->mutex);

    usize removed = 0;
    while (removed < count && queue->front != NULL) {
        QueueNode* temp = queue->front;
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

func queue_enqueue(Queue queue[static 1], void* data) {
    QueueNode* new_node = malloc(sizeof(QueueNode));
    if (!new_node)
        return ERR_GENRIC_BAD;

    new_node->data = data;
    new_node->next = NULL;
    mtx_lock(&queue->mutex);

    // Check if the queue has reached its maximum size
    if (queue->size >= queue->max_size) {
        size_t to_remove = queue->max_size / 4;
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
    return OK;
}

func queue_dequeue(Queue queue[static 1], void** data) {
    mtx_lock(&queue->mutex);

    if (queue->front == NULL) {
        mtx_unlock(&queue->mutex);
        return CONTAINER_EMPTY; // Queue is empty
    }

    QueueNode* temp = queue->front;
    *data = temp->data;

    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    queue->size--;

    mtx_unlock(&queue->mutex);

    free(temp);
    return OK;
}

void queue_destroy(Queue queue[static 1]) {
    mtx_lock(&queue->mutex);

    while (queue->front != NULL) {
        QueueNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }

    mtx_unlock(&queue->mutex);
    mtx_destroy(&queue->mutex);
    cnd_destroy(&queue->not_empty);
}
