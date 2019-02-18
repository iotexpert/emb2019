
#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "CapSenseThread.h"

void bleThread()
{
  DisplayMessage_t *msg;
  printf("Started BLE Thread\n");

  msg = displayPool.alloc();
  msg->command = BLE_SCREEN;
  msg->type = BLE_START;
  displayQueue.put(msg);

  while(1)
  {
    wait(1.0);
    printf("bleThread Alive\n");
  }

}
