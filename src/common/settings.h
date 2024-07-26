#pragma once

// Window
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 1280
#define WINDOW_SCALING

#define PLAYER_SIZE 64
#define TEXTURES_CAPACITY 16

#define REMOTE_UPDATE_FREQUENCY 24.0f // read and write to queue hz 
#define MAX_REMOTE_UPDATE_TIME 5.0f // max 5ms 
#define NETWORK_BUFFER_SIZE 1024

#define ROTATION_SPEED 270.0f
#define ACCELERATION_SPEED 700.0f
#define PLAYER_MAX_SPEED 500.0f
#define SHOOT_COOLDOWN 0.2f // seconds 

#define BULLET_INITIAL_TTL 1.3f // seconds
#define BULLET_SPEED 550.0f 
