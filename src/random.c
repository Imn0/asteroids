#include "common.h"

#define MAX_RANGES 10

static u8 RNG_idx = 0;
static const u8 random_table[256] = {
    0,   8,   109, 220, 222, 241, 149, 107, 75,  248, 254, 140, 16,  66,  74,
    21,  211, 47,  80,  242, 154, 27,  205, 128, 161, 89,  77,  36,  95,  110,
    85,  48,  212, 140, 211, 249, 22,  79,  200, 50,  28,  188, 52,  140, 202,
    120, 68,  145, 62,  70,  184, 190, 91,  197, 152, 224, 149, 104, 25,  178,
    252, 182, 202, 182, 141, 197, 4,   81,  181, 242, 145, 42,  39,  227, 156,
    198, 225, 193, 219, 93,  122, 175, 249, 0,   175, 143, 70,  239, 46,  246,
    163, 53,  163, 109, 168, 135, 2,   235, 25,  92,  20,  145, 138, 77,  69,
    166, 78,  176, 173, 212, 166, 113, 94,  161, 41,  50,  239, 49,  111, 164,
    70,  60,  2,   37,  171, 75,  136, 156, 11,  56,  42,  146, 138, 229, 73,
    146, 77,  61,  98,  196, 135, 106, 63,  197, 195, 86,  96,  203, 113, 101,
    170, 247, 181, 113, 80,  250, 108, 7,   255, 237, 129, 226, 79,  107, 112,
    166, 103, 241, 24,  223, 239, 120, 198, 58,  60,  82,  128, 3,   184, 66,
    143, 224, 145, 224, 81,  206, 163, 45,  63,  90,  168, 114, 59,  33,  159,
    95,  28,  139, 123, 98,  125, 196, 15,  70,  194, 253, 54,  14,  109, 226,
    71,  17,  161, 93,  186, 87,  244, 138, 20,  52,  123, 251, 26,  36,  17,
    46,  52,  231, 232, 76,  31,  221, 84,  37,  216, 165, 212, 106, 197, 242,
    98,  43,  39,  175, 254, 145, 190, 84,  118, 222, 187, 136, 120, 163, 236,
    249
};

f32 rand_float(f32 min, f32 max) {
    f32 scale = random_table[RNG_idx++] / (f32)255;
    return min + scale * (max - min);
}

f32 rand_float_seed(f32 min, f32 max, u8 seed) {
    f32 scale = random_table[seed] / (f32)255;
    return min + scale * (max - min);
}
/**
 * @brief random integer in ```[min, max]```
 *
 * @param min
 * @param max
 * @return i32
 */
i32 rand_i32(i32 min, i32 max) {
    return min + random_table[RNG_idx++] / (255 / (max - min + 1) + 1);
}
i32 rand_i32_seed(i32 min, i32 max, u8 seed) {
    return min + random_table[seed] / (255 / (max - min + 1) + 1);
}

f32 rand_float_range_seed(u8 seed, i32 num_ranges, ...) {

    ASSERT(num_ranges <= MAX_RANGES, "Too many random ranges given");

    va_list args;
    va_start(args, num_ranges);

    f32 total_length = 0.0;
    f32 start[MAX_RANGES];
    f32 end[MAX_RANGES];
    for (int i = 0; i < num_ranges; i++) {
        start[i] = (f32)va_arg(args, f64);
        end[i] = (f32)va_arg(args, f64);
        total_length += end[i] - start[i];
    }

    f32 rand_point = rand_float_seed(0.0, total_length, seed);

    f32 current_length = 0.0;
    f32 result = 0.0;
    for (int i = 0; i < num_ranges; i++) {
        f32 range_length = end[i] - start[i];
        if (rand_point < current_length + range_length) {
            result = start[i] + (rand_point - current_length);
            break;
        }
        current_length += range_length;
    }

    va_end(args);
    return result;
}

f32 rand_float_range(i32 num_ranges, ...) {

    ASSERT(num_ranges <= MAX_RANGES, "Too many random ranges given");

    
    va_list args;
    va_start(args, num_ranges);

    f32 total_length = 0.0;
    f32 start[MAX_RANGES];
    f32 end[MAX_RANGES];
    for (int i = 0; i < num_ranges; i++) {
        start[i] = (f32)va_arg(args, double);
        end[i] = (f32)va_arg(args, double);
        total_length += end[i] - start[i];
    }

    f32 rand_point = rand_float(0.0, total_length);

    f32 current_length = 0.0;
    f32 result = 0.0;
    for (int i = 0; i < num_ranges; i++) {
        f32 range_length = end[i] - start[i];
        if (rand_point < current_length + range_length) {
            result = start[i] + (rand_point - current_length);
            break;
        }
        current_length += range_length;
    }

    va_end(args);
    return result;
}
