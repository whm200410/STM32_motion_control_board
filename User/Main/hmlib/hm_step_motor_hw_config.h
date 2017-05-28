/************************************************************************
 $Revision: 43 $                                                           
************************************************************************/

#ifndef __HM_STEP_MOTOR_LIMITS_DEFS_H
#define __HM_STEP_MOTOR_LIMITS_DEFS_H
#include "stm32f10x.h"
#include "hm_types.h"
typedef struct hm_step_motor_hw_config_s
{
    /*gpio define*/
    GPIO_TypeDef    *ptCW_lmt;
    GPIO_TypeDef    *ptCCW_lmt;
    GPIO_TypeDef    *ptHome_lmt;   
    GPIO_TypeDef    *ptDir; 
    GPIO_TypeDef    *ptEna;
    
    /*extern line interupt function for CW, CCW, Home*/
    void            *irpFunCW;
    void            *irpFunCCW;
    void            *irpFunHome;
   
    /*External interrupt line for CW, CCW, Home*/
    uint32_t        extLineCW;  
    uint32_t        extLineCCW;
    uint32_t        extLineHome;  
    
    uint8_t         portSourceCW;
    uint8_t         portSourceCCW;
    uint8_t         portSourceHome;
    
    uint8_t         pinSourceCW;
    uint8_t         pinSourceCCW;
    uint8_t         pinSourceHome;   
    
    uint8_t         nirpCW;
    uint8_t         nirpCCW;
    uint8_t         nirpHome;
    
    uint8_t         benabled;

    
    bool            bStopImmediatelyCW;
    bool            bStopImmediatelyCCW;
    bool            bStopImmediatelyHome;
    
    uint16_t        pinCW_lmt;
    uint16_t        pinCCW_lmt;
    uint16_t        pinHome_lmt;
    uint16_t        pinDir;
    uint16_t        pinEna;
    
    uint16_t        rcc_mod1;  
    uint16_t        rcc_mod2; 
    bool            blast_hit_home;
}hm_step_motor_hw_config_t;

   

#endif
