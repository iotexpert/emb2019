
#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "CapSenseThread.h"
#include "WifiThread.h"

void wifiThread()
{
  DisplayMessage_t *msg;
  printf("Started WiFI Thread\n");

  msg = displayPool.alloc();
  msg->command = WIFI_SCREEN;
  msg->type = WIFI_CONNECT;
  displayQueue.put(msg);

  while(1)
  {
    wait(1.0);
    printf("WiFiThread Alive\n");
  }

}
