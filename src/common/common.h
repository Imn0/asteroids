#pragma once

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <sys/types.h>
#include <threads.h>
#include <SDL2/SDL.h>

#include "settings.h"

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef size_t usize;
typedef ssize_t isize;

typedef struct _v2_f32 { f32 x; f32 y; } V2f32;
typedef struct _v2_i32 { i32 x; i32 y; } V2i32;

#define PI 3.14159265359f
#define TAU (2.0f * PI)
#define PI_2 (PI / 2.0f)
#define PI_4 (PI / 4.0f)

static inline float deg_to_rad(f32 d) { return d * (PI / 180.0f); }
static inline float dot(V2f32 v0, V2f32 v1) { return (v0.x * v1.x) + (v0.y * v1.y); }
static inline float length(V2f32 vl) { return sqrtf(dot(vl, vl)); }
static inline float rad_to_deg(f32 d) { return d * (180.0f / PI); }

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }


/* FIFO QUEUE */

typedef enum funct_ret_t {
    ERR_OK = 0,
    ERR_GENRIC_BAD,
    QUEUE_OK,
    QUEUE_EMPTY,
    ERR_NETWORK,
    ERR_EMPTY,
    RET_ENTITY_REMOVE,
    ERR_MTX_INIT,
} func;
typedef enum funct_ret_t funct_ret_t;


typedef struct QueueNode {
    void* data;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    struct QueueNode* front;
    struct QueueNode* rear;
    mtx_t mutex;
    cnd_t not_empty;
    usize size;
    usize max_size;
} Queue;

i32 queue_init(Queue queue[static 1], usize max_size);
i32 queue_enqueue(Queue queue[static 1], void* data);
i32 queue_dequeue(Queue queue[static 1], void** data);
void queue_destroy(Queue queue[static 1]);
/* FIFO QUEUE */

/* LINKED LIST */

typedef struct LinkedListNode {
    void* data;
    struct LinkedListNode* next;
    struct LinkedListNode* prev;
    void (*dtor)(void* data);
} LinkedListNode;

typedef struct {
    struct LinkedListNode* head;
    struct LinkedListNode* tail;
    usize size;
} LinkedList;

typedef struct {
    LinkedListNode* node;
} LinkedListIter;

i32 ll_iter_assign(LinkedListIter iter[static 1], LinkedList list[static 1]);
bool ll_iter_end(LinkedListIter iter[static 1]);
i32 ll_iter_next(LinkedListIter iter[static 1]);
i32 ll_iter_prev(LinkedListIter iter[static 1]);
void* ll_iter_peek(LinkedListIter iter[static 1]);
void ll_iter_strip(LinkedListIter iter[static 1]);
i32 ll_init(LinkedList list[static 1]);
i32 ll_push_dtor(LinkedList list[static 1], void* data, void (*dtor)(void* data));
i32 ll_push(LinkedList list[static 1], void* data);
i32 ll_iter_remove_at(LinkedList list[static 1], LinkedListIter iter[static 1]);
i32 ll_pop(LinkedList list[static 1], void** data);
void ll_destroy(LinkedList list[static 1]);
/* LINKED LIST */
