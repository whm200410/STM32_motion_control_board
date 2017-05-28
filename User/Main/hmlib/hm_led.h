#ifndef __HM_LED_H
#define __HM_LED_H
#include "hm_types.h"

typedef enum 
{
    ltRun,
    ltError,
    ltCh1,
    ltCh2,
    ltCh3,
    ltCh4,
    ltMaxLed
} LED_TYPE_E;

void hm_led_init(void);
void hm_led_act(LED_TYPE_E, bool bon);
void hm_led_chan(uint32_t idx,  bool bon);
#endif
