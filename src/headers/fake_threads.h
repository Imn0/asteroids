#pragma once
#if defined(WIN32) && !defined(_MSC_VER) // MSCV and linux/mac

#define UNUSED(x) (void)(x)

typedef unsigned long int thrd_t;

typedef union {
    char __size[40];
} mtx_t;

typedef union {
    char __size[48];
} cnd_t;

enum {
    thrd_success = 0,
    thrd_busy = 1,
    thrd_error = 2,
    thrd_nomem = 3,
    thrd_timedout = 4
};

enum {
    mtx_plain = 0,
    mtx_recursive = 1,
    mtx_timed = 2
};


inline int mtx_init(mtx_t* __mutex, int __type) { UNUSED(__mutex); UNUSED(__type); return thrd_success; }
inline int cnd_init(cnd_t* __cond) { UNUSED(__cond); return thrd_success; }
inline void mtx_destroy(mtx_t* __mutex) { UNUSED(__mutex); }
inline int mtx_lock(mtx_t* __mutex) { UNUSED(__mutex);  return thrd_success; }
inline int mtx_unlock(mtx_t* __mutex) { UNUSED(__mutex);  return thrd_success; }
inline int cnd_signal(cnd_t* __cond) { UNUSED(__cond); return thrd_success; }
inline void cnd_destroy(cnd_t* __COND) { UNUSED(__COND); }

#endif