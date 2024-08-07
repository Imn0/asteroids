#include "common.h"

func ll_iter_assign(LinkedListIter* iter, LinkedList* list) {
    if (list->size == 0) {
        iter->node = NULL;
        return CONTAINER_EMPTY;
    }
    iter->node = list->head;
    iter->direction = LL_ITER_H_TO_T;
    return OK;
}

func ll_iter_assign_direction(LinkedListIter* iter,
                              LinkedList* list,
                              IterDirection direction) {
    if (list->size == 0) {
        iter->node = NULL;
        return CONTAINER_EMPTY;
    }
    if (direction == LL_ITER_H_TO_T) {
        iter->node = list->head;
    } else {
        iter->node = list->tail;
    }
    iter->direction = direction;
    return OK;
}

bool ll_iter_end(LinkedListIter* iter) { return iter->node == NULL; }

func ll_iter_next(LinkedListIter* iter) {
    if (iter->node == NULL) {
        return CONTAINER_EMPTY;
    }
    if (iter->direction == LL_ITER_H_TO_T) {
        iter->node = iter->node->next;
    } else {
        iter->node = iter->node->prev;
    }
    return OK;
}

func ll_iter_prev(LinkedListIter* iter) {
    if (iter->node == NULL) {
        return CONTAINER_EMPTY;
    }
    if (iter->direction == LL_ITER_H_TO_T) {
        iter->node = iter->node->prev;
    } else {
        iter->node = iter->node->next;
    }
    return OK;
}

void ll_iter_strip(LinkedListIter* iter) { iter->node = NULL; }

void* ll_iter_peek(LinkedListIter* iter) {
    if (iter->node == NULL) {
        return NULL;
    }
    return iter->node->data;
}

void ll_init(LinkedList* list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

func ll_push_back_dtor(LinkedList* list, void* data, void (*dtor)(void* data)) {
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
    } else {
        list->tail->next = node;
    }
    list->tail = node;
    list->size++;
    return ERR_OK;
}

func ll_push_back(LinkedList* list, void* data) {
    return ll_push_back_dtor(list, data, &free);
}

func ll_push_front_dtor(LinkedList* list,
                        void* data,
                        void (*dtor)(void* data)) {
    LinkedListNode* node = malloc(sizeof(LinkedListNode));
    if (node == NULL) {
        return ERR_GENRIC_BAD;
    }
    node->data = data;
    node->dtor = dtor;
    node->prev = NULL;
    node->next = list->head;

    if (list->size == 0) {
        list->tail = node;
    } else {
        list->head->prev = node;
    }
    list->head = node;
    list->size++;
    return OK;
}

func ll_push_front(LinkedList* list, void* data) {
    return ll_push_front_dtor(list, data, &free);
}

func ll_iter_remove_at(LinkedList* list, LinkedListIter* iter) {
    if (iter->node == NULL) {
        return CONTAINER_EMPTY;
    }

    LinkedListNode* node_to_remove = iter->node;

    LinkedListNode* next_node = (iter->direction == LL_ITER_H_TO_T)
                                        ? node_to_remove->next
                                        : node_to_remove->prev;

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

    iter->node = next_node;

    node_to_remove->dtor(node_to_remove->data);

    list->size--;

    return OK;
}
func ll_pop_back(LinkedList* list, void** data) {
    if (list->size == 0) {
        return CONTAINER_EMPTY;
    }
    LinkedListNode* node = list->tail;
    *data = node->data;
    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = node->prev;
        list->tail->next = NULL;
    }
    free(node);
    list->size--;
    return OK;
}

func ll_pop_front(LinkedList* list, void** data) {
    if (list->size == 0) {
        return CONTAINER_EMPTY;
    }
    LinkedListNode* node = list->head;
    *data = node->data;
    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = node->next;
        list->head->prev = NULL;
    }
    free(node);
    list->size--;
    return OK;
}

void ll_destroy(LinkedList* list) {
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
