/************************************************************************
 $Revision: 51 $                                                           
************************************************************************/

#include "stm32f10x.h"
#include "hm_led.h"
#include "hm_common_cfg.h"


typedef struct led_info_s
{
    GPIO_TypeDef        *port;
    uint16_t            pin;
    uint32_t            rcc; 
}led_info_t, *p_led_info_t;


static led_info_t   s_leds[ltMaxLed];

static void hm_led_cfg(void);


void hm_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    uint32_t idx;
    
    hm_led_cfg();
    
    
    for(idx = 0; idx < ltMaxLed; ++idx)
    {
        RCC_APB2PeriphClockCmd(s_leds[idx].rcc, ENABLE);
        
        GPIO_InitStructure.GPIO_Pin     = s_leds[idx].pin;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
        GPIO_Init(s_leds[idx].port, &GPIO_InitStructure);
			  GPIO_ResetBits(s_leds[idx].port, s_leds[idx].pin);
    }
}
void hm_led_chan(uint32_t idx,  bool bon)
{
    LED_TYPE_E chans[] = {ltCh1, ltCh2, ltCh3, ltCh4};
    if(idx >= HM_MAX_AXIS || idx >= sizeof(chans)/ sizeof(chans[0])) return;
    hm_led_act(chans[idx], bon);
}
void hm_led_act(LED_TYPE_E e, bool bon)
{
    if(bon)
    {
        GPIO_SetBits(s_leds[e].port, s_leds[e].pin);
    }
    else
    {
        GPIO_ResetBits(s_leds[e].port, s_leds[e].pin);
    }
}

void hm_led_cfg(void)
{

    
    s_leds[ltRun].rcc     = RCC_APB2Periph_GPIOB;
    s_leds[ltRun].port    = GPIOB;
    s_leds[ltRun].pin     = GPIO_Pin_7;

    
    s_leds[ltError].rcc     = RCC_APB2Periph_GPIOB;
    s_leds[ltError].port    = GPIOB;
    s_leds[ltError].pin     = GPIO_Pin_8;
   
    
    s_leds[ltCh1].rcc     = RCC_APB2Periph_GPIOB;
    s_leds[ltCh1].port    = GPIOB;
    s_leds[ltCh1].pin     = GPIO_Pin_9;
     
    
    s_leds[ltCh2].rcc     = RCC_APB2Periph_GPIOB;
    s_leds[ltCh2].port    = GPIOB;
    s_leds[ltCh2].pin     = GPIO_Pin_10;
       
    
    s_leds[ltCh3].rcc     = RCC_APB2Periph_GPIOB;
    s_leds[ltCh3].port    = GPIOB;
    s_leds[ltCh3].pin     = GPIO_Pin_14;

    
    s_leds[ltCh4].rcc     = RCC_APB2Periph_GPIOB;
    s_leds[ltCh4].port    = GPIOB;
    s_leds[ltCh4].pin     = GPIO_Pin_15;
          
}
