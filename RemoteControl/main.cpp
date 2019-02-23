#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "CapSenseThread.h"

Thread displayThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "DisplayThread");
Thread capSenseThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "CapSense Thread");
Thread bleThreadHandle(osPriorityNormal, 2*OS_STACK_SIZE, NULL, "BLEThread");
Thread wifiThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "WIFI Thread");

volatile GameMode_t gameMode;

int main()
{
	gameMode = MODE_SPLASH;

	displayThreadHandle.start(displayThread);
	capSenseThreadHandle.start(capSenseThread);
}
