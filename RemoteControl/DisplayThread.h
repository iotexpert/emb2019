#pragma once
#include "mbed.h"

typedef enum {
	BLE_SCREEN,
	WIFI_SCREEN,
	GAME_SCREEN,
	SWIPE_VALUE
} DisplayCommand_t;

/* Type of update to make to the display second byte in the queue */
typedef enum {
	BLE_START,
	BLE_ADVERTISE,
	BLE_CONNECT,
	BLE_SERVICE_DISC,
	REGISTER_NOTIFY,
	WIFI_CONNECT,
	AWS_RESOURCES,
	AWS_CONNECT,
	SUBSCRIBE_SHADOW,
	INIT_WIFI,
	INIT_BLE,
	WATER_VALUE
} DisplayType_t;


typedef struct {
  DisplayCommand_t command;
  DisplayType_t type;
  int32_t val1;
  int32_t val2;
} DisplayMessage_t;

extern void displayThread();

extern Queue<DisplayMessage_t, 10> displayQueue;
extern MemoryPool<DisplayMessage_t, 10> displayPool;
