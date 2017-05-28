/************************************************************************
 $Revision: 44 $                                                           
************************************************************************/

#ifndef __HM_USART_H
#define __HM_USART_H
#include "hm_types.h"
#include <stdint.h>
#include <stdio.h>


typedef void*   hm_uart_handler_t;

/*带有日志功能的, usart1表示输出到日志, 用于print, usart2 用于命令传输
若禁止了此功能,则表示*/
#define     xUSE_PRINT


/**
  * @brief  init uart interface
  * @param  none
  * @retval None
  */
 
void hm_uart_init(void);


/**
  * @brief  add call back when receving a char from uart
  * @param  func: call back function, will receive a char*
  * @retval id which can be unregister using hm_unreg_uart_rev_ck
  */
hm_uart_handler_t hm_reg_uart_rev_ck(hm_ck_func_t func);



/**
  * @brief  delete call back when receving a char from uart
  * @param  id return from hm_reg_uart_rev_ck
  * @retval none
  */
void hm_unreg_uart_rev_ck(hm_uart_handler_t);


/**
  * @brief  send message to cmd or log uart 
  * @param  buf and buf len
  * @retval none
  */
void hm_uart_send_cmd(uint8_t *pbuf, uint32_t len);
void hm_uart_send_log(uint8_t *pbuf, uint32_t len);

/**
  * @brief  turn on/off uart debug output
  * @param  boff = true off, false on
  * @retval none
  */
void hm_uart_debug_off(bool boff);

/**
  * @brief  get if uart debug output is turn on/off 
  * @param  one
  * @retval true on, false off
  */
bool hm_uart_is_debug_off(void);

#endif
