#pragma once

// Window
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 1280
#define WINDOW_SCALING
#define LINE_THICKNESS 2
// #define VSYNC_ENABLED
#define DEBUG_ENABLED
#define BACKGROUND_COLOR 0x15, 0x15, 0x15
// #define SOFTWARE_RENDERING
#define LOCAL_PLAYER_COLOR 0xff, 0xff, 0xff
#define REMOTE_PLAYER_COLOR 0x80, 0xff, 0x80
#define TEXTURES_CAPACITY 16

#define REMOTE_UPDATE_FREQUENCY 48.0f // read and write to queue hz
#define MAX_REMOTE_UPDATE_TIME 5.0f   // max 5ms
#define NETWORK_BUFFER_SIZE 1024

#define PLAYER_ROTATION_SPEED 270.0f
#define PLAYER_ACCELERATION_SPEED 700.0f
#define PLAYER_MAX_SPEED 550.0f
#define PLAYER_SHOOT_COOLDOWN 0.2f // seconds

#define BULLET_INITIAL_TTL 1.7f // seconds
#define BULLET_SPEED 650.0f

#define ASTEROID_MAX_POINTS 15
#define ASTEROID_MAX_ROTATION_SPEED 20.0f
#define ASTEROID_MIN_ROTATION_SPEED 3.0f

#define BIG_ROCK_KILL_POINTS 20
#define MEDIUM_ROCK_KILL_POINTS 50
#define SMALL_ROCK_KILL_POINTS 100

#define BIG_UFO_KILL_POINTS 50
#define SMALL_UFO_KILL_POINTS 150

#define BIG_UFO_SHOOT_DELAY 1.3f
#define SMALL_UFO_SHOOT_DELAY 1.9f
