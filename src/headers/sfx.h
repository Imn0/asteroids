#pragma once
#include "entity.h"

#define SOUNDS_NUM SFX_SIZE

typedef enum{
    SFX_SHOOT = 0,
    SFX_ROCK_SMALL,
    SFX_ROCK_MEDIUM,
    SFX_ROCK_BIG,
    SFX_THRUST,
    SFX_DEATH,
    SFX_EXTRA_LIFE,


    SFX_SIZE
} SFXType;


static SFXType sfx_rock[] = {
    [ROCK_BIG] = SFX_ROCK_BIG,
    [ROCK_MEDIUM] = SFX_ROCK_MEDIUM,
    [ROCK_SMALL] = SFX_ROCK_SMALL,
};


void init_sound();
void play_sound(SFXType sfx);
