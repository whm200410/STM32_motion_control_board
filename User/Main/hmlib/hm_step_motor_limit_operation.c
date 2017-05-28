/************************************************************************
 $Revision: 54 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_step_motor.h"
#include "hm_step_motor_defs.h"
#include "hm_uart.h"
#include "hm_step_motor_limit_operation.h"
#include <stdio.h>
#include "hm_common_cfg.h"
#include "hm_debug.h"
extern hm_step_info_t   s_axis_info[];
#define xCHECK_IN_MAIN
static void handler_extern_irq(void *func, uint32_t extline);
void hm_motor_hw_limit_init(uint32_t idx)
{
    hm_step_motor_hw_config_t *phw = &s_axis_info[idx].motor_hw_config;
    GPIO_InitTypeDef        GPIO_InitStructure;
#ifndef CHECK_IN_MAIN    
    NVIC_InitTypeDef        NVIC_InitStructure;
    EXTI_InitTypeDef        EXTI_InitStructure;
#endif    
    
    GPIO_InitStructure.GPIO_Pin         = phw->pinCW_lmt;
    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
    GPIO_Init(phw->ptCW_lmt, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin         = phw->pinCCW_lmt;
    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
    GPIO_Init(phw->ptCCW_lmt, &GPIO_InitStructure);    

    GPIO_InitStructure.GPIO_Pin         = phw->pinHome_lmt;
    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
    GPIO_Init(phw->ptHome_lmt, &GPIO_InitStructure);      

    GPIO_InitStructure.GPIO_Pin         = phw->pinDir;
    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
    GPIO_Init(phw->ptDir, &GPIO_InitStructure);      
    
    GPIO_InitStructure.GPIO_Pin         = phw->pinEna;
    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
    GPIO_Init(phw->ptEna, &GPIO_InitStructure);

#ifndef CHECK_IN_MAIN
    GPIO_EXTILineConfig(phw->portSourceCW , phw->pinSourceCW);
    GPIO_EXTILineConfig(phw->portSourceCCW , phw->pinSourceCCW);
    GPIO_EXTILineConfig(phw->portSourceHome , phw->pinSourceHome);
    
    
	
	/*外部中断输入*/
    EXTI_StructInit(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line        = phw->extLineCW; 
	EXTI_InitStructure.EXTI_Mode        = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger     = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd	    = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

    EXTI_InitStructure.EXTI_Line        = phw->extLineCCW; 
    EXTI_Init(&EXTI_InitStructure);
    
    EXTI_InitStructure.EXTI_Line        = phw->extLineHome; 
    EXTI_Init(&EXTI_InitStructure);


	/*外部中断线*/
	NVIC_InitStructure.NVIC_IRQChannel = phw->nirpCW ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE ;	
	NVIC_Init(&NVIC_InitStructure);
    
	NVIC_InitStructure.NVIC_IRQChannel = phw->nirpCCW ;
	NVIC_Init(&NVIC_InitStructure);  
    
	NVIC_InitStructure.NVIC_IRQChannel = phw->nirpHome ;
	NVIC_Init(&NVIC_InitStructure);      
#endif    
    hm_motor_enable_axis(idx, true);
}
void hm_motor_hw_enable_irq(uint32_t idx, bool bena)
{
    hm_step_motor_hw_config_t *phw = &s_axis_info[idx].motor_hw_config;
    EXTI_InitTypeDef        EXTI_InitStructure;
    FunctionalState         sta = bena ? ENABLE : DISABLE;
	/*外部中断输入*/
    EXTI_StructInit(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line        = phw->extLineCW; 
	EXTI_InitStructure.EXTI_Mode        = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger     = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd	    = sta;
	EXTI_Init(&EXTI_InitStructure);

    EXTI_InitStructure.EXTI_Line        = phw->extLineCCW; 
    EXTI_Init(&EXTI_InitStructure);
    
    EXTI_InitStructure.EXTI_Line        = phw->extLineHome; 
    EXTI_Init(&EXTI_InitStructure);    
}
void hm_motor_hw_limit_config(uint32_t idx)
{
    hm_step_motor_hw_config_t *phw = &s_axis_info[idx].motor_hw_config;
    phw->blast_hit_home = false;
    if(idx == 0)
    {
        phw->ptCW_lmt       = GPIOD;
        phw->ptCCW_lmt      = GPIOD;
        phw->ptHome_lmt     = GPIOD;
        phw->ptDir          = GPIOC;
        phw->ptEna          = GPIOC;
        
        
        phw->pinCW_lmt      = GPIO_Pin_0;
        phw->pinCCW_lmt     = GPIO_Pin_1;
        phw->pinHome_lmt    = GPIO_Pin_2;
        phw->pinDir         = GPIO_Pin_0;
        phw->pinEna         = GPIO_Pin_2;
        
        phw->extLineCW      = EXTI_Line0;
        phw->extLineCCW     = EXTI_Line1;
        phw->extLineHome    = EXTI_Line2;
        
        phw->irpFunCW       = EXTI0_IRQHandler;
        phw->irpFunCCW      = EXTI1_IRQHandler;
        phw->irpFunHome     = EXTI2_IRQHandler;
        
        phw->portSourceCW   = GPIO_PortSourceGPIOD;
        phw->portSourceCCW  = GPIO_PortSourceGPIOD;
        phw->portSourceHome = GPIO_PortSourceGPIOD;
        
        phw->pinSourceCW    = GPIO_PinSource0;
        phw->pinSourceCCW   = GPIO_PinSource1;
        phw->pinSourceHome  = GPIO_PinSource2; 
        
        phw->nirpCW         = EXTI0_IRQn;
        phw->nirpCCW        = EXTI1_IRQn;
        phw->nirpHome       = EXTI2_IRQn;
        
        
        phw->rcc_mod2       = RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC;
        phw->rcc_mod1       = 0;    
    }
    else if(idx == 1)
    {
        phw->ptCW_lmt       = GPIOD;
        phw->ptCCW_lmt      = GPIOD;
        phw->ptHome_lmt     = GPIOD;
        phw->ptDir          = GPIOC;
        phw->ptEna          = GPIOC;
        
        
        phw->pinCW_lmt      = GPIO_Pin_3;
        phw->pinCCW_lmt     = GPIO_Pin_4;
        phw->pinHome_lmt    = GPIO_Pin_5;
        phw->pinDir         = GPIO_Pin_3;
        phw->pinEna         = GPIO_Pin_4;
        
        phw->extLineCW      = EXTI_Line3;
        phw->extLineCCW     = EXTI_Line4;
        phw->extLineHome    = EXTI_Line5;
        
        phw->irpFunCW       = EXTI3_IRQHandler;
        phw->irpFunCCW      = EXTI4_IRQHandler;
        phw->irpFunHome     = EXTI9_5_IRQHandler;
 
        phw->portSourceCW   = GPIO_PortSourceGPIOD;
        phw->portSourceCCW  = GPIO_PortSourceGPIOD;
        phw->portSourceHome = GPIO_PortSourceGPIOD;
        
        phw->pinSourceCW    = GPIO_PinSource3;
        phw->pinSourceCCW   = GPIO_PinSource4;
        phw->pinSourceHome  = GPIO_PinSource5;   
    
        phw->nirpCW         = EXTI3_IRQn;
        phw->nirpCCW        = EXTI4_IRQn;
        phw->nirpHome       = EXTI9_5_IRQn;

        phw->rcc_mod2       = RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
        phw->rcc_mod1       = 0;    
    }    
    else if(idx == 2)
    {
        phw->ptCW_lmt       = GPIOC;
        phw->ptCCW_lmt      = GPIOC;
        phw->ptHome_lmt     = GPIOC;
        phw->ptDir          = GPIOD;
        phw->ptEna          = GPIOD;
        
        
        phw->pinCW_lmt      = GPIO_Pin_6;
        phw->pinCCW_lmt     = GPIO_Pin_7;
        phw->pinHome_lmt    = GPIO_Pin_8;
        phw->pinDir         = GPIO_Pin_6;
        phw->pinEna         = GPIO_Pin_7;
        
        phw->extLineCW      = EXTI_Line6;
        phw->extLineCCW     = EXTI_Line7;
        phw->extLineHome    = EXTI_Line8;
        
        phw->irpFunCW       = EXTI9_5_IRQHandler;
        phw->irpFunCCW      = EXTI9_5_IRQHandler;
        phw->irpFunHome     = EXTI9_5_IRQHandler;
 
        phw->portSourceCW   = GPIO_PortSourceGPIOC;
        phw->portSourceCCW  = GPIO_PortSourceGPIOC;
        phw->portSourceHome = GPIO_PortSourceGPIOC;
        
        phw->pinSourceCW    = GPIO_PinSource6;
        phw->pinSourceCCW   = GPIO_PinSource7;
        phw->pinSourceHome  = GPIO_PinSource8;   
        
        phw->nirpCW         = EXTI9_5_IRQn;
        phw->nirpCCW        = EXTI9_5_IRQn;
        phw->nirpHome       = EXTI9_5_IRQn;

        phw->rcc_mod2       = RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
        phw->rcc_mod1       = 0;    
    }      
    else if(idx == 3)
    {
        phw->ptCW_lmt       = GPIOE;
        phw->ptCCW_lmt      = GPIOE;
        phw->ptHome_lmt     = GPIOE;
        phw->ptDir          = GPIOC;
        phw->ptEna          = GPIOC;
        
        
        phw->pinCW_lmt      = GPIO_Pin_10;
        phw->pinCCW_lmt     = GPIO_Pin_11;
        phw->pinHome_lmt    = GPIO_Pin_12;
        phw->pinDir         = GPIO_Pin_10;
        phw->pinEna         = GPIO_Pin_11;
        
        phw->extLineCW      = EXTI_Line10;
        phw->extLineCCW     = EXTI_Line11;
        phw->extLineHome    = EXTI_Line12;
        
        phw->irpFunCW       = EXTI15_10_IRQHandler;
        phw->irpFunCCW      = EXTI15_10_IRQHandler;
        phw->irpFunHome     = EXTI15_10_IRQHandler;
  
        phw->portSourceCW   = GPIO_PortSourceGPIOE;
        phw->portSourceCCW  = GPIO_PortSourceGPIOE;
        phw->portSourceHome = GPIO_PortSourceGPIOE;
        
        phw->pinSourceCW    = GPIO_PinSource10;
        phw->pinSourceCCW   = GPIO_PinSource11;
        phw->pinSourceHome  = GPIO_PinSource12;   
        
        phw->nirpCW         = EXTI15_10_IRQn;
        phw->nirpCCW        = EXTI15_10_IRQn;
        phw->nirpHome       = EXTI15_10_IRQn;
        phw->rcc_mod2       = RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE;
        phw->rcc_mod1       = 0;    
    }      
    phw->bStopImmediatelyCW     = true;
    phw->bStopImmediatelyCCW    = true;
    phw->bStopImmediatelyHome   = true;
    
    
}
void EXTI0_IRQHandler(void)
{
//    HM_NET_PRINT("EXTI0_IRQHandler %d\r\n", 0);
    
    if(EXTI_GetITStatus(EXTI_Line0)!= RESET)
    {
        __disable_irq();
        EXTI_ClearITPendingBit(EXTI_Line0);
        handler_extern_irq(EXTI0_IRQHandler, EXTI_Line0);
        __enable_irq();
    } 
	      
}
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1)!= RESET)
    {
        __disable_irq();
        EXTI_ClearITPendingBit(EXTI_Line1);
        handler_extern_irq(EXTI1_IRQHandler, EXTI_Line1);
        __enable_irq();
    }    
}
void EXTI2_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2)!= RESET)
    {
        __disable_irq();
        EXTI_ClearITPendingBit(EXTI_Line2);
        handler_extern_irq(EXTI2_IRQHandler, EXTI_Line2);
        __enable_irq();
    }       
}
void EXTI3_IRQHandler(void)
{
//    HM_NET_PRINT("EXTI3_IRQHandler %d\r\n", 0);
    if(EXTI_GetITStatus(EXTI_Line3)!= RESET)
    {
        __disable_irq();
        EXTI_ClearITPendingBit(EXTI_Line3);
        handler_extern_irq(EXTI3_IRQHandler, EXTI_Line3);
        __enable_irq();
    }      
}
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4)!= RESET)
    {
        __disable_irq();
        EXTI_ClearITPendingBit(EXTI_Line4);
        handler_extern_irq(EXTI4_IRQHandler, EXTI_Line4);
        __enable_irq();
    }       
}
void EXTI9_5_IRQHandler(void)
{
    uint32_t extline = 0xffff;
//    HM_NET_PRINT("EXTI9_5_IRQHandler %d\r\n", 0);
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line5)!= RESET) extline = EXTI_Line5;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line6)!= RESET) extline = EXTI_Line6;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line7)!= RESET) extline = EXTI_Line7;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line8)!= RESET) extline = EXTI_Line8;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line9)!= RESET) extline = EXTI_Line9;
    if(extline != 0xffff)
    {
        __disable_irq();
        EXTI_ClearITPendingBit(extline);
        handler_extern_irq(EXTI9_5_IRQHandler, extline); 
        __enable_irq();
    }        
}
void EXTI15_10_IRQHandler(void)
{
    uint32_t extline = 0xffff;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line10)!= RESET) extline = EXTI_Line10;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line11)!= RESET) extline = EXTI_Line11;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line12)!= RESET) extline = EXTI_Line12;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line13)!= RESET) extline = EXTI_Line13;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line14)!= RESET) extline = EXTI_Line14;
    if(extline == 0xffff && EXTI_GetITStatus(EXTI_Line15)!= RESET) extline = EXTI_Line15;
    if(extline != 0xffff)
    {
        __disable_irq();
        EXTI_ClearITPendingBit(extline);
        handler_extern_irq(EXTI15_10_IRQHandler, extline);  
        __enable_irq();
    }        
	
}
void    hm_motor_limit_logical_check(void* param)
{
    uint32_t                    idx;
    hm_step_motor_hw_config_t   *phw        = NULL;
    hm_step_info_t              *psrd       = NULL;
    bool                        bdostop     = false;
    bool                        bRunning;
    bool                        bhitHome;
    for(idx = 0; idx < HM_MAX_AXIS; ++idx)
    {
        psrd        = &s_axis_info[idx];
        phw         = &psrd->motor_hw_config;
        bdostop     = false;
        bRunning    = (psrd->axis_state == asRuning);
        if(bRunning == false) continue;
        
        if(bRunning && psrd->dir == rdCW && hm_motor_hit_CW(idx))
        {
            bdostop = true;
        }
        if(bRunning && psrd->dir == rdCCW && hm_motor_hit_CCW(idx))
        {
            bdostop = true;
        }      
        if(bRunning && psrd->bdo_home)
        {
            bhitHome = hm_motor_hit_Home(idx);
            if(bhitHome)
            {
                if(!phw->blast_hit_home && !psrd->bhome_stop_low_triger)
                {
                    bdostop = true;
                }
                else if(psrd->bhome_stop_low_triger)
                {
                    bdostop = true;
                }
            }
            phw->blast_hit_home = bhitHome;
        }        
        if(!bdostop) continue;
        
        HM_NET_PRINT("!!hm_motor_limit_logical_check make axis = %d stop", idx);
        if(phw->bStopImmediatelyCW)
            hm_step_motor_stop_sundden(idx);
        else
            hm_step_motor_stop_smooth(idx);
    }     
}
void    hm_motor_limit_check_loop(void)
{
#ifdef CHECK_IN_MAIN
    uint32_t                    idx;
    hm_step_motor_hw_config_t   *phw        = NULL;
    hm_step_info_t              *psrd       = NULL;
    bool                        bdostop     = false;
    bool                        bhitHome;
    bool                        bRunning;
    for(idx = 0; idx < HM_MAX_AXIS; ++idx)
    {
        psrd        = &s_axis_info[idx];
        phw         = &psrd->motor_hw_config;
        bdostop     = false;
        bRunning    = (hm_step_motor_state(idx) == rsRUN);
        if(bRunning && psrd->dir == rdCW && hm_motor_hit_CW(idx))
        {
            bdostop = true;
        }
        if(bRunning && psrd->dir == rdCCW && hm_motor_hit_CCW(idx))
        {
            bdostop = true;
        }       
        if(bRunning && psrd->bdo_home)
        {
            bhitHome = hm_motor_hit_Home(idx);
            if(bhitHome)
            {
                if(!phw->blast_hit_home && !psrd->bhome_stop_low_triger)
                {
                    bdostop = true;
                }
                else if(psrd->bhome_stop_low_triger)
                {
                    bdostop = true;
                }
            }
            phw->blast_hit_home = bhitHome;
            
        }        
        if(!bdostop) continue;
        if(phw->bStopImmediatelyCW)
            hm_step_motor_stop_sundden(idx);
        else
            hm_step_motor_stop_smooth(idx);
    }    
#endif    
}
void handler_extern_irq(void *func, uint32_t extline)
{
    uint32_t                    idx;
    hm_step_motor_hw_config_t   *phw        = NULL;
    hm_step_info_t              *psrd       = NULL;
    bool                        bdostop     = false;
		bool												bRunning;
    for(idx = 0; idx < HM_MAX_AXIS; ++idx)
    {
        psrd    = &s_axis_info[idx];
        phw     = &psrd->motor_hw_config;
        bdostop = false;
				bRunning    = (psrd->axis_state == asRuning);
        if(phw->irpFunCW == func && phw->extLineCW == extline && hm_motor_hit_CW(idx))
        {
            if(bRunning && psrd->dir == rdCW) bdostop = true;
        }
        if(phw->irpFunCCW == func && phw->extLineCCW == extline && hm_motor_hit_CCW(idx))
        {
            if(bRunning && psrd->dir == rdCCW) bdostop = true;
        }       
        if(phw->irpFunHome == func && phw->extLineHome == extline && hm_motor_hit_Home(idx))
        {
            if(bRunning && psrd->bdo_home) bdostop = true;
        }        
        if(!bdostop) continue;
        if(phw->bStopImmediatelyCW)
            hm_step_motor_stop_sundden(idx);
        else
            hm_step_motor_stop_smooth(idx);
        break;
    }
 //   HM_NET_PRINT("exter_line = %x, void handler_extern_irq(void *func = %p)\r\n", extline, func);
}
const int                   nLoop   = 5000;
bool hm_motor_hit_CW(uint32_t axis)
{
    int                         idx     = 0;
    hm_step_motor_hw_config_t   *phw    = &s_axis_info[axis].motor_hw_config; 
    bool                        nfirst  = (GPIO_ReadInputDataBit(phw->ptCW_lmt, phw->pinCW_lmt) == HM_SENSE_HIT);
    
    if(!nfirst) return false;
    for(idx = 0; idx < nLoop; idx++);
    return (GPIO_ReadInputDataBit(phw->ptCW_lmt, phw->pinCW_lmt) == HM_SENSE_HIT);

}
bool hm_motor_hit_CCW(uint32_t axis)
{
    int                         idx     = 0;
    hm_step_motor_hw_config_t   *phw    = &s_axis_info[axis].motor_hw_config; 
    bool                        nfirst  = (GPIO_ReadInputDataBit(phw->ptCCW_lmt, phw->pinCCW_lmt) == HM_SENSE_HIT);
    
    if(!nfirst) return false;
    for(idx = 0; idx < nLoop; idx++);
    return (GPIO_ReadInputDataBit(phw->ptCCW_lmt, phw->pinCCW_lmt) == HM_SENSE_HIT);  
}
bool hm_motor_hit_Home(uint32_t axis)
{
    int                         idx     = 0;
    hm_step_motor_hw_config_t   *phw    = &s_axis_info[axis].motor_hw_config; 
    bool                        nfirst  = (GPIO_ReadInputDataBit(phw->ptHome_lmt, phw->pinHome_lmt) == HM_SENSE_HIT);
    
    if(!nfirst) return false;
    for(idx = 0; idx < nLoop; idx++);
    return (GPIO_ReadInputDataBit(phw->ptHome_lmt, phw->pinHome_lmt) == HM_SENSE_HIT);     
}
void hm_motor_set_dir(uint32_t axis, bool bCW)
{
    hm_step_motor_hw_config_t   *phw    = NULL;
    phw     = &s_axis_info[axis].motor_hw_config;   
    
    HM_NET_PRINT("void hm_motor_set_dir(axis = %d, bCW = %d)", axis, bCW);
    if(bCW)
    {
        GPIO_SetBits(phw->ptDir, phw->pinDir);
    }
    else
    {
        GPIO_ResetBits(phw->ptDir, phw->pinDir);
    }
}
bool hm_motor_enable_axis(uint32_t axis, bool benable)
{
    hm_step_motor_hw_config_t   *phw    = NULL;
    
    if(axis >= HM_MAX_AXIS) return false;
    phw     = &s_axis_info[axis].motor_hw_config;    
    if(benable && !phw->benabled)
    {
        GPIO_ResetBits(phw->ptEna, phw->pinEna);
        phw->benabled   = true;
    }
    else if(!benable && phw->benabled)
    {
        GPIO_SetBits(phw->ptEna, phw->pinEna);     
        phw->benabled   = false;
    }
    else return true;
    HM_NET_PRINT("void hm_motor_enable_axis(axis = %d, benable = %d)", axis, benable);
    return true;
}
uint8_t hm_motor_is_enable(uint32_t axis)
{
    hm_step_motor_hw_config_t   *phw    = NULL;
    phw     = &s_axis_info[axis].motor_hw_config;    
    
    return phw->benabled;    
}
void hm_motor_set_stop_mode(uint32_t axis, bool bsudCW, bool bsudCCW, bool bsudHome, bool bTcurve)
{
    hm_step_motor_hw_config_t   *phw    = NULL;
    if(axis >= HM_MAX_AXIS) return ;
    phw     = &s_axis_info[axis].motor_hw_config;  

    phw->bStopImmediatelyCW = bsudCW;
    phw->bStopImmediatelyCCW = bsudCCW;
    phw->bStopImmediatelyHome = bsudHome;
    
    if(bTcurve)s_axis_info[axis].method = TCurve;
    else s_axis_info[axis].method = sCurve;
}
void hm_motor_get_stop_mode(uint32_t axis, bool *bsudCW, bool *bsudCCW, bool *bsudHome, uint8_t *bTcurve)
{
    hm_step_motor_hw_config_t   *phw    = NULL;
    if(axis >= HM_MAX_AXIS) return ;
    phw     = &s_axis_info[axis].motor_hw_config;  

    *bsudCW     = phw->bStopImmediatelyCW;
    *bsudCCW    = phw->bStopImmediatelyCCW;
    *bsudHome   = phw->bStopImmediatelyHome; 
    *bTcurve    = (s_axis_info[axis].method == TCurve);
}
