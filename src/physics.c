#include "physics.h"
#include "game.h"

void decelerate_v2f32(V2f32 v[static 1], f32 decel_rate){
    
    // deaceleration 
    i32 i = 0;
    if (fabs(v->x) > 0.1f) {
        if (v->x < 0) { i = 1; }
        if (v->x > 0) { i = -1; }
        v->x += (i * decel_rate) * delta_time;
    }
    else {
        v->x = 0.0f;
    }

    i = 0;
    if (fabs(v->y) > 0.1f) {

        if (v->y < 0) { i = 1; }
        if (v->y > 0) { i = -1; }
        v->y += (i * decel_rate) * delta_time;
    }
    else {
        v->y = 0.0f;
    }

}
