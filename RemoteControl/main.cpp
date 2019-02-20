#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "CapSenseThread.h"

Thread displayThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "DisplayThread");
Thread capSenseThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "CapSense Thread");
Thread bleThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "BLEThread");
Thread wifiThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "WIFI Thread");

volatile GameMode_t gameMode;

int main()
{   uint16_t x=0;

    gameMode = MODE_SPLASH;

    displayThreadHandle.start(displayThread);
    capSenseThreadHandle.start(capSenseThread);

  /*
    // ARH delete this and exit the application thread probably
    while(1)
    {
        x += 1;
        wait(1.0);
    }
    */
}
