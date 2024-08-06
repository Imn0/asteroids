#include <SDL_mixer.h>

#include "common.h"
#include "sfx.h"


#define SHOOT_SOUND_NAME "shoot.wav"
#define BANG_BIG_SOUND_NAME "bang_big.wav"
#define BANG_MEDIUM_SOUND_NAME "bang_medium.wav"
#define BANG_SMALL_SOUND_NAME "bang_small.wav"
#define THRUST_SOUND_NAME "thrust.wav"
#define DEATH_SOUND_NAME "death.wav"
#define EXTRA_LIFE_NAME "death.wav"

Mix_Chunk* _sounds[SOUNDS_NUM];

void init_sound() {

    ASSERT(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048)
           == 0, "SDL mixer faild to initialize %s\n", Mix_GetError());

    char* exe_path = SDL_GetBasePath();
    char sounds_path[1024];
    snprintf(sounds_path, sizeof(sounds_path), "%s/../assets/sounds/", exe_path);


    char sound_path_buff[1024];
    snprintf(sound_path_buff, sizeof(sound_path_buff), "%s%s", sounds_path, SHOOT_SOUND_NAME);
    _sounds[SFX_SHOOT] = Mix_LoadWAV(sound_path_buff);
    if (_sounds[SFX_SHOOT] == NULL) { printf("error loading %s\n", SHOOT_SOUND_NAME); }

    snprintf(sound_path_buff, sizeof(sound_path_buff), "%s%s", sounds_path, BANG_BIG_SOUND_NAME);
    _sounds[SFX_ROCK_BIG] = Mix_LoadWAV(sound_path_buff);
    if (_sounds[SFX_ROCK_BIG] == NULL) { printf("error loading %s\n", BANG_BIG_SOUND_NAME); }


    snprintf(sound_path_buff, sizeof(sound_path_buff), "%s%s", sounds_path, BANG_MEDIUM_SOUND_NAME);
    _sounds[SFX_ROCK_MEDIUM] = Mix_LoadWAV(sound_path_buff);
    if (_sounds[SFX_ROCK_MEDIUM] == NULL) { printf("error loading %s\n", BANG_MEDIUM_SOUND_NAME); }


    snprintf(sound_path_buff, sizeof(sound_path_buff), "%s%s", sounds_path, BANG_SMALL_SOUND_NAME);
    _sounds[SFX_ROCK_SMALL] = Mix_LoadWAV(sound_path_buff);
    if (_sounds[SFX_ROCK_SMALL] == NULL) { printf("error loading %s\n", BANG_SMALL_SOUND_NAME); }


    snprintf(sound_path_buff, sizeof(sound_path_buff), "%s%s", sounds_path, THRUST_SOUND_NAME);
    _sounds[SFX_THRUST] = Mix_LoadWAV(sound_path_buff);
    if (_sounds[SFX_THRUST] == NULL) { printf("error loading %s\n", THRUST_SOUND_NAME); }

    snprintf(sound_path_buff, sizeof(sound_path_buff), "%s%s", sounds_path, DEATH_SOUND_NAME);
    _sounds[SFX_DEATH] = Mix_LoadWAV(sound_path_buff);
    if (_sounds[SFX_DEATH] == NULL) { printf("error loading %s\n", DEATH_SOUND_NAME); }


    snprintf(sound_path_buff, sizeof(sound_path_buff), "%s%s", sounds_path, EXTRA_LIFE_NAME);
    _sounds[SFX_EXTRA_LIFE] = Mix_LoadWAV(sound_path_buff);
    if (_sounds[SFX_EXTRA_LIFE] == NULL) { printf("error loading %s\n", EXTRA_LIFE_NAME); }


}

void play_sound(SFXType sfx) {
    if (_sounds[sfx] == NULL) { return; }

    if (Mix_PlayChannel(-1, _sounds[sfx], 0) == -1) {
        printf("Error playing sound %d : %s \n", sfx, Mix_GetError());
    }
}
