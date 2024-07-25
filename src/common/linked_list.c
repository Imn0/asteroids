#include "common.h"

i32 ll_iter_assign(LinkedListIter iter[static 1], LinkedList list[static 1]) {
    if (list->size == 0) {
        iter->node = NULL;
        return ERR_EMPTY;
    }
    iter->node = list->head;
    return ERR_OK;
}

bool ll_iter_end(LinkedListIter iter[static 1]){
    return iter->node == NULL;
}

i32 ll_iter_next(LinkedListIter iter[static 1]) {
    if (iter->node == NULL) {
        return ERR_EMPTY;
    }
    iter->node = iter->node->next;
    return ERR_OK;
}

i32 ll_iter_prev(LinkedListIter iter[static 1]) {
    if (iter->node == NULL) {
        return ERR_EMPTY;
    }
    iter->node = iter->node->prev;
    return ERR_OK;
}

void ll_iter_strip(LinkedListIter iter[static 1]) {
    iter->node = NULL;
}

void* ll_iter_peek(LinkedListIter iter[static 1]) {
    if (iter->node == NULL) {
        return NULL;
    }
    return iter->node->data;
}

i32 ll_init(LinkedList list[static 1]) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return ERR_OK;
}

i32 ll_push_dtor(LinkedList list[static 1], void* data, void (*dtor)(void* data)) {
    LinkedListNode* node = malloc(sizeof(LinkedListNode));
    if (node == NULL) {
        return ERR_GENRIC_BAD;
    }
    node->data = data;
    node->dtor = dtor;
    node->next = NULL;
    node->prev = list->tail;
    if (list->size == 0) {
        list->head = node;
    }
    else {
        list->tail->next = node;
    }
    list->tail = node;
    list->size++;
    return ERR_OK;
}

i32 ll_push(LinkedList list[static 1], void* data) {
    return ll_push_dtor(list, data, &free);
}

i32 ll_iter_remove_at(LinkedList list[static 1], LinkedListIter iter[static 1]){
    if (iter->node == NULL) {
        return ERR_EMPTY;
    }

    LinkedListNode* node_to_remove = iter->node;

    if (node_to_remove->prev) {
        node_to_remove->prev->next = node_to_remove->next;
    } else {
        list->head = node_to_remove->next;
    }

    if (node_to_remove->next) {
        node_to_remove->next->prev = node_to_remove->prev;
    } else {
        list->tail = node_to_remove->prev;
    }

    iter->node = node_to_remove->next;

    if (node_to_remove->dtor) {
        node_to_remove->dtor(node_to_remove->data);
    }
    free(node_to_remove);

    list->size--;

    return ERR_OK;
}

i32 ll_pop(LinkedList list[static 1], void** data) {
    if (list->size == 0) {
        return ERR_EMPTY;
    }
    LinkedListNode* node = list->tail;
    *data = node->data;
    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    }
    else {
        list->tail = node->prev;
        list->tail->next = NULL;
    }
    free(node);
    list->size--;
    return ERR_OK;
}

void ll_destroy(LinkedList list[static 1]) {
    LinkedListNode* node = list->head;
    while (node != NULL) {
        LinkedListNode* next = node->next;
        node->dtor(node->data);
        node = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}