#pragma once

#define WS_NUM_LEDS (144)

typedef enum{
    LEFT_STRING,
    RIGHT_STRING
}LED_SELECT_T;

extern void wsThread(void);
extern void WS_setRGB(LED_SELECT_T ledSide, uint16_t led,uint8_t red, uint8_t green, uint8_t blue);
extern void WS_fill_side(LED_SELECT_T ledSide, uint16_t fillNum, uint8_t red, uint8_t green, uint8_t blue);
extern void WS_clear_side(LED_SELECT_T ledSide);
extern void WS_fill_random(void);
extern uint32_t WS_percent_toLEDs(uint8_t percent);