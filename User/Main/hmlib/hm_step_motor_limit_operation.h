/************************************************************************
 $Revision: 54 $                                                           
************************************************************************/

#ifndef __HM_STEP_MOTOR_LIMIT_OPERATION_H
#define __HM_STEP_MOTOR_LIMIT_OPERATION_H

#include "hm_types.h"


struct hm_step_motor_hw_config_t;

/**
  * @brief  extern interupt of limit sense, use to stop axis if met limit or home sense
  * @param  none
  * @retval None
  */
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);


/**
  * @brief  fill the HW resource ,ex: limit ,dir ,enable port, bin, ext irp line
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval None
  */
void hm_motor_hw_limit_config(uint32_t idx);

/**
  * @brief  init the HW resource,fill the register ,ex: limit ,dir ,enable port, bin, ext irp line
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval None
  */
void hm_motor_hw_limit_init(uint32_t idx);

void hm_motor_hw_enable_irq(uint32_t idx, bool bena);

/**
  * @brief  detect if axis hit CW limit
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval CW sense status 
  */
bool hm_motor_hit_CW(uint32_t axis);

/**
  * @brief  detect if axis hit CCW limit
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval CCW sense status 
  */
bool hm_motor_hit_CCW(uint32_t axis);

/**
  * @brief  detect if axis hit Home limit
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval Home sense status 
  */
bool hm_motor_hit_Home(uint32_t axis);


/**
  * @brief  set direction of axis
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @param  bCW: CW or CCW
  * @retval none
  */
  
void hm_motor_set_dir(uint32_t axis, bool bCW);


/**
  * @brief  enable or disable axiss
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @param  benable: enable if true
  * @retval none
  */  
bool hm_motor_enable_axis(uint32_t axis, bool benable);

/**
  * @brief  return wether motor is enable or not
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval bool
  */  
uint8_t hm_motor_is_enable(uint32_t axis);


void    hm_motor_limit_check_loop(void);
void    hm_motor_limit_logical_check(void *);
/**
  * @brief  eet stop mode, true for suddent, false for smoonth mode
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval bool
  */ 
void hm_motor_set_stop_mode(uint32_t axis, bool bsudCW, bool bsudCCW, bool bsudHome, bool bTcurve);
void hm_motor_get_stop_mode(uint32_t axis, bool *bsudCW, bool *bsudCCW, bool *bsudHome, uint8_t *bTcurve);
#endif
