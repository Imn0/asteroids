/** @file */
#pragma once

#include <SDL.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#if !defined(WIN32) || defined(_MSC_VER) // MSCV and linux/mac
#include <threads.h>
#else
#include "fake_threads.h" // MinGW
#endif

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

typedef struct _v2_f32 {
    f32 x;
    f32 y;
} V2f32;
typedef struct _v2_i32 {
    i32 x;
    i32 y;
} V2i32;

#define PI 3.14159265359f
#define TAU (2.0f * PI)
#define PI_2 (PI / 2.0f)
#define PI_4 (PI / 4.0f)

f32 rand_float(f32 min, f32 max);
f32 rand_float_seed(f32 min, f32 max, u8 seed);

/**
 * @brief random integer in ```[min, max]```
 *
 * @param min
 * @param max
 * @return i32
 */
i32 rand_i32(i32 min, i32 max);
i32 rand_i32_seed(i32 min, i32 max, u8 seed);
f32 rand_float_range(i32 num_ranges, ...);
f32 rand_float_range_seed(u8 seed, i32 num_ranges, ...);

#ifndef _MSC_VER
#define min(_a, _b)                                                                                \
    ({                                                                                             \
        __typeof__(_a) __a = (_a), __b = (_b);                                                     \
        __a < __b ? __a : __b;                                                                     \
    })
#define max(_a, _b)                                                                                \
    ({                                                                                             \
        __typeof__(_a) __a = (_a), __b = (_b);                                                     \
        __a > __b ? __a : __b;                                                                     \
    })
#endif

static inline f32 deg_to_rad(f32 d) { return d * (PI / 180.0f); }
static inline f32 dot(V2f32 v0, V2f32 v1) { return (v0.x * v1.x) + (v0.y * v1.y); }
static inline f32 length(V2f32 vl) { return sqrtf(dot(vl, vl)); }
static inline f32 dist(V2f32 v0, V2f32 v1) {
    return length((V2f32) { .x = v1.x - v0.x, .y = v1.y - v0.y });
}
static inline f32 rad_to_deg(f32 d) { return d * (180.0f / PI); }
static inline V2f32 rotate_point(V2f32 point, V2f32 center, f32 angle_rad) {
    f32 s = sinf(angle_rad);
    f32 c = cosf(angle_rad);

    V2f32 rotated = { .x = point.x - center.x, .y = point.y - center.y };

    f32 xnew = rotated.x * c - rotated.y * s;
    f32 ynew = rotated.x * s + rotated.y * c;

    rotated.x = xnew + center.x;
    rotated.y = ynew + center.y;
    return rotated;
}

#define ASSERT(_e, ...)                                                                            \
    if (!(_e)) {                                                                                   \
        fprintf(stderr, __VA_ARGS__);                                                              \
        exit(1);                                                                                   \
    }

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

func queue_init(Queue* queue, usize max_size);
func queue_enqueue(Queue* queue, void* data);

/**
 * @brief Dequeues data from queue places it in data
 *
 * @param queue
 * @param data
 * @return ```OK```  when data was sucessfuly dequeued
 */
func queue_dequeue(Queue* queue, void** data);
void queue_destroy(Queue* queue);
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

func ll_iter_assign(LinkedListIter* iter, LinkedList* list);
func ll_iter_assign_direction(LinkedListIter* iter, LinkedList* list, IterDirection direction);
bool ll_iter_end(LinkedListIter* iter);
func ll_iter_next(LinkedListIter* iter);
func ll_iter_prev(LinkedListIter* iter);
void* ll_iter_peek(LinkedListIter* iter);
void ll_iter_strip(LinkedListIter* iter);
void ll_init(LinkedList* list);
func ll_push_back_dtor(LinkedList* list, void* data, void (*dtor)(void* data));
func ll_push_back(LinkedList* list, void* data);
func ll_push_front_dtor(LinkedList* list, void* data, void (*dtor)(void* data));
func ll_push_front(LinkedList* list, void* data);

/**
 * @brief frees node with contents at current iter position, moves iter to
 * previous node (in iteration direction)
 *
 * @param list
 * @param iter
 * @return func
 */
func ll_iter_remove_at(LinkedList* list, LinkedListIter* iter);
func ll_pop_back(LinkedList* list, void** data);
func ll_pop_front(LinkedList* list, void** data);
void ll_destroy(LinkedList* list);
/* LINKED LIST */

static inline void gfx_render_thick_line(SDL_Renderer* renderer,
                                         float x1,
                                         float y1,
                                         float x2,
                                         float y2,
                                         float thickness,
                                         SDL_Color color) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    dx /= length;
    dy /= length;

    float px = -dy;
    float py = dx;

    float halfThickness = thickness / 2.0f;
    SDL_Vertex vertices[4] = {
        { .position = { x1 + px * halfThickness, y1 + py * halfThickness },
         .color = color,
         .tex_coord = { 0, 0 } },
        { .position = { x1 - px * halfThickness, y1 - py * halfThickness },
         .color = color,
         .tex_coord = { 0, 0 } },
        { .position = { x2 - px * halfThickness, y2 - py * halfThickness },
         .color = color,
         .tex_coord = { 0, 0 } },
        { .position = { x2 + px * halfThickness, y2 + py * halfThickness },
         .color = color,
         .tex_coord = { 0, 0 } }
    };

    int indices[6] = { 0, 1, 2, 2, 3, 0 };

    SDL_RenderGeometry(renderer, NULL, vertices, 4, indices, 6);
}

static inline V2f32 get_random_screen_edge_position_away_from(f32 min_dist,
                                                              V2f32 pos1,
                                                              V2f32 pos2) {

    i32 max_tries = 32;
    i32 i = 0;

    f32 x = 0.0f;
    f32 y = 0.0f;
    
    do {
        i32 wall = rand_i32(0, 3);
        switch (wall) {
        case 0:
            x = 0.0f;
            y = rand_float(0.0f, WINDOW_HEIGHT);
            break;
        case 1:
            x = WINDOW_WIDTH;
            y = rand_float(0.0f, WINDOW_HEIGHT);
            break;
        case 2:
            x = rand_float(0.0f, WINDOW_WIDTH);
            y = 0.0f;
            break;
        case 3:
        default:
            x = rand_float(0.0f, WINDOW_WIDTH);
            y = WINDOW_WIDTH;
            break;
        }
        if (dist((V2f32) { x, y }, pos1) > min_dist &&
            dist((V2f32) { x, y }, pos2) > min_dist) {
            break;
        }

    } while (i++ < max_tries);
    return (V2f32) { x, y };
}

