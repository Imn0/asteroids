#include "common.h"
#include "entity.h"

typedef struct {
    f64 frame_time, logic_time, render_time;
    bool show_info;
    bool stop_logic;
} DebugState;

extern DebugState debug_state;

void debug_loop_start();
void debug_before_render();
void debug_after_render();
void debug_render();
void debug_log_death(V2f32 player_pos, Entity* entity);
