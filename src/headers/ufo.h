#pragma once

#include "common.h"

typedef enum { UFO_NONE, UFO_BIG, UFO_SMALL } UfoForm;

typedef struct {
    UfoForm form;
    V2f32 position, velocity;
    f32 angle_deg;
    f32 shoot_timer;
    f32 ufo_timer; // how long ufo was in its form
} Ufo;

typedef struct {
    UfoForm form;
    V2f32 position, velocity;
    f32 angle_deg;
} UfoCreateArgs;

extern Ufo game_ufo;

void ufo_init(UfoCreateArgs args, Ufo* ufo);
void ufo_update(Ufo* ufo);
void ufo_render(Ufo* ufo);
