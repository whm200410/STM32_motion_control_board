/************************************************************************
 $Revision: 51 $                                                           
************************************************************************/
/**
  ******************************************************************************
  * @file    TIM/DMABurst/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "hw_config.h"
#include "hm_timer.h"
#include "hm_msg_queue.h"
#include "hm_step_motor.h"
#include "hm_cmd_execute.h"
#include "hm_uart.h"
#include "hm_led.h"
#include "hm_gpio.h"
#include "hm_debug.h"
#include "hm_iwdg.h"
#include "netconf.h"
#include "stm32f107.h"
#include "hm_step_motor_limit_operation.h"
#include <stdio.h>
#include "hm_temperature.h"
#define USE_NET
#define USE_GPIO

void led_ck(void* param)
{
    static uint32_t s_i = 0;
    if(s_i % 2 == 0) hm_led_act(ltRun,true);
    else hm_led_act(ltRun,false);     
    s_i++;    
}

void netwk_ck(void* param)
{
    LwIP_Periodic_Handle(hm_get_cur_time());
}
void server_init(void);

void enable_all_Ports()
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
						ENABLE);
}

int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */    
    SystemInit();
    enable_all_Ports();

    hm_timer_init();
    hm_led_init();
#ifdef USE_NET    
    System_Setup();
#endif    
    
#ifdef USE_GPIO    
    hm_gpio_init();
#endif      

    hm_msg_queue_init();
    hm_timer_register(500, led_ck, NULL, false, true);
      
 //   
    hm_step_motor_init();
    hm_uart_init();
    DS18B20_Init();
#ifdef USE_NET    
    
    LwIP_Init();   
    server_init(); 
    hm_timer_register(1000, netwk_ck, NULL, false, false);
#endif    
    hm_led_act(ltRun,true);
    hm_cmd_exec_init();

  
    #if 0
    if(hm_iwdg_init())
    {
        HM_NET_PRINT("%s", "reset from watchdog");
        hm_led_act(ltError, true);
    }
    hm_timer_register(100, hm_iwdg_feed, NULL, false, false);  
    #endif
    hm_timer_register(100, hm_motor_limit_logical_check, NULL, false, true);      
    
    while (1)
    {
        hm_motor_limit_check_loop();
        hm_msg_queue_main();
        
    }
}



#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}
#endif

