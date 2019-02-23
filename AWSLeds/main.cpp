#include "mbed.h"
#include "WifiThread.h"
#include "cycfg_pins.h"
#include "cycfg_peripherals.h"
#include "wsleddma.h"

/*******************************************************************************
* Global variables
*******************************************************************************/
Thread wifiThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "WIFI Thread");
Thread wsLedThreadHandle(osPriorityNormal, OS_STACK_SIZE, NULL, "LED");

/* main() runs in its own thread in the OS */
int main()
{
  /* enable interrupts */
  __enable_irq();
  
  /* Start Threads */
  printf("Starting WS LED driver\r\n");
  wsLedThreadHandle.start(wsThread);
  printf("Starting WiFi/AWS Thread\r\n");
  wifiThreadHandle.start(wifiThread);
  
  /* Blink LEDs on/off once on each string */
  ThisThread::sleep_for(100);
  WS_fill_side(LEFT_STRING,  WS_percent_toLEDs(100), 0xFF, 0x00, 0x00);
  ThisThread::sleep_for(500);
  WS_clear_side(LEFT_STRING);
  ThisThread::sleep_for(100);
  WS_fill_side(RIGHT_STRING, WS_percent_toLEDs(100), 0x00, 0x00, 0xFF);
  ThisThread::sleep_for(500);
  WS_clear_side(RIGHT_STRING);

  while(1)
    {
      /* Blink heardbeat LED at 1Hz */
      Cy_GPIO_Inv(USER_LED_PORT, USER_LED_PIN);
      ThisThread::sleep_for(500);
    }
}


