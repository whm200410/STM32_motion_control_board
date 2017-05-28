/************************************************************************
 $Revision: 9 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_address.h"
#include "hm_common_cfg.h"
typedef struct addr_info_s
{
    GPIO_TypeDef        *port;
    uint16_t            pin;
}addr_info_t;

const uint32_t cADDR_BITS = 3;

static addr_info_t  s_addrs[cADDR_BITS];
static uint8_t      s_ip_addr[] = HM_IP_START;
static uint8_t      s_ip_macc[] = {31, 45, 13, 56, 24, 56};
static bool hm_addr_cfg(void);

bool     hm_addr_init(void)
{
    
    GPIO_InitTypeDef GPIO_InitStructure;
    uint32_t    idx;
    uint8_t     addr = 0;
    uint8_t     tmp;   
    if(!hm_addr_cfg()) return false;
    
    
    for(idx = 0; idx < cADDR_BITS; ++idx)
    {       
        GPIO_InitStructure.GPIO_Pin     = s_addrs[idx].pin;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
        GPIO_Init(s_addrs[idx].port, &GPIO_InitStructure);
    }    
    

    for(idx = 0; idx < cADDR_BITS; ++idx)
    {       
        tmp = GPIO_ReadInputDataBit(s_addrs[idx].port, s_addrs[idx].pin);
        tmp <<= idx;
        addr |= tmp;
    }
    s_ip_addr[3] += addr;
    s_ip_macc[5] += addr;
    return true;
}
uint8_t* hm_mac_addr(void)
{
    return s_ip_macc;
}
uint8_t* hm_ip_addr(void)
{
    return s_ip_addr;
}

bool hm_addr_cfg(void)
{
    uint32_t idx = 0;
    
    s_addrs[idx].port      = GPIOA;
    s_addrs[idx++].pin     = GPIO_Pin_7;

    s_addrs[idx].port      = GPIOA;
    s_addrs[idx++].pin     = GPIO_Pin_11;
 
    s_addrs[idx].port      = GPIOA;
    s_addrs[idx++].pin     = GPIO_Pin_12;    
    
    if(idx != cADDR_BITS) return false;
    return true;
}
