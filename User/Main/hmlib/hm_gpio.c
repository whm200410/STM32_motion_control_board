/************************************************************************
 $Revision: 50 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_gpio.h"
#include "hm_common_cfg.h"
#include "hm_debug.h"
typedef struct hm_gpio_info_s
{
    GPIO_TypeDef    *port;
    uint16_t        pin;
    
}hm_gpio_info_t, *p_hm_gpio_info_t;

static hm_gpio_info_t   s_gpio_out[HM_MAX_GPIO_OUT];
static hm_gpio_info_t   s_gpio_in[HM_MAX_GPIO_IN];

bool  hm_gpio_hw_config(void);
void        hm_gpio_init(void)
{
    uint32_t                idx = 0;
    GPIO_InitTypeDef        GPIO_InitStructure;
    
    hm_gpio_hw_config();
    
    for(idx = 0; idx < HM_MAX_GPIO_OUT; ++idx)
    { 
        GPIO_InitStructure.GPIO_Pin         = s_gpio_out[idx].pin;
        GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
        GPIO_Init(s_gpio_out[idx].port, &GPIO_InitStructure);
        GPIO_ResetBits(s_gpio_out[idx].port, s_gpio_out[idx].pin);
    }
    for(idx = 0; idx < HM_MAX_GPIO_IN; ++idx)
    { 
        GPIO_InitStructure.GPIO_Pin         = s_gpio_in[idx].pin;
        GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
        GPIO_Init(s_gpio_in[idx].port, &GPIO_InitStructure);
    }
}

bool hm_gpio_out_bit(uint32_t bitno, uint32_t val)
{
    if(bitno >= HM_MAX_GPIO_OUT) return false;
    if(val)
    {
        GPIO_SetBits(s_gpio_out[bitno].port, s_gpio_out[bitno].pin);
    }
    else
    {
        GPIO_ResetBits(s_gpio_out[bitno].port, s_gpio_out[bitno].pin);
    }
    
    return true;
}
const int nloop = 100;
uint32_t    hm_gpio_in_bit(uint32_t bitno)
{
    uint32_t    idx = 0;
    uint8_t     nval1;
    uint8_t     nval2;
    if(bitno >= HM_MAX_GPIO_IN) return 0xffff;
    nval1 = GPIO_ReadInputDataBit(s_gpio_in[bitno].port, s_gpio_in[bitno].pin);
    for(idx = 0; idx < nloop; ++idx)
    {
        nval2 = GPIO_ReadInputDataBit(s_gpio_in[bitno].port, s_gpio_in[bitno].pin);
        if(nval2 != nval1)
        {
            HM_NET_PRINT("!!!hm_gpio_in_bit error bitno = %d, nval1 = %d, nval2 = %d, idx = %d", bitno, nval1, nval2, idx);
        }
    }
    return nval2;
}
uint32_t    hm_gpio_get_out_bit(uint32_t bitno)
{

    if(bitno >= HM_MAX_GPIO_OUT) return 0xffff;
    GPIO_ReadOutputDataBit(s_gpio_out[bitno].port, s_gpio_out[bitno].pin);
    return GPIO_ReadOutputDataBit(s_gpio_out[bitno].port, s_gpio_out[bitno].pin);
}

bool hm_gpio_hw_config(void)
{
    uint32_t    idx = 0;
    s_gpio_in[idx].port     = GPIOC;
    s_gpio_in[idx++].pin    = GPIO_Pin_5;
    
    s_gpio_in[idx].port     = GPIOC;
    s_gpio_in[idx++].pin    = GPIO_Pin_9;

    s_gpio_in[idx].port     = GPIOC;
    s_gpio_in[idx++].pin    = GPIO_Pin_12;
    
    s_gpio_in[idx].port     = GPIOD;
    s_gpio_in[idx++].pin    = GPIO_Pin_11;

    s_gpio_in[idx].port     = GPIOD;
    s_gpio_in[idx++].pin    = GPIO_Pin_12;
    
    s_gpio_in[idx].port     = GPIOD;
    s_gpio_in[idx++].pin    = GPIO_Pin_13;
    
     s_gpio_in[idx].port     = GPIOD;
    s_gpio_in[idx++].pin    = GPIO_Pin_14;
    
    s_gpio_in[idx].port     = GPIOD;
    s_gpio_in[idx++].pin    = GPIO_Pin_15; 
    
    if(idx != HM_MAX_GPIO_IN) return false;
    
    
    idx = 0;
    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_0;
    
    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_1;

    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_2;
    
    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_3;   

    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_4;
    
    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_5;

    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_6;
    
    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_7;   

    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin    = GPIO_Pin_8;   

    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_13;
    
    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_14;

    s_gpio_out[idx].port    = GPIOE;
    s_gpio_out[idx++].pin   = GPIO_Pin_15;
    
    if(idx != HM_MAX_GPIO_OUT) return false;
    
    return true;
   
}
