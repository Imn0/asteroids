/** @file */
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


static u8 RNG_idx = 0;
static const u8 random_table[256] = {
    0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66 ,
    74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36 ,
    95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188 ,
    52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224 ,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242 ,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0 ,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235 ,
    25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113 ,
    94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75 ,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196 ,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113 ,
    80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241 ,
    24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224 ,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95 ,
    28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226 ,
    71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36 ,
    17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106 ,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136 ,
    120, 163, 236, 249
};

static inline f32 rand_float(f32 min, f32 max) {
    float scale = random_table[RNG_idx++] / (float)255;
    return min + scale * (max - min);
}

static inline f32 rand_float_seed(f32 min, f32 max, u8 seed) {
    float scale = random_table[seed] / (float)255;
    return min + scale * (max - min);
}
/**
 * @brief random integer in ```[min, max]```
 * 
 * @param min 
 * @param max 
 * @return i32 
 */
static inline i32 rand_i32(i32 min, i32 max) { return min + random_table[RNG_idx++] / (255 / (max - min + 1) + 1); }
static inline i32 rand_i32_seed(i32 min, i32 max, u8 seed) { return min + random_table[seed] / (255 / (max - min + 1) + 1); }
static inline f32 deg_to_rad(f32 d) { return d * (PI / 180.0f); }
static inline f32 dot(V2f32 v0, V2f32 v1) { return (v0.x * v1.x) + (v0.y * v1.y); }
static inline f32 length(V2f32 vl) { return sqrtf(dot(vl, vl)); }
static inline f32 rad_to_deg(f32 d) { return d * (180.0f / PI); }
static inline V2f32 rotate_point(V2f32 point, V2f32 center, float angle_rad) {
    f32 s = sinf(angle_rad);
    f32 c = cosf(angle_rad);

    V2f32 rotated = {
        .x = point.x - center.x,
        .y = point.y - center.y
    };

    f32 xnew = rotated.x * c - rotated.y * s;
    f32 ynew = rotated.x * s + rotated.y * c;

    rotated.x = xnew + center.x;
    rotated.y = ynew + center.y;
    return rotated;
}

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }




/* FIFO QUEUE */
typedef enum funct_ret_t {
    ERR_OK = 0,
    OK = 0,
    CONTAINER_EMPTY,
    RET_ENTITY_REMOVE,
    ERR_GENRIC_BAD = 0x10000000,
    ERR_NETWORK,
    ERR_MTX_INIT,
    ERR_BIND_SOC,
    ERR_LISTEN_SOC,
    ERR_CONNECT,
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

func queue_init(Queue queue[static 1], usize max_size);
func queue_enqueue(Queue queue[static 1], void* data);

/**
 * @brief Dequeues data from queue places it in data
 *
 * @param queue
 * @param data
 * @return ```OK```  when data was sucessfuly dequeued
 */
func queue_dequeue(Queue queue[static 1], void** data);
void queue_destroy(Queue queue[static 1]);
/* FIFO QUEUE */

/* LINKED LIST */
typedef enum {
    LL_ITER_H_TO_T,
    LL_ITER_T_TO_H,
} IterDirection;

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
    IterDirection direction;
} LinkedListIter;


func ll_iter_assign(LinkedListIter iter[static 1], LinkedList list[static 1]);
func ll_iter_assign_direction(LinkedListIter iter[static 1], LinkedList list[static 1], IterDirection direction);
bool ll_iter_end(LinkedListIter iter[static 1]);
func ll_iter_next(LinkedListIter iter[static 1]);
func ll_iter_prev(LinkedListIter iter[static 1]);
void* ll_iter_peek(LinkedListIter iter[static 1]);
void ll_iter_strip(LinkedListIter iter[static 1]);
void ll_init(LinkedList list[static 1]);
func ll_push_back_dtor(LinkedList list[static 1], void* data, void (*dtor)(void* data));
func ll_push_back(LinkedList list[static 1], void* data);
func ll_push_front_dtor(LinkedList list[static 1], void* data, void (*dtor)(void* data));
func ll_push_front(LinkedList list[static 1], void* data);

/**
 * @brief frees node with contents at current iter position, moves iter to previous node (in iteration direction)
 *
 * @param list
 * @param iter
 * @return func
 */
func ll_iter_remove_at(LinkedList list[static 1], LinkedListIter iter[static 1]);
func ll_pop_back(LinkedList list[static 1], void** data);
func ll_pop_front(LinkedList list[static 1], void** data);
void ll_destroy(LinkedList list[static 1]);
/* LINKED LIST */
