#pragma once
typedef enum {
  MODE_SPLASH,
  MODE_CONNECT,
  MODE_CONNECT_BLE,
  MODE_CONNECT_WIFI,
  MODE_GAME
} GameMode_t;

#define LED_ON 0
#define LED_OFF 1

extern volatile GameMode_t gameMode;
extern bool positionMode;
extern Thread wifiThreadHandle;
extern Thread bleThreadHandle;
