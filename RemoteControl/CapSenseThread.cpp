#include "mbed.h"
#include "cycfg_capsense.h"
#include "CapSenseThread.h"
#include "DisplayThread.h"
#include "global.h"
#include "BleThread.h"
#include "WifiThread.h"

#define dbg_printf(...)
//#define dbg_printf printf

Semaphore capsenseSemaphore;
Queue<int32_t, 10> swipeQueue;
MemoryPool<int32_t, 10> swipePool;

void CapSense_Interrupt(void)
{
    Cy_CapSense_InterruptHandler(CapSense_HW, &cy_capsense_context);
}

// This function is called at the end of a capsense scan
void capSenseCallback(cy_stc_active_scan_sns_t * ptrActiveScan)
{
  capsenseSemaphore.release();
}

void capSenseThread(void)
{
    DigitalOut sldLED(SLD_LED,LED_OFF);

    Cy_CapSense_Init(&cy_capsense_context);

    const cy_stc_sysint_t CapSense_ISR_cfg =
    {
        .intrSrc = csd_interrupt_IRQn,
        .intrPriority = 7u,
    };
    Cy_SysInt_Init(&CapSense_ISR_cfg, &CapSense_Interrupt);
    NVIC_ClearPendingIRQ(CapSense_ISR_cfg.intrSrc);
    NVIC_EnableIRQ(CapSense_ISR_cfg.intrSrc);
    Cy_CapSense_RegisterCallback(CY_CAPSENSE_END_OF_SCAN_E, capSenseCallback, &cy_capsense_context);
    Cy_CapSense_Enable(&cy_capsense_context);
    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

    int startPos = 0xFFFF;
    int endPos;

    for (;;)
    {
        capsenseSemaphore.wait();
        Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);
        if(0uL != Cy_CapSense_IsWidgetActive(CY_CAPSENSE_BTBTN_WDGT_ID, &cy_capsense_context))
        {
          dbg_printf("CapSenseThread:Bluetooth Button\n");
          if(gameMode == MODE_CONNECT)
          {
            gameMode = MODE_CONNECT_BLE;
            bleThreadHandle.start(bleThread);
          }
        }

        if(0uL != Cy_CapSense_IsWidgetActive(CY_CAPSENSE_WIFIBTN_WDGT_ID, &cy_capsense_context))
        {
          dbg_printf("CapSenseThread:WiFi Button\n");
          if(gameMode == MODE_CONNECT)
          {
            gameMode = MODE_CONNECT_WIFI;
            wifiThreadHandle.start(wifiThread);
          }
        }

        if(0uL != Cy_CapSense_IsWidgetActive(CY_CAPSENSE_SLIDER_WDGT_ID, &cy_capsense_context))
        {
          sldLED = LED_ON;
          cy_stc_capsense_touch_t *sldrTouch = Cy_CapSense_GetTouchInfo(CY_CAPSENSE_SLIDER_WDGT_ID, &cy_capsense_context);
          endPos = sldrTouch->ptrPosition->x;
          if(startPos == 0xFFFF)
          {
            startPos = endPos;
            dbg_printf("CapSenseThread:Set Start =%u\n",startPos);
          }
        }
        else // No touch
        {
          sldLED = LED_OFF;
          if(startPos != 0xFFFF)
          {
            DisplayMessage_t *msg;
            dbg_printf("CapSenseThread: Swipe s:%u e%u Val=%d\n",startPos,endPos,(endPos - startPos));
            // Test code
            #if 0
            msg = displayPool.alloc();
            if(msg)
            {
              msg->command = GAME_SCREEN;
              msg->type = INIT_BLE;
              displayQueue.put(msg);
            }
            #endif

            if(gameMode == MODE_GAME)
            {
              // Send the swipe value to the display
              msg = displayPool.alloc();
              if(msg)
              {
                msg->command = SWIPE_VALUE;
                msg->val1 = endPos - startPos;
                displayQueue.put(msg);
              }
              // Send the swipe value to either BLE or  WiFi to be processes
              int32_t *swipeMsg;
              swipeMsg = swipePool.alloc();
              if(swipeMsg)
              {
                *swipeMsg = endPos - startPos;
                dbg_printf("CapSenseThread: swipeMsg %d\n",*swipeMsg);
                swipeQueue.put(swipeMsg);
              }
            }
            startPos = 0xFFFF;
            }
          }
          Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
    }
}
