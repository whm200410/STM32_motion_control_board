/************************************************************************
 $Revision: 9 $                                                           
************************************************************************/

#ifndef __HM_GPIO_H
#define __HM_GPIO_H
#include "hm_types.h"

/**
  * @brief  gpio init
  * @param  none
  * @retval None
  */
void        hm_gpio_init(void);


/**
  * @brief  set bit value of specific bit
  * @param  bitno: from 0 to HM_MAX_GPIO_OUT - 1.
  * @param  val : 0 or 1
  * @retval succesed or not
  */
bool        hm_gpio_out_bit(uint32_t bitno, uint32_t val);

/**
  * @brief  get bit value of specific in bit
  * @param  bitno: from 0 to HM_MAX_GPIO_OUT - 1.
  * @retval 1 or 0 or 0xffff if wrong
  */
uint32_t    hm_gpio_in_bit(uint32_t bitno);


/**
  * @brief  get bit value of specific out bit
  * @param  bitno: from 0 to HM_MAX_GPIO_OUT - 1.
  * @retval 1 or 0 or 0xffff if wrong
  */
uint32_t    hm_gpio_get_out_bit(uint32_t bitno);
#endif
